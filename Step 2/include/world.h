/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_WORLD_H
#define STEP_2_WORLD_H

#include <util/std.h>
#include <math/vectors.h>

namespace Raytracing {

    class Object {
        public:
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
            // return true if the ray intersects with this object, only between min and max
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const = 0;
            virtual ~Object() = default;
    };

    class SphereObject : public Object {
        private:
            vec4 position;
            PRECISION_TYPE radius;

        public:
            SphereObject(const vec4& position, PRECISION_TYPE radius): position(position), radius(radius) {}

            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class World {
        private:
            // store all the objects in the world,
            std::vector<Object*> objects;
            /*TODO: create a kd-tree or bvh version to store the objects
             * this way we can easily tell if a ray is near and object or not
             * saving on computation
             */
        public:
            World() = default;
            World(const World& world) = delete;
            World(const World&& world) = delete;

            inline void add(Object* object) { objects.push_back(object); }

            [[nodiscard]] virtual Object::HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            ~World();

    };
}

#endif //STEP_2_WORLD_H
