#pragma once

#include "ge2.h"

class GlowEffect;

class ShadowApplication : public ge2::Application
{
public:
	ShadowApplication(int argc, char *argv[]);
	~ShadowApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::DebugCamera *m_camera = nullptr;
	ge2::Node        *m_scene = nullptr;

	ge2::DirectionalLight *m_directionalLight = nullptr;
	ge2::SpotLight        *m_spotLight = nullptr;
	ge2::PointLight       *m_pointLight = nullptr;
};
