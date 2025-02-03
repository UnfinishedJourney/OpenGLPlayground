#version 460 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D u_HDRTexture;
uniform float u_Exposure;
uniform float u_LWhite;
uniform float u_logAvg;

void main() {
    // Sample the HDR color.
    vec3 hdrColor = texture(u_HDRTexture, TexCoords).rgb;
    
    // Compute the pixel's luminance.
    float Lw = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));
    
    // Scale luminance using exposure and log-average.
    float Lm = (u_Exposure / u_logAvg) * Lw;
    
    // Apply the modified Reinhard tone mapping operator.
    float Lwhite2 = u_LWhite * u_LWhite;
    float Ld = (Lm * (1.0 + Lm / Lwhite2)) / (1.0 + Lm);
    
    // Compute scaling factor to adjust the original color.
    float scale = (Lw > 0.0) ? (Ld / Lw) : 0.0;
    vec3 toneMapped = hdrColor * scale;
    
    FragColor = vec4(toneMapped, 1.0);
}