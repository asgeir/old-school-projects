#pragma once

#include "ge2camera.h"

#include "SDL_events.h"

namespace ge2 {

class PerspectiveCamera;
class Node;

class DebugCamera
{
public:
	DebugCamera();
	~DebugCamera();

	PerspectiveCamera *camera() const { return m_camera; }

	float azimuth() const { return m_azimuth; }
	float altitude() const { return m_altitude; }
	glm::vec3 position() const;
	float speed() const { return m_cameraSpeed; }

	void setAzimuth(float azimuth) { m_azimuth = azimuth; }
	void setAltitude(float altitude) { m_altitude = altitude; }
	void setPosition(const glm::vec3 &position);
	void setSpeed(float speed) { m_cameraSpeed = speed; }

	void handleEvent(const SDL_Event &event);
	void update();

private:
	PerspectiveCamera *m_camera = nullptr;
	float              m_cameraSpeed = 10.0f;
	Node              *m_root = nullptr;

	float              m_azimuth = 0.0f;
	float              m_altitude = 0.0f;
	bool               m_firstEventIgnored = false;
};

} // namespace ge2
