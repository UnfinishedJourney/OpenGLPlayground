#version 460 core

layout (triangles, fractional_even_spacing, cw) in;

in vec2 tcTexCoord[];
out vec3 teNormal;
out vec2 teTexCoord;

uniform sampler2D u_HeightMap;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    // Interpolate texture coordinates
    vec2 texCoord = tcTexCoord[0] * gl_TessCoord.x +
                    tcTexCoord[1] * gl_TessCoord.y +
                    tcTexCoord[2] * gl_TessCoord.z;

    // Calculate world position
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 position = p0 * gl_TessCoord.x +
                    p1 * gl_TessCoord.y +
                    p2 * gl_TessCoord.z;

    // Apply height from height map
    float height = texture(u_HeightMap, texCoord).r;
    position.y += height;

    // Calculate normal (for flat shading)
    vec3 edge1 = p1 - p0;
    vec3 edge2 = p2 - p0;
    teNormal = normalize(cross(edge1, edge2));

    teTexCoord = texCoord;

    // Transform to clip space
    gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);
}