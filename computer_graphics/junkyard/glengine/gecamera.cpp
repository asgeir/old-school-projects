#include "gecamera.h"

#include <glm/gtc/matrix_transform.hpp>

void Camera::constructOrthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
	m_projection = glm::ortho(left, right, bottom, top, zNear, zFar);
}

glm::mat4 Camera::viewMatrix() const
{
	if (m_type == Camera::Orthographic) {
		return glm::mat4{1.0f};
	}
//	else {
//		return glm::inverse(transform());
//	}
	return glm::mat4{1.0f};
}

glm::mat4 Camera::viewProjectionMatrix() const
{
	return m_projection * viewMatrix();
}
