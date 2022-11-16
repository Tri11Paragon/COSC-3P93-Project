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
     *
     * @param ray
     * @param min
     * @param max
     * @return
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
}