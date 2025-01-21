#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 6) uniform sampler2D texAmbient;

void runAlphaTest(float alpha, float alphaThreshold)
{
	if (alphaThreshold > 0.0)
	{
		mat4 thresholdMatrix = mat4(
			1.0  / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
			13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
			4.0  / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
			16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
		);

		alpha = clamp(alpha - 0.5 * thresholdMatrix[int(mod(gl_FragCoord.x, 4.0))][int(mod(gl_FragCoord.y, 4.0))], 0.0, 1.0);

		if (alpha < alphaThreshold)
			discard;
	}
}

void main()
{
    vec4 texColor = texture(texAlbedo, v_TexCoord);
	vec4 ambientColor = texture(texAmbient, v_TexCoord);
    if (texColor.a < 0.5)
        discard;
    //runAlphaTest(texColor.a, 0.01);
    color = 0.5 * (texColor + ambientColor);

	color = texColor;
}