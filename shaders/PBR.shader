#shader vertex
#version 410 core

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 
 
out vec3 viewNorm; 
out vec3 viewPosition; 

uniform mat4 u_MVP;
uniform mat4 u_ModelView;
uniform mat3 u_NormalMatrix;


void main() {
  viewNorm = normalize( u_NormalMatrix * normals);
  viewPosition = 
       (u_ModelView * vec4(positions,1.0)).xyz;

  gl_Position = u_MVP * vec4(positions,1.0);
}

#shader fragment
#version 410 core

layout(location = 0) out vec4 color;
in vec3 viewNorm; 
in vec3 viewPosition; 


uniform struct MaterialInfo {
  float Ks;
  float Kd;
  float Shininess;
} Material;

uniform struct LightInfo {
  vec4 Position; 
  vec3 Color; 
} lights[4];

const float PI = 3.14;

float GGX(float NoH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NoH2 = NoH*NoH;
    float numerator = a2;
    float denominator = (NoH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
    return numerator / denominator;
}

vec3 Fresnel(float HoV, vec3 metalness)
{
    return metalness + (1.0 - metalness) * 
                   pow(clamp(1.0 - HoV, 0.0, 1.0), 5.0);
}

float GASchlick(float Ndot, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float numerator   = Ndot;
    float denominator = Ndot * (1.0 - k) + k;
    return numerator / denominator;
}

float GASmith(float NoV, float NoL, float roughness)
{
    float gas2  = GASchlick(NoV, roughness);
    float gas1  = GASchlick(NoL, roughness);
    return gas1 * gas2;
}

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

void main()
{
  
  vec3 N = normalize(viewNorm);
  vec3 V = vec3(0.0, 0.0, 1.0);

  vec3  albedo = vec3(0.8, 0.3, 0.4);
  float metallic = 1.0;
  float roughness = 1.0;
  float ao = 1.0;

  vec3 metalness = vec3(0.01);
  metalness = mix(metalness, albedo, metallic);

  vec3 Lo = vec3(0.0);

  for( int i = 0; i < 4; i++ )
  {
    vec3 L = normalize(lights[i].Position.xyz - viewPosition);
    vec3 H = normalize(V + L);

    float distance    = length(lights[i].Position.xyz - viewPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = lights[i].Color;

    // Cook-Torrance BRDF
    float D = GGX(max(dot(N, H), 0.0), roughness);
    float G   = GASmith(max(dot(N, V), 0.0), max(dot(N, L), 0.0), roughness);
    vec3 F    = Fresnel(max(dot(H, V), 0.0), metalness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    float NoL = max(dot(N, L), 0.0);
    Lo += (albedo / PI + specular) * radiance * NoL;

      //Lo += phongModel( i, viewPosition, N );
  }

  color.xyz = Lo;
  color.w = 1.0;
}



