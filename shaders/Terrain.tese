#version 460 core

layout(triangles, equal_spacing, cw) in; // Input primitive type

void main()
{
    // Barycentric coordinates
    vec3 barycentric = gl_TessCoord;

    // Interpolate the vertex positions
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    // Compute the new position
    vec4 pos = barycentric.x * p0 + barycentric.y * p1 + barycentric.z * p2;

    gl_Position = pos;
}