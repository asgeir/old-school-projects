#pragma once

#include "collidable.h"
#include "gequad.h"

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

class Quad;

class Paddle : public Collidable
{
public:
	Paddle();
	Paddle(const Paddle &other) = delete;
	~Paddle();

	Paddle &operator=(const Paddle &rhs) = delete;

	Renderable *construct();
	void destruct();

	glm::vec3 ballAnchor();

	virtual glm::vec3 position() const override;
	virtual AABB aabb() const override;

	virtual void collisionEvent(int sides, Collidable *other) override;
	void update();

private:
	Quad m_paddle;
	glm::vec3 m_direction;
};
