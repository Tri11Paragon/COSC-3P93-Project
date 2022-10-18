/*
 * Created by Brett Terpstra 6920201 on 18/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_TYPES_H
#define STEP_2_TYPES_H

#include <math/vectors.h>
#include <math/colliders.h>

// there were some files which needed access to these types
// but including them from world.h would've resulted in circular includes,
// so I moved them here.

namespace Raytracing {
    struct HitData {
        // all the other values only matter if this is true
        bool hit{false};
        // the hit point on the object
        vec4 hitPoint{};
        // the normal of that hit point
        vec4 normal{};
        // the length of the vector from its origin in its direction.
        PRECISION_TYPE length{0};
    };

    struct ScatterResults {
        // returns true to recast the ray with the provided ray
        bool scattered;
        // the new ray to be cast if scattered
        Ray newRay;
        // the color of the material
        vec4 attenuationColor;
    };

    class Material {
        private:
            // most materials will need an albedo
            vec4 baseColor;
        public:
            explicit Material(const vec4& baseColor): baseColor(baseColor) {}

            // returns true if the ray was scattered along with the scattered ray, otherwise will return false with empty ray.
            // the returned vec4 is the attenuation color
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const = 0;

            [[nodiscard]] vec4 getBaseColor() const { return baseColor; }
    };

    class Object {
        protected:
            vec4 position;
            Material* material;
            AABB aabb;
        public:
            explicit Object(Material* material, const vec4& position): material(material), position(position) {};
            // return true if the ray intersects with this object, only between min and max
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const = 0;

            [[nodiscard]] Material* getMaterial() const { return material; }
            [[nodiscard]] vec4 getPosition() const {return position;}
            [[nodiscard]] AABB getAABB() const {return aabb;}

            virtual ~Object() = default;
    };
}

#endif //STEP_2_TYPES_H
