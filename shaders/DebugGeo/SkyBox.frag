#version 460 core

layout(location = 0) out vec4 color;
in vec3 Vec;
layout(binding = 8) uniform samplerCube u_Texture;

void main() {
    vec3 texColor = texture(u_Texture, normalize(Vec)).rgb;
    color = vec4( pow( texColor, vec3(1.0/2.2) ), 1.0 );
    //color = vec4(texColor,1);
}
