#pragma once

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

#include <vector>
#include <string>

#define GE_PI 3.14159265f

namespace ge2 {

class Mesh;
class Renderer;
class ResourceManager;

extern Renderer *geRenderer;
extern ResourceManager *geResourceMgr;

extern const glm::vec3 kUnitVectorX;
extern const glm::vec3 kUnitVectorY;
extern const glm::vec3 kUnitVectorZ;

typedef std::vector<std::string> StringList;
typedef std::vector<Mesh *> MeshList;

enum class ShaderAttribute : GLint
{
	VertexPosition,
	VertexTextureCoordinates,
	VertexNormals
};

enum class FragmentBuffer : GLint
{
	Color,
	Glow,
	CrepuscularRays,

	NumBuffers
};

enum class StandardUniformBlocks : GLint
{
	Lights,

	User
};

enum class CubeDirection : GLint
{
	PositiveX,
	NegativeX,
	PositiveY,
	NegativeY,
	PositiveZ,
	NegativeZ,

	NumDirections
};

enum {
	kTextureColor         = (1 << 0),
	kTextureDepth         = (1 << 1),
	kTextureFloatingPoint = (1 << 2),
	kTextureMipMapped     = (1 << 3),
	kTextureClampUvs      = (1 << 4),
	kTextureFlagsMask     = kTextureColor|kTextureDepth|kTextureFloatingPoint|kTextureMipMapped|kTextureClampUvs
};

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

} // namespace ge2
