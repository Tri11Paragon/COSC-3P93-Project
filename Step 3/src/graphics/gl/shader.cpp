/*
 * Created by Brett Terpstra 6920201 on 23/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 * TODO: rewrite this
 */
#include "graphics/gl/shader.h"
#include <fstream>
#include <ios>

namespace Raytracing {
    Shader::Shader(const std::string& vertex, const std::string& fragment, bool loadString) {
        if (loadString){
            vertexShaderID = loadShaderString(vertex, GL_VERTEX_SHADER);
            checkCompileErrors(vertexShaderID, "VERTEX", vertex);
            fragmentShaderID = loadShaderString(fragment, GL_FRAGMENT_SHADER);
            checkCompileErrors(fragmentShaderID, "FRAGMENT", fragment);
        } else {
            vertexShaderID = loadShader(vertex, GL_VERTEX_SHADER);
            checkCompileErrors(vertexShaderID, "VERTEX", vertex);
            fragmentShaderID = loadShader(fragment, GL_FRAGMENT_SHADER);
            checkCompileErrors(fragmentShaderID, "FRAGMENT", fragment);
        }
        programID = glCreateProgram();
        // attach the loaded shaders to the Shader program
        glAttachShader(programID, vertexShaderID);
        checkCompileErrors(vertexShaderID, "VERTEX", vertex);
        glAttachShader(programID, fragmentShaderID);
        checkCompileErrors(fragmentShaderID, "FRAGMENT", fragment);
        // link and make sure that our program is valid.
        glLinkProgram(programID);
        checkCompileErrors(programID, "PROGRAM", vertex);
        glValidateProgram(programID);
        use();
        setUniformBlockLocation("Matrices", 1);
        setUniformBlockLocation("LightSpaceMatrices", 3);
        glUseProgram(0);
    }
    
    Shader::Shader(const std::string& vertex, const std::string& geometry, const std::string& fragment, bool loadString) {
        if (loadString){
            vertexShaderID = loadShaderString(vertex, GL_VERTEX_SHADER);
            checkCompileErrors(vertexShaderID, "VERTEX", vertex);
            geometryShaderID = loadShaderString(geometry, GL_GEOMETRY_SHADER);
            checkCompileErrors(geometryShaderID, "GEOMETRY", geometry);
            fragmentShaderID = loadShaderString(fragment, GL_FRAGMENT_SHADER);
            checkCompileErrors(fragmentShaderID, "FRAGMENT", fragment);
        } else {
            vertexShaderID = loadShader(vertex, GL_VERTEX_SHADER);
            checkCompileErrors(vertexShaderID, "VERTEX", vertex);
            geometryShaderID = loadShader(geometry, GL_GEOMETRY_SHADER);
            checkCompileErrors(geometryShaderID, "GEOMETRY", geometry);
            fragmentShaderID = loadShader(fragment, GL_FRAGMENT_SHADER);
            checkCompileErrors(fragmentShaderID, "FRAGMENT", fragment);
        }
        programID = glCreateProgram();
        // attach the loaded shaders to the Shader program
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, geometryShaderID);
        glAttachShader(programID, fragmentShaderID);
        // link and make sure that our program is valid.
        glLinkProgram(programID);
        checkCompileErrors(programID, "PROGRAM", vertex);
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
        // 1. retrieve the vertex/fragment source code from filePath
        std::string shaderSource;
        std::ifstream vShaderFile;
        if (!vShaderFile.good()){
            flog << "Shader file not found.\n";
            throw std::runtime_error("Shader file not found!");
        }
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            vShaderFile.open(file);
            std::stringstream shaderStream;
            // read file's buffer contents into streams
            shaderStream << vShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            // convert stream into std::string
            shaderSource = shaderStream.str();
        } catch(std::ifstream::failure& e) {
            flog << "Unable to read Shader file! " << file << "\n";
            return -1;
        }
        
        const char* shaderCode = shaderSource.c_str();
        // creates a Shader
        unsigned int shaderID = glCreateShader(type);
        // puts the loaded Shader code into the graphics card
        glShaderSource(shaderID, 1, &shaderCode, NULL);
        // Compile it
        glCompileShader(shaderID);
        // make sure there is no errors
        /*int status = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
        if (!status) {
            char* log;
            int length = 0;
            glGetShaderInfoLog(shaderID, 512, &length, log);
            flog << "Error long length: " << length << "\n";
            flog << (log) << "\n";
            flog << "Could not compile Shader! (Shader type: "
                                 << (type == GL_VERTEX_SHADER ? "vertex" : type == GL_GEOMETRY_SHADER ? "geometry" : "fragment") << ")\n";
            flog << "Shader File: " << file << "\n";
            return -1;
        }*/
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
    
    void Shader::checkCompileErrors(GLuint Shader, const std::string& type, const std::string& shaderPath) {
        GLint success;
        GLchar infoLog[2048];
        if(type != "PROGRAM") {
            glGetShaderiv(Shader, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(Shader, 2048, NULL, infoLog);
                std::cout << shaderPath << "\n";
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(Shader, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(Shader, 2048, NULL, infoLog);
                std::cout << shaderPath << "\n";
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
}