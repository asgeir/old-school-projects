#pragma once

#include "SDL_events.h"

namespace ge2 {

class StateManager;

class State
{
public:
	State();
	State(const State &other) = delete;
	virtual ~State();

	State &operator=(const State &rhs) = delete;

	State *nextState() const { return m_nextState; }
	void setNextState(State *state) { m_nextState = state; }

	virtual void onTransitionIn(StateManager *manager, void *userdata = nullptr) {}
	virtual void *onTransitionOut() { return nullptr; }

	virtual void handleEvent(const SDL_Event &event) {}
	virtual void update() {}

private:
	State *m_nextState = nullptr;
};

class StateManager
{
public:
	StateManager();
	StateManager(const StateManager &other) = delete;
	~StateManager();

	StateManager &operator=(const State &rhs) = delete;

	void start(State *state, void *userdata = nullptr);
	void transition();

	void handleEvent(const SDL_Event &event);
	void update();

private:
	State *m_currentState = nullptr;
};

} // namespace ge2
