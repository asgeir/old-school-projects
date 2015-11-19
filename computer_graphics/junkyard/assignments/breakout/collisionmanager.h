#pragma once

#include "collidable.h"

#include <vector>

typedef std::vector<Collidable *> CollidableList;

class CollisionManager
{
public:
	CollisionManager();

	void setActiveCollider(Collidable *collider);

	void addPassiveCollider(Collidable *collider);
	void removePassiveCollider(Collidable *collider);
	void removeAllPassiveColliders();

	void update();

private:
	int testIntersection(const AABB &a, const AABB &b);

	Collidable *m_activeCollider = nullptr;
	CollidableList m_passiveColliders;
};
