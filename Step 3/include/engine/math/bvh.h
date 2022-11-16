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
    #include <graphics/imgui/imgui.h>

#endif

#include <utility>

// A currently pure header implementation of a BVH. TODO: make source file.
// this is also for testing and might not make it into the step 2.

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
    
    struct BVHNode {
        private:
            static Raytracing::Mat4x4 getTransform(const AABB& _aabb) {
                Raytracing::Mat4x4 transform{};
                auto center = _aabb.getCenter();
                transform.translate(center);
                auto xRadius = _aabb.getXRadius(center) * 2;
                auto yRadius = _aabb.getYRadius(center) * 2;
                auto zRadius = _aabb.getZRadius(center) * 2;
                transform.scale(float(xRadius), float(yRadius), float(zRadius));
                return transform;
            }
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
            int index;
            int hit = 0;
            BVHNode(std::vector<BVHObject> objs, AABB aabb, BVHNode* left, BVHNode* right): objs(std::move(objs)), aabb(std::move(aabb)),
                                                                                            left(left), right(right) {
                index = count++;
            }
            BVHHitData firstHitRayIntersectTraversal(const Ray& r, PRECISION_TYPE min, PRECISION_TYPE max);
            #ifdef COMPILE_GUI
            void draw(Shader& worldShader) {
                worldShader.setVec3("color", {1.0, 1.0, 1.0});
                aabbVAO->bind();
                if (selected == index) {
                    if (selected == index && ImGui::BeginListBox("", ImVec2(250, 350))) {
                        std::stringstream strs;
                        strs << aabb;
                        ImGui::Text("%s", strs.str().c_str());
                        for (const auto& item: objs) {
                            auto pos = item.ptr->getPosition();
                            std::stringstream stm;
                            stm << item.aabb;
                            ImGui::Text("%s,\n\t%s", (std::to_string(pos.x()) + " " + std::to_string(pos.y()) + " " + std::to_string(pos.z())).c_str(),
                                        stm.str().c_str());
                        }
                        ImGui::EndListBox();
                    }
                    
                    for (const auto& obj: objs) {
                        auto transform = getTransform(obj.aabb);
                        worldShader.setMatrix("transform", transform);
                        aabbVAO->draw(worldShader);
                    }
                    auto transform = getTransform(aabb);
                    worldShader.setMatrix("transform", transform);
                    aabbVAO->draw(worldShader);
                    
                    /*auto splitAABBs = aabb.splitByLongestAxis();
                    transform = getTransform(splitAABBs.second);
                    worldShader.setMatrix("transform", transform);
                    aabbVAO->draw(worldShader);
                    transform = getTransform(splitAABBs.first);
                    worldShader.setMatrix("transform", transform);
                    aabbVAO->draw(worldShader);*/
                }
                if (hit){
                    if (hit == 1)
                        worldShader.setVec3("color", {0.0, 0.0, 1.0});
                    else if (hit == 2)
                        worldShader.setVec3("color", {0.0, 1.0, 0.0});
                    else
                        worldShader.setVec3("color", {1.0, 0.5, 0.5});
                    auto transform = getTransform(aabb);
                    worldShader.setMatrix("transform", transform);
                    aabbVAO->draw(worldShader);
                }
            }
            void gui() const {
                int c1 = -1;
                int c2 = -1;
                if (left != nullptr)
                    c1 = left->index;
                if (right != nullptr)
                    c2 = right->index;
                std::string t;
                if (c1 == -1 && c2 == -1)
                    t = " LEAF";
                else
                    t = " L: " + std::to_string(c1) + " R: " + std::to_string(c2);
                if (ImGui::Selectable(("S: " + std::to_string(objs.size()) + " I: " + std::to_string(index) + t).c_str(), selected == index))
                    selected = index;
            }
            #endif
            ~BVHNode() {
                delete (left);
                delete (right);
            }
    };
    
    class BVHTree {
        private:
            BVHNode* root = nullptr;
            
            // splits the objs in the vector based on the provided AABBs
            static BVHPartitionedSpace partition(const std::pair<AABB, AABB>& aabbs, const std::vector<BVHObject>& objs);
            
            static bool vectorEquals(const BVHPartitionedSpace& oldSpace, const BVHPartitionedSpace& newSpace){
                if (oldSpace.left.size() != newSpace.left.size() || oldSpace.right.size() != newSpace.right.size())
                    return false;
                for (int i = 0; i < oldSpace.left.size(); i++){
                    if (oldSpace.left[i].aabb != newSpace.left[i].aabb)
                        return false;
                }
                for (int i = 0; i < oldSpace.right.size(); i++){
                    if (oldSpace.right[i].aabb != newSpace.right[i].aabb)
                        return false;
                }
                return true;
            }
            
            BVHNode* addObjectsRecur(const std::vector<BVHObject>& objects, const BVHPartitionedSpace& prevSpace) {
                // create a volume for the entire world.
                // yes, we could use a recursion provided AABB, but that wouldn't be minimum, only half. this ensures that we have a minimum AABB.
                AABB world;
                for (const auto& obj: objects)
                    world = world.expand(obj.aabb);
    
                // then split and partition the world
                auto splitAABBs = world.splitByLongestAxis();
                auto partitionedObjs = partition(splitAABBs, objects);
                if (vectorEquals(prevSpace, partitionedObjs)){
                    splitAABBs = world.splitAlongAxis();
                    partitionedObjs = partition(splitAABBs, objects);
                }
                
                if ((objects.size() <= 1 && !objects.empty())) {
                    return new BVHNode(objects, world, nullptr, nullptr);
                } else if (objects.empty()) // should never reach here!!
                    return nullptr;
                
                BVHNode* left = nullptr;
                BVHNode* right = nullptr;
                // don't try to explore nodes which don't have anything in them.
                if (!partitionedObjs.left.empty())
                    left = addObjectsRecur(partitionedObjs.left, partitionedObjs);
                if (!partitionedObjs.right.empty())
                    right = addObjectsRecur(partitionedObjs.right, partitionedObjs);
                
                if (left == nullptr && right == nullptr)
                    return new BVHNode(objects, world, left, right);
                else
                    return new BVHNode({}, world, left, right);
            }
            #ifdef COMPILE_GUI
            void drawNodesRecur(Shader& worldShader, BVHNode* node) {
                node->draw(worldShader);
                if (node->left != nullptr)
                    drawNodesRecur(worldShader, node->left);
                if (node->right != nullptr)
                    drawNodesRecur(worldShader, node->right);
            }
            void guiNodesRecur(BVHNode* node) {
                node->gui();
                if (node->left != nullptr)
                    guiNodesRecur(node->left);
                if (node->right != nullptr)
                    guiNodesRecur(node->right);
            }
            #endif
            void reset(BVHNode* node){
                if (node == nullptr)
                    return;
                node->hit = false;
                reset(node->left);
                reset(node->right);
            }
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
            
            void addObjects(const std::vector<Object*>& objects) {
                if (root != nullptr)
                    throw std::runtime_error("BVHTree already exists. What are you trying to do?");
                // move all the object's aabb's into world position
                std::vector<BVHObject> objs;
                for (auto* obj: objects) {
                    // we don't want to store all the AABBs which don't exist: ie spheres
                    if (obj->getAABB().isEmpty()) {
                        noAABBObjects.push_back(obj);
                        continue;
                    }
                    
                    BVHObject bvhObject;
                    // returns a copy of the AABB object and assigns it in to the tree storage object
                    bvhObject.aabb = obj->getAABB().translate(obj->getPosition());
                    // which means we don't have to do memory management, since we are using the pointer without ownership or coping now.
                    bvhObject.ptr = obj;
                    objs.push_back(bvhObject);
                }
                root = addObjectsRecur(objs, {});
            }
            
            std::vector<BVHObject> rayFirstHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            std::vector<BVHObject> rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max);
            void resetNodes(){
                reset(root);
            }
            
            #ifdef COMPILE_GUI
            // renders all the debug VAOs on screen.
            void render(Shader& worldShader) {
                ImGui::Begin(("BVH Data "), nullptr, ImGuiWindowFlags_NoCollapse);
                worldShader.use();
                worldShader.setInt("useWhite", 1);
                worldShader.setVec3("color", {1.0, 1.0, 1.0});
                {
                    ImGui::BeginChild("left pane", ImVec2(180, 0), true);
                    guiNodesRecur(root);
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("item view",
                                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                                      true,
                                      ImGuiWindowFlags_AlwaysAutoResize); // Leave room for 1 line below us
                    drawNodesRecur(worldShader, root);
                    ImGui::EndChild();
                    ImGui::EndGroup();
                }
                worldShader.setInt("useWhite", 0);
                ImGui::End();
            }
            #endif
            
            ~BVHTree() {
                delete (root);
            }
    };
    
}

#endif //STEP_2_BVH_H
