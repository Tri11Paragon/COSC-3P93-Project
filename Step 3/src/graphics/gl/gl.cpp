/*
 * Created by Brett Terpstra 6920201 on 26/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#include <graphics/gl/gl.h>
#include <GL/glx.h>
#include "engine/image/stb_image.h"

#ifndef USE_GLFW
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;

PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;

void assignGLFunctionPointers() {
    
    glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC) glXGetProcAddress((unsigned char*) "glCreateVertexArrays");
    glCreateBuffers = (PFNGLCREATEBUFFERSPROC) glXGetProcAddress((unsigned char*) "glCreateBuffers");
    glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC) glXGetProcAddress((unsigned char*) "glNamedBufferData");
    glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC) glXGetProcAddress((unsigned char*) "glNamedBufferSubData");
    
    glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) glXGetProcAddress((unsigned char*) "glEnableVertexArrayAttrib");
    glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC) glXGetProcAddress((unsigned char*) "glVertexArrayAttribBinding");
    glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC) glXGetProcAddress((unsigned char*) "glVertexArrayAttribFormat");

}
#endif

unsigned int createVAO() {
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    return vaoID;
}

unsigned int VAO::storeData(int attrNumber, int coordSize, int stride, long offset, int length, const float* data) {
    unsigned int vboID;
    glGenBuffers(1, &vboID);
    
    VBOs.push_back(vboID);
    
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    // 4 bytes / float
    glBufferData(GL_ARRAY_BUFFER, length << 2, data, GL_STATIC_DRAW);
    glVertexAttribPointer(attrNumber,coordSize,GL_FLOAT,false,stride, (void *) offset);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return vboID;
}
unsigned int VAO::storeData(int length, const unsigned int *data) {
    unsigned int eboID;
    glGenBuffers(1, &eboID);
    
    VBOs.push_back(eboID);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    // 4 bytes / int
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length << 2, data, GL_STATIC_DRAW);
    
    return eboID;
}
unsigned int VAO::createInstanceVBO(int count, int bytePerInstance) {
    if (currentTransforms < 0) {
        unsigned int vboID;
        glGenBuffers(1, &vboID);
    
        instanceVBO = vboID;
    
        currentTransforms = count;
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glBufferData(GL_ARRAY_BUFFER, (long) count * bytePerInstance, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    
        return vboID;
    }
    if (currentTransforms < count){
        // double as to be more efficient. (prevents constant allocations when adding data per frame)
        // n -> log n
        currentTransforms = count * 2;
        // update the vertex buffer on the GPU if our count is out of sync.
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, (long) currentTransforms * bytePerInstance, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    // otherwise do nothing.
    return instanceVBO;
}
void VAO::addInstancedAttribute(int attribute, int dataSize, int dataLengthBytes, int offset) const {
    // bind buffer like normal
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBindVertexArray(VaoID);
    glEnableVertexAttribArray(attribute);
    // tell opengl how large this attribute is expected to be
    glVertexAttribPointer(attribute, dataSize, GL_FLOAT, GL_FALSE, dataLengthBytes, (void *) (long)offset);
    // how many per instance. 1 mean every triangle gets one copy of the data specified by ^
    glVertexAttribDivisor(attribute, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
VAO::VAO(const std::vector<float>& verts, const std::vector<float>& uvs, const std::vector<unsigned int>& indices): VaoID(createVAO()) {
    this->drawCount = (int)indices.size();
    glBindVertexArray(VaoID);
    // enable the attributes, prevents us from having to do this later.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // store index data
    storeData((int)indices.size(), indices.data());
    // store vertex data
    storeData(0, 3, 3 * sizeof(float), 0, (int)verts.size(), verts.data());
    // store texture UV data
    storeData(1, 2, 2 * sizeof(float), 0, (int)uvs.size(), uvs.data());
    unbind();
}
VAO::VAO(const std::vector<Raytracing::Triangle>& triangles): VaoID(createVAO()) {
    this->drawCount = (int)triangles.size() * 3;
    glBindVertexArray(VaoID);
    // enable the attributes, prevents us from having to do this later.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // convert vertex data
    std::vector<float> verts;
    std::vector<float> uvs;
    std::vector<float> normals;
    for (const Raytracing::Triangle& t : triangles){
        verts.push_back(float(t.vertex1.x()));
        verts.push_back(float(t.vertex1.y()));
        verts.push_back(float(t.vertex1.z()));
        
        uvs.push_back(float(t.uv1.x()));
        uvs.push_back(float(t.uv1.y()));
        
        normals.push_back(float(t.normal1.x()));
        normals.push_back(float(t.normal1.y()));
        normals.push_back(float(t.normal1.z()));
    
        verts.push_back(float(t.vertex2.x()));
        verts.push_back(float(t.vertex2.y()));
        verts.push_back(float(t.vertex2.z()));
    
        uvs.push_back(float(t.uv2.x()));
        uvs.push_back(float(t.uv2.y()));
    
        normals.push_back(float(t.normal2.x()));
        normals.push_back(float(t.normal2.y()));
        normals.push_back(float(t.normal2.z()));
    
        verts.push_back(float(t.vertex3.x()));
        verts.push_back(float(t.vertex3.y()));
        verts.push_back(float(t.vertex3.z()));
    
        uvs.push_back(float(t.uv3.x()));
        uvs.push_back(float(t.uv3.y()));
    
        normals.push_back(float(t.normal3.x()));
        normals.push_back(float(t.normal3.y()));
        normals.push_back(float(t.normal3.z()));
    }
    // store vertex data
    storeData(0, 3, 3 * sizeof(float), 0, (int)verts.size(), verts.data());
    // store texture UV data
    storeData(1, 2, 2 * sizeof(float), 0, (int)uvs.size(), uvs.data());
    // store normal data
    storeData(2, 3, 3 * sizeof(float), 0, (int)normals.size(), normals.data());
    // create an instance buffer with a large number of positions (stored in matrices)
    // this way we don't have to expand it later, since I don't think I'll use 1k
//    createInstanceVBO(1000, sizeof(Raytracing::Mat4x4));
//    // send the positions as attributes
//    addInstancedAttribute(3, 4, sizeof(Raytracing::Mat4x4), 0);
//    addInstancedAttribute(4, 4, sizeof(Raytracing::Mat4x4), 16);
//    addInstancedAttribute(5, 4, sizeof(Raytracing::Mat4x4), 32);
//    addInstancedAttribute(6, 4, sizeof(Raytracing::Mat4x4), 48);
    unbind();
}
VAO::VAO(const std::vector<std::shared_ptr<Raytracing::Triangle>>& triangles): VaoID(createVAO()) {
    this->drawCount = (int)triangles.size() * 3;
    glBindVertexArray(VaoID);
    // enable the attributes, prevents us from having to do this later.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // convert vertex data
    std::vector<float> verts;
    std::vector<float> uvs;
    std::vector<float> normals;
    for (const auto& t : triangles){
        verts.push_back(float(t->vertex1.x()));
        verts.push_back(float(t->vertex1.y()));
        verts.push_back(float(t->vertex1.z()));
        
        uvs.push_back(float(t->uv1.x()));
        uvs.push_back(float(t->uv1.y()));
        
        normals.push_back(float(t->normal1.x()));
        normals.push_back(float(t->normal1.y()));
        normals.push_back(float(t->normal1.z()));
        
        verts.push_back(float(t->vertex2.x()));
        verts.push_back(float(t->vertex2.y()));
        verts.push_back(float(t->vertex2.z()));
        
        uvs.push_back(float(t->uv2.x()));
        uvs.push_back(float(t->uv2.y()));
        
        normals.push_back(float(t->normal2.x()));
        normals.push_back(float(t->normal2.y()));
        normals.push_back(float(t->normal2.z()));
        
        verts.push_back(float(t->vertex3.x()));
        verts.push_back(float(t->vertex3.y()));
        verts.push_back(float(t->vertex3.z()));
        
        uvs.push_back(float(t->uv3.x()));
        uvs.push_back(float(t->uv3.y()));
        
        normals.push_back(float(t->normal3.x()));
        normals.push_back(float(t->normal3.y()));
        normals.push_back(float(t->normal3.z()));
    }
    // store vertex data
    storeData(0, 3, 3 * sizeof(float), 0, (int)verts.size(), verts.data());
    // store texture UV data
    storeData(1, 2, 2 * sizeof(float), 0, (int)uvs.size(), uvs.data());
    // store normal data
    storeData(2, 3, 3 * sizeof(float), 0, (int)normals.size(), normals.data());
    unbind();
}
void VAO::bind() const {
    glBindVertexArray(VaoID);
}
void VAO::unbind() {
    glBindVertexArray(0);
    VaoID;
}
void VAO::draw(Raytracing::Shader& shader, const std::vector<Raytracing::Vec4>& positions) const {
    // this should only update if we are drawing with more positions than we already have allocated.
//    createInstanceVBO((int)positions.size(), sizeof(Raytracing::Mat4x4));
//    std::vector<Raytracing::Mat4x4> data;
//    for (const Raytracing::Vec4& v : positions){
//        // identity matrix
//        Raytracing::Mat4x4 mat4X4 {};
//        // translate to position
//        mat4X4 = mat4X4.translate(v);
//        data.push_back(mat4X4);
//    }
    // send translated matrices as data to the GPU, TODO: since this will likely be used on static geometry, try making this more static!
//    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
//    //glBufferData(GL_ARRAY_BUFFER, datas.size() * sizeof(LightData), NULL, GL_STREAM_DRAW);
//    glBufferSubData(GL_ARRAY_BUFFER, 0, (long)(data.size() * sizeof(Raytracing::Mat4x4)), &data[0]);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    // then finally draw
    //glDrawArraysInstanced(GL_TRIANGLES, 0, drawCount * 3, (int)positions.size());
    for (const auto& v : positions) {
        Raytracing::Mat4x4 transform {};
        transform.translate(v);
        shader.setMatrix("transform", transform);
        glDrawArrays(GL_TRIANGLES, 0, drawCount);
    }
}
void VAO::draw(Raytracing::Shader& shader) {
    glDrawArrays(GL_TRIANGLES, 0, drawCount);
}
void VAO::draw() const {
    if (drawCount < 0)
        return;
    else
        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);
}
VAO::~VAO() {
    const unsigned int vao = VaoID;
    glDeleteVertexArrays(1, &vao);
    for (const unsigned int vbo : VBOs){
        glDeleteBuffers(1, &vbo);
    }
    glDeleteBuffers(1, &instanceVBO);
}
VAO::VAO(const std::vector<float>& verts, const std::vector<float>& uvs): VaoID(createVAO()) {
    this->drawCount = (int)verts.size();
    glBindVertexArray(VaoID);
    // enable the attributes, prevents us from having to do this later.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // store vertex data
    storeData(0, 3, 3 * sizeof(float), 0, (int)verts.size(), verts.data());
    // store texture UV data
    storeData(1, 2, 2 * sizeof(float), 0, (int)uvs.size(), uvs.data());
    // store normal data
    //storeData(2, 3, 3 * sizeof(float), 0, (int)normals.size(), normals.data());
    unbind();
}

Texture::Texture(){

}

Texture::Texture(const std::string& path) {
    data = loadTexture(path);
    if (data == nullptr){
        flog << "There was an error loading the image file " << path;
        throw std::runtime_error("Error loading image from file!");
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int GL_RGB_SETTING = channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB_SETTING, width, height, 0, GL_RGB_SETTING, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(data);
    data = nullptr;
}

Texture::Texture(Raytracing::Image* image): _image(image), width(image->getWidth()), height(image->getHeight()), channels(3) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);
    //glGenerateMipmap(GL_TEXTURE_2D);
    
    // no sense sending data now since this texture is likely all black.
}

unsigned char *Texture::loadTexture(const std::string& path) {
    // TODO: add more image processing options
    stbi_set_flip_vertically_on_load(true);
    unsigned char *dta = stbi_load(path.c_str(), &width, &height, &channels, 0);
    //if (stbi__g_failure_reason) {
    //    flog << "STB Error Reason: ";
    //    flog << stbi__g_failure_reason;
    //}
    return dta;
}
Texture::~Texture() {
    tlog << "Deleting Texture {" << textureID << "}\n";
    glDeleteTextures(1, &textureID);
    data = nullptr;
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    textureID;
}

void Texture::enableGlTextures(int textureCount) {
    for (int i = 0; i < textureCount; i++)
        glActiveTexture(GL_TEXTURE0 + i);
    textureID;
}
void Texture::updateImage() {
    glBindTexture(GL_TEXTURE_2D, textureID);
    // unfortunately we do have to put the data into a format that OpenGL can read. This is a TODO:?
    data = new unsigned char[(unsigned long)(width) * (unsigned long)height * 3];
    int pixelIndex = 0;
    // slightly different order from STBi
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            const auto V = Raytracing::Vec4(255.0, 255.0, 255.0, 255.0);
            auto pVec = _image->getPixelColor(i, j) * V;
            data[pixelIndex++] = static_cast<unsigned char>(pVec.r());
            data[pixelIndex++] = static_cast<unsigned char>(pVec.g());
            data[pixelIndex++] = static_cast<unsigned char>(pVec.z());
        }
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[](data);
    data = nullptr;
}
