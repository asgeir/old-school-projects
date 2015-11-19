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
	int shadowId;
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
in vec4 positionLight1;
in vec4 positionLight2;

uniform float ge_oneOverShadowMapResolution;
uniform sampler2D ge_shadowMap1;
uniform samplerCube ge_shadowCubeMap1;
uniform float ge_shadowBias1;
uniform float ge_shadowFarPlane1;
uniform sampler2D ge_shadowMap2;
uniform samplerCube ge_shadowCubeMap2;
uniform float ge_shadowBias2;
uniform float ge_shadowFarPlane2;

uniform MaterialProperties ge_materialProperties;
uniform float ge_specularStrength;

layout(std140) uniform ge_Lights {
	LightProperties ge_lights[kMaxLights];
};

// http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
float lightAttenuation(float lightRadius, float cutoff, float dist)
{
	float d = max(dist - lightRadius, 0.0);
	float denom = d/lightRadius + 1.0;
	float attenuation = 1.0 / (denom*denom);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);
	return max(attenuation, 0.0);
}

// http://http.developer.nvidia.com/GPUGems/gpugems_ch11.html
float shadowmapOffsetLookup(sampler2D map, vec4 lightSpacePosition, vec2 offset, float bias)
{
	vec3 projected = (lightSpacePosition.xyz - vec3(0.0, 0.0, bias)) / lightSpacePosition.w;
	projected.xy = (offset * ge_oneOverShadowMapResolution) + projected.xy;
	if (texture(map, projected.xy).z < projected.z) {
		return 1.0;
	}
	return 0.0;
}

float shadowmapLookup(int light, float cosTheta)
{
	float shadowBias;
	vec4 positionLight;
	if (light == 1) {
		shadowBias = ge_shadowBias1;
		positionLight = positionLight1;
	} else {
		shadowBias = ge_shadowBias2;
		positionLight = positionLight2;
	}

	float bias = shadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, 0.01);

	float sum = 0.0;
	for (float y = -0.75; y <= 0.75; y += 0.5) {
		for (float x = -0.75; x <= 0.75; x += 0.5) {
			if (light == 1) {
				sum += shadowmapOffsetLookup(ge_shadowMap1, positionLight, vec2(x, y), bias);
			} else {
				sum += shadowmapOffsetLookup(ge_shadowMap2, positionLight, vec2(x, y), bias);
			}
		}
	}
	float shadowCoeff = sum / 16.0;

	return 1.0 - shadowCoeff;
}

float shadowCubemapLookup(int light, float cosTheta)
{
	float shadowBias;
	float shadowFarPlane;
	vec4 positionLight;
	if (light == 1) {
		shadowBias = ge_shadowBias1;
		shadowFarPlane = ge_shadowFarPlane1;
		positionLight = positionLight1;
	} else {
		shadowBias = ge_shadowBias2;
		shadowFarPlane = ge_shadowFarPlane2;
		positionLight = positionLight2;
	}

	float bias = shadowBias * tan(acos(cosTheta));
	bias = clamp(bias, 0.0, 0.01);

	// http://stackoverflow.com/a/19485001/764349
	float zFar = shadowFarPlane;
	float zNear = 0.1;
	vec3 absPos = abs(vec3(positionLight));
	float localZ = max(absPos.x, max(absPos.y, absPos.z));
	float normalizedZ = (zFar + zNear) / (zFar - zNear) - (2 * zFar * zNear) / (zFar - zNear) / localZ;
	float distanceToLight = (normalizedZ + 1.0) * 0.5;

	float zValue;
	if (light == 1) {
		zValue = texture(ge_shadowCubeMap1, vec3(positionLight)).z;
	} else {
		zValue = texture(ge_shadowCubeMap2, vec3(positionLight)).z;
	}
	if (zValue < (distanceToLight - bias)) {
		return 0.0; // in shadow
	}

	return 1.0;
}

vec4 lighting(vec4 fragmentColor)
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

		float shadowAttenuation = 1.0;
		if (ge_lights[light].shadowId > 0) {
			if (!ge_lights[light].isLocal || ge_lights[light].isSpot) {
				shadowAttenuation = shadowmapLookup(ge_lights[light].shadowId, clamp(dot(normal, lightDirection), 0.0, 1.0));
			} else {
				shadowAttenuation = shadowCubemapLookup(ge_lights[light].shadowId, clamp(dot(normal, lightDirection), 0.0, 1.0));
			}
		}

		if (!ge_lights[light].isLocal) {
			scatteredLight += ge_lights[light].ambient * ge_materialProperties.ambient;
		}
		scatteredLight += ge_lights[light].color * ge_materialProperties.diffuse * diffuse * attenuation * shadowAttenuation;

		reflectedLight += ge_lights[light].color * ge_materialProperties.specular * specular * attenuation * shadowAttenuation;
	}

	return vec4(min(ge_materialProperties.emission + scatteredLight * fragmentColor.rgb + reflectedLight, vec3(1.0)), fragmentColor.a);
}
