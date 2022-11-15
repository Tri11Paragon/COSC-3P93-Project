/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
// Yes, globals are bad.
#include "engine/util/debug.h"
#include <config.h>
#ifdef COMPILE_GUI
    #include <graphics/gl/gl.h>
#endif

bool* haltExecution;
bool* pauseRaytracing;
bool* haltRaytracing;

namespace Raytracing {
    std::unordered_map<std::string, std::shared_ptr<profiler>> profiles;
    #ifdef COMPILE_GUI
        std::shared_ptr<VAO> aabbVAO = nullptr;
        int count = 0;
        int selected = 0;
    #endif
}