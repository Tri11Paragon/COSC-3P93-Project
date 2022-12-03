/*
 * Created by Brett Terpstra 6920201 on 03/12/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 * contains the opencl raytracing code. The file name is a pun (I hope that was implied otherwise I guess I'm lame)
 */

#ifndef STEP_3_OPEN_RAY_TRACING_H
#define STEP_3_OPEN_RAY_TRACING_H


#include <engine/util/std.h>
#include <config.h>
#include <engine/image/image.h>
#include <engine/types.h>
#include <engine/world.h>
#include <engine/util/memory_util.h>

#ifdef COMPILE_OPENCL

#include <opencl/cl.h>

namespace Raytracing {
    
    class OpenClRaytracer {
        private:
            CLProgram* program;
            Image& image;
            size_t localWorks[2]{8, 8};
        public:
            OpenClRaytracer(const std::string& programLocation, Image& image, World& world);
            
            ~OpenClRaytracer();
            
            void storeObjects(const std::vector<Object*>& objects, size_t totalWorldBytes);
            
            void run();
    };
    
}
#endif

#endif //STEP_3_OPEN_RAY_TRACING_H
