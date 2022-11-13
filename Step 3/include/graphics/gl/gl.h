/*
 * Created by Brett Terpstra 6920201 on 26/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 * stores / handles GL related stuff like implementing functions and common classes
 */

#ifndef STEP_3_GL_H
#define STEP_3_GL_H

#include <engine/util/std.h>
#include <engine/types.h>
#include <engine/image/image.h>
#include <config.h>
#include <graphics/gl/shader.h>

#ifndef USE_GLFW
    #include <GLES3/gl32.h>
    #include <GLES3/gl3.h>
    #include <GL/gl.h>
    #include "graphics/gl/glext.h"

extern PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
extern PFNGLCREATEBUFFERSPROC glCreateBuffers;
extern PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
extern PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;

extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
extern PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
extern PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
#else
    
    #include <graphics/gl/glad/gl.h>

#endif


class Shapes {
    public:
        struct cubeVertexBuilder {
            std::vector<float> cubeVerticesRaw = {
                    -0.5f, -0.5f, -0.5f,
                    0.5f, -0.5f, -0.5f,
                    0.5f,  0.5f, -0.5f,
                    0.5f,  0.5f, -0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f, -0.5f,
                    
                    -0.5f, -0.5f,  0.5f,
                    0.5f, -0.5f,  0.5f,
                    0.5f,  0.5f,  0.5f,
                    0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                    
                    -0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f, -0.5f,
                    -0.5f, -0.5f, -0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f,  0.5f,  0.5f,
                    
                    0.5f,  0.5f,  0.5f,
                    0.5f,  0.5f, -0.5f,
                    0.5f, -0.5f, -0.5f,
                    0.5f, -0.5f, -0.5f,
                    0.5f, -0.5f,  0.5f,
                    0.5f,  0.5f,  0.5f,
                    
                    -0.5f, -0.5f, -0.5f,
                    0.5f, -0.5f, -0.5f,
                    0.5f, -0.5f,  0.5f,
                    0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f, -0.5f,
                    
                    -0.5f,  0.5f, -0.5f,
                    0.5f,  0.5f, -0.5f,
                    0.5f,  0.5f,  0.5f,
                    0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
            };
            std::vector<float> cubeUVs = {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
            
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
            
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
            
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
            
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 1.0f,
            
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 1.0f
            };
            static cubeVertexBuilder getCubeExtends(float xRadius, float yRadius, float zRadius) {
                cubeVertexBuilder builder {};
                // Can we use the transformation matrix? Yes.
                // Are we going to? No.
                // Why? No good reason. Perhaps a TODO:?
                builder.cubeVerticesRaw = {
                        -xRadius, -yRadius, -zRadius,
                        xRadius, -yRadius, -zRadius,
                        xRadius,  yRadius, -zRadius,
                        xRadius,  yRadius, -zRadius,
                        -xRadius,  yRadius, -zRadius,
                        -xRadius, -yRadius, -zRadius,
                        
                        -xRadius, -yRadius,  zRadius,
                        xRadius, -yRadius,  zRadius,
                        xRadius,  yRadius,  zRadius,
                        xRadius,  yRadius,  zRadius,
                        -xRadius,  yRadius,  zRadius,
                        -xRadius, -yRadius,  zRadius,
                        
                        -xRadius,  yRadius,  zRadius,
                        -xRadius,  yRadius, -zRadius,
                        -xRadius, -yRadius, -zRadius,
                        -xRadius, -yRadius, -zRadius,
                        -xRadius, -yRadius,  zRadius,
                        -xRadius,  yRadius,  zRadius,
                        
                        xRadius,  yRadius,  zRadius,
                        xRadius,  yRadius, -zRadius,
                        xRadius, -yRadius, -zRadius,
                        xRadius, -yRadius, -zRadius,
                        xRadius, -yRadius,  zRadius,
                        xRadius,  yRadius,  zRadius,
                        
                        -xRadius, -yRadius, -zRadius,
                        xRadius, -yRadius, -zRadius,
                        xRadius, -yRadius,  zRadius,
                        xRadius, -yRadius,  zRadius,
                        -xRadius, -yRadius,  zRadius,
                        -xRadius, -yRadius, -zRadius,
                        
                        -xRadius,  yRadius, -zRadius,
                        xRadius,  yRadius, -zRadius,
                        xRadius,  yRadius,  zRadius,
                        xRadius,  yRadius,  zRadius,
                        -xRadius,  yRadius,  zRadius,
                        -xRadius,  yRadius, -zRadius,
                };
                return builder;
            }
        };
};

// since we are doing everything with raytracing
// the purpose of these utility classes are purely for debug
// such as drawing bounding boxes around a BVH
class Texture {
    private:
        Texture(const Texture& that); // Disable Copy Constructor
        Texture& operator=(const Texture& that); // Disable Copy Assignment
    protected:
        unsigned int textureID;
        int width, height, channels;
        unsigned char* loadTexture(const std::string& path);
        
        Raytracing::Image* _image = nullptr;
        unsigned char* data;
    
    public:
        Texture(Texture&&) noexcept = delete; // Disable move constructor.
        Texture& operator=(Texture&&) noexcept = delete; // Disable Move Assignment
        Texture();
        explicit Texture(const std::string& path);
        explicit Texture(Raytracing::Image* image);
        ~Texture();
        void updateImage();
        void bind() const;
        void unbind();
        void enableGlTextures(int textureCount);
};

class VAO {
    private:
        unsigned int VaoID, instanceVBO;
        std::vector<unsigned int> VBOs;
        int drawCount = -1, currentTransforms = -1;
        // vertex data
        unsigned int storeData(int attrNumber, int coordSize, int stride, long offset, int length, const float* data);
        // element data (indices)
        unsigned int storeData(int length, const unsigned int* data);
        // instance data
        unsigned int createInstanceVBO(int count, int bytePerInstance);
        // used much in the same way that store data sets an attribute where the data is expected
        // except this sets based on the master instance vbo, telling the GPU where to use the data and when.
        void addInstancedAttribute(int attribute, int dataSize, int dataLengthBytes, int offset) const;
        // disable bad constructors
        // we can't just make copies of GPU objects like we can on the CPU. It's stupidly expensive.
        VAO() = default;
        VAO(const VAO& that); // Disable Copy Constructor
        VAO& operator=(const VAO& that); // Disable Copy Assignment
    public:
        VAO(VAO&&) noexcept = delete; // Disable move constructor.
        VAO& operator=(VAO&&) noexcept = delete; // Disable Move Assignment
        
        explicit VAO(const std::vector<Raytracing::Triangle>& triangles);
        VAO(const std::vector<float>& verts, const std::vector<float>& uvs, const std::vector<unsigned int>& indices);
        VAO(const std::vector<float>& verts, const std::vector<float>& uvs);
        
        void bind() const;
        void unbind();
        // draws as if it where a fullscreen quad (literally used for that)
        void draw() const;
        // draw as if it's a box that we need to bulk draw.
        void draw(Raytracing::Shader& shader, const std::vector<Raytracing::Vec4>& positions);
        ~VAO();
};

// should be called by the window class to get the function pointers.
void assignGLFunctionPointers();

#endif //STEP_3_GL_H
