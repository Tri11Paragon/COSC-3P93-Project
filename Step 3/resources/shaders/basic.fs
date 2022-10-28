#version 330 core

out vec4 FragColor;

in vec2 outUv;

uniform sampler2D rayTexture;

void main() {
    vec4 textureColor = texture(rayTexture, outUv);

// textureColor.rgb
    FragColor = vec4(textureColor.rgb, 1.0f);
}