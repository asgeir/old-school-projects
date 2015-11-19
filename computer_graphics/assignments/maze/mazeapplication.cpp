#include "mazeapplication.h"

#include "ge2common.h"
#include "ge2renderer.h"
#include "ge2resourcemgr.h"

#include <cstdlib>
#include <ctime>

using namespace ge2;

MazeApplication::MazeApplication(int argc, char *argv[])
{
	srand(time(nullptr));
	geRenderer->setTitle("Maze");

	geResourceMgr->setAssetDirectory("../assets");

	if (argc >= 2) {
		geResourceMgr->setAssetDirectory(argv[1]);
	}

	m_gameState.initialize();
	m_winState.initialize();

	m_gameState.setNextState(&m_winState);
	m_winState.setNextState(&m_gameState);

	m_stateManager.start(&m_gameState);
}

MazeApplication::~MazeApplication()
{
}

void MazeApplication::handleEvent(const SDL_Event &event)
{
	m_stateManager.handleEvent(event);
}

void MazeApplication::update()
{
	m_stateManager.update();
}
