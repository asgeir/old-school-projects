#pragma once

#include "ge2.h"

class CrepuscularApplication : public ge2::Application
{
public:
	CrepuscularApplication(int argc, char *argv[]);
	~CrepuscularApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::DebugCamera           *m_camera = nullptr;
	ge2::Node                  *m_scene = nullptr;
	ge2::Node                  *m_blueLightSource = nullptr;
	ge2::Node                  *m_orangeLightSource = nullptr;

	ge2::Compositor            *m_compositor = nullptr;
	ge2::CompositorEffectList   m_compositorEffects;
	ge2::AntiAliasingEffect    *m_antiAliasingEffect = nullptr;
	ge2::CrepuscularRaysEffect *m_blueCrepuscularRaysEffect = nullptr;
	ge2::CrepuscularRaysEffect *m_orangeCrepuscularRaysEffect = nullptr;
	ge2::TonemapEffect         *m_tonemapEffect = nullptr;

	ge2::Framebuffer           *m_blueLightFramebuffer = nullptr;
	ge2::Framebuffer           *m_orangeLightFramebuffer = nullptr;
};
