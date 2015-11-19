#include "end_state.h"
#include "game_state.h"

#include "SDL_events.h"

#include <cstdlib>
#include <ctime>

extern const char * const kApplicationTitle = "Breakout";
extern const int kWindowWidth = 800;
extern const int kWindowHeight = 600;

namespace {

StateManager stateMgr;
GameState gameState;
EndState endState;

} // namespace

void geInitApplication()
{
	srand(time(nullptr));

	initializeCommon();

	gameState.setNextState(&endState);
	endState.setNextState(&gameState);

	stateMgr.start(&gameState);
}

void geHandleEvent(const SDL_Event &event)
{
}

void geDisplayFrame()
{
	stateMgr.update();
}
