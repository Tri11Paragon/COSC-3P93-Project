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
#include "engine/raytracing.h"

namespace Raytracing {
    
    /**
     * A semi-working implementation of an OpenCL raytracer. Two major issues pertain to its lack of completion:
     * 1. I ran out of time
     * 2. Doing the wrong thing in OpenCL seems to crash my computer with 0 warning and 0 error messages. Fun. Especially when restarting takes several minutes.
     */
    class OpenClRaytracer {
        private:
            CLProgram* program;
            Image& image;
            Camera& camera;
            // 16x16 can be used here but 8x8 works great on my GPU.
            size_t localWorks[2]{8, 8};
            size_t maxTriangleSize = 0;
            size_t objectCount = 0;
        public:
            OpenClRaytracer(const std::string& programLocation, Image& image, Camera& camera, World& world);
            
            ~OpenClRaytracer();
            
            /**
             * Stores the objects in the world inside the byte buffer provided.
             * @param buffer buffer to store objects into
             * @param totalWorldBytes total bytes between all the objects in the world.
             */
            void storeObjects(unsigned char* buffer, size_t totalWorldBytes);
            
            /**
             * Updates the camera vectors on the GPU
             */
            void updateCameraInformation();
            
            /**
             * stores the objects into a buffer which is loaded into the GPU.
             * @param objects objects in world
             * @param totalWorldBytes total bytes taken by the objects
             * @return the buffer containing the objects. Must be manually deleted.
             */
            unsigned char* createObjectBuffer(const std::vector<Object*>& objects, size_t totalWorldBytes);
            
            void run();
    };
    
}
#endif

#endif //STEP_3_OPEN_RAY_TRACING_H
