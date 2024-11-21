#version 460 core

layout(vertices = 3) out; // Processing triangles

void main()
{
    // Pass through the vertex positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Set tessellation levels to a fixed value
    // You can adjust these values to see different levels of tessellation
    gl_TessLevelOuter[0] = 5.0;
    gl_TessLevelOuter[1] = 5.0;
    gl_TessLevelOuter[2] = 5.0;
    gl_TessLevelInner[0] = 5.0;

    // Only the first invocation sets the tessellation levels
    if (gl_InvocationID == 0)
    {
        // Tessellation levels are already set above
    }
}