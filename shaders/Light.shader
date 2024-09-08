#shader vertex
#version 410 core

layout(location = 0) in vec4 positions;
layout(location = 1) in vec4 normals;
layout(location = 2) in vec2 uvs;

out vec3 v_Normals;
out vec2 v_TexCoords;
out vec2 params;

uniform vec4 u_L_pos;
uniform float u_L_intensity;
uniform mat4 u_MVP;
uniform mat4 u_MV;
uniform mat4 u_Proj;

void main()
{
  vec3 lightDir = normalize(positions.xyz - u_L_pos.xyz);
  params.x = max(dot(normals.xyz, lightDir), 0.0);;
  params.y = u_L_intensity;
  gl_Position = u_MVP * positions;
  v_Normals = normals.xyz;
  v_TexCoords = uvs;
}

#shader fragment
#version 410 core

layout(location = 0) out vec4 color;
in vec3 v_Normals;
in vec2 v_TexCoords;
in vec2 params;

uniform sampler2D u_Texture;

void main()
{
  vec4 texColor = texture(u_Texture, v_TexCoords.xy);
  texColor.xyz *= params.x*params.y;
  color = texColor;
}