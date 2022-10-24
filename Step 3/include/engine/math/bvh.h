/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_BVH_H
#define STEP_2_BVH_H

#include "engine/util/std.h"
#include "engine/types.h"

#include <utility>

// A currently pure header implementation of a BVH. TODO: make source file.
// this is also for testing and might not make it into the step 2.

namespace Raytracing {
    
    struct BVHNode {
        public:
            std::vector<Object*> objs;
            AABB aabb;
            BVHNode* left;
            BVHNode* right;
            BVHNode(std::vector<Object*> objs, AABB aabb, BVHNode* left, BVHNode* right): objs(std::move(objs)), aabb(std::move(aabb)),
                                                                                                        left(left), right(right) {}
            ~BVHNode() {
                delete (left);
                delete (right);
            }
    };
    
    class BVHTree {
        private:
            const int MAX_TREE_DEPTH = 50;
            BVHNode* root = nullptr;
            
            void del() {
                // delete copied objects
                for (auto* obj : root->objs)
                    delete(obj);
                delete (root);
            }
            
            // splits the objs in the vector based on the provided AABBs
            static std::pair<std::vector<Object*>, std::vector<Object*>>
                    partition(const std::pair<AABB, AABB>& aabbs, const std::vector<Object*>& objs) {
                std::vector<Object*> a1;
                std::vector<Object*> a2;
                for (auto* obj: objs) {
                    // if this object doesn't have an AABB, we cannot use a BVH on it
                    if (obj->getAABB().isEmpty()) {
                        throw std::runtime_error("Invalid AABB provided to the BVH! (Your implementation is flawed)");
                    }
                    if (obj->getAABB().intersects(aabbs.first)) {
                        a1.push_back(obj);
                    } else if (obj->getAABB().intersects(aabbs.second)) {
                        a2.push_back(obj);
                    }
                    //tlog << "OBJ: " << obj->getAABB() << " " << obj->getAABB().intersects(aabbs.first) << " " << obj->getAABB().intersects(aabbs.second) << " " << objs.size() << "\n";
                }
                //tlog << "we split into two of sizes: " << a1.size() << "  " << a2.size() << " orig size: " << (a1.size() + a2.size()) << "\n";
                return {a1, a2};
            }
            
            BVHNode* addObjectsRecur(const std::vector<Object*>& objects, unsigned long prevSize) {
                //ilog << "size: " << objects.size() << "\n";
                // prevSize was required to solve some really weird bugs
                // which are a TODO:
                if ((objects.size() <= 2 && !objects.empty()) || prevSize == objects.size()) {
                    AABB local;
                    for (const auto& obj: objects)
                        local = local.expand(obj->getAABB());
                    return new BVHNode(objects, local, nullptr, nullptr);
                } else if (objects.empty()) // should never reach here!!
                    return nullptr;
                // create a volume for the entire world.
                // yes, we could use the recursion provided AABB,
                // but that wouldn't be minimum, only half.
                // this ensures that we have a minimum AABB.
                AABB world;
                for (const auto& obj: objects) {
                    //tlog << obj->getAABB();
                    world = world.expand(obj->getAABB());
                }
                //tlog << "\n";
                // then split and partition the world
                auto spltAABB = world.splitByLongestAxis();
                //dlog << "We have " << world << " being split into: \n\t" << spltAABB.first << "\n\t" << spltAABB.second << "\n";
                auto partitionedObjs = partition(spltAABB, objects);
                
                BVHNode* left = nullptr;
                BVHNode* right = nullptr;
                // don't try to explore nodes which don't have anything in them.
                if (!partitionedObjs.first.empty())
                    left = addObjectsRecur(partitionedObjs.first, objects.size());
                if (!partitionedObjs.second.empty())
                    right = addObjectsRecur(partitionedObjs.second, objects.size());
                
                return new BVHNode(objects, world, left, right);
            }
            static std::vector<Object*>
                    traverseFindRayIntersection(BVHNode* node, const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
                // check for intersections on both sides of the tree
                if (node->left != nullptr) {
                    if (node->left->aabb.intersects(ray, min, max))
                        return traverseFindRayIntersection(node->left, ray, min, max);
                }
                // since each aabb should be minimum, we shouldn't have to traverse both sides.
                // we want to reduce our problem size by half each iteration anyways
                // divide and conquer and so on
                if (node->right != nullptr)
                    if (node->right->aabb.intersects(ray, min, max))
                        return traverseFindRayIntersection(node->left, ray, min, max);
                // return the objects of the lowest BVH node we can find
                // if this is implemented properly this should only contain one, maybe two objects
                // which is much faster! (especially when dealing with triangles)
                return node->objs;
            }
        public:
            std::vector<Object*> noAABBObjects;
            explicit BVHTree(const std::vector<Object*>& objectsInWorld) {
                addObjects(objectsInWorld);
            }
            
            void addObjects(const std::vector<Object*>& objects) {
                if (root != nullptr)
                    del();
                // move all the object's aabb's into world position
                std::vector<Object*> objs;
                for (auto* obj: objects) {
                    // we don't want to store all the AABBs which don't exist
                    // ie spheres
                    if (obj->getAABB().isEmpty()) {
                        //tlog << "Goodbye\n";
                        noAABBObjects.push_back(obj);
                        continue;
                    }
                    Object* objCopy = obj->clone();
                    objCopy->setAABB(obj->getAABB().translate(obj->getPosition()));
                    objs.push_back(objCopy);
                }
                root = addObjectsRecur(objs, -1);
            }
            
            std::vector<Object*> rayIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
                return traverseFindRayIntersection(root, ray, min, max);
            }
            
            ~BVHTree() {
                del();
            }
    };
    
}

#endif //STEP_2_BVH_H
