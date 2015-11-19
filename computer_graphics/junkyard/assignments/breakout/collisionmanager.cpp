#include "collisionmanager.h"

#include <algorithm>

CollisionManager::CollisionManager()
{
}

void CollisionManager::setActiveCollider(Collidable *collider)
{
	m_activeCollider = collider;
}

void CollisionManager::addPassiveCollider(Collidable *collider)
{
	m_passiveColliders.push_back(collider);
}

void CollisionManager::removePassiveCollider(Collidable *collider)
{
	std::remove(m_passiveColliders.begin(), m_passiveColliders.end(), collider);
}

void CollisionManager::removeAllPassiveColliders()
{
	m_passiveColliders.clear();
}

void CollisionManager::update()
{
	if (!m_activeCollider) {
		return;
	}

	AABB activeAABB = m_activeCollider->aabb();
	for (auto collider : m_passiveColliders) {
		int sides = testIntersection(activeAABB, collider->aabb()) & kCollisionFlagMask;
		if (sides) {
			m_activeCollider->collisionEvent(sides, collider);
			collider->collisionEvent(sides ^ kCollisionFlagMask, m_activeCollider);
		}
	}
}

int CollisionManager::testIntersection(const AABB &a, const AABB &b)
{
	float halfHeightA = a.height / 2.0f;
	float halfWidthA = a.width / 2.0f;

	float halfHeightB = b.height / 2.0f;
	float halfWidthB = b.width / 2.0f;

	float minXA = a.center[0] - halfWidthA;
	float maxXA = a.center[0] + halfWidthA;
	float minYA = a.center[1] - halfHeightA;
	float maxYA = a.center[1] + halfHeightA;

	float minXB = b.center[0] - halfWidthB;
	float maxXB = b.center[0] + halfWidthB;
	float minYB = b.center[1] - halfHeightB;
	float maxYB = b.center[1] + halfHeightB;

	if (maxXA < minXB) return 0; // a is left of b
	if (minXA > maxXB) return 0; // a is right of b
	if (maxYA < minYB) return 0; // a is above b
	if (minYA > maxYB) return 0; // a is below b

	int sides = 0;

	if (maxYA > minYB && minYA < minYB) sides |= kCollisionFlagTop;
	if (maxYA > maxYB && minYA < maxYB) sides |= kCollisionFlagBottom;
	if (maxXA > minXB && minXA < minXB) sides |= kCollisionFlagRight;
	if (maxXA > maxXB && minXA < maxXB) sides |= kCollisionFlagLeft;

	if (!sides) sides = kCollisionFlagMask;

	return sides;
}
