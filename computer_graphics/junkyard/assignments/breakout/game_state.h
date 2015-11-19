#pragma once

#include "ball.h"
#include "collisionmanager.h"
#include "common.h"
#include "paddle.h"
#include "state.h"
#include "wall.h"

class GameState : public State
{
public:
	GameState();
	GameState(const GameState &other) = delete;
	virtual ~GameState();

	GameState &operator=(const GameState &rhs) = delete;

	virtual void onTransitionIn(StateManager *manager, void *userdata = nullptr) override;
	virtual void *onTransitionOut() override;
	virtual void update() override;

private:
	CollisionManager m_collisionManager;
	Wall m_bricks;
	Paddle m_paddle;
	Ball m_ball;

	RenderList m_basicRenderables;
	RenderList m_ballRenderables;

	bool m_userWon = false;
	StateManager *m_stateManager = nullptr;
};
