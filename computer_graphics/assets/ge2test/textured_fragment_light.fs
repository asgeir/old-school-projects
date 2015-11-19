#version 150

struct MaterialProperties {
	vec3 emission;    // light produced by the material
	vec3 ambient;     // what part of ambient light is reflected
	vec3 diffuse;     // what part of diffuse light is scattered
	vec3 specular;    // what part of specular light is scattered
	float shininess;  // exponent for sharpening specular reflection
};

struct LightProperties {
	bool isEnabled;
	bool isLocal;
	bool isSpot;
	vec3 ambient;
	vec3 color;
	vec3 position;
	vec3 halfVector;
	vec3 coneDirection;
	float spotCosCutoff;
	float spotExponent;
	float radius;
	float cutoff;
};

const int kMaxLights = 10;

in vec2 textureCoordinates;
in vec3 normal;
in vec3 position;

uniform sampler2D textureData;
uniform float textureGamma = 1.0 / 2.2;
uniform MaterialProperties ge_materialProperties;
uniform float ge_specularStrength;

layout(std140) uniform ge_Lights {
	LightProperties ge_lights[kMaxLights];
};

out vec4 ge_fragmentColor;

#define gammaCorrection(color, gamma) vec3(pow(color.r, 1.0 / gamma), pow(color.g, 1.0 / gamma), pow(color.b, 1.0 / gamma))

// http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
float lightAttenuation(float lightRadius, float cutoff, float dist)
{
	float d = max(dist - lightRadius, 0.0);
	float denom = d/lightRadius + 1.0;
	float attenuation = 1.0 / (denom*denom);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);
	return max(attenuation, 0.0);
}

void main()
{
	vec3 scatteredLight = vec3(0.0); // or to a global ambient light
	vec3 reflectedLight = vec3(0.0);

	// loop over all the lights
	for (int light = 0; light < kMaxLights; ++light) {
		if (!ge_lights[light].isEnabled) {
			continue;
		}

		vec3 halfVector;
		vec3 lightDirection = ge_lights[light].position;
		float attenuation = 1.0;

		// for local lights, compute per-fragment direction
		// halfVector, and attenuation
		if (ge_lights[light].isLocal) {
			lightDirection = lightDirection - position;
			float lightDistance = length(lightDirection);
			lightDirection = lightDirection / lightDistance;

			attenuation = lightAttenuation(ge_lights[light].radius, ge_lights[light].cutoff, lightDistance);

			if (ge_lights[light].isSpot) {
				float spotCos = dot(lightDirection, -ge_lights[light].coneDirection);
				if (spotCos < ge_lights[light].spotCosCutoff) {
					attenuation = 0.0;
				} else {
					attenuation *= pow(spotCos, ge_lights[light].spotExponent);
				}
			}

			halfVector = normalize(lightDirection + vec3(0.0, 0.0, 1.0));  // lightDirection + eyeDirection
		} else {
			halfVector = ge_lights[light].halfVector;
		}

		float diffuse = max(0.0, dot(normal, lightDirection));
		float specular = max(0.0, dot(normal, halfVector));

		if (diffuse == 0.0) {
			specular = 0.0;
		} else {
			specular = pow(specular, ge_materialProperties.shininess) * ge_specularStrength;
		}

		scatteredLight += ge_lights[light].ambient * ge_materialProperties.ambient * attenuation;
		scatteredLight += ge_lights[light].color * ge_materialProperties.diffuse * diffuse * attenuation;

		reflectedLight += ge_lights[light].color * ge_materialProperties.specular * specular * attenuation;
	}

	vec4 texel = texture(textureData, textureCoordinates);
	vec3 rgb = min(ge_materialProperties.emission + gammaCorrection(texel.xyz, textureGamma) * scatteredLight + reflectedLight, vec3(1.0));
	ge_fragmentColor = vec4(rgb, texel.a);
}
