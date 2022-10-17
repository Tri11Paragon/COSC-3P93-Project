/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <world.h>
#include <raytracing.h>

namespace Raytracing {

    World::~World() {
        for (auto* p: objects)
            delete (p);
        for (const auto& p: materials)
            delete (p.second);
    }

    HitData SphereObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
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

    std::pair<HitData, Object*> World::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        auto hResult = HitData{false, vec4(), vec4(), max};
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

    ScatterResults DiffuseMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        vec4 newRay = hitData.normal + Raytracing::Raycaster::randomUnitVector().normalize();

        // rays that are close to zero are liable to floating point precision errors
        if (newRay.x() < EPSILON && newRay.y() < EPSILON && newRay.z() < EPSILON && newRay.w() < EPSILON)
            newRay = hitData.normal;

        return {true, Ray{hitData.hitPoint, newRay}, getBaseColor()};
    }

    ScatterResults MetalMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        // create a ray reflection
        vec4 newRay = reflect(ray.getDirection().normalize(), hitData.normal);
        // make sure our reflected ray is outside the sphere and doesn't point inwards
        bool shouldReflect = vec4::dot(newRay, hitData.normal) > 0;
        return {shouldReflect, Ray{hitData.hitPoint, newRay}, getBaseColor()};
    }

    ScatterResults BrushedMetalMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        // create a ray reflection
        vec4 newRay = reflect(ray.getDirection().normalize(), hitData.normal);
        // make sure our reflected ray is outside the sphere and doesn't point inwards
        bool shouldReflect = vec4::dot(newRay, hitData.normal) > 0;
        return {shouldReflect, Ray{hitData.hitPoint, newRay + Raycaster::randomUnitVector() * fuzzyness}, getBaseColor()};
    }

    HitData TriangleObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        // Möller–Trumbore intersection algorithm
        vec4 edge1, edge2, h, s, q;
        PRECISION_TYPE a, f, u, v;
        edge1 = (theTriangle.vertex2 + position) - (theTriangle.vertex1 + position);
        edge2 = (theTriangle.vertex3 + position) - (theTriangle.vertex1 + position);

        h = vec4::cross(ray.getDirection(), edge2);
        a = vec4::dot(edge1, h);

        if (a > -EPSILON && a < EPSILON)
            return {false, vec4(), vec4(), 0}; //parallel to triangle

        f = 1.0 / a;
        s = ray.getStartingPoint() - (theTriangle.vertex1 + position);
        u = f * vec4::dot(s, h);

        if (u < 0.0 || u > 1.0)
            return {false, vec4(), vec4(), 0};

        q = vec4::cross(s, edge1);
        v = f * vec4::dot(ray.getDirection(), q);
        if (v < 0.0 || u + v > 1.0)
            return {false, vec4(), vec4(), 0};

        // At this stage we can compute t to find out where the intersection point is on the line.
        PRECISION_TYPE t = f * vec4::dot(edge2, q);
        if (t > EPSILON) {
            // ray intersects
            vec4 rayIntersectionPoint = ray.along(t);
            vec4 normal;
            if (theTriangle.hasNormals) // TODO: deal with n2 and n3
                normal = theTriangle.normal1;
            else {
                // standard points to normal algorithm but using already computed edges
                normal = vec4{edge1.y() * edge2.z(), edge1.z() * edge2.x(), edge1.x() * edge2.y()} -
                         vec4{edge1.z() * edge2.y(), edge1.x() * edge2.z(), edge1.y() * edge2.x()};
            }
            return {true, rayIntersectionPoint, normal, t};
        }
        return {false, vec4(), vec4(), 0};
    }
}