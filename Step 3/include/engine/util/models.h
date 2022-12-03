/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_MODELS_H
#define STEP_2_MODELS_H

#include "engine/util/std.h"
#include "engine/math/vectors.h"
#include "engine/math/colliders.h"

namespace Raytracing {
    
    // triangle type for model loading
    struct Triangle {
        public:
            Vec4 vertex1, vertex2, vertex3;
            Vec4 normal1, normal2, normal3;
            Vec4 uv1, uv2, uv3;
            AABB aabb;
            
            Triangle(
                    const Vec4& v1, const Vec4& v2, const Vec4& v3, const Vec4& uv1, const Vec4& uv2, const Vec4& uv3, const Vec4& n1, const Vec4& n2,
                    const Vec4& n3
            ): vertex1(v1), vertex2(v2), vertex3(v3), uv1(uv1), uv2(uv2), uv3(uv3), normal1(n1), normal2(n2), normal3(n3) {}
    };
    
    // face type for model loading
    struct face {
        int v1, v2, v3;
        int uv1, uv2, uv3;
        int n1, n2, n3;
    };
    
    struct ModelData {
        // storing all this data is memory inefficient
        // since normals and vertices are only vec3s
        // and uvs are vec2s
        // TODO: create lower order vector classes
        std::vector<Vec4> vertices;
        std::vector<Vec4> uvs;
        std::vector<Vec4> normals;
        std::vector<face> faces;
    };
    
    struct TriangulatedModel {
        std::vector<std::shared_ptr<Triangle>> triangles;
        AABB aabb;
        
        explicit TriangulatedModel(const ModelData& data);
    };
    
    class OBJLoader {
        private:
        public:
            static ModelData loadModel(const std::string& file);
    };
}
#endif //STEP_2_MODELS_H
