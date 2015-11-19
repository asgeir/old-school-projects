#pragma once

#include "collidable.h"
#include "gequad.h"

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

class Quad;

class Brick : public Collidable
{
public:
	Brick();
	Brick(const Brick &other) = delete;
	~Brick();

	Brick &operator=(const Brick &rhs) = delete;

	Renderable *construct(int x, int y);
	void destruct();

	virtual glm::vec3 position() const override;
	virtual AABB aabb() const override;

	Renderable *renderable() { return &m_brick; }
	bool visible() const { return m_visible; }

	virtual void collisionEvent(int sides, Collidable *other) override;

private:
	Quad m_brick;
	bool m_visible = true;
};
