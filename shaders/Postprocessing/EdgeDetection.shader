#shader vertex
#version 460 core

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec2 texcoords; 

out vec2 TexCoords;

void main() {
    TexCoords = texcoords;
    gl_Position = vec4(positions, 1.0);
}

#shader fragment
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D RenderTex;
uniform float EdgeThreshold;
uniform vec2 texelSize; // 1.0 / texture size

void main()
{
    // Sobel Operator for Edge Detection

    // Sample neighboring pixels
    float s00 = dot(texture(RenderTex, TexCoords + vec2(-texelSize.x, texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
    float s10 = dot(texture(RenderTex, TexCoords + vec2(-texelSize.x, 0.0)).rgb, vec3(0.2126, 0.7152, 0.0722));
    float s20 = dot(texture(RenderTex, TexCoords + vec2(-texelSize.x, -texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));

    float s01 = dot(texture(RenderTex, TexCoords + vec2(0.0, texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
    float s21 = dot(texture(RenderTex, TexCoords + vec2(0.0, -texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));

    float s02 = dot(texture(RenderTex, TexCoords + vec2(texelSize.x, texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
    float s12 = dot(texture(RenderTex, TexCoords + vec2(texelSize.x, 0.0)).rgb, vec3(0.2126, 0.7152, 0.0722));
    float s22 = dot(texture(RenderTex, TexCoords + vec2(texelSize.x, -texelSize.y)).rgb, vec3(0.2126, 0.7152, 0.0722));

    // Compute gradients
    float gx = s00 + 2.0 * s10 + s20 - (s02 + 2.0 * s12 + s22);
    float gy = s00 + 2.0 * s01 + s02 - (s20 + 2.0 * s21 + s22);

    // Compute gradient magnitude squared
    float g = gx * gx + gy * gy;

    // Thresholding
    if(g > EdgeThreshold)
        FragColor = vec4(1.0); // Edge detected (white)
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Non-edge (black)
}