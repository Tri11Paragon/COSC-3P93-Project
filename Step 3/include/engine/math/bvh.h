/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_BVH_H
#define STEP_2_BVH_H

#include "engine/util/std.h"
#include "engine/types.h"
#include "engine/util/models.h"
#include <config.h>

#ifdef COMPILE_GUI
    
    #include <graphics/gl/gl.h>
    #include <graphics/imgui/imgui.h>

#endif

#include <utility>

namespace Raytracing {
    
    #ifdef COMPILE_GUI
    extern std::shared_ptr<VAO> aabbVAO;
    extern int count;
    extern int selected;
    #endif
    
    struct BVHObject {
        Object* ptr = nullptr;
        AABB aabb;
    };
    
    struct BVHPartitionedSpace {
        std::vector<BVHObject> left;
        std::vector<BVHObject> right;
    };
    
    inline bool operator==(const BVHPartitionedSpace& left, const BVHPartitionedSpace& right) {
        if (left.left.size() != right.left.size() || left.right.size() != right.right.size())
            return false;
        for (int i = 0; i < left.left.size(); i++) {
            if (left.left[i].aabb != right.left[i].aabb)
                return false;
        }
        for (int i = 0; i < left.right.size(); i++) {
            if (left.right[i].aabb != right.right[i].aabb)
                return false;
        }
        return true;
    }
    
    struct BVHNode {
        public:
            struct BVHHitData {
                BVHNode* ptr{};
                AABBHitData data{};
                bool hit = false;
            };
            std::vector<BVHObject> objs;
            AABB aabb;
            BVHNode* left;
            BVHNode* right;
            // debug ints.
            int index, hit = 0;
            BVHNode(std::vector<BVHObject> objs, AABB aabb, BVHNode* left, BVHNode* right): objs(std::move(objs)), aabb(std::move(aabb)),
                                                                                            left(left), right(right) {index = count++;}
            ~BVHNode() {
                delete (left);
                delete (right);
            }
    };
    
    class BVHTree {
        private:
            BVHNode* root = nullptr;
            
            static BVHPartitionedSpace partition(const std::pair<AABB, AABB>& aabbs, const std::vector<BVHObject>& objs);
            BVHNode* addObjectsRecursively(const std::vector<BVHObject>& objects, const BVHPartitionedSpace& prevSpace);
        public:
            std::vector<Object*> noAABBObjects;
            explicit BVHTree(const std::vector<Object*>& objectsInWorld) {
                addObjects(objectsInWorld);
                #ifdef COMPILE_GUI
                auto aabbVertexData = Shapes::cubeVertexBuilder{};
                if (aabbVAO == nullptr)
                    aabbVAO = std::make_shared<VAO>(aabbVertexData.cubeVerticesRaw, aabbVertexData.cubeUVs);
                #endif
            }
            
            void addObjects(const std::vector<Object*>& objects);
            BVHNode* getRoot() {return root;}
            
            std::vector<BVHObject> rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            ~BVHTree() {
                delete (root);
            }
    };
    
    struct TriangleBVHObject {
        Vec4 position;
        std::shared_ptr<Triangle> tri;
        AABB aabb;
    };
    
    struct TriangleBVHPartitionedSpace {
        std::vector<TriangleBVHObject> left;
        std::vector<TriangleBVHObject> right;
    };
    
    inline bool operator==(const TriangleBVHPartitionedSpace& left, const TriangleBVHPartitionedSpace& right) {
        if (left.left.size() != right.left.size() || left.right.size() != right.right.size())
            return false;
        for (int i = 0; i < left.left.size(); i++) {
            if (left.left[i].aabb != right.left[i].aabb)
                return false;
        }
        for (int i = 0; i < left.right.size(); i++) {
            if (left.right[i].aabb != right.right[i].aabb)
                return false;
        }
        return true;
    }
    
    struct TriangleBVHNode {
        struct BVHHitData {
            TriangleBVHNode* ptr{};
            AABBHitData data{};
            bool hit = false;
        };
        std::vector<TriangleBVHObject> objs;
        AABB aabb;
        TriangleBVHNode* left;
        TriangleBVHNode* right;
        // debug ints.
        int index, hit = 0;
        BVHHitData firstHitRayIntersectTraversal(const Ray& r, PRECISION_TYPE min, PRECISION_TYPE max);
        TriangleBVHNode(std::vector<TriangleBVHObject> objs, AABB aabb, TriangleBVHNode* left, TriangleBVHNode* right)
                : objs(std::move(objs)), aabb(std::move(aabb)), left(left), right(right) {}
        ~TriangleBVHNode() {
            delete (left);
            delete (right);
        }
    };
    
    class TriangleBVHTree {
        private:
            TriangleBVHNode* root = nullptr;
            
            static TriangleBVHPartitionedSpace partition(const std::pair<AABB, AABB>& aabbs, const std::vector<TriangleBVHObject>& objs);
            TriangleBVHNode* addObjectsRecursively(const std::vector<TriangleBVHObject>& objects, const TriangleBVHPartitionedSpace& prevSpace);
        public:
            explicit TriangleBVHTree(const std::vector<TriangleBVHObject>& objectsInWorld) {
                addObjects(objectsInWorld);
                #ifdef COMPILE_GUI
                auto aabbVertexData = Shapes::cubeVertexBuilder{};
                if (aabbVAO == nullptr)
                    aabbVAO = std::make_shared<VAO>(aabbVertexData.cubeVerticesRaw, aabbVertexData.cubeUVs);
                #endif
            }
            
            void addObjects(const std::vector<TriangleBVHObject>& objects);
            TriangleBVHNode* getRoot() {return root;}
            
            std::vector<TriangleBVHObject> rayFirstHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            std::vector<TriangleBVHObject> rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            ~TriangleBVHTree() {
                delete (root);
            }
    };
    
}

#endif //STEP_2_BVH_H
