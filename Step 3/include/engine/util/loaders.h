/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_LOADERS_H
#define STEP_3_LOADERS_H

#include <engine/util/std.h>

namespace Raytracing {
    
    class ShaderLoader {
        public:
            static void define(const std::string& key,  const std::string& replacement);
            static std::string loadShaderFile(const std::string& path);
    };
    
}

#endif //STEP_3_LOADERS_H
