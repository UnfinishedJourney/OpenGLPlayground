vec2 calculateUVSimpleParallax(sampler2D heightMap, vec2 uv, vec3 viewTangent)
{
    // Define a margin in UV space where the parallax effect should fade out.
    const float margin = 0.05;
    // Compute the distance from uv to the nearest edge.
    float distanceToEdge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    // Generate a fade factor: 0 at the edges, and 1 when safely inside the texture.
    float edgeFade = smoothstep(0.0, margin, distanceToEdge);

    // Parallax parameters.
    const float bumpFactor = 0.02;
    float height = 1.0 - texture(heightMap, uv).r;
    vec2 delta = viewTangent.xy * height * bumpFactor / viewTangent.z;
    
    // Blend based on the view angle.
    float blend = smoothstep(0.1, 0.5, abs(viewTangent.z));
    
    // Apply the parallax offset, modulated by the edge fade.
    vec2 finalUV = uv - delta * blend * edgeFade;
    
    // Optionally clamp finalUV to [0,1] to ensure no sampling outside the texture.
    finalUV = clamp(finalUV, vec2(0.0), vec2(1.0));
    
    return finalUV;
}