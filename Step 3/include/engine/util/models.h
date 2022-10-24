/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_MODELS_H
#define STEP_2_MODELS_H

#include "std.h"
#include "engine/math/vectors.h"
#include "engine/math/colliders.h"
#include "engine/math/bvh.h"

namespace Raytracing {

    struct ModelData {
        public:
            // storing all this data is memory inefficient
            // since normals and vertices are only vec3s
            // and uvs are vec2s
            // TODO: create lower order vector classes
            std::vector<Vec4> vertices;
            std::vector<Vec4> uvs;
            std::vector<Vec4> normals;
            std::vector<face> faces;
            AABB aabb;

            std::vector<Triangle> toTriangles() {
                std::vector<Triangle> triangles;

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

                return triangles;
            }
        
            // creates a BVH tree and returns the list of objects we created. make sure to delete them.
            static std::vector<Object*> createBVHTree(std::vector<Triangle>& triangles, const Vec4& pos) {
                std::vector<Object*> objects;
                for (auto& tri : triangles){
                    Object* obj = new EmptyObject(pos, tri.aabb, tri);
                    objects.push_back(obj);
                }
                return objects;
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
