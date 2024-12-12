#version 460 core
in vec2 vUV;
out vec4 outColor;

layout(binding = 0) uniform sampler2DArray texFlipbookArray;
uniform int currentFrame;

void main() {
    outColor = texture(texFlipbookArray, vec3(vUV, currentFrame));
}