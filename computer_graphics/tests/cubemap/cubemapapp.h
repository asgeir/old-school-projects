#pragma once

#include "ge2.h"

class CubemapApplication : public ge2::Application
{
public:
	CubemapApplication(int argc, char *argv[]);
	~CubemapApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::DebugCamera     *m_camera = nullptr;
	ge2::Node            *m_scene = nullptr;
	ge2::Node            *m_rotationNode = nullptr;
	ge2::Node            *m_reflectiveNode = nullptr;

	ge2::CubeFramebuffer *m_environmentMap = nullptr;
	ge2::Mesh            *m_reflectiveRefractiveSphere = nullptr;
	ge2::Material        *m_reflectiveMaterial = nullptr;
	ge2::Material        *m_refractiveMaterial = nullptr;

	ge2::Compositor           *m_compositor = nullptr;
	ge2::CompositorEffectList  m_compositorEffects;
	ge2::AntiAliasingEffect   *m_antiAliasingEffect = nullptr;
	ge2::BloomEffect          *m_bloomEffect = nullptr;
	ge2::TonemapEffect        *m_tonemapEffect = nullptr;
};
