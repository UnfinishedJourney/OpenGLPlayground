#version 460 core

// We include your common material / lighting includes:
#include "Common/Common.shader"
#include "Common/Material.shader"
#include "Common/LightsFunctions.shader"
#include "Common/Parallax.shader"
#include "Common/PCF.shader"
#include "Common/PBR.shader"

// For the shadow map
layout(binding = 10) uniform sampler2DShadow u_ShadowMap;


// We output final color
layout(location = 0) out vec4 out_FragColor;

// Inputs from your vertex shader
in vec3 wPos;
in vec3 wNormal;
in vec2 uv;
// We'll also have the shadow coords from the vertex shader
in vec4 PosLightMap;
in mat3  TBN;          

// Start of main
void main()
{
    LightData ld = lightsData[0];
    vec3 lightDirection = normalize(ld.position.xyz);
	vec3 lightColor = ld.color.xyz;
    float alphaTest = uMaterial.Mtl1.w;
    vec2 uvParallax = uv;

    vec4 Kd  = vec4(0.0, 0.0, 0.0, 1.0);
    if (HasTexture(0)) {
        Kd  = texture(uTexDiffuse, uvParallax);
        Kd.rgb = SRGBtoLINEAR(Kd).rgb;
        if (Kd.a < 0.02)
            discard;
    }

    vec4 Kao = vec4(1.0, 1.0, 1.0, 1.0);
    if (HasTexture(3))
    {
        Kao = texture(uTexAO, uvParallax);
    }

    vec4 Ke  = vec4(0.0, 0.0, 0.0, 1.0);

    if (HasTexture(4)) {
        Ke  = texture(uTexEmissive, uvParallax);
        Ke.rgb = SRGBtoLINEAR(Ke).rgb;
    }

    vec3 n = normalize(wNormal);
    if (HasTexture(1)) {

	    vec3 normalSample = texture(uTexNormal, uvParallax).xyz;
        n = normalize(TBN * (normalSample * 2.0 - 1.0));

	    // normal mapping
	    //n = perturbNormal(n, normalize(u_CameraPos.xyz - wPos), normalSample, uv);
    }

    vec4 mrSample = vec4(1.0, 1.0, 0.0, 1.0);
    vec2 MeR = mrSample.yz;
    if (HasTexture(2)) {
        vec4 roughMet = texture(uTexMetalRoughness, uvParallax);
        //out_FragColor = vec4(roughMet.a, 0.0, 0.0, 1.0);
        //return;
	    //mrSample = roughMet;
        //MeR = mrSample.rg;
        //mrSample.g = roughMet.a;
        //mrSample.b *= roughMet.a;
        //mrSample.r = 1.0;
        //mrSample.gb = MeR;
    }



    float shadowFactor = PCF(u_ShadowMap, PosLightMap, 5);

	PBRInfo pbrInputs;
	// image-based lighting
    float t = 0.5;
	//vec3 color1 = (shadowFactor*t + 1.0-t)*calculatePBRInputsMetallicRoughness(Kd, n, u_CameraPos.xyz, wPos, mrSample, pbrInputs);
    vec3 color1 = (shadowFactor*t + 1.0-t)*calculatePBRInputsDiffuse(Kd, n, u_CameraPos.xyz, wPos, mrSample, pbrInputs);
    color1 += calculatePBRLightContribution( pbrInputs, lightDirection, lightColor );
	// one hardcoded light source
	//color1 += shadowFactor*calculatePBRInputsMetallicRoughness( pbrInputs, lightDirection, lightColor);
	// ambient occlusion
	color1 = color1 * ( Kao.r < 0.01 ? 1.0 : Kao.r );
	// emissive
	//color1 = pow( Ke.rgb + color1, vec3(1.0/2.2) );

    color1 = Ke.rgb + color1;
	out_FragColor = vec4(color1, 1.0);
    return;
}