#pragma once

#include "ge2.h"

class GlowEffect;

class CompositorApplication : public ge2::Application
{
public:
	CompositorApplication(int argc, char *argv[]);
	~CompositorApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::DebugCamera          *m_camera = nullptr;
	ge2::Node                 *m_scene = nullptr;
	ge2::Node                 *m_cubeNode = nullptr;

	ge2::Compositor           *m_compositor = nullptr;
	ge2::CompositorEffectList  m_compositorEffects;
	ge2::AntiAliasingEffect   *m_antiAliasingEffect = nullptr;
	ge2::BloomEffect          *m_bloomEffect = nullptr;
	ge2::GlowEffect           *m_glowEffect = nullptr;
	ge2::TonemapEffect        *m_tonemapEffect = nullptr;
};
