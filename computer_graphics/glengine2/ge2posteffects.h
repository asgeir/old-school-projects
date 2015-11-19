#pragma once

#include "ge2compositor.h"

namespace ge2 {

class Camera;
class Node;

class AntiAliasingEffect : public CompositorEffect
{
public:
	AntiAliasingEffect();

	virtual void compose(Compositor *compositor, bool firstInChain) override;

private:
	Material *m_antiAliasingFilter = nullptr;
};

class BloomEffect : public CompositorEffect
{
public:
	BloomEffect(glm::vec3 whitePoint = glm::vec3{1.0f});

	glm::vec3 whitePoint() const { return m_whitePoint; }
	void setWhitePoint(glm::vec3 whitePoint) { m_whitePoint = whitePoint; }

	virtual void compose(Compositor *compositor, bool firstInChain) override;

private:
	glm::vec3  m_whitePoint;
	Material  *m_highPassFilter = nullptr;
};

class CrepuscularRaysEffect : public CompositorEffect
{
public:
	CrepuscularRaysEffect();

	Camera *camera() const { return m_camera; }
	float decay() const { return m_decay; }
	float density() const { return m_density; }
	float exposure() const { return m_exposure; }
	Node *lightNode() const { return m_lightNode; }
	Texture2D *lightStencil() const { return m_lightStencil; }
	float weight() const { return m_weight; }

	void setCamera(Camera *camera) { m_camera = camera; }
	void setDecay(float decay) { m_decay = decay; }
	void setDensity(float density) { m_density = density; }
	void setExposure(float exposure) { m_exposure = exposure; }
	void setLightNode(Node *node) { m_lightNode = node; }
	void setLightStencil(Texture2D *stencil) { m_lightStencil = stencil; }
	void setWeight(float weight) { m_weight = weight; }

	virtual void compose(Compositor *compositor, bool firstInChain) override;

private:
	Camera    *m_camera = nullptr;
	Node      *m_lightNode = nullptr;
	Texture2D *m_lightStencil = nullptr;

	float m_decay = 0.93f;
	float m_density = 0.96f;
	float m_exposure = 0.6f;
	float m_weight = 0.4f;
};

class GlowEffect : public CompositorEffect
{
public:
	GlowEffect();

	virtual void compose(Compositor *compositor, bool firstInChain) override;
};

#ifdef __APPLE__
constexpr float GE_PLATFORM_OUTPUT_GAMMA = 1.8f;
#else
constexpr float GE_PLATFORM_OUTPUT_GAMMA = 2.2f;
#endif

class TonemapEffect : public CompositorEffect
{
public:
	TonemapEffect(float exposure = 1.0f, float outputGamma = GE_PLATFORM_OUTPUT_GAMMA, glm::vec3 whitePoint = glm::vec3{1.0f});

	float exposure() const { return m_exposure; }
	float outputGamma() const { return m_outputGamma; }
	glm::vec3 whitePoint() const { return m_whitePoint; }

	void setExposure(float exposure) { m_exposure = exposure; }
	void setOutputGamma(float outputGamma) { m_outputGamma = outputGamma; }
	void setWhitePoint(glm::vec3 whitePoint) { m_whitePoint = whitePoint; }

	virtual void compose(Compositor *compositor, bool firstInChain) override;

private:
	float     m_exposure;
	float     m_outputGamma;
	glm::vec3 m_whitePoint;
	Material *m_tonemapEffect = nullptr;
};

} // namespace ge2
