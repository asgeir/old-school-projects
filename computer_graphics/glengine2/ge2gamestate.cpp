#include "ge2gamestate.h"

using namespace ge2;

State::State()
{
}

State::~State()
{
}

StateManager::StateManager()
{
}

StateManager::~StateManager()
{
}

void StateManager::start(State *state, void *userdata)
{
	if (m_currentState) {
		m_currentState->onTransitionOut();
	}
	m_currentState = state;
	m_currentState->onTransitionIn(this, userdata);
}

void StateManager::transition()
{
	if (m_currentState && m_currentState->nextState()) {
		void *userdata = m_currentState->onTransitionOut();
		m_currentState = m_currentState->nextState();
		m_currentState->onTransitionIn(this, userdata);
	}
}

void StateManager::handleEvent(const SDL_Event &event)
{
	if (m_currentState) {
		m_currentState->handleEvent(event);
	}
}

void StateManager::update()
{
	if (m_currentState) {
		m_currentState->update();
	}
}
