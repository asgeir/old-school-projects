#include "paddle.h"
#include "ball.h"
#include "gecommon.h"
#include "getime.h"

#include "SDL.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace {

const float kPaddleWidth = 75.0f;
const float kPaddleHeight = 15.0f;
const float kPaddleSpeed = 800.0f;
const float kPaddleToBallHorizontalFactor = 0.2;

}

Paddle::Paddle()
	: m_paddle{}
	, m_direction{0.0f}
{
}

Paddle::~Paddle()
{
}

Renderable *Paddle::construct()
{
	m_paddle.construct(kPaddleWidth, kPaddleHeight, glm::vec3{0.0f, 0.0f, 1.0f});
	m_paddle.setTransform(glm::translate(glm::vec3((kWindowWidth/2), 3 * kPaddleHeight, 0)));
	m_direction = glm::vec3{0.0f};

	return &m_paddle;
}

void Paddle::destruct()
{
	m_paddle.destruct();
}

glm::vec3 Paddle::ballAnchor()
{
	return glm::vec3(m_paddle.transform() * glm::vec4(glm::vec3(0.0f, 1.5f * kPaddleHeight, 0.0f), 1.0f));
}

glm::vec3 Paddle::position() const
{
	return glm::vec3(m_paddle.transform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

AABB Paddle::aabb() const
{
	return AABB { position(), kPaddleWidth, kPaddleHeight };
}

void Paddle::collisionEvent(int sides, Collidable *other)
{
	if (sides & kCollisionFlagTop) {
		Ball *ball = static_cast<Ball *>(other);
		ball->setHeading(ball->heading() + (m_direction * kPaddleToBallHorizontalFactor));
	}
}

void Paddle::update()
{
	float deltaTime = Time::deltaTime();

    const Uint8 *state = SDL_GetKeyboardState(NULL);

	m_direction = glm::vec3{0.0f};
    if (state[SDL_SCANCODE_LEFT]) {
        m_direction -= kUnitVectorX;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        m_direction += kUnitVectorX;
    }

    if (glm::dot(m_direction, m_direction)) {
        m_direction = glm::normalize(m_direction);

		glm::vec4 &pos = m_paddle.transform()[3];
        if (pos.x <= (kPaddleWidth/2.0f) && m_direction.x < 0){
            m_direction.x = 0;
            pos.x = (kPaddleWidth/2.0f);
        }
        if (pos.x >= (kWindowWidth-(kPaddleWidth/2.0f)) && m_direction.x > 0) {
            m_direction.x = 0;
            pos.x = (kWindowWidth-(kPaddleWidth/2.0f));
		}

        if (glm::dot(m_direction, m_direction)) {
			m_paddle.setTransform(glm::translate(m_paddle.transform(), glm::normalize(m_direction) * kPaddleSpeed * deltaTime));
        }
    }
}
