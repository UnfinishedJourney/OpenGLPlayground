#include "Lights.shader"
#include "Material.shader"


vec3 CalculatePhongLighting(vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 result = vec3(0.0);
    for (uint i = 0; i < numLights; ++i) { // Use numLights from LightsBuffer
        LightData light = lightsData[i];

        // Extract light color and intensity
        vec3 lightColor = light.color.xyz;
        float intensity = light.color.w;

        // Ambient component
        vec3 ambient = material.Ka * lightColor * intensity;

        // Diffuse component
        vec3 lightDir = normalize(light.position.xyz - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = material.Kd * diff * lightColor * intensity;

        // Specular component
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = material.Ks * spec * lightColor * intensity;

        result += ambient + diffuse + specular;
    }
    return result;
}

vec3 CalculateBlinnPhongLighting(vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 result = vec3(0.0);
    for (uint i = 0; i < numLights; ++i) { // Use numLights from LightsBuffer
        LightData light = lightsData[i];

        // Extract light color and intensity
        vec3 lightColor = light.color.xyz;
        float intensity = light.color.w;

        // Ambient component
        vec3 ambient = material.Ka * lightColor * intensity;

        // Diffuse component
        vec3 lightDir = normalize(light.position.xyz - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = material.Kd * diff * lightColor * intensity;

        // Specular component (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular = material.Ks * spec * lightColor * intensity;

        result += ambient + diffuse + specular;
    }
    return result;
}

uniform int levels = 3;          // Number of shading levels
uniform float scaleFactor = 1;   // Scaling factor for intensity

// Function to calculate Toon Lighting
vec3 CalculateToonLighting(vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 result = vec3(0.0);
    
    for (uint i = 0; i < numLights; ++i) { // Iterate over all lights
        LightData light = lightsData[i];

        // Extract light color and intensity
        vec3 lightColor = light.color.xyz;
        float intensity = light.color.w;

        // Ambient component remains the same as Phong
        vec3 ambient = material.Ka * lightColor * intensity;

        // Diffuse component with quantization
        vec3 lightDir = normalize(light.position.xyz - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        // Quantize the diffuse term
        float quantDiff = floor(diff * float(levels)) / float(levels);
        vec3 diffuse = material.Kd * quantDiff * lightColor * intensity * scaleFactor;

        // Specular component with quantization
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // Quantize the specular term
        float quantSpec = floor(spec * float(levels)) / float(levels);
        vec3 specular = material.Ks * quantSpec * lightColor * intensity * scaleFactor;

        // Accumulate the results
        result += ambient + diffuse + specular;
    }
    
    return result;
}
