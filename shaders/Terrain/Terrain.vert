#version 460 core

layout(location = 0) in vec3 aPos;

void main()
{
    //gl_Position = vec4(aPos, 1.0);
    gl_Position = vec4(aPos.x, aPos.z, aPos.y, 0.0);
}