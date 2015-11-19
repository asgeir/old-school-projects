#pragma once

#include "brick.h"

#include <vector>

class CollisionManager;

typedef std::vector<Brick> BrickList;

class Wall
{
public:
	Wall();

	void construct(CollisionManager *collisionManager);

	BrickList &bricks() { return m_bricks; }
	RenderList visibleBricks();

private:
	BrickList m_bricks;
	CollisionManager *m_collisionManager = nullptr;
};
