struct LightData {
    vec4 position; // xyz: position, w: unused or could be used for attenuation
    vec4 color;    // xyz: color, w: intensity
};

layout(std140, binding = 1) buffer LightsBuffer {
    uint numLights;
    LightData lightsData[];
};