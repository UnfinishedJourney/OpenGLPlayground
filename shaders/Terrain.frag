in vec3 teNormal;
in vec2 teTexCoord;

out vec4 FragColor;

uniform vec3 u_LightDir = normalize(vec3(0.5, 1.0, 0.3));

void main()
{
    // Simple height-based coloring
    float height = texture(u_HeightMap, teTexCoord).r;
    vec3 color = mix(vec3(0.2, 0.5, 0.1), vec3(0.6, 0.5, 0.3), smoothstep(0.0, 1.0, height));

    // Simple diffuse lighting
    float diff = max(dot(teNormal, u_LightDir), 0.0);
    vec3 lighting = color * diff + vec3(0.1); // Ambient term

    FragColor = vec4(lighting, 1.0);
}