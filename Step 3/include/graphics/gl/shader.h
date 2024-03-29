/*
 * Created by Brett Terpstra 6920201 on 23/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_SHADER_H
#define STEP_3_SHADER_H

#include "engine/util/std.h"
#include "engine/math/vectors.h"
#include <config.h>

#ifndef USE_GLFW
    #include <GL/gl.h>
    #include <GLES3/gl32.h>
    #include <GL/glx.h>
    #include <GL/glu.h>
#else
    
    #include <graphics/gl/glad/gl.h>

#endif


// opengl shader

namespace Raytracing {
    
    class Shader {
        private:
            struct IntDefaultedToMinusOne {
                GLint i = -1;
            };
            // we can have shaders of many types in OpenGL
            unsigned int programID = 0;
            // but we will only make use of these two for now
            unsigned int vertexShaderID = 0;
            unsigned int fragmentShaderID = 0;
            // while these will remain unused.
            unsigned int geometryShaderID = 0;
            unsigned int tessalationShaderID = 0;
            std::unordered_map<std::string, IntDefaultedToMinusOne> uniformVars;
            
            static unsigned int loadShader(const std::string& file, int type);
            
            // loads from a string rather than a file!
            static unsigned int loadShaderString(const std::string& str, int type);
            
            GLint getUniformLocation(const std::string& name);
        
        public:
            Shader(const std::string& vertex, const std::string& fragment, bool loadString = false);
            
            Shader(const std::string& vertex, const std::string& geometry, const std::string& fragment, bool loadString = false);
            
            // used to set the location of VAOs to the in variables in opengl shaders. (using layouts instead)
            void bindAttribute(int attribute, const std::string& name);
            
            // used to set location of shared UBOs (unused)
            void setUniformBlockLocation(const std::string& name, int location);
            
            // set various data-types.
            void setBool(const std::string& name, bool value);
            
            void setInt(const std::string& name, int value);
            
            void setFloat(const std::string& name, float value);
            
            void setMatrix(const std::string& name, Mat4x4& matrix);
            
            void setVec4(const std::string& name, const Vec4& vec);
            
            void setVec3(const std::string& name, const Vec4& vec);
            
            void setVec2(const std::string& name, float x, float y);
            
            void setVec3(const std::string& name, float x, float y, float z);
            
            void setVec4(const std::string& name, float x, float y, float z, float w);
            
            void use();
            
            ~Shader();
    };
    
}

#endif //STEP_3_SHADER_H
