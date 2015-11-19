#pragma once

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

#define GE_PI 3.14159265f

extern const glm::vec3 kUnitVectorX;
extern const glm::vec3 kUnitVectorY;
extern const glm::vec3 kUnitVectorZ;

extern const char * const kApplicationTitle;
extern const int kWindowWidth;
extern const int kWindowHeight;

constexpr inline GLubyte *bufferOffset(size_t bytes)
{
	return static_cast<GLubyte *>(0) + bytes;
}

constexpr inline float degToRad(float angle)
{
	return angle * GE_PI / 180;
}

constexpr inline float radToDeg(float angle)
{
	return angle * 180 / GE_PI;
}
