#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;

out vec2 outUv;

void main() {
    gl_Position = vec4(inPos, 1.0);
    outUv = inUv;
}