#version 460 core

#include "Common/Material.shader"

layout(location = 0) out vec4 color;

layout(binding = 0) uniform samplerCube u_Texture;

in vec3 wPos;
in vec3 wNormal;
in vec2 uv;

void runAlphaTest(float alpha, float alphaThreshold)
{
	if (alphaThreshold > 0.0)
	{
		mat4 thresholdMatrix = mat4(
			1.0  / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
			13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
			4.0  / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
			16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
		);

		alpha = clamp(alpha - 0.5 * thresholdMatrix[int(mod(gl_FragCoord.x, 4.0))][int(mod(gl_FragCoord.y, 4.0))], 0.0, 1.0);

		if (alpha < alphaThreshold)
			discard;
	}
}

void main()
{
	vec3 Ka     = uMaterial.Mtl0.xyz;   // ambient color
    // float Ni  = uMaterial.Mtl0.w;    // index of refraction if needed

    vec3 Kd     = uMaterial.Mtl1.xyz;   // diffuse color
    float alpha = uMaterial.Mtl1.w;     // overall alpha/opacity

    vec3 Ks     = uMaterial.Mtl2.xyz;   // specular color
    float Ns    = uMaterial.Mtl2.w;     // specular exponent (shininess)

    vec3 Ke     = uMaterial.Mtl3.xyz;   // emissive color (if used)
    // float extra= uMaterial.Mtl3.w;   // any extra param


	if (HasTexture(0)) {
        vec3 albedoSample = texture(uTexAlbedo, uv).rgb;
        Kd *= albedoSample;
    }

	vec3 N = normalize(wNormal);

	//vec3 ambientEnv = texture(u_Texture, N).rgb;
    
    // Compute the ambient term by modulating the sampled color with the material property
    //vec3 ambient = Ka * ambientEnv;

	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = Kd * diff;

    //vec3 finalColor = ambient + diffuse;
    
    color = vec4(diffuse, 1.0);
	return;
}