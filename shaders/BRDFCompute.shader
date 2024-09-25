#shader compute
#version 460 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform uint NUM_SAMPLES;

layout (binding = 1) buffer DST {
    vec2 data[];
} dst;

const uint BRDF_W = 256u;
const uint BRDF_H = 256u;
const float PI = 3.1415926536;

float random(uint seed)
{
    uint bits = (seed << 16u) | (seed >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return fract(sin(rdi) * 43758.5453);
}

vec2 hammersley2d(uint i, uint N) 
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) / float(N), rdi);
}

vec3 importanceSample_GGX(vec2 Xi, float roughness, vec3 normal) 
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, normal));
    vec3 tangentY = normalize(cross(normal, tangentX));

    return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float k = (roughness * roughness) / 2.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

vec2 BRDF(float NoV, float roughness)
{
    const vec3 N = vec3(0.0, 0.0, 1.0);
    vec3 V = vec3(sqrt(1.0 - NoV * NoV), 0.0, NoV);

    vec2 LUT = vec2(0.0);
    for(uint i = 0u; i < NUM_SAMPLES; i++)
    {
        vec2 Xi = hammersley2d(i, NUM_SAMPLES);
        vec3 H = importanceSample_GGX(Xi, roughness, N);
        vec3 L = 2.0 * dot(V, H) * H - V;

        float dotNL = max(dot(N, L), 0.0);
        float dotNV = max(dot(N, V), 0.0);
        float dotVH = max(dot(V, H), 0.0); 
        float dotNH = max(dot(H, N), 0.0);

        if (dotNL > 0.0)
        {
            float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
            float G_Vis = (G * dotVH) / (dotNH * dotNV);
            float Fc = pow(1.0 - dotVH, 5.0);
            LUT += vec2((1.0 - Fc) * G_Vis, Fc * G_Vis);
        }
    }
    return LUT / float(NUM_SAMPLES);
}

void main() 
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    if (x >= BRDF_W || y >= BRDF_H)
        return;

    float NoV = (float(x) + 0.5) / float(BRDF_W);
    float roughness = 1.0 - (float(y) + 0.5) / float(BRDF_H);

    vec2 v = BRDF(NoV, roughness);

    uint offset = y * BRDF_W + x;

    dst.data[offset] = v;
}