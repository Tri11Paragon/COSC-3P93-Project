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
        // corresponds to the AABB pair used to generate the partitioned space
        std::vector<BVHObject> left;
        std::vector<BVHObject> right;
    };
    
    inline bool operator==(const BVHPartitionedSpace& left, const BVHPartitionedSpace& right) {
        // sometimes the partition function creates spaces with the same objects, which leads to infinite recursion.
        // We can check if the spaces are equal and change the strategy used to split the AABB.
        // and if that fails we can use this to just end the recursion.
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
                                                                                            left(left), right(right) {
#ifdef COMPILE_GUI
                index = count++;
#endif
            }
            
            ~BVHNode() {
                delete (left);
                delete (right);
            }
    };
    
    class BVHTree {
        private:
            BVHNode* root = nullptr;
            
            /**
             * partition the objects to each AABB based on if they intersect with or not. if intersects both left will be preferred.
             */
            static BVHPartitionedSpace partition(const std::pair<AABB, AABB>& aabbs, const std::vector<BVHObject>& objs);
            
            /**
             * Internal function to add objects to the correct BVH node. Creates the entire tree structure recursively. DO NOT USE.
             */
            BVHNode* addObjectsRecursively(const std::vector<BVHObject>& objects, const BVHPartitionedSpace& prevSpace);
            
            /**
             * adds all the objects provided to the BVH, skipping objects which don't have proper AABBs
             */
            void addObjects(const std::vector<Object*>& objects);
        
        public:
            std::vector<Object*> noAABBObjects;
            
            /**
             * creates the BVH using the provided objects, which should come from the world.
             * @param objectsInWorld objects from the world to create the BVH which
             */
            explicit BVHTree(const std::vector<Object*>& objectsInWorld) {
                addObjects(objectsInWorld);
#ifdef COMPILE_GUI
                if (aabbVAO == nullptr) {
                    // create a basic cube for displaying the AABBs when debugging
                    auto aabbVertexData = Shapes::cubeVertexBuilder{};
                    aabbVAO = std::make_shared<VAO>(aabbVertexData.cubeVerticesRaw, aabbVertexData.cubeUVs);
                }
#endif
            }
            
            /**
             * @return the root node of the BVH tree
             */
            BVHNode* getRoot() { return root; }
            
            /**
             * @param ray ray to check intersection with
             * @param min min of the ray to check
             * @param max max of the ray to check
             * @return a list of objects which the ray might intersect with between min and max
             */
            std::vector<BVHObject> rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            
            ~BVHTree() {
                delete (root);
            }
    };
    
    /**
     * Everything below this line is unused due to inconsistencies in performance. Some parts of the code may contain references to this
     * however the actual algorithm doesn't take into account BVHs at the object level.
     */
    
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
            if (!(left.left[i].aabb == right.left[i].aabb))
                return false;
        }
        for (int i = 0; i < left.right.size(); i++) {
            if (!(left.right[i].aabb == right.right[i].aabb))
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
                : objs(std::move(objs)), aabb(std::move(aabb)), left(left), right(right) {
#ifdef COMPILE_GUI
            index = count++;
#endif
        }
        
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
            int index;
            
            explicit TriangleBVHTree(const std::vector<TriangleBVHObject>& objectsInWorld) {
                addObjects(objectsInWorld);
#ifdef COMPILE_GUI
                auto aabbVertexData = Shapes::cubeVertexBuilder{};
                if (aabbVAO == nullptr)
                    aabbVAO = std::make_shared<VAO>(aabbVertexData.cubeVerticesRaw, aabbVertexData.cubeUVs);
                index = count++;
#endif
            }
            
            void addObjects(const std::vector<TriangleBVHObject>& objects);
            
            TriangleBVHNode* getRoot() { return root; }
            
            std::vector<TriangleBVHObject> rayFirstHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            
            std::vector<TriangleBVHObject> rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            
            ~TriangleBVHTree() {
                delete (root);
            }
    };
    
}

#endif //STEP_2_BVH_H
