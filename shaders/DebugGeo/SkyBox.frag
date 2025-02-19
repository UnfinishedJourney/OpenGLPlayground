#version 460 core

layout(location = 0) out vec4 color;
in vec3 Vec;
layout(binding = 8) uniform samplerCube u_Texture;

// Convert RGB to HSV
vec3 rgb2hsv(vec3 c) {
    float cMax = max(c.r, max(c.g, c.b));
    float cMin = min(c.r, min(c.g, c.b));
    float delta = cMax - cMin;
    
    vec3 hsv;
    hsv.z = cMax; // value
    
    // Hue
    if (delta < 0.00001) {
        hsv.x = 0.0;
    } else if (cMax == c.r) {
        hsv.x = mod((c.g - c.b) / delta, 6.0);
    } else if (cMax == c.g) {
        hsv.x = ((c.b - c.r) / delta) + 2.0;
    } else {
        hsv.x = ((c.r - c.g) / delta) + 4.0;
    }
    hsv.x /= 6.0;
    if (hsv.x < 0.0)
        hsv.x += 1.0;
    
    // Saturation
    hsv.y = (cMax <= 0.0) ? 0.0 : (delta / cMax);
    
    return hsv;
}

// Convert HSV back to RGB
vec3 hsv2rgb(vec3 c) {
    float h = c.x * 6.0;
    float s = c.y;
    float v = c.z;
    
    int i = int(floor(h));
    float f = h - float(i);
    float p = v * (1.0 - s);
    float q = v * (1.0 - s * f);
    float t = v * (1.0 - s * (1.0 - f));
    
    if (i == 0)
        return vec3(v, t, p);
    else if (i == 1)
        return vec3(q, v, p);
    else if (i == 2)
        return vec3(p, v, t);
    else if (i == 3)
        return vec3(p, q, v);
    else if (i == 4)
        return vec3(t, p, v);
    else
        return vec3(v, p, q);
}

void main() {
    // Sample and gamma-correct the texture.
    vec3 texColor = texture(u_Texture, normalize(Vec)).rgb;
    vec3 gammaCorrected = texColor;
    //vec3 gammaCorrected = pow(texColor, vec3(1.0/2.2));
    
    // Convert to HSV.
    vec3 hsv = rgb2hsv(gammaCorrected);
    
    // Use smoothstep to tone down brightness when value is high.
    float threshold = 0.8;
    // When the brightness is above the threshold, smooth it down.
    hsv.z = mix(hsv.z, threshold, smoothstep(threshold, 1.0, hsv.z));
    
    // Convert back to RGB.
    vec3 finalColor = hsv2rgb(hsv);
    
    // Output with gamma correction applied.
    float t = 0.0;
    color = vec4(t*finalColor+(1.0-t)*gammaCorrected, 1.0);
}