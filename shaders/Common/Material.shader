struct PackedMtlParams {
    vec4 Mtl0;  // (Ka.xyz, Ni)
    vec4 Mtl1;  // (Kd.xyz, d)
    vec4 Mtl2;  // (Ks.xyz, Ns)
    vec4 Mtl3;  // (Ke.xyz, extra)
};

uniform PackedMtlParams uMaterial;       // Mtl0..Mtl3

// Uniforms
uniform int uMaterial_textureUsageFlags; // bitmask


layout(binding = 0) uniform sampler2D uTexAlbedo;
layout(binding = 1) uniform sampler2D uTexNormal;
layout(binding = 2) uniform sampler2D uTexMetalRoughness;
layout(binding = 3) uniform sampler2D uTexAO;
layout(binding = 4) uniform sampler2D uTexEmissive;
layout(binding = 5) uniform sampler2D uTexAmbient;
layout(binding = 6) uniform sampler2D uTexHeight;
layout(binding = 7) uniform sampler2D uTexBRDFLUT; // if you want, etc.
layout(binding = 9) uniform samplerCube u_EnvironmentMapDiffuse;   // Diffuse irradiance map

// A helper to check bits in the bitmask:
bool HasTexture(int bitIndex)
{
    return (uMaterial_textureUsageFlags & (1 << bitIndex)) != 0;
}