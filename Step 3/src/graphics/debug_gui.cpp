/*
 * Created by Brett Terpstra 6920201 on 30/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <graphics/debug_gui.h>
#include <engine/util/std.h>
#include <graphics/imgui/imgui.h>

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
}