#pragma once

#include "state.h"

#include "gequad.h"

class EndState : public State
{
public:
	EndState();
	EndState(const EndState &other) = delete;
	virtual ~EndState();

	EndState &operator=(const EndState &rhs) = delete;

	virtual void onTransitionIn(StateManager *manager, void *userdata = nullptr) override;
	virtual void *onTransitionOut() override;
	virtual void update() override;

private:
	Quad m_endScreen;
	StateManager *m_manager = nullptr;

	float m_timeRemaining = 0.0f;
};
