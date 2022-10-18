/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_BVH_H
#define STEP_2_BVH_H

#include <util/std.h>
#include <types.h>

// A currently pure header implementation of a BVH. TODO: make source file.
// this is also for testing and might not make it into the step 2.

namespace Raytracing {

    class BVHNode {
        private:
            void* obj;
            AABB aabb;
            BVHNode* left;
            BVHNode* right;
        public:
            BVHNode(void* obj, AABB aabb, BVHNode* left, BVHNode* right): obj(obj), aabb(std::move(aabb)), left(left), right(right) {}
            ~BVHNode() {
                delete(left);
                delete(right);
            }
    };

    class BVHTree {
        private:
            BVHNode* root = nullptr;
        public:
            explicit BVHTree(const std::vector<Object*>& objectsInWorld) {
                // create a volume for the entire world.
                AABB world;
                for (const auto& obj : objectsInWorld)
                    if (!obj->getAABB().isEmpty())
                        world.expand(obj->getAABB());
                // world sized bvh node isn't associated with a specific object
                // only leafs should be non-null, and we might need to change it to a vector.
                root = new BVHNode(nullptr, world, nullptr, nullptr);
            }
            ~BVHTree(){
                delete(root);
            }
    };

}

#endif //STEP_2_BVH_H
