#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

out vec2 outUv;
out vec3 outNormal;

uniform mat4 transform;
uniform mat4 projectMatrix;
uniform mat4 ViewMatrix;

void main() {
    gl_Position = projectMatrix * ViewMatrix * transform * vec4(inPos, 1.0);
    outUv = inUv;
    outNormal = inNormal;
}