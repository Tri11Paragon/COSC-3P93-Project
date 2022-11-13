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
            SphereObject(const Vec4& position, PRECISION_TYPE radius, Material* material): radius(radius), Object(material, position) {
               // aabb = AABB(position.x(), position.y(), position.z(), radius);
            }

            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            virtual Object* clone(){
                return new SphereObject(position, radius, material);
            }
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
            ModelData& data;
            // basically we have to store this crap here because c++ loves to copy stuff
            //std::vector<Object*> createdTreeObjects{};
            //BVHTree* tree = nullptr;
        public:
            ModelObject(const Vec4& position, ModelData& data, Material* material): Object(material, position), data(data) {
                // since all of this occurs before the main ray tracing algorithm it's fine to do sequentially
                triangles = data.toTriangles();
                this->aabb = data.aabb;
                //createdTreeObjects = Raytracing::ModelData::createBVHTree(triangles, position);
                //tree = new BVHTree(createdTreeObjects);
            }
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            virtual Object* clone() {
                return new ModelObject(position, data, material);
            }
            virtual ~ModelObject() {
                // Disabled for now, causing bugs when on release mode.
                //for (auto* p : createdTreeObjects)
                //    delete(p);
                //delete(tree);
            }
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
            TexturedMaterial(const std::string& file): Material({}) {
                
            }
    };

    class World {
        private:
            // store all the objects in the world,
            std::vector<Object*> objects;
            /*TODO: create a kd-tree or bvh version to store the objects
             * this way we can easily tell if a ray is near and object or not
             * saving on computation
             */
            // TODO: the above todo has been done, now we need to test the performance advantage of the BVH
            std::unique_ptr<BVHTree> bvhObjects;
            std::unordered_map<std::string, Material*> materials;
        public:
            World() = default;
            World(const World& world) = delete;
            World(const World&& world) = delete;

            // Called by the raytracer class after all objects have been added to the world
            // this allows us to generate a statically unchanging BVH for easy rendering
            void generateBVH();
            #ifdef COMPILE_GUI
                void drawBVH(Shader& worldShader) {bvhObjects->render(worldShader);}
            #endif

            inline void add(Object* object) { objects.push_back(object); }

            inline void addMaterial(const std::string& materialName, Material* mat) { materials.insert({materialName, mat}); }

            inline Material* getMaterial(const std::string& materialName) { return materials.at(materialName); }

            [[nodiscard]] virtual std::pair<HitData, Object*> checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            ~World();

    };
}

#endif //STEP_2_WORLD_H
