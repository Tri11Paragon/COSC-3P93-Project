/*
 * Created by Brett Terpstra 6920201 on 18/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_TYPES_H
#define STEP_2_TYPES_H

#include "engine/math/vectors.h"
#include "engine/math/colliders.h"

#include <utility>

// there were some files which needed access to these types
// but including them from world.h would've resulted in circular includes,
// so I moved them here.

namespace Raytracing {

    struct HitData {
        // all the other values only matter if this is true
        bool hit{false};
        // the hit point on the object
        Vec4 hitPoint{};
        // the normal of that hit point
        Vec4 normal{};
        // the length of the vector from its origin in its direction.
        PRECISION_TYPE length{0};
    };

    struct ScatterResults {
        // returns true to recast the ray with the provided ray
        bool scattered;
        // the new ray to be cast if scattered
        Ray newRay;
        // the color of the material
        Vec4 attenuationColor;
    };
    
    // triangle type for model loading
    struct Triangle {
        public:
            Vec4 vertex1, vertex2, vertex3;
            Vec4 normal1, normal2, normal3;
            Vec4 uv1, uv2, uv3;
            bool hasNormals = false;
            AABB aabb;
            
            Triangle(const Vec4& v1, const Vec4& v2, const Vec4& v3): vertex1(v1), vertex2(v2), vertex3(v3) {}
            
            Triangle(const Vec4& v1, const Vec4& v2, const Vec4& v3,
                     const Vec4& n1, const Vec4& n2, const Vec4& n3): vertex1(v1), vertex2(v2), vertex3(v3),
                                                                      hasNormals(true), normal1(n1), normal2(n2), normal3(n3) {}
            
            Triangle(const Vec4& v1, const Vec4& v2, const Vec4& v3,
                     const Vec4& uv1, const Vec4& uv2, const Vec4& uv3,
                     const Vec4& n1, const Vec4& n2, const Vec4& n3): vertex1(v1), vertex2(v2), vertex3(v3),
                                                                      uv1(uv1), uv2(uv2), uv3(uv3),
                                                                      hasNormals(true), normal1(n1), normal2(n2), normal3(n3) {}
            
            // slow method, not really required as all normals should be equal
            [[nodiscard]] Vec4 findClosestNormal(const Vec4& point) const {
                // no need to sqrt as exact distance doesn't matter
                auto n1Dist = (point - normal1).lengthSquared();
                auto n2Dist = (point - normal2).lengthSquared();
                auto n3Dist = (point - normal3).lengthSquared();
                return (n1Dist < n2Dist && n1Dist < n3Dist) ? normal1 : (n2Dist < n3Dist ? normal2 : normal3);
            }
    };
    
    // face type for model loading
    struct face {
        int v1, v2, v3;
        int uv1, uv2, uv3;
        int n1, n2, n3;
    };

    class Material {
        protected:
            // most materials will need an albedo
            Vec4 baseColor;
        public:
            explicit Material(const Vec4& baseColor): baseColor(baseColor) {}

            // returns true if the ray was scattered along with the scattered ray, otherwise will return false with empty ray.
            // the returned vec4 is the attenuation color
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const = 0;

            [[nodiscard]] Vec4 getBaseColor() const { return baseColor; }
            virtual ~Material() = default;
    };

    class Object {
        protected:
            AABB aabb;
            Vec4 position;
            Material* material;
        public:
            Object(Material* material, const Vec4& position): material(material), position(position), aabb({}) {};
            // return true if the ray intersects with this object, only between min and max
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const = 0;

            [[nodiscard]] Material* getMaterial() const { return material; }
            virtual Object* clone() = 0;
            virtual AABB& getAABB() { return aabb; }
            virtual void setAABB(const AABB& ab) { this->aabb = ab; }
            [[nodiscard]] Vec4 getPosition() const { return position; }
            virtual ~Object() = default;
    };
    
    // used for using an object, mostly BVH
    class EmptyObject : public Object {
        protected:
        public:
            Triangle& tri;
            EmptyObject(const Vec4& position, const AABB& a, Triangle& tri): Object(nullptr, position), tri(tri) {this->aabb = a;};
            // unused
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const {
                wlog << "Warning! A empty object has made its way into the raycaster!\n";
                return {};
            }
            virtual Object* clone(){return new EmptyObject(position, aabb, tri);}
    };
}

#endif //STEP_2_TYPES_H
