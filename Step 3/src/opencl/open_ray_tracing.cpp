/*
 * Created by Brett Terpstra 6920201 on 03/12/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <opencl/open_ray_tracing.h>

namespace Raytracing {
    
    // we aren't sending the triangle AABB only the vertex information
    constexpr size_t triangleNumOfBytes = sizeof(Triangle) - sizeof(AABB);
    constexpr size_t aabbNumOfBytes = sizeof(AABB);
    
    OpenClRaytracer::OpenClRaytracer(const std::string& programLocation, Image& image, World& world): image(image) {
        auto objectsInWorld = world.getObjectsInWorld();
        
        program = new CLProgram(programLocation);
        OpenCL::createCLProgram(*program);
        program->createKernel("raycast");
        
        program->createImage("outputImage", image.getWidth(), image.getHeight());
        size_t totalWorldBytes = 0;
        for (auto* object: objectsInWorld) {
            auto model = dynamic_cast<ModelObject*>(object);
            if (model == nullptr)
                continue;
            auto triangles = model->getTriangles();
            
            // non-model objects are not supported by the OpenCL renderer.
            totalWorldBytes += triangles.size() * (triangleNumOfBytes);
            // only add AABB bytes from model objects.
            totalWorldBytes += aabbNumOfBytes;
        }
        program->createBuffer("objects", CL_MEM_READ_WRITE, totalWorldBytes);
        storeObjects(objectsInWorld, totalWorldBytes);
        
        program->setKernelArgument("raycast", "outputImage", 0);
        program->setKernelArgument("raycast", "objects", 0);
    }
    
    OpenClRaytracer::~OpenClRaytracer() {
        delete (program);
    }
    
    void OpenClRaytracer::storeObjects(const std::vector<Object*>& objects, size_t totalWorldBytes) {
        size_t currentIndex = 0;
        auto buffer = new unsigned char[totalWorldBytes];
        for (auto* object: objects) {
            auto model = dynamic_cast<ModelObject*>(object);
            // we cannot write non-triangle based objects, as a result we will just pretend they don't exist.
            if (model == nullptr)
                continue;
            auto triangles = model->getTriangles();
            // write all the triangles from the model into the buffer
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
            // finally we want to pack the object's AABB in. Just in case I have time to do a GPU BVH
            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMin());
            MemoryConvert::writeVectorBytes(buffer, currentIndex, model->getAABB().getMax());
        }
        // send the buffer to the GPU
        program->writeBuffer("objects", totalWorldBytes, buffer);
        delete[] buffer;
    }
    
    void OpenClRaytracer::run() {
        size_t works[2]{(size_t) image.getWidth(), (size_t) image.getHeight()};
        program->runKernel("raycast", works, localWorks, 2);
        program->readImage("outputImage", image);
    }
}