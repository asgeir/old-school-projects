#pragma once

#include "ge2.h"

class TestApplication : public ge2::Application
{
public:
	TestApplication(int argc, char *argv[]);
	~TestApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::DebugCamera *m_camera = nullptr;
	ge2::Node        *m_cubeNode = nullptr;
	ge2::Node        *m_scene = nullptr;
	ge2::Material    *m_animatedMaterial = nullptr;
	ge2::Material    *m_cylinderAnimatedMaterial = nullptr;
	ge2::Texture2D   *m_texture = nullptr;
	ge2::MeshList     m_meshList;

	ge2::PointLight   m_whitePointLight;
	ge2::PointLight   m_redPointLight;
};
