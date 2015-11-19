#version 150

#ge_include "standard/lighting.fs"

uniform sampler2D textureData;
uniform float textureGamma = 1.0 / 2.2;

out vec4 ge_fragmentColor;

#define gammaCorrection(color, gamma) vec3(pow(color.r, 1.0 / gamma), pow(color.g, 1.0 / gamma), pow(color.b, 1.0 / gamma))

void main()
{
	vec4 texel = texture(textureData, textureCoordinates);
	ge_fragmentColor = lighting(gammaCorrection(texel.xyz, textureGamma));
}
