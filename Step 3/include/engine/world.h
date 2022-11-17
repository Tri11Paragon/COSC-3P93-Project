/*
 * Created by Brett Terpstra 6920201 on 16/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_2_WORLD_H
#define STEP_2_WORLD_H

#include "engine/util/std.h"
#include "engine/math/vectors.h"
#include "engine/util/models.h"
#include "engine/math/bvh.h"
#include "types.h"
#include "graphics/gl/shader.h"

#include <utility>


namespace Raytracing {

    class SphereObject : public Object {
        private:
            PRECISION_TYPE radius;
        public:
            SphereObject(const Vec4& position, PRECISION_TYPE radius, Material* material): radius(radius), Object(material, position) {}

            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class TriangleObject : public Object {
        private:
            Triangle theTriangle;
        public:
            TriangleObject(const Vec4& position, Triangle tri, Material* material): Object(material, position), 
                                                                                            theTriangle(std::move(tri)) {}
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            virtual Object* clone() {
                return new TriangleObject(position, theTriangle, material);
            }
    };

    class ModelObject : public Object {
        private:
            std::vector<Triangle> triangles;
        public:
            ModelObject(const Vec4& position, ModelData& data, Material* material): Object(material, position) {
                // since all of this occurs before the main ray tracing algorithm it's fine to do sequentially
                TriangulatedModel model {data};
                this->triangles = model.triangles;
                this->aabb = std::move(model.aabb);
                #ifdef COMPILE_GUI
                    vao = new VAO(triangles);
                #endif
            }
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };

    class DiffuseMaterial : public Material {
        private:
        public:
            explicit DiffuseMaterial(const Vec4& scatterColor): Material(scatterColor) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class MetalMaterial : public Material {
        protected:
            static inline Vec4 reflect(const Vec4& incomingVector, const Vec4& normal) {
                return incomingVector - 2 * Vec4::dot(incomingVector, normal) * normal;
            }

        public:
            explicit MetalMaterial(const Vec4& metalColor): Material(metalColor) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class BrushedMetalMaterial : public MetalMaterial {
        private:
            PRECISION_TYPE fuzzyness;
        public:
            explicit BrushedMetalMaterial(const Vec4& metalColor, PRECISION_TYPE fuzzyness): MetalMaterial(metalColor), fuzzyness(fuzzyness) {}

            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const;
    };

    class TexturedMaterial : public Material {
        public:
            explicit TexturedMaterial(const std::string& file): Material({}) {
                
            }
    };
    
    struct WorldConfig {
        bool useBVH = true;
        bool padding[7];
    };

    class World {
        private:
            // store all the objects in the world,
            std::vector<Object*> objects;
            std::unique_ptr<BVHTree> bvhObjects;
            std::unordered_map<std::string, Material*> materials;
            WorldConfig m_config;
        public:
            explicit World(WorldConfig config): m_config(config) {};
            World(const World& world) = delete;
            World(const World&& world) = delete;

            // Called by the raytracer class after all objects have been added to the world
            // this allows us to generate a statically unchanging BVH for easy rendering
            void generateBVH();
            #ifdef COMPILE_GUI
                // currently disabled. TODO: BVH renderer class
                void drawBVH(Shader& worldShader) {}
            #endif

            inline void add(Object* object) { objects.push_back(object); }

            inline void add(const std::string& materialName, Material* mat) { materials.insert({materialName, mat}); }

            inline Material* getMaterial(const std::string& materialName) { return materials.at(materialName); }
            [[nodiscard]] inline BVHTree* getBVH() { return bvhObjects.get(); }
            [[nodiscard]] inline std::vector<Object*> getObjectsInWorld(){return objects; }

            [[nodiscard]] virtual std::pair<HitData, Object*> checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            ~World();

    };
}

#endif //STEP_2_WORLD_H
