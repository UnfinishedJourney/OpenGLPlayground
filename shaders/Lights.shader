#shader vertex
#version 410 core

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 
 
out vec3 Color_lights; 

uniform mat4 u_MVP;
uniform mat4 u_ModelView;
uniform mat3 u_NormalMatrix;

uniform struct MaterialInfo {
  float Ks;
  float Kd;
  float Shininess;
} Material;

uniform struct LightInfo {
  vec4 Position; // Light position in eye coords.
  vec3 Color;        // light color
} lights[4];


vec3 phongModel( int light, vec3 position, vec3 n ) { 
  vec3 s = normalize( lights[light].Position.xyz - position );
  float sDotN = max( dot(s,n), 0.0 );
  float diffuse = Material.Kd * sDotN;
  float spec = 0.0;
  if( sDotN > 0.0 ) {
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect( -s, n );
    spec = Material.Ks *
            pow( max( dot(r,v), 0.0 ), Material.Shininess );
  }

  return lights[light].Color * (diffuse + spec);
}

void main() {
  vec3 camNorm = normalize( u_NormalMatrix * normals);
  vec3 camPosition = 
       (u_ModelView * vec4(positions,1.0)).xyz;

  // Evaluate the lighting equation, for each light
  Color_lights = vec3(0.0);
  for( int i = 0; i < 4; i++ )
      Color_lights += phongModel( i, camPosition, camNorm );

  gl_Position = u_MVP * vec4(positions,1.0);
}

#shader fragment
#version 410 core

layout(location = 0) out vec4 color;
in vec3 Color_lights;

void main()
{
  color.xyz = Color_lights;
  color.w = 1.0;
}
