/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include "engine/world.h"
#include "engine/raytracing.h"
#include "engine/image/stb_image.h"

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
        // calculate the uv coords and normalize to [0, 1]
        PRECISION_TYPE u = (atan2(-RayAtRoot.z(), RayAtRoot.x()) + std::numbers::pi) / (2 * std::numbers::pi);
        PRECISION_TYPE v = acos(RayAtRoot.y()) / std::numbers::pi;
        return {true, RayAtRoot, normal, root, clamp(u, 0, 1.0), clamp(v, 0, 1.0)};
    }

    std::pair<HitData, Object*> World::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        // actually speeds up rendering by about 110,000ms (total across 16 threads)
        if (bvhObjects != nullptr && m_config.useBVH){
            auto hResult = HitData{false, Vec4(), Vec4(), max};
            Object* objPtr = nullptr;
            
            auto intersected = bvhObjects->rayAnyHitIntersect(ray, min, max);
            
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
    
    ScatterResults TexturedMaterial::scatter(const Ray& ray, const HitData& hitData) const {
        Vec4 newRay = hitData.normal + Raytracing::Raycaster::randomUnitVector().normalize();
    
        // rays that are close to zero are liable to floating point precision errors
        if (newRay.x() < EPSILON && newRay.y() < EPSILON && newRay.z() < EPSILON && newRay.w() < EPSILON)
            newRay = hitData.normal;
    
        return {true, Ray{hitData.hitPoint, newRay}, getColor(hitData.u, hitData.v, hitData.hitPoint)};
    }
    Vec4 TexturedMaterial::getColor(PRECISION_TYPE u, PRECISION_TYPE v, const Vec4& point) const {
        // if we are unable to load the image return the debug color.
        // This causes major issues (force this to happen, you'll see), indicates issue + looks really cool.
        if (!data)
            return Vec4{0, 1, 0.2} * Vec4{u, v, 1.0};
        
        // if you render out the debug color above you'll notice that the UV coords are rotated.
        // you can also see this from the debug view, which *as of now* is rendering based on UV coords * normals * red
        // so let's transform it back and ensure that our UV coords are within image bounds.
        u = clamp(u, 0, 1);
        // fix that pesky issue
        v = 1.0 - clamp(v, 0, 1);
        
        auto imageX = (int)(width * u);
        auto imageY = (int)(height * v);
    
        if (imageX >= width)  imageX = width-1;
        if (imageY >= height) imageY = height-1;
        
        // since stbi loads in RGB8 [0, 255] but the engine works on [0, 1] we need to scale the data down.
        // this is best done with a single division followed by multiple multiplication.
        // since this function needs to be cheap to run.
        const PRECISION_TYPE colorFactor = 1.0 / 255.0;
        const auto pixelData = data + (imageY * rowWidth + imageX * channels);
        
        return {pixelData[0] * colorFactor, pixelData[1] * colorFactor, pixelData[2] * colorFactor};
    }
    TexturedMaterial::TexturedMaterial(const std::string& file) : Material({}) {
        // we are going to have to ignore transparency for now. TODO:?
        data = stbi_load(file.c_str(), &width, &height, &channels, 0);
        if (!data)
            flog << "Unable to load image file " << file << "!\n";
        else
            ilog << "Loaded image " << file << "!\n";
        rowWidth = width * channels;
    }
    TexturedMaterial::~TexturedMaterial() {
        delete(data);
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
            
            // calculate triangle UV
            // calculate the vector that runs between the vertex and the intersection point for all three vertices
            auto vertex1ToIntersect = theTriangle.vertex1 - rayIntersectionPoint;
            auto vertex2ToIntersect = theTriangle.vertex2 - rayIntersectionPoint;
            auto vertex3ToIntersect = theTriangle.vertex3 - rayIntersectionPoint;
            
            // the magnitude of the cross product of two vectors is double the area formed by the triangle of their intersection.
            auto fullArea = 1 / Vec4::cross(theTriangle.vertex1 - theTriangle.vertex2, theTriangle.vertex1 - theTriangle.vertex3).magnitude();
            // scale the area of sub triangles to be proportion to the area of the triangle
            auto areaVert1 = Vec4::cross(vertex2ToIntersect, vertex3ToIntersect).magnitude() * fullArea;
            auto areaVert2 = Vec4::cross(vertex3ToIntersect, vertex1ToIntersect).magnitude() * fullArea;
            auto areaVert3 = Vec4::cross(vertex1ToIntersect, vertex2ToIntersect).magnitude() * fullArea;
            
            // that area is how much each UV factors into the final UV coord
            auto uv = theTriangle.uv1 * areaVert1 + theTriangle.uv2 * areaVert2 + theTriangle.uv3 * areaVert3;
            
            return {true, rayIntersectionPoint, normal, t, clamp(uv.x(), 0, 1.0), clamp(uv.y(), 0, 1.0)};
        }
        
        return {false, Vec4(), Vec4(), 0};
    }

    HitData ModelObject::checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
        auto hResult = HitData{false, Vec4(), Vec4(), max};
        
        //auto tris = triangleBVH->rayAnyHitIntersect(ray, min, max);
        
        // must check through all the triangles in the object
        // respecting depth along the way
        // but reducing the max it can reach my the last longest vector length.
        for (const auto& t : triangles) {
            auto cResult = checkIfTriangleGotHit(*t, position, ray, min, hResult.length);
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