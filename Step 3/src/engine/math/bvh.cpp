/*
 * Created by Brett Terpstra 6920201 on 16/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <engine/math/bvh.h>
#include <queue>

namespace Raytracing {
    
    /*
     * BVH Node Class
     * -------------------------------------------------------------------------
     */
    
    /**
     * Returns the first bounding tree with objects in it hit by the provided ray.
     * @param r the ray to check for intersection with
     * @param min the min t allowed in intersection
     * @param max the max t value allowed in intersection
     * @return BVHHitData, a structure containing the node, AABB intersection and whether we actually hit anything or not.
     */
    BVHNode::BVHHitData BVHNode::firstHitRayIntersectTraversal(const Ray& r, PRECISION_TYPE min, PRECISION_TYPE max) {
        // first we need to check if the ray actually intersects with this node,
        auto ourHitData = aabb.intersects(r, min, max);
        // if it doesn't we need to immediately return, which prunes all subtrees to this node.
        if (!ourHitData.hit)
            return {this, ourHitData, false};
        // storage for left and right subtree results.
        BVHHitData leftHit{};
        BVHHitData rightHit{};
        // make sure we actually have a left or right node, and if we do traverse them recursively
        if (left != nullptr)
            leftHit = left->firstHitRayIntersectTraversal(r, min, max);
        if (right != nullptr)
            rightHit = right->firstHitRayIntersectTraversal(r, min, max);
        // once we are either in a bounding box with objects or we are not.
        // to ensure that we keep order and return the first bounding box with objects hit by the ray,
        // we must check if the left tree is closer than the right tree, or we didn't hit the right tree.
        // if we didn't hit the right tree than we can propagate the left tree up
        if (leftHit.hit && (leftHit.data.tMax < rightHit.data.tMax || !rightHit.hit))
            return leftHit;
        // the same goes for the right tree. if we reach this point the left tree didn't get it or was farther than the right tree
        // and so we do the same check. make sure that the right tree is closer, or we didn't hit the left tree.
        else if (rightHit.hit && (rightHit.data.tMax < leftHit.data.tMax || !leftHit.hit))
            return rightHit;
        // if we hit neither tree in all likelihood this is a leaf node, and we can return accordingly
        // but if this isn't a leaf node there will be problems. So if this isn't a leaf node (doesn't have objects)
        // we can pretend like we didn't hit anything.
        return {this, ourHitData, !objs.empty()};
    }
    
    
    
    
    
    
    
    
    /*
     * BVH Node Class
     * -------------------------------------------------------------------------
     */
    
    /**
     * Creates a BVH using the supplied vector of objects
     * @param objects objects used to generate the BVH
     */
    void BVHTree::addObjects(const std::vector<Object*>& objects) {
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
    
    /**
     * Splits the objects into the two distinct spaces provided in the aabbs param based on their intersection
     * This is left side based and produces two vectors which are unique.
     * @param aabbs the two spaces which to split the objs vector into
     * @param objs object vector to be split into aabbs
     * @return BVHPartitionedSpace, a structure with two vectors containing the split objects. Is left side based and unique.
     */
    BVHPartitionedSpace BVHTree::partition(const std::pair<AABB, AABB>& aabbs, const std::vector<BVHObject>& objs) {
        BVHPartitionedSpace space;
        for (const auto& obj: objs) {
            // if this object doesn't have an AABB, we cannot use a BVH on it. If this ever fails we have a problem with the implementation.
            RTAssert(!obj.aabb.isEmpty());
            if (aabbs.first.intersects(obj.aabb))
                space.left.push_back(obj);
            else if (aabbs.second.intersects(obj.aabb))
                space.right.push_back(obj);
        }
        return space;
    }
    
    /**
     * Returns the object array from the first bounding tree with objects in it hit by the provided ray.
     * @firstHitRayIntersectTraversal for more information.
     */
    std::vector<BVHObject> BVHTree::rayFirstHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
        RTAssert(root != nullptr);
        auto results = root->firstHitRayIntersectTraversal(ray, min, max);
        RTAssert(results.ptr != nullptr);
        if (results.hit)
            return results.ptr->objs;
        else
            return {};
    }
    /**
     * Returns all the objects intersected by the provided ray. The returned objects are in no particular order.
     * @param ray to use in AABB intersection
     * @param min min t allowed for intersection search
     * @param max max t allowed
     * @return a unordered array of objects intersected by ray in this BVH.
     */
    std::vector<BVHObject> BVHTree::rayAnyHitIntersect(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
        std::queue<BVHNode*> nodes{};
        std::vector<BVHObject> objects;
        nodes.push(root);
        
        while (!nodes.empty()) {
            auto* node = nodes.front();
            
            auto AABB = node->aabb;
            auto nodeHitData = AABB.intersects(ray, min, max);
            if (nodeHitData.hit) {
                if (node->left != nullptr)
                    nodes.push(node->left);
                if (node->right != nullptr)
                    nodes.push(node->right);
    
                if (!node->objs.empty()) {
                    for (const auto& obj: node->objs)
                        objects.push_back(obj);
                }
            }
            nodes.pop();
        }
        return objects;
    }
    BVHNode* BVHTree::addObjectsRecur(const std::vector<BVHObject>& objects, const BVHPartitionedSpace& prevSpace) {
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
    bool BVHTree::vectorEquals(const BVHPartitionedSpace& oldSpace, const BVHPartitionedSpace& newSpace) {
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
}

/**
 * UNUSED CODE:
 */
//    #ifdef COMPILE_GUI
//    // renders all the debug VAOs on screen.
//    void render(Shader& worldShader) {
//        ImGui::Begin(("BVH Data "), nullptr, ImGuiWindowFlags_NoCollapse);
//        worldShader.use();
//        worldShader.setInt("useWhite", 1);
//        worldShader.setVec3("color", {1.0, 1.0, 1.0});
//        {
//            ImGui::BeginChild("left pane", ImVec2(180, 0), true);
//            guiNodesRecur(root);
//            ImGui::EndChild();
//        }
//        ImGui::SameLine();
//        {
//            ImGui::BeginGroup();
//            ImGui::BeginChild("item view",
//                              ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
//                              true,
//                              ImGuiWindowFlags_AlwaysAutoResize); // Leave room for 1 line below us
//            drawNodesRecur(worldShader, root);
//            ImGui::EndChild();
//            ImGui::EndGroup();
//        }
//        worldShader.setInt("useWhite", 0);
//        ImGui::End();
//    }
//    #endif
//#ifdef COMPILE_GUI
//    void drawNodesRecur(Shader& worldShader, BVHNode* node) {
//        node->draw(worldShader);
//        if (node->left != nullptr)
//            drawNodesRecur(worldShader, node->left);
//        if (node->right != nullptr)
//            drawNodesRecur(worldShader, node->right);
//    }
//    void guiNodesRecur(BVHNode* node) {
//        node->gui();
//        if (node->left != nullptr)
//            guiNodesRecur(node->left);
//        if (node->right != nullptr)
//            guiNodesRecur(node->right);
//    }
//#endif

// BVH NODE

//    static Raytracing::Mat4x4 getTransform(const AABB& _aabb) {
//        Raytracing::Mat4x4 transform{};
//        auto center = _aabb.getCenter();
//        transform.translate(center);
//        auto xRadius = _aabb.getXRadius(center) * 2;
//        auto yRadius = _aabb.getYRadius(center) * 2;
//        auto zRadius = _aabb.getZRadius(center) * 2;
//        transform.scale(float(xRadius), float(yRadius), float(zRadius));
//        return transform;
//    }

//    #ifdef COMPILE_GUI
//    void draw(Shader& worldShader) {
//        worldShader.setVec3("color", {1.0, 1.0, 1.0});
//        aabbVAO->bind();
//        if (selected == index) {
//            if (selected == index && ImGui::BeginListBox("", ImVec2(250, 350))) {
//                std::stringstream strs;
//                strs << aabb;
//                ImGui::Text("%s", strs.str().c_str());
//                for (const auto& item: objs) {
//                    auto pos = item.ptr->getPosition();
//                    std::stringstream stm;
//                    stm << item.aabb;
//                    ImGui::Text("%s,\n\t%s", (std::to_string(pos.x()) + " " + std::to_string(pos.y()) + " " + std::to_string(pos.z())).c_str(),
//                                stm.str().c_str());
//                }
//                ImGui::EndListBox();
//            }
//
//            for (const auto& obj: objs) {
//                auto transform = getTransform(obj.aabb);
//                worldShader.setMatrix("transform", transform);
//                aabbVAO->draw(worldShader);
//            }
//            auto transform = getTransform(aabb);
//            worldShader.setMatrix("transform", transform);
//            aabbVAO->draw(worldShader);
//
//            /*auto splitAABBs = aabb.splitByLongestAxis();
//            transform = getTransform(splitAABBs.second);
//            worldShader.setMatrix("transform", transform);
//            aabbVAO->draw(worldShader);
//            transform = getTransform(splitAABBs.first);
//            worldShader.setMatrix("transform", transform);
//            aabbVAO->draw(worldShader);*/
//        }
//        if (hit){
//            if (hit == 1)
//                worldShader.setVec3("color", {0.0, 0.0, 1.0});
//            else if (hit == 2)
//                worldShader.setVec3("color", {0.0, 1.0, 0.0});
//            else
//                worldShader.setVec3("color", {1.0, 0.5, 0.5});
//            auto transform = getTransform(aabb);
//            worldShader.setMatrix("transform", transform);
//            aabbVAO->draw(worldShader);
//        }
//    }
//    void gui() const {
//        int c1 = -1;
//        int c2 = -1;
//        if (left != nullptr)
//            c1 = left->index;
//        if (right != nullptr)
//            c2 = right->index;
//        std::string t;
//        if (c1 == -1 && c2 == -1)
//            t = " LEAF";
//        else
//            t = " L: " + std::to_string(c1) + " R: " + std::to_string(c2);
//        if (ImGui::Selectable(("S: " + std::to_string(objs.size()) + " I: " + std::to_string(index) + t).c_str(), selected == index))
//            selected = index;
//    }
//    #endif