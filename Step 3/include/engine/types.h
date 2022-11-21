/*
 * Created by Brett Terpstra 6920201 on 18/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_TYPES_H
#define STEP_2_TYPES_H

#include "engine/math/vectors.h"
#include "engine/math/colliders.h"
#include <config.h>

#ifdef COMPILE_GUI
    #include <graphics/gl/gl.h>
#endif


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
        // Texture UV Coords.
        PRECISION_TYPE u,v;
    };

    struct ScatterResults {
        // returns true to recast the ray with the provided ray
        bool scattered;
        // the new ray to be cast if scattered
        Ray newRay;
        // the color of the material
        Vec4 attenuationColor;
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
            [[nodiscard]] virtual Vec4 getColor(PRECISION_TYPE u, PRECISION_TYPE v, const Vec4& point) const = 0;

            [[nodiscard]] Vec4 getBaseColor() const { return baseColor; }
            virtual ~Material() = default;
    };
    
    struct DebugBVHData {
        void* bvhTree;
        bool isRegular;
    };

    class Object {
        protected:
            AABB aabb;
            Vec4 position;
            Material* material;
            #ifdef COMPILE_GUI
                VAO* vao = nullptr;
            #endif
        public:
            Object(Material* material, const Vec4& position): material(material), position(position), aabb({}) {};
            // return true if the ray intersects with this object, only between min and max
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const = 0;

            [[nodiscard]] Material* getMaterial() const { return material; }
            [[nodiscard]] virtual AABB& getAABB() { return aabb; }
            // FIXME: Use of void* is inadvisable. Although this is only for debug consider another method.
            [[nodiscard]] virtual DebugBVHData getBVHTree(){ return {nullptr, false}; }
            [[nodiscard]] Vec4 getPosition() const { return position; }
            virtual void setAABB(const AABB& ab) { this->aabb = ab; }
            #ifdef COMPILE_GUI
                [[nodiscard]] inline VAO* getVAO(){return vao;}
            #endif
            virtual ~Object() = default;
    };
}

#endif //STEP_2_TYPES_H
