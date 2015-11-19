#pragma once

#include <glm/glm.hpp>

class Node
{
public:
	virtual ~Node() {}

	glm::mat4 &transform() { return transformMatrix; }
	const glm::mat4 &transform() const { return transformMatrix; }
	void setTransform(glm::mat4 matrix) { transformMatrix = matrix; }

private:
	glm::mat4 transformMatrix{1.0f};
};
