#pragma once

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

extern const glm::vec3 kUnitVectorX;
extern const glm::vec3 kUnitVectorY;
extern const glm::vec3 kUnitVectorZ;

extern const int kWindowWidth;
extern const int kWindowHeight;

struct ProgramIndices
{
	GLint vertexIndex = -1;
	GLint colorIndex = -1;
	GLint textureIndex = -1;
	GLint uvIndex = -1;
	GLint modelViewProjectionIndex = -1;
};
