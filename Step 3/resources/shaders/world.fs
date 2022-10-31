#version 330 core

out vec4 FragColor;
in vec2 outUv;
in vec3 outNormal;

uniform sampler2D tex;

const vec3 lightDir = vec3(1.0, 1.0, 1.0);

void main() {
    vec4 textureColor = texture(tex, outUv);
    //FragColor = vec4(textureColor.rgb, 1.0f);
    FragColor = vec4(vec3(1.0, 0.0f, 0.0f) * dot(lightDir, outNormal), 1.0f);
}