#include "ge2cubemap.h"

using namespace ge2;

Cubemap::Cubemap()
{
}

Cubemap::~Cubemap()
{
	destruct();
}

void Cubemap::construct(int size)
{
	construct(size, kTextureColor);
}

void Cubemap::construct(int size, int flags)
{
	destruct();

	if (!(flags & kTextureFlagsMask) || size <= 0) {
		return;
	}

	m_size = size;

	glGenTextures(1, &m_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (flags & kTextureColor) {
		GLint internalFormat = (flags & kTextureFloatingPoint) ? GL_RGB32F : GL_RGBA;
		for (int face = 0; face < 6; ++face) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		}
	} else if (flags == kTextureDepth) {
		for (int face = 0; face < 6; ++face) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT32F, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		}
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::destruct()
{
	glDeleteTextures(1, &m_cubemap);
	m_size = 0;
}

void Cubemap::bind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
}

void Cubemap::unbind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
