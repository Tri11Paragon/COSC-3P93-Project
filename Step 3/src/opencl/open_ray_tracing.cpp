/*
 * Created by Brett Terpstra 6920201 on 03/12/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <opencl/open_ray_tracing.h>

#include <cstddef>
#include "engine/util/loaders.h"

namespace Raytracing {
    
    // we aren't sending the triangle AABB only the vertex information
    constexpr size_t triangleNumOfBytes = (sizeof(Triangle) - sizeof(AABB)) / 2;
    constexpr size_t aabbNumOfBytes = sizeof(float) * 4 * 2;
    constexpr size_t vecBytes = sizeof(float) * 4;
    
    OpenClRaytracer::OpenClRaytracer(const std::string& programLocation, Image& image, Camera& camera, World& world):
            image(image), camera(camera) {
        auto objectsInWorld = world.getObjectsInWorld();
        objectCount = 0;
        // pre-calculate the space needed for objects, since every object must statically store these number of triangles, even if not used.
        for (auto* object: objectsInWorld) {
            auto model = dynamic_cast<ModelObject*>(object);
            if (model == nullptr)
                continue;
            // only include objects which we can actually store.
            objectCount++;
            auto triangles = model->getTriangles();
            if (triangles.size() > maxTriangleSize)
                maxTriangleSize = triangles.size();
        }
        
        // 3 vectors per object, 1 size type per object, maxTriangleSize triangles.
        size_t totalWorldBytes =
                (vecBytes * 3) * objectCount + sizeof(unsigned long) * objectCount + triangleNumOfBytes * maxTriangleSize * objectCount;
        //auto objectBuffer = createObjectBuffer(objectsInWorld, totalWorldBytes);
        
        ShaderLoader::define("maxTriangleCount", std::to_string(maxTriangleSize));
        ShaderLoader::define("objectCount", std::to_string(objectCount));
        ShaderLoader::define("imageWidth", std::to_string(image.getWidth()));
        ShaderLoader::define("imageHeight", std::to_string(image.getHeight()));
        
        // load up information about the camera. Since these don't generally chance at runtime we can load them up at compile time
        // however this means that changes made in debug mode do not transfer.
//        auto origin = camera.getImageOrigin();
//        ShaderLoader::define(
//                "imageOrigin",
//                "(float3)(" + std::to_string(origin.x()) + ", " + std::to_string(origin.y()) + ", " + std::to_string(origin.z()) + ")"
//        );
//        auto horiz = camera.getHorizontalAxis();
//        ShaderLoader::define(
//                "horizontalAxis",
//                "(float3)(" + std::to_string(horiz.x()) + ", " + std::to_string(horiz.y()) + ", " + std::to_string(horiz.z()) + ")"
//        );
//        auto vert = camera.getVerticalAxis();
//        ShaderLoader::define(
//                "verticalAxis",
//                "(float3)(" + std::to_string(vert.x()) + ", " + std::to_string(vert.y()) + ", " + std::to_string(vert.z()) + ")"
//        );
//        auto pos = camera.getPosition();
//        ShaderLoader::define(
//                "cameraPosition",
//                "(float3)(" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ", " + std::to_string(pos.z()) + ")"
//        );
        
        program = new CLProgram(programLocation);
        OpenCL::createCLProgram(*program);
        program->createKernel("raycast");
        
        program->createImage("outputImage", image.getWidth(), image.getHeight());
        
        program->createBuffer("objects", CL_MEM_READ_WRITE, totalWorldBytes);
        program->createBuffer("cameraData", CL_MEM_READ_WRITE, sizeof(float) * 3 * 4);
        //storeObjects(objectBuffer, totalWorldBytes);
        
        program->setKernelArgument("raycast", "outputImage", 0);
        program->setKernelArgument("raycast", "objects", 1);
        program->setKernelArgument("raycast", "cameraData", 2);
        updateCameraInformation();
    }
    
    OpenClRaytracer::~OpenClRaytracer() {
        delete (program);
    }
    
    void OpenClRaytracer::storeObjects(unsigned char* buffer, size_t totalWorldBytes) {
        // send the buffer to the GPU
        program->writeBuffer("objects", totalWorldBytes, buffer);
        delete[] buffer;
    }
    
    void OpenClRaytracer::run() {
        size_t works[2]{(size_t) image.getWidth(), (size_t) image.getHeight()};
        program->runKernel("raycast", works, localWorks, 2);
        program->readImage("outputImage", image);
    }
    
    unsigned char* OpenClRaytracer::createObjectBuffer(const std::vector<Object*>& objects, size_t totalWorldBytes) {
        // disable clang warning about making this function static.
        maxTriangleSize;
        size_t currentIndex = 0;
        auto buffer = new unsigned char[totalWorldBytes];
        for (auto* object: objects) {
            auto model = dynamic_cast<ModelObject*>(object);
            // we cannot write non-triangle based objects, as a result we will just pretend they don't exist.
            if (model == nullptr)
                continue;
            auto triangles = model->getTriangles();
            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMin());
            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMax());
            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getPosition());
            // this allows us to tell OpenCL how much of the pre-allocated triangle array we are actually using. Basically a C99 vector without any functions.
            MemoryConvert::writeBytes(buffer, currentIndex, (unsigned long) triangles.size());
            for (auto& triangle: triangles) {
                // write vertex
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex1);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex2);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex3);
                // next in the struct is normals
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal1);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal2);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal3);
                // finally the UVs
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv1);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv2);
                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv3);
            }
        }
//        for (auto* object: objects) {
//            auto model = dynamic_cast<ModelObject*>(object);
//            // we cannot write non-triangle based objects, as a result we will just pretend they don't exist.
//            if (model == nullptr)
//                continue;
//            auto triangles = model->getTriangles();
//            // write the size of the triangles... ie how many triangles we need to read to completely read the model.
//            // we have to send this information because OpenCL needs to know the size ahead of time.
//            MemoryConvert::writeBytes(buffer, currentIndex, (unsigned long) triangles.size());
//            maxTriangleSize = std::max(triangles.size(), maxTriangleSize);
//            // write all the triangles from the model into the buffer
//            for (auto& triangle: triangles) {
//                // write vertex
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex1);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex2);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->vertex3);
//                // next in the struct is normals
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal1);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal2);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->normal3);
//                // finally the UVs
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv1);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv2);
//                MemoryConvert::writeVectorBytes(buffer, currentIndex, triangle->uv3);
//            }
//            // finally we want to pack the object's AABB in. Just in case I have time to do a GPU BVH
//            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMin());
//            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMax());
//            // and the position for rendering.
//            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getPosition());
//        }
        return buffer;
    }
    void OpenClRaytracer::updateCameraInformation() {
        unsigned char buffer[sizeof(float) * 3 * 4];
        size_t currentIndex = 0;
        MemoryConvert::writeVectorBytes(buffer, currentIndex, camera.getPosition());
        MemoryConvert::writeVectorBytes(buffer, currentIndex, camera.getVerticalAxis());
        MemoryConvert::writeVectorBytes(buffer, currentIndex, camera.getHorizontalAxis());
        MemoryConvert::writeVectorBytes(buffer, currentIndex, camera.getImageOrigin());
        program->writeBuffer("cameraData", sizeof(float) * 3 * 4, buffer);
    }
}