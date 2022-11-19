/*
 * Created by Brett Terpstra 6920201 on 30/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <graphics/debug_gui.h>
#include <engine/util/std.h>
#include <graphics/imgui/imgui.h>

#include <utility>

// this file is only included if the compile gui setting is enabled in CMake
namespace Raytracing {
    
    std::vector<std::pair<std::string, std::function<void()>>> tabs;
    
    void DebugUI::render(const std::function<void()>& generalTab) {
        if (ImGui::Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            if (ImGui::BeginTabBar("debugTabs")){
                // Always have the general tab for starting / stopping the ray tracer
                if (ImGui::BeginTabItem("General")) {
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                                ImGui::GetIO().Framerate);
                    generalTab();
                    ImGui::EndTabItem();
                }
    
                // add any extra tabs after
                for (const auto& tab: tabs) {
                    if (ImGui::BeginTabItem(tab.first.c_str())) {
                        tab.second();
                        ImGui::EndTabItem();
                    }
                }
    
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
    void DebugUI::registerTab(const std::string& name, const std::function<void()>& tabFunc) {
        tabs.emplace_back(name, tabFunc);
    }
    
    std::vector<std::shared_ptr<DebugObject>> objects;
    
    void DebugMenus::render() {
        for (const auto& obj : objects)
            obj->render();
    }
    void DebugMenus::add(const std::shared_ptr<DebugObject>& object) {
        objects.push_back(object);
    }
    void DebugMenus::remove(const std::shared_ptr<DebugObject>& object) {
        objects.erase(std::remove_if(objects.begin(), objects.end(), [&](const auto& item) -> bool {
            return item.get() == object.get();
        }), objects.end());
    }
    DebugBVH::DebugBVH(BVHTree* bvhTree, Shader& shader): m_bvhTree(bvhTree), m_shader(shader) {
        DebugMenus::add(std::shared_ptr<DebugObject>(this));
    }
    DebugBVH::DebugBVH(TriangleBVHTree* bvhTree, Shader& shader): m_triangleBVHTree(bvhTree), m_shader(shader) {
        DebugMenus::add(std::shared_ptr<DebugObject>(this));
    }
    
    static Raytracing::Mat4x4 getTransform(const AABB& _aabb) {
        Raytracing::Mat4x4 transform{};
        auto center = _aabb.getCenter();
        transform.translate(center);
        auto xRadius = _aabb.getXRadius(center) * 2;
        auto yRadius = _aabb.getYRadius(center) * 2;
        auto zRadius = _aabb.getZRadius(center) * 2;
        transform.scale(float(xRadius), float(yRadius), float(zRadius));
        return transform;
    }
    
    void draw(Shader& worldShader, BVHNode* node) {
        worldShader.setVec3("color", {1.0, 1.0, 1.0});
        aabbVAO->bind();
        if (selected == node->index) {
            if (selected == node->index && ImGui::BeginListBox("", ImVec2(250, 350))) {
                std::stringstream strs;
                strs << node->aabb;
                ImGui::Text("%s", strs.str().c_str());
                for (const auto& item: node->objs) {
                    auto pos = item.ptr->getPosition();
                    std::stringstream stm;
                    stm << item.aabb;
                    ImGui::Text("%s,\n\t%s", (std::to_string(pos.x()) + " " + std::to_string(pos.y()) + " " + std::to_string(pos.z())).c_str(),
                                stm.str().c_str());
                }
                ImGui::EndListBox();
            }

            for (const auto& obj: node->objs) {
                auto transform = getTransform(obj.aabb);
                worldShader.setMatrix("transform", transform);
                aabbVAO->draw(worldShader);
            }
            auto transform = getTransform(node->aabb);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);

            /*auto splitAABBs = aabb.splitByLongestAxis();
            transform = getTransform(splitAABBs.second);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);
            transform = getTransform(splitAABBs.first);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);*/
        }
        if (node->hit){
            if (node->hit == 1)
                worldShader.setVec3("color", {0.0, 0.0, 1.0});
            else if (node->hit == 2)
                worldShader.setVec3("color", {0.0, 1.0, 0.0});
            else
                worldShader.setVec3("color", {1.0, 0.5, 0.5});
            auto transform = getTransform(node->aabb);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);
        }
    }
    void gui(BVHNode* node) {
        
        int c1 = -1;
        int c2 = -1;
        if (node->left != nullptr)
            c1 = node->left->index;
        if (node->right != nullptr)
            c2 = node->right->index;
        std::string t;
        if (c1 == -1 && c2 == -1)
            t = " LEAF";
        else
            t = " L: " + std::to_string(c1) + " R: " + std::to_string(c2);
        if (ImGui::Selectable(("S: " + std::to_string(node->objs.size()) + " I: " + std::to_string(node->index) + t).c_str(), selected == node->index))
            selected = node->index;
    }
    
    void drawNodesRecur(Shader& worldShader, BVHNode* node) {
        draw(worldShader, node);
        if (node->left != nullptr)
            drawNodesRecur(worldShader, node->left);
        if (node->right != nullptr)
            drawNodesRecur(worldShader, node->right);
    }
    void guiNodesRecur(BVHNode* node) {
        gui(node);
        if (node->left != nullptr)
            guiNodesRecur(node->left);
        if (node->right != nullptr)
            guiNodesRecur(node->right);
    }
    
    void draw(Shader& worldShader, TriangleBVHNode* node) {
        worldShader.setVec3("color", {1.0, 1.0, 1.0});
        aabbVAO->bind();
        if (selected == node->index) {
            if (selected == node->index && ImGui::BeginListBox("", ImVec2(250, 350))) {
                std::stringstream strs;
                strs << node->aabb;
                ImGui::Text("%s", strs.str().c_str());
                for (const auto& item: node->objs) {
                    auto pos = item.position;
                    std::stringstream stm;
                    stm << item.aabb;
                    ImGui::Text("%s,\n\t%s", (std::to_string(pos.x()) + " " + std::to_string(pos.y()) + " " + std::to_string(pos.z())).c_str(),
                                stm.str().c_str());
                }
                ImGui::EndListBox();
            }
            
            for (const auto& obj: node->objs) {
                auto transform = getTransform(obj.aabb);
                worldShader.setMatrix("transform", transform);
                aabbVAO->draw(worldShader);
            }
            auto transform = getTransform(node->aabb);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);
            
            /*auto splitAABBs = aabb.splitByLongestAxis();
            transform = getTransform(splitAABBs.second);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);
            transform = getTransform(splitAABBs.first);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);*/
        }
        if (node->hit){
            if (node->hit == 1)
                worldShader.setVec3("color", {0.0, 0.0, 1.0});
            else if (node->hit == 2)
                worldShader.setVec3("color", {0.0, 1.0, 0.0});
            else
                worldShader.setVec3("color", {1.0, 0.5, 0.5});
            auto transform = getTransform(node->aabb);
            worldShader.setMatrix("transform", transform);
            aabbVAO->draw(worldShader);
        }
    }
    void gui(TriangleBVHNode* node) {
        
        int c1 = -1;
        int c2 = -1;
        if (node->left != nullptr)
            c1 = node->left->index;
        if (node->right != nullptr)
            c2 = node->right->index;
        std::string t;
        if (c1 == -1 && c2 == -1)
            t = " LEAF";
        else
            t = " L: " + std::to_string(c1) + " R: " + std::to_string(c2);
        if (ImGui::Selectable(("S: " + std::to_string(node->objs.size()) + " I: " + std::to_string(node->index) + t).c_str(), selected == node->index))
            selected = node->index;
    }
    
    void drawNodesRecur(Shader& worldShader, TriangleBVHNode* node) {
        draw(worldShader, node);
        if (node->left != nullptr)
            drawNodesRecur(worldShader, node->left);
        if (node->right != nullptr)
            drawNodesRecur(worldShader, node->right);
    }
    void guiNodesRecur(TriangleBVHNode* node) {
        gui(node);
        if (node->left != nullptr)
            guiNodesRecur(node->left);
        if (node->right != nullptr)
            guiNodesRecur(node->right);
    }
    
    void DebugBVH::render() {
        if (m_bvhTree != nullptr) {
            ImGui::Begin(("BVH Data "), nullptr, ImGuiWindowFlags_NoCollapse);
            m_shader.use();
            m_shader.setInt("useWhite", 1);
            m_shader.setVec3("color", {1.0, 1.0, 1.0});
            {
                ImGui::BeginChild("left pane", ImVec2(180, 0), true);
                guiNodesRecur(m_bvhTree->getRoot());
                ImGui::EndChild();
            }
            ImGui::SameLine();
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("item view",
                                  ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                                  true,
                                  ImGuiWindowFlags_AlwaysAutoResize); // Leave room for 1 line below us
                drawNodesRecur(m_shader, m_bvhTree->getRoot());
                ImGui::EndChild();
                ImGui::EndGroup();
            }
            m_shader.setInt("useWhite", 0);
            ImGui::End();
        }
        if (m_triangleBVHTree) {
            ImGui::Begin(("TBVH Data "), nullptr, ImGuiWindowFlags_NoCollapse);
            m_shader.use();
            m_shader.setInt("useWhite", 1);
            m_shader.setVec3("color", {1.0, 1.0, 1.0});
            {
                ImGui::BeginChild("left pane", ImVec2(180, 0), true);
                guiNodesRecur(m_triangleBVHTree->getRoot());
                ImGui::EndChild();
            }
            ImGui::SameLine();
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("item view",
                                  ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                                  true,
                                  ImGuiWindowFlags_AlwaysAutoResize); // Leave room for 1 line below us
                drawNodesRecur(m_shader, m_triangleBVHTree->getRoot());
                ImGui::EndChild();
                ImGui::EndGroup();
            }
            m_shader.setInt("useWhite", 0);
            ImGui::End();
        }
    }
    DebugBVH::~DebugBVH() {
        DebugMenus::remove(std::shared_ptr<DebugObject>(this));
    }
}