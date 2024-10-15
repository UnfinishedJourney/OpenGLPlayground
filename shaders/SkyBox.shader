#shader vertex
#version 430 core

layout(location = 0) in vec3 position;
//layout(location = 1) in vec2 uvs;

out vec3 Vec;
uniform mat4 u_MVP;

void main() {
    Vec = position;
    gl_Position = u_MVP * vec4(50.0*position,1.0);
}

#shader fragment
#version 430 core

layout( location = 0 ) out vec4 FragColor;
in vec3 Vec;
uniform samplerCube u_Texture;

void main() {
    //FragColor = vec4(1.0, 1.0, 0.0, 1.0);
    //vec3 texColor = texture(u_Texture, vec3(0.5, 0.5, 0.5)).rgb;
    //FragColor.xyz = texColor;
    //return;
    vec3 texColor = texture(u_Texture, normalize(Vec)).rgb;
    texColor = pow( texColor, vec3(1.0/2.2));
    FragColor = vec4(texColor,1);
}