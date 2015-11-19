#include "game_state.h"
#include "common.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>

GameState::GameState()
{
}

GameState::~GameState()
{
}

void GameState::onTransitionIn(StateManager *manager, void *userdata)
{
	(void)userdata;
	m_stateManager = manager;

	m_basicRenderables.push_back(m_paddle.construct());
	m_ballRenderables.push_back(m_ball.construct());

	m_collisionManager.setActiveCollider(&m_ball);
	m_collisionManager.addPassiveCollider(&m_paddle);

	m_bricks.construct(&m_collisionManager);

	float angle = (-GE_PI/4.0f) + ((GE_PI/2.0f) * rand()) / (RAND_MAX + 1.0f);
	m_ball.setHeading(glm::vec3(glm::rotate(glm::mat4(1.0f), angle, kUnitVectorZ) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

	m_userWon = false;
}

void *GameState::onTransitionOut()
{
	m_basicRenderables.clear();
	m_ballRenderables.clear();

	m_collisionManager.setActiveCollider(nullptr);
	m_collisionManager.removeAllPassiveColliders();

	return reinterpret_cast<void *>(m_userWon);
}

void GameState::update()
{
	m_collisionManager.update();

	m_paddle.update();
	m_ball.update();

	if ((m_ball.position()[1] + 25) < m_paddle.position()[1]) {
		m_stateManager->transition();
		return;
	}

	renderer.render(m_ballRenderables, &ballRenderState);
	renderer.render(m_basicRenderables, &basicRenderState);

	RenderList visibleBricks = m_bricks.visibleBricks();
	renderer.render(visibleBricks, &basicRenderState);

	if (visibleBricks.empty()) {
		m_userWon = true;
		m_stateManager->transition();
		return;
	}
}
