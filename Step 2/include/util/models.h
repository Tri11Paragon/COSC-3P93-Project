/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_MODELS_H
#define STEP_2_MODELS_H

#include <util/std.h>
#include <math/vectors.h>

namespace Raytracing {

    struct Triangle {
        vec4 vertex1, vertex2, vertex3;
        bool hasNormals = false;
        vec4 normal1, normal2, normal3;
        vec4 uv1, uv2, uv3;

        Triangle(const vec4& v1, const vec4& v2, const vec4& v3): vertex1(v1), vertex2(v2), vertex3(v3) {}

        Triangle(const vec4& v1, const vec4& v2, const vec4& v3,
                 const vec4& n1, const vec4& n2, const vec4& n3): vertex1(v1), vertex2(v2), vertex3(v3),
                                                                  hasNormals(true), normal1(n1), normal2(n2), normal3(n3) {}
    };

    struct ModelData {
        // storing all this data is memory inefficient
        // since normals and vertices are only vec3s
        // and uvs are vec2s
        // TODO: create lower order vector classes
        std::vector<vec4> vertices;
        std::vector<vec4> uvs;
        std::vector<vec4> normals;
        std::vector<int> indices;

        std::vector<Triangle> toTriangles() {

        }
    };

    class ModelLoader {
        private:
        public:
            virtual ModelData loadModel(std::string file) = 0;
    };

    class OBJLoader : public ModelLoader {
        private:
        public:
            virtual ModelData loadModel(std::string file);
    };
}
#endif //STEP_2_MODELS_H
