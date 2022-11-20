/*
 * Created by Brett Terpstra 6920201 on 23/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 * TODO: rewrite this
 */
#include "graphics/gl/shader.h"
#include <fstream>
#include <ios>
#include <engine/util/loaders.h>

namespace Raytracing {
    
    void checkLinkerErrors(GLuint programID, const std::string& vertex, const std::string& fragment, const std::string& geometry){
        GLint success;
        GLchar infoLog[2048];
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(programID, 2048, NULL, infoLog);
            flog << "Unable to link program! " << vertex << " " << fragment << " " << geometry << "\n";
            flog << infoLog << "\n";
            return;
        }
    }
    
    Shader::Shader(const std::string& vertex, const std::string& fragment, bool loadString) {
        if (loadString){
            vertexShaderID = loadShaderString(vertex, GL_VERTEX_SHADER);
            fragmentShaderID = loadShaderString(fragment, GL_FRAGMENT_SHADER);
        } else {
            vertexShaderID = loadShader(vertex, GL_VERTEX_SHADER);
            fragmentShaderID = loadShader(fragment, GL_FRAGMENT_SHADER);
        }
        programID = glCreateProgram();
        // attach the loaded shaders to the Shader program
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        // link and make sure that our program is valid.
        glLinkProgram(programID);
        checkLinkerErrors(programID, vertex, fragment, "");
        glValidateProgram(programID);
        use();
        setUniformBlockLocation("Matrices", 1);
        setUniformBlockLocation("LightSpaceMatrices", 3);
        glUseProgram(0);
    }
    
    Shader::Shader(const std::string& vertex, const std::string& geometry, const std::string& fragment, bool loadString) {
        if (loadString){
            vertexShaderID = loadShaderString(vertex, GL_VERTEX_SHADER);
            geometryShaderID = loadShaderString(geometry, GL_GEOMETRY_SHADER);
            fragmentShaderID = loadShaderString(fragment, GL_FRAGMENT_SHADER);
        } else {
            vertexShaderID = loadShader(vertex, GL_VERTEX_SHADER);
            geometryShaderID = loadShader(geometry, GL_GEOMETRY_SHADER);
            fragmentShaderID = loadShader(fragment, GL_FRAGMENT_SHADER);
        }
        programID = glCreateProgram();
        // attach the loaded shaders to the Shader program
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, geometryShaderID);
        glAttachShader(programID, fragmentShaderID);
        // link and make sure that our program is valid.
        glLinkProgram(programID);
        checkLinkerErrors(programID, vertex, fragment, geometry);
        glValidateProgram(programID);
        use();
        setUniformBlockLocation("Matrices", 1);
        setUniformBlockLocation("LightSpaceMatrices", 3);
        glUseProgram(0);
    }
    
    unsigned int Shader::loadShaderString(const std::string &str, int type) {
        const char* shaderCode = str.c_str();
        // creates a Shader
        unsigned int shaderID = glCreateShader(type);
        // puts the loaded Shader code into the graphics card
        glShaderSource(shaderID, 1, &shaderCode, NULL);
        // Compile it
        glCompileShader(shaderID);
        return shaderID;
    }
    
    unsigned int Shader::loadShader(const std::string &file, int type) {
        auto shaderSource = ShaderLoader::loadShaderFile(file);
        const char* shaderCode = shaderSource.c_str();
        // creates a Shader
        unsigned int shaderID = glCreateShader(type);
        // puts the loaded Shader code into the graphics card
        glShaderSource(shaderID, 1, &shaderCode, NULL);
        // Compile it
        glCompileShader(shaderID);
        // make sure there are no errors
        GLint success;
        GLchar infoLog[2048];
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(shaderID, 2048, NULL, infoLog);
            flog << "Unable to compile " << file << "\n";
            flog << "Of type " << (type == GL_VERTEX_SHADER ? "Vertex Shader" : type == GL_FRAGMENT_SHADER ? "Fragment Shader" : "Other Shader") << "\n";
            flog << infoLog << "\n";
        }
        return shaderID;
    }
    
    void Shader::use() {
        glUseProgram(programID);
    }
    
    void Shader::bindAttribute(int attribute, const std::string& name) {
        use();
        glBindAttribLocation(programID, attribute, name.c_str());
    }
    
    void Shader::setUniformBlockLocation(const std::string& name, int location) {
        use();
        glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, name.c_str()), location);
    }
    
    GLint Shader::getUniformLocation(const std::string &name) {
        if (uniformVars[name].i != -1)
            return uniformVars[name].i;
        unsigned int loc = glGetUniformLocation(programID, name.c_str());
        uniformVars[name].i = loc;
        return loc;
    }
    
    Shader::~Shader() {
        glUseProgram(0);
        // remove all the shaders from the program
        glDetachShader(programID, vertexShaderID);
        if (geometryShaderID)
            glDetachShader(programID, geometryShaderID);
        if (tessalationShaderID)
            glDetachShader(programID, tessalationShaderID);
        glDetachShader(programID, fragmentShaderID);
        
        // delete the shaders
        glDeleteShader(vertexShaderID);
        if (geometryShaderID)
            glDeleteShader(geometryShaderID);
        if (tessalationShaderID)
            glDeleteShader(tessalationShaderID);
        glDeleteShader(fragmentShaderID);
        
        // delete the Shader program
        glDeleteProgram(programID);
    }
    
    void Shader::setBool(const std::string &name, bool value) {
        glUniform1i(getUniformLocation(name), (int)value);
    }
    
    void Shader::setInt(const std::string &name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }
    
    void Shader::setFloat(const std::string &name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }
    
    void Shader::setMatrix(const std::string &name, Mat4x4& matrix) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix.ptr() );
    }

    void Shader::setVec3(const std::string &name, const Vec4& vec) {
        glUniform3f(getUniformLocation(name), vec.x(), vec.y(), vec.z());
    }
    
    void Shader::setVec4(const std::string &name, const Vec4& vec) {
        glUniform4f(getUniformLocation(name), vec.x(), vec.y(), vec.z(), vec.w());
    }
    
    void Shader::setVec2(const std::string &name, float x, float y) {
        glUniform2f(getUniformLocation(name), x, y);
    }
    
    void Shader::setVec3(const std::string &name, float x, float y, float z) {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    
    void Shader::setVec4(const std::string &name, float x, float y, float z, float w) {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }
}