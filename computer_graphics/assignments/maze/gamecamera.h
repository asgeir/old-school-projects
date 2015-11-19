#pragma once

#include "SDL_events.h"

#include <glm/glm.hpp>

namespace ge2 {

class PerspectiveCamera;
class Node;

}

enum class CollisionFlags
{
	North = 1 << 0,
	West  = 1 << 1,
	South = 1 << 2,
	East  = 1 << 3
};

class GameCamera
{
public:
	GameCamera();
	~GameCamera();

	ge2::PerspectiveCamera *camera() const { return m_camera; }

	float     azimuth() const { return m_azimuth; }
	float     altitude() const { return m_altitude; }
	uint32_t  collisionFlags() const { return m_collisionFlags; }
	glm::vec3 position() const;
	float     speed() const { return m_cameraSpeed; }
	bool      verticalMovementAllowed() const { return m_verticalMovementAllowed; }

	void setAzimuth(float azimuth) { m_azimuth = azimuth; }
	void setAltitude(float altitude) { m_altitude = altitude; }
	void setCollisionFlags(uint32_t flags) { m_collisionFlags = flags; }
	void setPosition(const glm::vec3 &position);
	void setSpeed(float speed) { m_cameraSpeed = speed; }
	void setVerticalMovementAllowed(bool allowed) { m_verticalMovementAllowed = allowed; }

	void handleEvent(const SDL_Event &event);
	void update();

private:
	ge2::PerspectiveCamera *m_camera = nullptr;
	float                   m_cameraSpeed = 30.0f;
	bool                    m_verticalMovementAllowed = false;
	ge2::Node              *m_root = nullptr;

	float                   m_azimuth = 0.0f;
	float                   m_altitude = 0.0f;
	uint32_t                m_collisionFlags = 0;
	bool                    m_firstEventIgnored = false;
};
