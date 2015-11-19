#include "wall.h"
#include "collisionmanager.h"

namespace {

const int kRowCount	= 18;
const int kColumnCount = 10;
const int kNumberOfBricks = kRowCount * kColumnCount;

}

Wall::Wall()
	: m_bricks(kNumberOfBricks)
{
}

void Wall::construct(CollisionManager *collisionManager)
{
	m_collisionManager = collisionManager;
	for (int i = 0; i < kNumberOfBricks; ++i) {
		m_bricks[i].construct(i % 10, i / 10);
		collisionManager->addPassiveCollider(&m_bricks[i]);
	}
}

RenderList Wall::visibleBricks()
{
	RenderList renderList;
	for (int i = 0, e = m_bricks.size(); i < e; ++i) {
		if (m_bricks[i].visible()) {
			renderList.push_back(m_bricks[i].renderable());
		} else if (m_collisionManager) {
			m_collisionManager->removePassiveCollider(&m_bricks[i]);
		}
	}

	return renderList;
}
