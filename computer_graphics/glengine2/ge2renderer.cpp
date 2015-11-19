#include "ge2renderer.h"
#include "ge2camera.h"
#include "ge2cubeframebuffer.h"
#include "ge2cubemap.h"
#include "ge2framebuffer.h"
#include "ge2material.h"
#include "ge2mesh.h"
#include "ge2node.h"
#include "ge2resourcemgr.h"
#include "ge2shader.h"
#include "ge2texture2d.h"
#include "ge2time.h"

#include "gl_core_3_2.h"

#include <iostream>

#ifndef __APPLE__
#	define DEPTH_TEXTURE_SAMPLERS_WORK
#endif

using namespace ge2;

namespace {

const size_t kRendererLightsBufferSize = sizeof(LightProperties) * kRendererMaxLights;
const size_t kShadowMapResolution = 1024;

const glm::mat4 kShadowMapBiasMatrix{
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
};

const std::string kShadowMapShaderAndMaterialName = "_ge_internal_shadow_map_shader";

const std::string kShadowMapVertexShader = R"(
	#version 150

	in vec4 ge_position;

	uniform mat4 ge_modelViewProjection;

	void main()
	{
		gl_Position = ge_modelViewProjection * ge_position;
	}
)";

#ifdef DEPTH_TEXTURE_SAMPLERS_WORK
const std::string kShadowMapFragmentShader = R"(
	#version 150

	out vec4 ignoredColor;

	void main()
	{
		ignoredColor = vec4(1.0);
	}
)";
#else
const std::string kShadowMapFragmentShader = R"(
	#version 150

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = vec4(gl_FragCoord.z);
	}
)";
#endif

} // namespace

void DirectionalLight::populateLightProperties(LightProperties &properties)
{
	properties.isEnabled = enabled();
	properties.isLocal = 0;
	properties.isSpot = 0;
	properties.ambient = glm::vec4{ambientColor(), 0.0f};
	properties.color = glm::vec4{color(), 0.0f};
	properties.position = glm::normalize(glm::vec4{m_direction, 0.0f});
	properties.halfVector = glm::vec4{0.0f};
	properties.coneDirection = glm::vec3{0.0f};
	properties.spotCosCutoff = 0.0f;
	properties.spotExponent = 0.0f;
	properties.radius = 0.0f;
	properties.cutoff = 0.0f;
}

void PointLight::populateLightProperties(LightProperties &properties)
{
	properties.isEnabled = enabled();
	properties.isLocal = 1;
	properties.isSpot = 0;
	properties.ambient = glm::vec4{ambientColor(), 0.0f};
	properties.color = glm::vec4{color(), 0.0f};
	properties.position = glm::vec4{0.0f};
	properties.halfVector = glm::vec4{0.0f};
	properties.coneDirection = glm::vec3{0.0f};
	properties.spotCosCutoff = 0.0f;
	properties.spotExponent = 0.0f;
	properties.radius = m_radius;
	properties.cutoff = m_cutoff;
}

void SpotLight::populateLightProperties(LightProperties &properties)
{
	properties.isEnabled = enabled();
	properties.isLocal = 1;
	properties.isSpot = 1;
	properties.ambient = glm::vec4{ambientColor(), 0.0f};
	properties.color = glm::vec4{color(), 0.0f};
	properties.position = glm::vec4{0.0f};
	properties.halfVector = glm::vec4{0.0f};
	properties.coneDirection = glm::vec3{0.0f, 0.0f, -1.0f};
	properties.spotCosCutoff = cos(m_angle / 2.0f);
	properties.spotExponent = m_exponent;
	properties.radius = m_length;
	properties.cutoff = m_cutoff;
}

Renderer::Renderer(SDL_Window *window)
	: m_window(window)
{
	SDL_GetWindowSize(window, &m_windowWidth, &m_windowHeight);

	glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
	glClearDepth(m_clearDepth);
	glClearStencil(m_clearStencil);

	glGenBuffers(1, &m_lightsBufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsBufferObject);
	glBufferData(GL_UNIFORM_BUFFER, kRendererLightsBufferSize, m_lightProperties.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, (GLint)StandardUniformBlocks::Lights, m_lightsBufferObject, 0, kRendererLightsBufferSize);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	Shader *shadowMapShader = geResourceMgr->loadShaderFromStrings(
		kShadowMapShaderAndMaterialName,
		kShadowMapVertexShader,
		kShadowMapFragmentShader,
		{ }
	);
	m_shadowMapMaterial = geResourceMgr->createMaterial(kShadowMapShaderAndMaterialName, shadowMapShader);
	m_shadowMapMaterial->setFaceCulling(false);

	for (size_t i = 0, e = m_shadowData.size(); i < e; ++i) {
		m_shadowData[i].shadowMap = new Framebuffer;
		m_shadowData[i].cubeShadowMap = new CubeFramebuffer;

#ifdef DEPTH_TEXTURE_SAMPLERS_WORK
		m_shadowData[i].shadowMap->construct(kShadowMapResolution, kShadowMapResolution, true, { });
		m_shadowData[i].cubeShadowMap->construct(kShadowMapResolution, true, { });
#else
		m_shadowData[i].shadowMap->construct(kShadowMapResolution, kShadowMapResolution, true, { FragmentBuffer::Color });
		m_shadowData[i].cubeShadowMap->construct(kShadowMapResolution, true, { FragmentBuffer::Color });
#endif
	}
}

Renderer::~Renderer()
{
	for (size_t i = 0, e = m_shadowData.size(); i < e; ++i) {
		delete m_shadowData[i].shadowMap;
		delete m_shadowData[i].cubeShadowMap;
	}
}

Camera *Renderer::activeCamera()
{
	return m_camera;
}

void Renderer::setActiveCameraAndLights(Camera *camera, LightInfoList &lights)
{
	m_camera = camera;
	m_activeLights = lights;

	for (auto &light : m_lightProperties) {
		light.isEnabled = 0;
		light.shadowId = 0;
	}
	if (!m_camera) {
		return;
	}

	int nextShadowId = 1;
	glm::mat4 viewMatrix = m_camera->viewMatrix();
	for (size_t i = 0; i < m_activeLights.size(); ++i) {
		m_activeLights[i].light->populateLightProperties(m_lightProperties[i]);
		m_activeLights[i].props = &m_lightProperties[i];

		if (nextShadowId <= kNumShadowMaps && m_activeLights[i].light->enabled() && m_activeLights[i].light->castsShadows()) {
			m_lightProperties[i].shadowId = nextShadowId++;
		}

		if (m_lightProperties[i].isLocal) {
			m_lightProperties[i].position = viewMatrix * m_activeLights[i].modelMatrix[3];
		} else {
			m_lightProperties[i].position = viewMatrix * -m_lightProperties[i].position;
			m_lightProperties[i].halfVector = glm::normalize(m_lightProperties[i].position + glm::vec4{0.0f, 0.0f, 1.0f, 0.0f});
		}

		if (m_lightProperties[i].isSpot) {
			glm::mat3 modelMatrix = glm::transpose(glm::inverse(glm::mat3(m_activeLights[i].modelMatrix)));
			m_lightProperties[i].coneDirection = glm::normalize(glm::vec3{
				viewMatrix * glm::vec4{
					modelMatrix * m_lightProperties[i].coneDirection,
					0.0f
				}
			});
		}
	}
}

void Renderer::setClearColorValue(const glm::vec4 &color)
{
	m_clearColor = color;
	glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
}

void Renderer::setClearDepthValue(float value)
{
	m_clearDepth = value;
	glClearDepth(m_clearDepth);
}

void Renderer::setClearStencilValue(int value)
{
	m_clearStencil = value;
	glClearStencil(m_clearStencil);
}

void Renderer::setTitle(const char *title)
{
	SDL_SetWindowTitle(m_window, title);
}

void Renderer::resize(int width, int height)
{
	SDL_SetWindowSize(m_window, width, height);
	m_windowWidth = width;
	m_windowHeight = height;
}

void Renderer::clear(int clearFlags)
{
	GLbitfield glClearFlags = 0;
	if (clearFlags & kClearFlagColor) {
		glClearFlags |= GL_COLOR_BUFFER_BIT;
	}
	if (clearFlags & kClearFlagDepth) {
		glClearFlags |= GL_DEPTH_BUFFER_BIT;
	}
	if (clearFlags & kClearFlagStencil) {
		glClearFlags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(glClearFlags);
}

void Renderer::render(RenderableList &renderables, bool isShadowPass, Material *overrideMaterial)
{
	if (!m_camera || renderables.empty()) {
		return;
	}

	int firstShadowMapTextureUnit = 0;
	if (overrideMaterial) {
		firstShadowMapTextureUnit = overrideMaterial->bind();
	}

	glm::mat4 viewMatrix = m_camera->viewMatrix();
	glm::mat4 projectionMatrix = m_camera->projectionMatrix();

	glm::mat3 viewMatrixLinear = glm::transpose(glm::inverse(glm::mat3(viewMatrix)));
	glm::mat3 viewMatrixLinearInverse = glm::inverse(viewMatrixLinear);

	if (!isShadowPass) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_lightsBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, kRendererLightsBufferSize, m_lightProperties.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Draw renderables
	for (auto renderable : renderables) {
		if (isShadowPass && !renderable.castsShadows) {
			continue;
		}

		if (!renderable.node->enabled()) {
			continue;
		}

		for (auto mesh : renderable.node->meshList()) {
			Material *material = overrideMaterial ? nullptr : mesh->material();
			Shader *shader = nullptr;

			if (material) {
				shader = material->shader();
				firstShadowMapTextureUnit = material->bind();
			} else {
				shader = overrideMaterial->shader();
			}

			glm::mat4 modelView = viewMatrix * renderable.modelMatrix;
			glm::mat4 modelViewProjection = projectionMatrix * modelView;
			shader->setUniform("ge_modelViewProjection", modelViewProjection);
			shader->setUniform("ge_modelView", modelView);
			shader->setUniform("ge_normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelView))));
			shader->setUniform("ge_specularStrength", m_specularStrength);
			shader->setUniform("ge_viewMatrixLinear", viewMatrixLinear);
			shader->setUniform("ge_viewMatrixLinearInverse", viewMatrixLinearInverse);
			shader->setUniform("ge_totalSeconds", Time::totalSeconds());

			if (!isShadowPass) {
				shader->setUniform("ge_oneOverShadowMapResolution", 1.0f / (float)kShadowMapResolution);
				int textureUnit = firstShadowMapTextureUnit;

#ifdef DEPTH_TEXTURE_SAMPLERS_WORK
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[0].shadowMap->depthBuffer()->bind();
				shader->setUniform("ge_shadowMap1", textureUnit++);
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[0].cubeShadowMap->depthBuffer()->bind();
				shader->setUniform("ge_shadowCubeMap1", textureUnit++);
				shader->setUniform("ge_shadowBias1", m_shadowData[0].shadowBias);
				shader->setUniform("ge_shadowFarPlane1", m_shadowData[0].shadowFarPlane);
				shader->setUniform("ge_modelViewProjectionLight1", m_shadowData[0].lightViewProjectionMatrix * renderable.modelMatrix);

				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[1].shadowMap->depthBuffer()->bind();
				shader->setUniform("ge_shadowMap2", textureUnit++);
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[1].cubeShadowMap->depthBuffer()->bind();
				shader->setUniform("ge_shadowCubeMap2", textureUnit++);
				shader->setUniform("ge_shadowBias2", m_shadowData[1].shadowBias);
				shader->setUniform("ge_shadowFarPlane2", m_shadowData[1].shadowFarPlane);
				shader->setUniform("ge_modelViewProjectionLight2", m_shadowData[1].lightViewProjectionMatrix * renderable.modelMatrix);
#else
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[0].shadowMap->colorBuffer(FragmentBuffer::Color)->bind();
				shader->setUniform("ge_shadowMap1", textureUnit++);
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[0].cubeShadowMap->colorBuffer(FragmentBuffer::Color)->bind();
				shader->setUniform("ge_shadowCubeMap1", textureUnit++);
				shader->setUniform("ge_shadowBias1", m_shadowData[0].shadowBias);
				shader->setUniform("ge_shadowFarPlane1", m_shadowData[0].shadowFarPlane);
				shader->setUniform("ge_modelViewProjectionLight1", m_shadowData[0].lightViewProjectionMatrix * renderable.modelMatrix);

				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[1].shadowMap->colorBuffer(FragmentBuffer::Color)->bind();
				shader->setUniform("ge_shadowMap2", textureUnit++);
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				m_shadowData[1].cubeShadowMap->colorBuffer(FragmentBuffer::Color)->bind();
				shader->setUniform("ge_shadowCubeMap2", textureUnit++);
				shader->setUniform("ge_shadowBias2", m_shadowData[1].shadowBias);
				shader->setUniform("ge_shadowFarPlane2", m_shadowData[1].shadowFarPlane);
				shader->setUniform("ge_modelViewProjectionLight2", m_shadowData[1].lightViewProjectionMatrix * renderable.modelMatrix);
#endif
			}

			mesh->draw();

			if (!isShadowPass) {
				int textureUnit = firstShadowMapTextureUnit;

				glActiveTexture(GL_TEXTURE0 + (textureUnit++));  // flat
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0 + (textureUnit++));  // cube
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0 + (textureUnit++));  // flat
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0 + (textureUnit++));  // cube
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			if (material) {
				material->unbind();
			}
		}
	}

	if (overrideMaterial) {
		overrideMaterial->unbind();
	}
}

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
void Renderer::updateShadowMaps(RenderableList &renderables)
{
	Camera *storedCamera = m_camera;
	m_camera = nullptr;

	for (auto &shadowData : m_shadowData) {
		shadowData.shadowBias = 1.0f;
		shadowData.lightViewProjectionMatrix = glm::mat4{1.0f};
		shadowData.shadowFarPlane = 1.0f;
	}

	for (auto lightInfo : m_activeLights) {
		if (lightInfo.light && lightInfo.props && lightInfo.props->shadowId) {
			int shadowId = lightInfo.props->shadowId - 1;

			switch (lightInfo.light->lightType()) {
			case Light::Type::Directional:
				{
					m_shadowData[shadowId].shadowMap->bind();
					clear(kClearFlagDepth);

					DirectionalLight *light = static_cast<DirectionalLight *>(lightInfo.light);

					OrthographicCamera lightCamera{
						light->horizontal()[0], light->horizontal()[1],
						light->vertical()[0], light->vertical()[1],
						light->depth()[0], light->depth()[1]
					};
					lightCamera.setRotation(glm::quat_cast(glm::inverse(glm::lookAt(-light->direction(), glm::vec3{0.0f}, kUnitVectorY))));
					m_camera = &lightCamera;

					m_shadowData[shadowId].shadowBias = 0.005f;
					m_shadowData[shadowId].lightViewProjectionMatrix = kShadowMapBiasMatrix * lightCamera.viewProjectionMatrix();

					render(renderables, true, m_shadowMapMaterial);

					m_shadowData[shadowId].shadowMap->unbind();
					m_camera = nullptr;
				}
				break;
			case Light::Type::Point:
				{
					PointLight *light = static_cast<PointLight *>(lightInfo.light);

					m_shadowData[shadowId].cubeShadowMap->setNear(0.1f);
					// http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
					m_shadowData[shadowId].shadowFarPlane =
						light->radius() * (glm::sqrt(glm::length(light->color()) / light->cutoff()) - 1.0f);
					m_shadowData[shadowId].cubeShadowMap->setFar(m_shadowData[shadowId].shadowFarPlane);
					m_shadowData[shadowId].cubeShadowMap->setPosition(glm::vec3(lightInfo.modelMatrix[3]));
					m_shadowData[shadowId].cubeShadowMap->update(
						[this, &renderables] (PerspectiveCamera *faceCamera) {
							this->m_camera = faceCamera;
							this->clear(kClearFlagDepth);
							this->render(renderables, true, m_shadowMapMaterial);
						}
					);

					m_shadowData[shadowId].shadowBias = 0.005f;
					m_shadowData[shadowId].lightViewProjectionMatrix =
						glm::translate(glm::mat4{1.0f}, glm::vec3(-lightInfo.modelMatrix[3]));

					m_camera = nullptr;
				}
				break;
			case Light::Type::Spot:
				{
					m_shadowData[shadowId].shadowMap->bind();
					clear(kClearFlagDepth);

					SpotLight *light = static_cast<SpotLight *>(lightInfo.light);

					PerspectiveCamera lightCamera{light->angle(), 1.0f, 0.001f, 1000.0f};
					lightCamera.setTransform(lightInfo.modelMatrix);
					m_camera = &lightCamera;

					m_shadowData[shadowId].shadowBias = 0.00005f;
					m_shadowData[shadowId].lightViewProjectionMatrix = kShadowMapBiasMatrix * lightCamera.viewProjectionMatrix();

					render(renderables, true, m_shadowMapMaterial);

					m_shadowData[shadowId].shadowMap->unbind();
					m_camera = nullptr;
				}
				break;
			default:
				std::cerr << "Unhandled light type" << std::endl;
				break;
			}
		}
	}

	m_camera = storedCamera;
}
