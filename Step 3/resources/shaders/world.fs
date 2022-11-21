#version 330 core

out vec4 FragColor;
in vec2 outUv;
in vec3 outNormal;

uniform sampler2D tex;
uniform int useWhite;
uniform vec3 color;

const vec3 lightDir = vec3(1.0, 1.0, 1.0);

void main() {
    vec4 textureColor = texture(tex, outUv);
    //FragColor = vec4(textureColor.rgb, 1.0f);
    if (useWhite == 0)
        FragColor = vec4(vec3(1.0, 0.0f, 0.0f) * dot(lightDir, outNormal) * vec3(outUv, 1.0), 1.0f);
    else
        FragColor = vec4(color, 1.0f);
}