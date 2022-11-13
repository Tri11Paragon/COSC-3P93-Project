/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_BVH_H
#define STEP_2_BVH_H

#include "engine/util/std.h"
#include "engine/types.h"
#include <config.h>
#ifdef COMPILE_GUI
    #include <graphics/gl/gl.h>
#endif

#include <utility>

// A currently pure header implementation of a BVH. TODO: make source file.
// this is also for testing and might not make it into the step 2.

namespace Raytracing {
    
    #ifdef COMPILE_GUI
        struct BVHDebugVAO {
            VAO* vaoPtr;
            Vec4 position;
        };
    #endif
    
    struct BVHObject {
        Object* ptr = nullptr;
        AABB aabb;
    };
    
    struct BVHPartitionedSpace {
        std::vector<BVHObject> left;
        std::vector<BVHObject> right;
    };
    
    struct BVHNode {
        public:
            std::vector<BVHObject> objs;
            AABB aabb;
            BVHNode* left;
            BVHNode* right;
            BVHNode(std::vector<BVHObject> objs, AABB aabb, BVHNode* left, BVHNode* right): objs(std::move(objs)), aabb(std::move(aabb)),
                                                                                                        left(left), right(right) {}
            ~BVHNode() {
                delete (left);
                delete (right);
            }
    };
    
    class BVHTree {
        private:
            #ifdef COMPILE_GUI
                std::vector<BVHDebugVAO> aabbVAOs;
            #endif
            const int MAX_TREE_DEPTH = 50;
            BVHNode* root = nullptr;
            
            // splits the objs in the vector based on the provided AABBs
            static BVHPartitionedSpace partition(const std::pair<AABB, AABB>& aabbs, const std::vector<BVHObject>& objs) {
                BVHPartitionedSpace space;
                for (const auto& obj: objs) {
                    // if this object doesn't have an AABB, we cannot use a BVH on it
                    // If this ever fails we have a problem with the implementation.
                    RTAssert(!obj.aabb.isEmpty());
                    if (obj.aabb.intersects(aabbs.first)) {
                        space.left.push_back(obj);
                    }
                    if (obj.aabb.intersects(aabbs.second)) {
                        space.right.push_back(obj);
                    }
                }
                return space;
            }
            
            BVHNode* addObjectsRecur(const std::vector<BVHObject>& objects, unsigned long prevSize) {
                //ilog << "size: " << objects.size() << "\n";
                // prevSize was required to solve some really weird bugs
                // which are a TODO:
                if ((objects.size() <= 2 && !objects.empty()) || prevSize == objects.size()) {
                    AABB local;
                    for (const auto& obj: objects)
                        local = local.expand(obj.aabb);
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
                    world = world.expand(obj.aabb);
                }
                //tlog << "\n";
                // then split and partition the world
                auto splitAABBs = world.splitByLongestAxis();
                auto partitionedObjs = partition(splitAABBs, objects);
                
                BVHNode* left = nullptr;
                BVHNode* right = nullptr;
                // don't try to explore nodes which don't have anything in them.
                if (!partitionedObjs.left.empty())
                    left = addObjectsRecur(partitionedObjs.left, objects.size());
                if (!partitionedObjs.right.empty())
                    right = addObjectsRecur(partitionedObjs.right, objects.size());
                
                return new BVHNode(objects, world, left, right);
            }
            static std::vector<BVHObject> traverseFindRayIntersection(BVHNode* node, const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
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
                    throw std::runtime_error("BVHTree already exists. What are you trying to do?");
                // move all the object's aabb's into world position
                std::vector<BVHObject> objs;
                for (auto* obj: objects) {
                    // we don't want to store all the AABBs which don't exist
                    // ie spheres
                    if (obj->getAABB().isEmpty()) {
                        noAABBObjects.push_back(obj);
                        continue;
                    }
    
                    #ifdef COMPILE_GUI
                        // create a VAO for debugging the AABB bounds.
                        BVHDebugVAO vaoStorage {};
                        auto aabbCenter = obj->getAABB().getCenter();
                        auto aabbXRadius = obj->getAABB().getXRadius(aabbCenter);
                        auto aabbYRadius = obj->getAABB().getYRadius(aabbCenter);
                        auto aabbZRadius = obj->getAABB().getZRadius(aabbCenter);
                        auto aabbVertexData = Shapes::cubeVertexBuilder::getCubeExtends(float(aabbXRadius), float(aabbYRadius), float(aabbZRadius));
                        vaoStorage.vaoPtr = new VAO(aabbVertexData.cubeVerticesRaw, aabbVertexData.cubeUVs);
                        vaoStorage.position = obj->getPosition();
                        aabbVAOs.push_back(vaoStorage);
                    #endif
                    
                    BVHObject bvhObject;
                    // returns a copy of the AABB object and assigns it in to the tree storage object
                    bvhObject.aabb = obj->getAABB().translate(obj->getPosition());
                    // which means we don't have to do memory management, since we are using the pointer without ownership or coping now.
                    bvhObject.ptr = obj;
                    objs.push_back(bvhObject);
                }
                root = addObjectsRecur(objs, -1);
            }
            
            std::vector<BVHObject> rayIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
                return traverseFindRayIntersection(root, ray, min, max);
            }
        
            #ifdef COMPILE_GUI
                // renders all the debug VAOs on screen.
                void render (Shader& worldShader){
                    worldShader.use();
                    worldShader.setInt("useWhite", 1);
                    for (const auto& debugVAO : aabbVAOs){
                        debugVAO.vaoPtr->bind();
                        debugVAO.vaoPtr->draw(worldShader, {debugVAO.position});
                    }
                    worldShader.setInt("useWhite", 0);
                }
            #endif
            
            ~BVHTree() {
                delete (root);
                #ifdef COMPILE_GUI
                    for (const auto& debugVAORef : aabbVAOs)
                        delete(debugVAORef.vaoPtr);
                #endif
            }
    };
    
}

#endif //STEP_2_BVH_H
