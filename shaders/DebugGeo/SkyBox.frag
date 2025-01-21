#version 460 core

layout(location = 0) out vec4 color;
in vec3 Vec;
uniform samplerCube u_Texture;

void main() {
    vec3 texColor = texture(u_Texture, normalize(Vec)).rgb;
    color = vec4(texColor,1);
}
