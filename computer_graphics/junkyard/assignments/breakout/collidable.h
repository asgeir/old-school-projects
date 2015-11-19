#pragma once

#include <glm/glm.hpp>

enum
{
	kCollisionFlagTop    = 0x1,
	kCollisionFlagLeft   = 0x2,
	kCollisionFlagBottom = 0x4,
	kCollisionFlagRight  = 0x8,
	kCollisionFlagMask   = 0xf
};

struct AABB
{
	glm::vec3 center;
	float width;
	float height;
};

class Collidable
{
public:
	virtual ~Collidable() {}

	virtual glm::vec3 position() const = 0;
	virtual AABB aabb() const = 0;

	virtual void collisionEvent(int sides, Collidable *other) {}
};
