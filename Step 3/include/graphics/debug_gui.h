/*
 * Created by Brett Terpstra 6920201 on 30/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_DEBUG_GUI_H
#define STEP_3_DEBUG_GUI_H

#include <functional>
#include <memory>
#include <string>
#include <engine/math/bvh.h>

/**
 * This stuff can safely be ignored as it is purely for me to debug
 */
namespace Raytracing {
    class DebugUI {
        public:
            static void render(const std::function<void()>& generalTab);
            
            static void registerTab(const std::string& name, const std::function<void()>& tabFunc);
    };
    
    class DebugObject {
        public:
            virtual void render() = 0;
    };
    
    class DebugMenus {
        public:
            static void add(const std::shared_ptr<DebugObject>& object);
            
            static void remove(DebugObject* object);
            
            static void render();
    };
    
    class DebugBVH : public DebugObject {
        private:
            BVHTree* m_bvhTree = nullptr;
            TriangleBVHTree* m_triangleBVHTree = nullptr;
            Shader& m_shader;
        public:
            explicit DebugBVH(BVHTree* bvhTree, Shader& shader);
            
            explicit DebugBVH(TriangleBVHTree* bvhTree, Shader& shader);
            
            void render();
            
            ~DebugBVH();
    };
}

#endif //STEP_3_DEBUG_GUI_H
