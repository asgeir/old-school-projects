#pragma once

#include "gamestate.h"
#include "winstate.h"

#include "ge2application.h"
#include "ge2gamestate.h"

class MazeApplication : public ge2::Application
{
public:
	MazeApplication(int argc, char *argv[]);
	~MazeApplication();

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	ge2::StateManager m_stateManager;
	GameState         m_gameState;
	WinState          m_winState;
};
