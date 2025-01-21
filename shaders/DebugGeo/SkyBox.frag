#version 460 core

layout(location = 0) out vec4 color;
in vec3 Vec;
uniform samplerCube u_Texture;

void main() {
    vec3 texColor = texture(u_Texture, normalize(Vec)).rgb;
    //texColor = pow( texColor, vec3(1.0/2.2)); //need to check gamma correction
    color = vec4(texColor,1);
}
