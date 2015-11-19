#pragma once

#include "genode.h"

class Camera : public Node
{
public:
	enum CameraType
	{
		Orthographic
	};

	void constructOrthographic(float left, float right, float bottom, float top, float zNear, float zFar);

	glm::mat4 viewMatrix() const;
	glm::mat4 viewProjectionMatrix() const;

private:
	CameraType m_type = Orthographic;
	glm::mat4 m_projection{1.0f};
};
