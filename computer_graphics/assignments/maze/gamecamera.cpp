#include "gamecamera.h"

#include "ge2camera.h"
#include "ge2common.h"
#include "ge2renderer.h"
#include "ge2time.h"

#include "SDL_mouse.h"

using namespace ge2;

GameCamera::GameCamera()
{
	m_root = new Node;

	m_camera = new PerspectiveCamera(degToRad(40), geRenderer->width(), geRenderer->height(), 0.1f, 1000.0f);
	m_root->addChild(m_camera);

	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_SetWindowGrab(geRenderer->window(), SDL_TRUE);
}

GameCamera::~GameCamera()
{
	delete m_root;

	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_SetWindowGrab(geRenderer->window(), SDL_FALSE);
}

glm::vec3 GameCamera::position() const
{
	return m_camera->position();
}

void GameCamera::setPosition(const glm::vec3 &position)
{
	m_camera->setPosition(position);
}

void GameCamera::handleEvent(const SDL_Event &event)
{
}

void GameCamera::update()
{
	int mouseDeltaX = 0;
	int mouseDeltaY = 0;
	SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
	if (m_firstEventIgnored && (mouseDeltaX || mouseDeltaY)) {
		m_azimuth  -= (float)mouseDeltaX * (GE_PI / 16.0f) * Time::deltaTime();
		m_altitude -= (float)mouseDeltaY * (GE_PI / 16.0f) * Time::deltaTime();
	}
	m_firstEventIgnored = true;

	while (m_azimuth > ( 2 * GE_PI)) m_azimuth -= 2 * GE_PI;
	while (m_azimuth < (-2 * GE_PI)) m_azimuth += 2 * GE_PI;
	if (m_altitude >  degToRad(89.0f)) { m_altitude =  degToRad(89.0f); }
	if (m_altitude < -degToRad(89.0f)) { m_altitude = -degToRad(89.0f); }

	// The camera has -z as the forward vector but the spherical coordinates
	// have +x as the forward vector. By pretending that azimuth is pi/2 higher
	// than it is we get +x == -z
	float adjustedAzimuth = m_azimuth + (GE_PI / 2.0f);
	glm::vec3 forward{
		cos(m_altitude) * cos(adjustedAzimuth),
		sin(m_altitude),
		cos(m_altitude) * sin(adjustedAzimuth) * -1.0f
	};

	glm::vec3 right{
		sin(adjustedAzimuth),
		0,
		cos(adjustedAzimuth)
	};

	glm::vec3 up = glm::cross(right, forward);
	glm::mat4 lookAt = glm::transpose(glm::mat4{glm::vec4{right, 0.0f}, glm::vec4{up, 0.0f}, glm::vec4{-forward, 0.0f}, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}});

	m_camera->setRotation(glm::quat_cast(glm::inverse(lookAt)));

	const Uint8 *state = SDL_GetKeyboardState(NULL);

	glm::vec3 move = glm::vec3{0.0f};
	if (state[SDL_SCANCODE_W]) {
		move += forward;
	} else if (state[SDL_SCANCODE_S]) {
		move -= forward;
	}
	if (state[SDL_SCANCODE_A]) {
		move -= right;
	} else if (state[SDL_SCANCODE_D]) {
		move += right;
	}
	if (m_verticalMovementAllowed && state[SDL_SCANCODE_Q]) {
		move += up;
	} else if (m_verticalMovementAllowed && state[SDL_SCANCODE_E]) {
		move -= up;
	}

	if (m_collisionFlags & (uint32_t)CollisionFlags::North && move[2] < 0.0f) {
		move[2] = 0.0f;
	}
	if (m_collisionFlags & (uint32_t)CollisionFlags::South && move[2] > 0.0f) {
		move[2] = 0.0f;
	}
	if (m_collisionFlags & (uint32_t)CollisionFlags::East && move[0] > 0.0f) {
		move[0] = 0.0f;
	}
	if (m_collisionFlags & (uint32_t)CollisionFlags::West && move[0] < 0.0f) {
		move[0] = 0.0f;
	}

	if (!m_verticalMovementAllowed) {
		move[1] = 0.0f;
	}

	if (glm::dot(move, move)) {
		m_camera->setPosition(m_camera->position() + glm::normalize(move) * m_cameraSpeed * Time::deltaTime());
	}

	m_collisionFlags = 0;
}
