/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_WORLD_H
#define STEP_2_WORLD_H

#include <util/std.h>
#include <math/vectors.h>
#include <util/models.h>

#include <utility>

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
        private:
            Material* material;
        public:
            explicit Object(Material* material): material(material) {};
            // return true if the ray intersects with this object, only between min and max
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const = 0;

            Material* getMaterial() { return material; }

            virtual ~Object() = default;
    };

    class SphereObject : public Object {
        private:
            vec4 position;
            PRECISION_TYPE radius;
        public:
            SphereObject(const vec4& position, PRECISION_TYPE radius, Material* material): position(position), radius(radius), Object(material) {}

            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class TriangleObject : public Object {
        private:
            vec4 position;
            Triangle theTriangle;
        public:
            TriangleObject(const vec4& position, Triangle theTriangle, Material* material): Object(material), position(position),
                                                                                            theTriangle(std::move(theTriangle)) {}
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class DiffuseMaterial : public Material {
        private:
        public:
            explicit DiffuseMaterial(const vec4& scatterColor): Material(scatterColor) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class MetalMaterial : public Material {
        protected:
            static inline vec4 reflect(const vec4& incomingVector, const vec4& normal) {
                return incomingVector - 2 * vec4::dot(incomingVector, normal) * normal;
            }

        public:
            explicit MetalMaterial(const vec4& metalColor): Material(metalColor) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class BrushedMetalMaterial : public MetalMaterial {
        private:
            PRECISION_TYPE fuzzyness;
        public:
            explicit BrushedMetalMaterial(const vec4& metalColor, PRECISION_TYPE fuzzyness): MetalMaterial(metalColor), fuzzyness(fuzzyness) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class World {
        private:
            // store all the objects in the world,
            std::vector<Object*> objects;
            /*TODO: create a kd-tree or bvh version to store the objects
             * this way we can easily tell if a ray is near and object or not
             * saving on computation
             */
            std::unordered_map<std::string, Material*> materials;
        public:
            World() = default;
            World(const World& world) = delete;
            World(const World&& world) = delete;

            inline void add(Object* object) { objects.push_back(object); }

            inline void addMaterial(const std::string& materialName, Material* mat) { materials.insert({materialName, mat}); }

            inline Material* getMaterial(const std::string& materialName) { return materials.at(materialName); }

            [[nodiscard]] virtual std::pair<HitData, Object*> checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            ~World();

    };
}

#endif //STEP_2_WORLD_H
