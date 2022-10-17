/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <world.h>

namespace Raytracing {

    World::~World() {
        for (auto* p : objects)
            delete(p);
    }

    Object::HitData SphereObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        PRECISION_TYPE radiusSquared = radius * radius;
        // move the ray to be with respects to the sphere
        vec4 RayWRTSphere = ray.getStartingPoint() - position;
        // now determine the discriminant for the quadratic formula for the function of line sphere intercept
        PRECISION_TYPE a = ray.getDirection().lengthSquared();
        PRECISION_TYPE b = Raytracing::vec4::dot(RayWRTSphere, ray.getDirection());
        PRECISION_TYPE c = RayWRTSphere.lengthSquared() - radiusSquared;
        // > 0: the hit has two roots, meaning we hit both sides of the sphere
        // = 0: the ray has one root, we hit the edge of the sphere
        // < 0: ray isn't inside the sphere.
        PRECISION_TYPE discriminant = b * b - (a * c);

        // < 0: ray isn't inside the sphere. Don't need to bother calculating the roots.
        if (discriminant < 0)
            return {false, vec4(), vec4(), 0};

        // now we have to find the root which exists inside our range [min,max]
        auto root = (-b - std::sqrt(discriminant)) / a;
        // if the first root isn't in our range
        if (root < min || root > max) {
            // check the second root
            root = (-b + std::sqrt(discriminant)) / a;
            if (root < min || root > max) {
                // if the second isn't in the range then we also must return false.
                return {false, vec4(), vec4(), 0};
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

    Object::HitData World::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        auto hResult = Object::HitData{false, vec4(), vec4(), max};
        for (auto* obj : objects){
            // check up to the point of the last closest hit,
            // will give the closest object's hit result
            auto cResult = obj->checkIfHit(ray, min, hResult.length);
            if (cResult.hit)
                hResult = cResult;
        }
        return hResult;
    }
}