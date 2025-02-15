float PCF(sampler2DShadow shadowMap, vec4 shadowCoord, int kernelSize)
{
    // do the perspective divide
    vec3 sc = shadowCoord.xyz / shadowCoord.w;

    // if out of [0..1], no shadow (treat as lit)
    if(any(lessThan(sc, vec3(0.0))) || any(greaterThan(sc, vec3(1.0))))
        return 1.0;

    // 3x3 PCF
    
    float sum = 0.0;
    float texSize = float( textureSize(shadowMap, 0 ).x ); 
    float offset = 1.0 / texSize;
    int range = kernelSize / 2;

    for(int yy=-range; yy<=range; yy++)
    {
        for(int xx=-range; xx<=range; xx++)
        {
            vec4 offCoord = shadowCoord;
            offCoord.xy += vec2(xx, yy) * offset;
            float bias = 0.0005;
            offCoord.z -= bias;
            sum += textureProj(shadowMap, offCoord);
        }
    }
    return sum / (kernelSize * kernelSize);
}