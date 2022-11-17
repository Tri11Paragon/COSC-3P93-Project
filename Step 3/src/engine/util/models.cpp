/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/util/models.h"
#include <fstream>
#include <ios>

Raytracing::ModelData Raytracing::OBJLoader::loadModel(const std::string& file) {
    std::ifstream modelFile;
    
    modelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    modelFile.open(file);
    std::stringstream modelSource;
    
    std::cout.flush();
    // read the entire file into a string
    modelSource << modelFile.rdbuf();

    modelFile.close();
    std::string modelUnprocessedSource = modelSource.str();

    auto lines = String::split(modelUnprocessedSource, "\n");
    ilog << "Loading of model file " << file << " complete! Now processing " << lines.size() << " lines.\n";

    ModelData data;

    for (const auto& line : lines){
        auto spaces = String::split(line, " ");
        if (line.starts_with("v ")) { // vertex
            data.vertices.emplace_back(std::stof(spaces[1]), std::stof(spaces[2]), std::stof(spaces[3]));
        } else if (line.starts_with("vt ")){ // uv
            data.uvs.emplace_back(std::stof(spaces[1]), std::stof(spaces[2]), 0);
        } else if (line.starts_with("vn ")){ // normal
            data.normals.emplace_back(std::stof(spaces[1]), std::stof(spaces[2]), std::stof(spaces[3]));
        } else if (line.starts_with("f ")){ // face
            // we've reached the faces, we'll need to process them later.
            auto t1 = String::split(spaces[1], "/");
            auto t2 = String::split(spaces[2], "/");
            auto t3 = String::split(spaces[3], "/");
            face f {};

            // obj files are 1 indexed,
            // but arrays are 0 indexed,
            // must be transformed.
            f.v1 = std::stoi(t1[0])-1;
            f.v2 = std::stoi(t2[0])-1;
            f.v3 = std::stoi(t3[0])-1;

            f.uv1 = std::stoi(t1[1])-1;
            f.uv2 = std::stoi(t2[1])-1;
            f.uv3 = std::stoi(t3[1])-1;

            f.n1 = std::stoi(t1[2])-1;
            f.n2 = std::stoi(t2[2])-1;
            f.n3 = std::stoi(t3[2])-1;

            data.faces.push_back(f);
        }
    }


    ilog << "Completed extracting vertex data from model file " << file << "!\n";
    return data;
}
Raytracing::TriangulatedModel::TriangulatedModel(const Raytracing::ModelData& data) {
    auto faces = data.faces;
    auto vertices = data.vertices;
    auto uvs = data.uvs;
    auto normals = data.normals;
    
    PRECISION_TYPE minX = infinity, minY = infinity, minZ = infinity, maxX = ninfinity, maxY = ninfinity, maxZ = ninfinity;
    
    for (face f: faces) {
        Triangle t {vertices[f.v1], vertices[f.v2], vertices[f.v3],
                    uvs[f.uv1], uvs[f.uv2], uvs[f.uv3],
                    normals[f.n1], normals[f.n2], normals[f.n3]};
        
        PRECISION_TYPE tMinX = infinity, tMinY = infinity, tMinZ = infinity, tMaxX = ninfinity, tMaxY = ninfinity, tMaxZ = ninfinity;
        // find the min and max of all the triangles
        tMinX = std::min(t.vertex1.x(), std::min(t.vertex2.x(), std::min(t.vertex3.x(), tMinX)));
        tMinY = std::min(t.vertex1.y(), std::min(t.vertex2.y(), std::min(t.vertex3.y(), tMinY)));
        tMinZ = std::min(t.vertex1.z(), std::min(t.vertex2.z(), std::min(t.vertex3.z(), tMinZ)));
        
        tMaxX = std::max(t.vertex1.x(), std::max(t.vertex2.x(), std::max(t.vertex3.x(), tMaxX)));
        tMaxY = std::max(t.vertex1.y(), std::max(t.vertex2.y(), std::max(t.vertex3.y(), tMaxY)));
        tMaxZ = std::max(t.vertex1.z(), std::max(t.vertex2.z(), std::max(t.vertex3.z(), tMaxZ)));
        
        // create a AABB for model local BVH
        t.aabb = {tMinX, tMinY, tMinZ, tMaxX, tMaxY, tMaxZ};
        
        // and of course for a model AABB,
        minX = std::min(tMinX, minX);
        minY = std::min(tMinY, minY);
        minZ = std::min(tMinZ, minZ);
        
        maxX = std::max(tMaxX, maxX);
        maxY = std::max(tMaxY, maxY);
        maxZ = std::max(tMaxZ, maxZ);
        
        triangles.push_back(t);
    }
    // to generate a AABB
    aabb = {minX, minY, minZ, maxX, maxY, maxZ};
}
