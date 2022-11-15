/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/world.h"
#include "engine/raytracing.h"

namespace Raytracing {

    World::~World() {
        for (auto* p: objects)
            delete (p);
        for (const auto& p: materials)
            delete (p.second);
        //delete(bvhObjects);
    }

    HitData SphereObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        PRECISION_TYPE radiusSquared = radius * radius;
        // move the ray to be with respects to the sphere
        Vec4 RayWRTSphere = ray.getStartingPoint() - position;
        // now determine the discriminant for the quadratic formula for the function of line sphere intercept
        PRECISION_TYPE a = ray.getDirection().lengthSquared();
        PRECISION_TYPE b = Raytracing::Vec4::dot(RayWRTSphere, ray.getDirection());
        PRECISION_TYPE c = RayWRTSphere.lengthSquared() - radiusSquared;
        // > 0: the hit has two roots, meaning we hit both sides of the sphere
        // = 0: the ray has one root, we hit the edge of the sphere
        // < 0: ray isn't inside the sphere.
        PRECISION_TYPE discriminant = b * b - (a * c);

        // < 0: ray isn't inside the sphere. Don't need to bother calculating the roots.
        if (discriminant < 0)
            return {false, Vec4(), Vec4(), 0};

        // now we have to find the root which exists inside our range [min,max]
        auto root = (-b - std::sqrt(discriminant)) / a;
        // if the first root isn't in our range
        if (root < min || root > max) {
            // check the second root
            root = (-b + std::sqrt(discriminant)) / a;
            if (root < min || root > max) {
                // if the second isn't in the range then we also must return false.
                return {false, Vec4(), Vec4(), 0};
            }
        }
        // the hit point is where the ray is when extended to the root
        auto RayAtRoot = ray.along(root);
        // The normal of a sphere is just the point of the hit minus the center position
        auto normal = (RayAtRoot - position).normalize();

        /*if (Raytracing::vec4::dot(ray.getDirection(), normal) > 0.0) {
            tlog << "ray inside sphere\n";
        } else
            tlog << "ray outside sphere\n";
        */
        return {true, RayAtRoot, normal, root};
    }

    std::pair<HitData, Object*> World::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        // actually speeds up rendering by about 110,000ms (total across 16 threads)
        if (bvhObjects != nullptr){
            auto hResult = HitData{false, Vec4(), Vec4(), max};
            Object* objPtr = nullptr;
            
            auto intersected = bvhObjects->rayIntersect(ray, min, max);
            
            //tlog << " Our intersections found: " << intersected.size() << " objs\n";
            
            //dlog << "Intersections " << intersected.objs.size() << " " << ray << "\n";
            
            for (const auto& ptr : intersected) {
                auto cResult = ptr.ptr->checkIfHit(ray, min, hResult.length);
                if (cResult.hit) {
                    hResult = cResult;
                    objPtr = ptr.ptr;
                }
            }
            // after we check the BVH, we have to check for other missing objects
            // since stuff like spheres currently don't have AABB and AABB isn't a requirement
            // for the object class (to be assigned)
            for (auto* obj: bvhObjects->noAABBObjects) {
                // check up to the point of the last closest hit,
                // will give the closest object's hit result
                auto cResult = obj->checkIfHit(ray, min, hResult.length);
                if (cResult.hit) {
                    hResult = cResult;
                    objPtr = obj;
                }
            }
            
            return {hResult, objPtr};
        } else {
            // rejection algo without using a binary space partitioning data structure
            auto hResult = HitData{false, Vec4(), Vec4(), max};
            Object* objPtr = nullptr;
            for (auto* obj: objects) {
                // check up to the point of the last closest hit,
                // will give the closest object's hit result
                auto cResult = obj->checkIfHit(ray, min, hResult.length);
                if (cResult.hit) {
                    hResult = cResult;
                    objPtr = obj;
                }
            }
            return {hResult, objPtr};
        }
    }

    void World::generateBVH() {
        bvhObjects = std::make_unique<BVHTree>(objects);
    }

    ScatterResults DiffuseMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        Vec4 newRay = hitData.normal + Raytracing::Raycaster::randomUnitVector().normalize();

        // rays that are close to zero are liable to floating point precision errors
        if (newRay.x() < EPSILON && newRay.y() < EPSILON && newRay.z() < EPSILON && newRay.w() < EPSILON)
            newRay = hitData.normal;

        return {true, Ray{hitData.hitPoint, newRay}, getBaseColor()};
    }

    ScatterResults MetalMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        // create a ray reflection
        Vec4 newRay = reflect(ray.getDirection().normalize(), hitData.normal);
        // make sure our reflected ray is outside the sphere and doesn't point inwards
        bool shouldReflect = Vec4::dot(newRay, hitData.normal) > 0;
        return {shouldReflect, Ray{hitData.hitPoint, newRay}, getBaseColor()};
    }

    ScatterResults BrushedMetalMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        // create a ray reflection
        Vec4 newRay = reflect(ray.getDirection().normalize(), hitData.normal);
        // make sure our reflected ray is outside the sphere and doesn't point inwards
        bool shouldReflect = Vec4::dot(newRay, hitData.normal) > 0;
        return {shouldReflect, Ray{hitData.hitPoint, newRay + Raycaster::randomUnitVector() * fuzzyness}, getBaseColor()};
    }

    static HitData checkIfTriangleGotHit(const Triangle& theTriangle, const Vec4& position, const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) {
        // Möller–Trumbore intersection algorithm
        // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        Vec4 edge1, edge2, h, s, q;
        PRECISION_TYPE a, f, u, v;
        edge1 = (theTriangle.vertex2 + position) - (theTriangle.vertex1 + position);
        edge2 = (theTriangle.vertex3 + position) - (theTriangle.vertex1 + position);

        h = Vec4::cross(ray.getDirection(), edge2);
        a = Vec4::dot(edge1, h);

        if (a > -EPSILON && a < EPSILON)
            return {false, Vec4(), Vec4(), 0}; //parallel to triangle

        f = 1.0 / a;
        s = ray.getStartingPoint() - (theTriangle.vertex1 + position);
        u = f * Vec4::dot(s, h);

        if (u < 0.0 || u > 1.0)
            return {false, Vec4(), Vec4(), 0};

        q = Vec4::cross(s, edge1);
        v = f * Vec4::dot(ray.getDirection(), q);
        if (v < 0.0 || u + v > 1.0)
            return {false, Vec4(), Vec4(), 0};

        // At this stage we can compute t to find out where the intersection point is on the line.
        PRECISION_TYPE t = f * Vec4::dot(edge2, q);
        // keep t in reasonable bounds, ensuring we respect depth
        if (t > EPSILON && t >= min && t <= max) {
            // ray intersects
            Vec4 rayIntersectionPoint = ray.along(t);
            Vec4 normal;
            // normal = theTriangle.findClosestNormal(rayIntersectionPoint - position);
            if (theTriangle.hasNormals) // returning the closest normal is extra computation when n1 would likely be fine.
                normal = theTriangle.normal1;
            else {
                // standard points to normal algorithm but using already computed edges
                normal = Vec4{edge1.y() * edge2.z(), edge1.z() * edge2.x(), edge1.x() * edge2.y()} -
                         Vec4{edge1.z() * edge2.y(), edge1.x() * edge2.z(), edge1.y() * edge2.x()};
            }
            return {true, rayIntersectionPoint, normal, t};
        }
        return {false, Vec4(), Vec4(), 0};
    }

    HitData TriangleObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        return checkIfTriangleGotHit(theTriangle, position, ray, min, max);
    }

    HitData ModelObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        auto hResult = HitData{false, Vec4(), Vec4(), max};
        // must check through all the triangles in the object
        // respecting depth along the way
        // but reducing the max it can reach my the last longest vector length.
        for (const Triangle& t : triangles) {
            auto cResult = checkIfTriangleGotHit(t, position, ray, min, hResult.length);
            if (cResult.hit)
                hResult = cResult;
        }
        /*auto hResult = HitData{false, Vec4(), Vec4(), max};
        
        auto intersected = tree->rayIntersect(ray, min, max);
        
        for (auto t : intersected){
            // apparently this kind of casting is okay
            // which makes sense since the actual data behind it is a empty object
            // just this is really bad and im too annoyed to figure out a better way. TODO:.
            auto cResult = checkIfTriangleGotHit(((EmptyObject*)(t))->tri, position, ray, min, hResult.length);
            if (cResult.hit)
                hResult = cResult;
        }*/
        
        return hResult;
    }
}