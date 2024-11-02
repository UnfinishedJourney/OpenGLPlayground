#include "Lights.shader"
#include "Material.shader"

vec3 CalculatePhongLighting(vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 result = vec3(0.0);
    for (uint i = 0; i < 1; ++i) {
        Light light = lights[i];

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