#version 460 core

// -----------------------------------------------------------------------------
// Uniforms and UBOs (make sure these are defined in your included files)
// -----------------------------------------------------------------------------
#include "Common/Common.shader"   // Should define u_CameraPos, euv.
#include "Common/Lights.shader"   // Should define lightsData[] and numLights
#include "Common/Material.shader"
#include "Common/PBR.shader"
// -----------------------------------------------------------------------------
// Inputs / Outputs
// -----------------------------------------------------------------------------
in vec2  uv;          // Texture coordinates
in vec3  fragPos;     // World-space position
in vec3  normal;      // World-space normal (from geometry or partially from a normal map)
in mat3  TBN;         // Tangent-Bitangent-Normal matrix

out vec4 out_FragColor;

// -----------------------------------------------------------------------------
// Texture Samplers
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
const float PI = 3.14159265359;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{

    vec3 lightDirection = normalize(vec3(-1.0, 0.7,-0.5));


    vec4 Kao = texture(uTexAO, uv);
	vec4 Ke  = texture(uTexEmissive, uv);
	vec4 Kd  = texture(uTexDiffuse, uv);
	vec2 MeR = texture(uTexMetalRoughness, uv).yz;

    Kd.rgb = SRGBtoLINEAR(Kd).rgb;

	// world-space normal
	vec3 n = normalize(normal);

	vec3 normalSample = texture(uTexNormal, uv).xyz;

	// normal mapping
	n = perturbNormal(n, normalize(u_CameraPos.xyz - fragPos), normalSample, uv);

	vec4 mrSample = texture(uTexMetalRoughness, uv);

	PBRInfo pbrInputs;
	Ke.rgb = SRGBtoLINEAR(Ke).rgb;
	// image-based lighting
	vec3 color1 = 1.0*calculatePBRInputsMetallicRoughness(Kd, n, u_CameraPos.xyz, fragPos, mrSample, pbrInputs);
	// one hardcoded light source
	color1 += 3.0*calculatePBRLightContribution( pbrInputs, lightDirection, vec3(1.0) );
	// ambient occlusion
	color1 = color1 * ( Kao.r < 0.01 ? 1.0 : Kao.r );
	// emissive
	//color1 = pow( Ke.rgb + color1, vec3(1.0/2.2) );

    color1 = Ke.rgb + color1;
	out_FragColor = vec4(color1, 1.0);

    return;

}