/*
 * Created by Brett Terpstra 6920201 on 18/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 * this is a direct one-to-one copy of the profiler class used in my Game Engine
 * it functions very well especially when used in a GUI context,
 * so why reinvent the wheel right?
 * So to avoid any kind of self plagiarism, I fully credit the source which is here:
 * https://github.com/Tri11Paragon/Trapdoor-Engine/tree/dev/C%2B%2B%20Engine
 *
 * 2022-12-12 update:
 * the above is not 100% true anymore. The functions are very similar though.
 */

#ifndef STEP_2_DEBUG_H
#define STEP_2_DEBUG_H

#include "std.h"
#include <config.h>
#include <mutex>

#ifdef COMPILE_GUI
    
    #include <graphics/debug_gui.h>

#endif

namespace Raytracing {
    class profiler;
    
    extern std::unordered_map<std::string, std::shared_ptr<profiler>> profiles;
    
    class DebugTab {
        protected:
            std::string name;
        public:
            virtual void render() {}
            
            std::string getName() {
                return name;
            }
    };
    
    class profiler : public DebugTab {
        private:
            long _start = 0;
            long _end = 0;
            std::unordered_map<std::string, std::pair<long, long>> timings;
            std::mutex timerLock{};
        public:
            explicit profiler(std::string name);
            
            void start();
            
            void start(const std::string& name);
            
            static void start(const std::string& name, const std::string& tabName) {
                static std::mutex staticLock{};
                // since we are using threads here the change is the use of this mutex.
                std::scoped_lock lock(staticLock);
                if (profiles.contains(name)) {
                    auto p = profiles.at(name);
                    p->start(tabName);
                } else {
                    auto p = std::make_shared<profiler>(name);
                    profiles.insert(std::pair(name, p));
                    p->start(tabName);
                }
            }
            
            void end();
            
            void end(const std::string& name);
            
            static void end(const std::string& name, const std::string& tabName) {
                static std::mutex staticLock{};
                std::scoped_lock lock(staticLock);
                try {
                    profiles.at(name)->end(tabName);
                } catch (std::exception& e) {}
            }
            
            void print();
            
            static void print(const std::string& name) {
                static std::mutex staticLock{};
                std::scoped_lock lock(staticLock);
                try {
                    profiles.at(name)->print();
                } catch (std::exception& e) {}
            }
            
            void endAndPrint();
            
            static void endAndPrint(const std::string& name, const std::string& tabName) {
                profiler::end(name, tabName);
                profiler::print(name);
            }
            
            void render();
            
            static void render(int count) {
                for (const auto& p : profiles)
                    p.second->render();
            }
            
            ~profiler() = default;
    };
    
}

#endif //STEP_2_DEBUG_H
