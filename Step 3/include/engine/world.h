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

#include <config.h>

#ifdef COMPILE_GUI
    
    #include "graphics/gl/shader.h"
    #include "graphics/debug_gui.h"

#endif

#include <utility>


namespace Raytracing {
    
    class SphereObject : public Object {
        private:
            PRECISION_TYPE radius;
        public:
            SphereObject(const Vec4& position, PRECISION_TYPE radius, Material* material):
                    radius(radius), Object(material, position) {}
            
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };
    
    class ModelObject : public Object {
        private:
            std::vector<std::shared_ptr<Triangle>> triangles;
            std::unique_ptr<TriangleBVHTree> triangleBVH;
        public:
            ModelObject(const Vec4& position, ModelData& data, Material* material):
                    Object(material, position) {
                // since all of this occurs before the main ray tracing algorithm it's fine to do sequentially
                TriangulatedModel model{data};
                this->triangles = model.triangles;
                this->aabb = std::move(model.aabb);
                // a required step to generate a BVH however we aren't using the triangle bvh due to issues with it
                // so ignore this and sequential triangle BVH nonsense.
                std::vector<TriangleBVHObject> triangulatedObjects;
                for (const auto& tri : triangles) {
                    TriangleBVHObject triangleObject;
                    triangleObject.tri = tri;
                    triangleObject.aabb = tri->aabb;
                    triangleObject.position = position;
                    triangulatedObjects.push_back(triangleObject);
                }
                triangleBVH = std::make_unique<TriangleBVHTree>(triangulatedObjects);
#ifdef COMPILE_GUI
                vao = new VAO(triangles);
#endif
            }
            
            [[nodiscard]] virtual DebugBVHData getBVHTree() { return {triangleBVH.get(), false}; }
            
            [[nodiscard]] virtual std::vector<std::shared_ptr<Triangle>> getTriangles() { return triangles; }
            
            [[nodiscard]] virtual HitData checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
    };
    
    class DiffuseMaterial : public Material {
        private:
        public:
            explicit DiffuseMaterial(const Vec4& scatterColor):
                    Material(scatterColor) {}
            
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const override;
    };
    
    class MetalMaterial : public Material {
        protected:
            static inline Vec4 reflect(const Vec4& incomingVector, const Vec4& normal) {
                return incomingVector - 2 * Vec4::dot(incomingVector, normal) * normal;
            }
        
        public:
            explicit MetalMaterial(const Vec4& metalColor):
                    Material(metalColor) {}
            
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const override;
    };
    
    class BrushedMetalMaterial : public MetalMaterial {
        private:
            PRECISION_TYPE fuzzyness;
        public:
            explicit BrushedMetalMaterial(const Vec4& metalColor, PRECISION_TYPE fuzzyness):
                    MetalMaterial(metalColor), fuzzyness(fuzzyness) {}
            
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const override;
        
    };
    
    class TexturedMaterial : public Material {
        protected:
            int width{}, height{}, channels{};
            float scale = 1;
            unsigned char* data;
        public:
            explicit TexturedMaterial(const std::string& file);
            
            explicit TexturedMaterial(const std::string& file, float scale);
            
            [[nodiscard]] virtual ScatterResults scatter(const Ray& ray, const HitData& hitData) const override;
            
            [[nodiscard]] Vec4 getColor(PRECISION_TYPE u, PRECISION_TYPE v) const;
            
            ~TexturedMaterial();
    };
    
    struct WorldConfig {
        bool useBVH = true;
        bool padding[7]{};
#ifdef COMPILE_GUI
        Shader& worldShader;
        
        explicit WorldConfig(Shader& shader):
                worldShader(shader) {}

#endif
    };
    
    class World {
        private:
            // store all the objects in the world,
            std::vector<Object*> objects;
            std::unique_ptr<BVHTree> bvhObjects;
            std::unordered_map<std::string, Material*> materials;
            WorldConfig m_config;
        public:
            explicit World(WorldConfig config):
                    m_config(config) {};
            
            World(const World& world) = delete;
            
            World(const World&& world) = delete;
            
            // Called by the raytracer class after all objects have been added to the world
            // this allows us to generate a statically unchanging BVH for easy rendering
            void generateBVH();
            
            inline void add(Object* object) {
                objects.push_back(object);
#ifdef COMPILE_GUI
                // this will show up in the debug mode
                // disabled because we aren't using object local BVHs
                //if (object->getBVHTree().bvhTree != nullptr && !object->getBVHTree().isRegular)
                //    new DebugBVH{(TriangleBVHTree*) object->getBVHTree().bvhTree, m_config.worldShader};
#endif
            }
            
            inline void add(const std::string& materialName, Material* mat) { materials.insert({materialName, mat}); }
            
            inline Material* getMaterial(const std::string& materialName) { return materials.at(materialName); }
            
            [[nodiscard]] inline BVHTree* getBVH() { return bvhObjects.get(); }
            
            [[nodiscard]] inline std::vector<Object*> getObjectsInWorld() { return objects; }
            
            /**
             * goes through the entire world using the BVH to determine if the ray has hit anything
             * @param ray ray to check
             * @param min min of the ray
             * @param max max of the ray
             * @return HitData about the closest object that was hit.
             */
            [[nodiscard]] virtual std::pair<HitData, Object*> checkIfHit(const Ray& ray, PRECISION_TYPE min, PRECISION_TYPE max) const;
            
            ~World();
        
    };
}

#endif //STEP_2_WORLD_H
