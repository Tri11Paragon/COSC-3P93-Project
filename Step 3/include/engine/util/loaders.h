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
            /**
             * Creates a define which will be replaced as the shader is loaded.
             * @param key name of the define to replace
             * @param replacement the string which the define will be set to.
             */
            static void define(const std::string& key, const std::string& replacement);
            
            /**
             * loads a line-terminated string from the file. Will preprocess defines and attempt to recursively load includes.
             * @param path path to file to load
             * @return a single string where each line is terminated with a \n
             */
            static std::string loadShaderFile(const std::string& path);
    };
    
}

#endif //STEP_3_LOADERS_H
