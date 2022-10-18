/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_WORLD_H
#define STEP_2_WORLD_H

#include <util/std.h>
#include <math/vectors.h>
#include <util/models.h>
#include <math/bvh.h>
#include <types.h>

#include <utility>

namespace Raytracing {

    class SphereObject : public Object {
        private:
            PRECISION_TYPE radius;
        public:
            SphereObject(const vec4& position, PRECISION_TYPE radius, Material* material): radius(radius), Object(material, position) {}

            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class TriangleObject : public Object {
        private:
            Triangle theTriangle;
        public:
            TriangleObject(const vec4& position, Triangle theTriangle, Material* material): Object(material, position),
                                                                                            theTriangle(std::move(theTriangle)) {}
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class ModelObject : public Object {
        private:
            std::vector<Triangle> triangles;
        public:
            ModelObject(const vec4& position, ModelData data, Material* material): Object(material, position) {
                triangles = data.toTriangles();
                this->aabb = data.aabb;
            }
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
