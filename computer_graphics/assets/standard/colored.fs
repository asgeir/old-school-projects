#version 150

struct MaterialProperties {
	vec3 emission;    // light produced by the material
	vec3 ambient;     // what part of ambient light is reflected
	vec3 diffuse;     // what part of diffuse light is scattered
	vec3 specular;    // what part of specular light is scattered
	float shininess;  // exponent for sharpening specular reflection
};

uniform MaterialProperties ge_materialProperties;

out vec4 ge_fragmentColor;

void main()
{
	ge_fragmentColor = vec4(ge_materialProperties.diffuse, 1.0);
}
