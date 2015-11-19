#include "brick.h"
#include "gequad.h"
#include "gecommon.h"

#include <glm/gtx/transform.hpp>

namespace {

const float kBrickWidth = 75.0f;
const float kBrickHeight = 15.0f;
const float kBrickMargin = 14.0f + (kBrickWidth / 2);
const float kBrickPadding = 2.5f;

const float kBrickBaseLine = 15 * kBrickHeight;

}

Brick::Brick()
{
}

Brick::~Brick()
{
}

Renderable *Brick::construct(int x, int y)
{
	m_brick.construct(kBrickWidth, kBrickHeight, glm::vec3{1.0f, 0.0f, 0.0f});
	m_brick.setTransform(glm::translate(glm::vec3(kBrickMargin + x * (kBrickWidth + kBrickPadding), kBrickBaseLine + y * (kBrickHeight + kBrickPadding), 0)));
	m_visible = true;
	return &m_brick;
}

void Brick::destruct()
{
	m_brick.destruct();
	m_visible = false;
}

glm::vec3 Brick::position() const
{
	return glm::vec3(m_brick.transform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

AABB Brick::aabb() const
{
	return AABB { position(), kBrickWidth, kBrickHeight };
}

void Brick::collisionEvent(int sides, Collidable *other)
{
	m_visible = false;
}
