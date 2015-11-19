#include "ball.h"
#include "gecommon.h"
#include "getime.h"
#include "gequad.h"

#include <glm/gtx/transform.hpp>

namespace {

const float kBallDiameter = 15.0f;
const float kBallSpeed = 150.0f;

}

Ball::Ball()
{
}

Ball::~Ball()
{
}

Renderable *Ball::construct()
{
	m_ball.construct(kBallDiameter, kBallDiameter, glm::vec3{0.0f, 1.0f, 0.0f});
	m_ball.setTransform(glm::translate(glm::vec3(400.0f, 80.0f, 0.0f)));
	return &m_ball;
}

void Ball::destruct()
{
	m_ball.destruct();
}

glm::vec3 Ball::position() const
{
	return glm::vec3(m_ball.transform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

AABB Ball::aabb() const
{
	return AABB { position(), kBallDiameter, kBallDiameter };
}

void Ball::collisionEvent(int sides, Collidable *other)
{
	if (((sides & kCollisionFlagTop) && (m_heading[1] > 0)) || ((sides & kCollisionFlagBottom) && (m_heading[1] < 0))) {
		m_heading[1] = m_heading[1] * -1;
	} else if (((sides & kCollisionFlagLeft) && (m_heading[0] < 0)) || ((sides && kCollisionFlagRight) && (m_heading[0] > 0))) {
		m_heading[0] = m_heading[0] * -1;
	}
}

void Ball::update()
{
	if (glm::dot(m_heading, m_heading) == 0) {
		return;
	}

	if (((position()[0] < (kBallDiameter / 2.0)) && m_heading[0] < 0) || ((position()[0] > (kWindowWidth - (kBallDiameter / 2.0))) && m_heading[0] > 0)) {
		m_heading[0] = m_heading[0] * -1;
	}
	if (((position()[1] < (kBallDiameter / 2.0)) && m_heading[1] < 0) || ((position()[1] > (kWindowHeight - (kBallDiameter / 2.0))) && m_heading[1] > 0)) {
		m_heading[1] = m_heading[1] * -1;
	}

	m_ball.setTransform(glm::translate(m_ball.transform(), m_heading * kBallSpeed * Time::deltaTime()));
}
