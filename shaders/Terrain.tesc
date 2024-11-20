#version 460 core

layout (vertices = 3) out;

in vec2 vTexCoord[];
out vec2 tcTexCoord[];

uniform mat4 u_ModelViewMatrix;
uniform sampler2D u_HeightMap;

void main()
{
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    barrier();

    if (gl_InvocationID == 0)
    {
        // Calculate tessellation levels based on distance to the camera
        float distances[3];
        for (int i = 0; i < 3; ++i)
        {
            float height = texture(u_HeightMap, vTexCoord[i]).r;
            vec4 pos = u_ModelViewMatrix * (gl_in[i].gl_Position + vec4(0.0, height, 0.0, 0.0));
            distances[i] = length(pos.xyz);
        }

        float avgDistance = (distances[0] + distances[1] + distances[2]) / 3.0;
        float tessLevel = clamp(20.0 - avgDistance, 1.0, 20.0);

        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelInner[0] = tessLevel;
    }
}