#pragma once

#include "collidable.h"
#include "gequad.h"

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

class Quad;

class Ball : public Collidable
{
public:
	Ball();
	Ball(const Ball &other) = delete;
	~Ball();

	Ball &operator=(const Ball &rhs) = delete;

	Renderable *construct();
	void destruct();

	virtual glm::vec3 position() const override;
	virtual AABB aabb() const override;

	Renderable *renderable() { return &m_ball; }

	glm::vec3 heading() const { return m_heading; }
	void setHeading(glm::vec3 heading)
	{
		m_heading = heading;
		if (glm::dot(m_heading, m_heading)) {
			m_heading = glm::normalize(m_heading);
		}
	}

	virtual void collisionEvent(int sides, Collidable *other) override;
	void update();

private:
	Quad m_ball;
	glm::vec3 m_heading{0.7071f, 0.7071f, 0.0f};
};
