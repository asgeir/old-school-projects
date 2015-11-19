#include "end_state.h"
#include "common.h"

#include "getime.h"

#include <glm/gtc/matrix_transform.hpp>

namespace {

const float kEndScreenDuration = 2.0f;

}

EndState::EndState()
{
}

EndState::~EndState()
{
}

void EndState::onTransitionIn(StateManager *manager, void *userdata)
{
	bool userWon = static_cast<bool>(userdata);
	m_manager = manager;

	if (userWon) {
		m_endScreen.construct(kWindowWidth, kWindowHeight, glm::vec3(0.0f, 1.0f, 0.0f));
	} else {
		m_endScreen.construct(kWindowWidth, kWindowHeight, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	m_endScreen.setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(kWindowWidth / 2.0f, kWindowHeight / 2.0f, 0.0f)));

	m_timeRemaining = kEndScreenDuration;
}

void *EndState::onTransitionOut()
{
	return nullptr;
}

void EndState::update()
{
	if (m_timeRemaining <= 0.0f) {
		m_manager->transition();
		return;
	}

	m_timeRemaining -= Time::deltaTime();

	RenderList renderables{&m_endScreen};
	renderer.render(renderables, &basicRenderState);
}
