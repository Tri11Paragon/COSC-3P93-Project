/*
 * Created by Brett Terpstra 6920201 on 23/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_SHADER_H
#define STEP_3_SHADER_H

#include <engine/util/std.h>
#include <engine/math/vectors.h>
#include <GL/gl.h>
#include <GLES3/gl32.h>
#include <GL/glx.h>
#include <GL/glu.h>

// opengl shader

namespace Raytracing {
    
    class shader {
        private:
            struct IntDefaultedToMinusOne {
                unsigned int i = -1;
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
            static unsigned int loadShader(const std::string &file, int type);
            // loads from a string rather than a file!
            static unsigned int loadShaderString(const std::string &str, int type);
            unsigned int getUniformLocation(const std::string &name);
            static void checkCompileErrors(unsigned int shader, const std::string& type, const std::string& shaderPath);
        public:
            shader(const std::string& vertex, const std::string& fragment, bool loadString = false);
            shader(const std::string& vertex, const std::string& geometry, const std::string& fragment, bool loadString = false);
            // used to set the location of VAOs to the in variables in opengl shaders.
            void bindAttribute(int attribute, std::string name);
            // used to set location of shared UBOs
            void setUniformBlockLocation(std::string name, int location);
            // set various data-types.
            void setBool(const std::string &name, bool value);
            void setInt(const std::string &name, int value);
            void setFloat(const std::string &name, float value);
            void setMatrix(const std::string &name, glm::mat4x4 matrix);
            void setVec4(const std::string &name, Vec4 vec);
            void setVec2(const std::string &name, float x, float y);
            void setVec3(const std::string &name, float x, float y, float z);
            void setVec4(const std::string &name, float x, float y, float z, float w);
            void use();
            ~shader();
    };
    
}

#endif //STEP_3_SHADER_H
