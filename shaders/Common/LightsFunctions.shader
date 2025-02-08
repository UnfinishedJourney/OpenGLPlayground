#include "Lights.shader"     // for numLights, lightsData, etc.
#include "Material.shader"    // for uMaterial

//--------------------------------------------
// Shared function: computeLightDir()
//--------------------------------------------
struct LightResult {
    vec3 lightDir;
    float attenuation; // =1 if directional
};

LightResult computeLightDir(in LightData ld, in vec3 fragPos)
{
    LightResult lr;
    float wComp = ld.position.w;

    if (wComp < 0.5)
    {
        // directional
        lr.lightDir     = normalize(-ld.position.xyz);
        lr.attenuation  = 1.0; // no distance falloff
    }
    else
    {
        // point
        vec3 toLight = ld.position.xyz - fragPos;
        float dist   = length(toLight);
        lr.lightDir  = toLight / dist;

        // e.g. 1/(dist^2) if you want
        // or 1/(1 + 0.09*dist + 0.032*dist^2) etc.
        // We'll keep it simple:
        lr.attenuation = 1.0 / (dist*dist);
    }
    return lr;
}

//--------------------------------------------
// PHONG
//--------------------------------------------
vec3 CalculatePhongLighting(vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 Ka = uMaterial.Mtl0.xyz;
    vec3 Kd = uMaterial.Mtl1.xyz;
    vec3 Ks = uMaterial.Mtl2.xyz;

    float Ns = uMaterial.Mtl2.w;

    vec3 total = vec3(0.0);

    for (uint i = 0; i < numLights; i++)
    {
        LightData ld = lightsData[i];

        // color & intensity
        vec3  Lcol = ld.color.xyz;
        float Lint = ld.color.w;

        // get direction & attenuation
        LightResult lr = computeLightDir(ld, fragPos);

        // Ambient
        vec3 ambient = Ka * Lcol * Lint;

        // Diffuse
        float diff = max(dot(normal, lr.lightDir), 0.0);
        vec3 diffuse = Kd * diff * Lcol * Lint * lr.attenuation;

        // Specular
        vec3 reflectDir = reflect(-lr.lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
        vec3 specular = Ks * spec * Lcol * Lint * lr.attenuation;

        total += (ambient + diffuse + specular);
    }

    return total;
}

//--------------------------------------------
// BLINN-PHONG
//--------------------------------------------
vec3 CalculateBlinnPhongLighting(vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 Ka = uMaterial.Mtl0.xyz;
    vec3 Kd = uMaterial.Mtl1.xyz;
    vec3 Ks = uMaterial.Mtl2.xyz;
    float Ns = uMaterial.Mtl2.w;

    vec3 total = vec3(0.0);
    for (uint i = 0; i < numLights; i++)
    {
        LightData ld = lightsData[i];
        vec3  Lcol = ld.color.xyz;
        float Lint = ld.color.w;

        LightResult lr = computeLightDir(ld, fragPos);

        // Ambient
        vec3 ambient = Ka * Lcol * Lint;

        // Diffuse
        float diff = max(dot(normal, lr.lightDir), 0.0);
        vec3 diffuse = Kd * diff * Lcol * Lint * lr.attenuation;

        // Specular (blinn-phong)
        vec3 halfwayDir = normalize(lr.lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), Ns);
        vec3 specular = Ks * spec * Lcol * Lint * lr.attenuation;

        total += (ambient + diffuse + specular);
    }
    return total;
}

//--------------------------------------------
// TOON
//--------------------------------------------
uniform int levels = 3;
uniform float scaleFactor = 1.0;

vec3 CalculateToonLighting(vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 Ka = uMaterial.Mtl0.xyz;
    vec3 Kd = uMaterial.Mtl1.xyz;
    vec3 Ks = uMaterial.Mtl2.xyz;
    float Ns = uMaterial.Mtl2.w;

    vec3 total = vec3(0.0);
    for (uint i=0; i<numLights; i++)
    {
        LightData ld = lightsData[i];
        vec3  Lcol = ld.color.xyz;
        float Lint = ld.color.w;

        LightResult lr = computeLightDir(ld, fragPos);

        // Ambient
        vec3 ambient = Ka * Lcol * Lint;

        // Diffuse (quantized)
        float diff = max(dot(normal, lr.lightDir), 0.0);
        float quantDiff = floor(diff * float(levels)) / float(levels);
        vec3 diffuse = Kd * quantDiff * Lcol * Lint * lr.attenuation * scaleFactor;

        // Specular (quantized)
        vec3 reflectDir = reflect(-lr.lightDir, normal);
        float rawSpec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
        float quantSpec = floor(rawSpec * float(levels)) / float(levels);
        vec3 specular = Ks * quantSpec * Lcol * Lint * lr.attenuation * scaleFactor;

        total += (ambient + diffuse + specular);
    }
    return total;
}