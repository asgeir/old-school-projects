#pragma once

#include "ge2gamestate.h"

namespace ge2 {

class FullscreenQuad;
class Material;
class Texture2D;

}

class WinState : public ge2::State
{
public:
	WinState();
	~WinState();

	void initialize();

	virtual void onTransitionIn(ge2::StateManager *manager, void *userdata = nullptr) override;
	virtual void *onTransitionOut() override;

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::StateManager   *m_stateManager = nullptr;
	ge2::FullscreenQuad *m_fullscreenQuad = nullptr;
	ge2::Material       *m_fullscreenQuadMaterial = nullptr;
	ge2::Texture2D      *m_winTexture = nullptr;

	float                m_timeRemaining = 0.0f;
};
