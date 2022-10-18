/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_MODELS_H
#define STEP_2_MODELS_H

#include <util/std.h>
#include <math/vectors.h>
#include <math/colliders.h>

namespace Raytracing {

    struct Triangle {
        public:
            vec4 vertex1, vertex2, vertex3;
            bool hasNormals = false;
            vec4 normal1, normal2, normal3;
            vec4 uv1, uv2, uv3;
            AABB aabb;

            Triangle(const vec4& v1, const vec4& v2, const vec4& v3): vertex1(v1), vertex2(v2), vertex3(v3) {}

            Triangle(const vec4& v1, const vec4& v2, const vec4& v3,
                     const vec4& n1, const vec4& n2, const vec4& n3): vertex1(v1), vertex2(v2), vertex3(v3),
                                                                      hasNormals(true), normal1(n1), normal2(n2), normal3(n3) {}

            Triangle(const vec4& v1, const vec4& v2, const vec4& v3,
                     const vec4& uv1, const vec4& uv2, const vec4& uv3,
                     const vec4& n1, const vec4& n2, const vec4& n3): vertex1(v1), vertex2(v2), vertex3(v3),
                                                                      uv1(uv1), uv2(uv2), uv3(uv3),
                                                                      hasNormals(true), normal1(n1), normal2(n2), normal3(n3) {}

            [[nodiscard]] vec4 findClosestNormal(const vec4& point) const {
                // no need to sqrt as exact distance doesn't matter
                auto n1Dist = (point - normal1).lengthSquared();
                auto n2Dist = (point - normal2).lengthSquared();
                auto n3Dist = (point - normal3).lengthSquared();
                return (n1Dist < n2Dist && n1Dist < n3Dist) ? normal1 : (n2Dist < n3Dist ? normal2 : normal3);
            }
    };

    struct face {
        int v1, v2, v3;
        int uv1, uv2, uv3;
        int n1, n2, n3;
    };

    struct ModelData {
        public:
            // storing all this data is memory inefficient
            // since normals and vertices are only vec3s
            // and uvs are vec2s
            // TODO: create lower order vector classes
            std::vector<vec4> vertices;
            std::vector<vec4> uvs;
            std::vector<vec4> normals;
            std::vector<face> faces;
            AABB aabb;

            std::vector<Triangle> toTriangles() {
                std::vector<Triangle> triangles;

                PRECISION_TYPE minX = INFINITY, minY = INFINITY, minZ = INFINITY, maxX = -INFINITY, maxY = -INFINITY, maxZ = -INFINITY;

                for (face f: faces) {
                    Triangle t {vertices[f.v1], vertices[f.v2], vertices[f.v3],
                                uvs[f.uv1], uvs[f.uv2], uvs[f.uv3],
                                normals[f.n1], normals[f.n2], normals[f.n3]};

                    PRECISION_TYPE tMinX = INFINITY, tMinY = INFINITY, tMinZ = INFINITY, tMaxX = -INFINITY, tMaxY = -INFINITY, tMaxZ = -INFINITY;
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

                return triangles;
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
