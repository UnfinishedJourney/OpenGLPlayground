#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 flipbookPosition;
uniform float aspectRatio;

void main() {
    vUV = aUV;
    vUV -= 1.0;
    vUV = abs(vUV);
    //gl_Position = vec4(aPos + flipbookPosition, 0.0, 1.0);
    gl_Position = vec4(aPos.x / aspectRatio, aPos.y, 0.0, 1.0);
}