/*
 * Created by Brett Terpstra 6920201 on 18/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <util/debug.h>
#include <chrono>

namespace Raytracing {
    profiler::profiler(std::string name) {
        this->name = name;
        // currently unused as we don't have a UI yet.
        //TD::debugUI::addTab(this);
    }
    
    void profiler::start() {
        start("Unnamed");
    }
    void profiler::start(const std::string& name) {
        auto p1 = std::chrono::high_resolution_clock::now();
        _start = std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count();
        timings[name] = std::pair<long, long>(_start, 0);
    }
    
    void profiler::end() {
        end("Unnamed");
    }
    void profiler::end(const std::string& name) {
        auto p1 = std::chrono::high_resolution_clock::now();
        _end = std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count();
        timings[name] = std::pair<long, long>(timings[name].first, _end);
    }
    
    void profiler::print() {
        ilog << "Profiler " << name << " recorded: \n";
        for (std::pair<std::string, std::pair<long, long>> e : timings){
            ilog << "\t" << e.first << " took " << ((double)(e.second.second - e.second.first) / 1000000.0) << "ms to run!\n";
        }
        
    }
    
    void profiler::endAndPrint() {
        end();
        print();
    }
    
    void profiler::render() {
        // currently unused as we don't have a UI yet.
        /*ImGui::Text("CPU Timings:");
        ImGui::Indent();
        for (std::pair<std::string, std::pair<long, long>> e : timings) {
            ImGui::Text("Elapsed Time(%s):  %fms", e.first.c_str(), (double) ((e.second.second - e.second.first) / 1000000.0));
        }
        ImGui::Unindent();
        ImGui::NewLine();*/
    }
    
    profiler::~profiler() {
        // currently unused as we don't have a UI yet.
        //TD::debugUI::deleteTab(this);
    }
}
