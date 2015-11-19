#include "texture2d.h"

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
	destruct();
}

void Texture2D::construct(int width, int height)
{
	construct(width, height, kTextureColor);
}

void Texture2D::construct(int width, int height, int flags)
{
	destruct();

	if (!(flags & kTextureFlagsMask) || width <= 0 || height <= 0) {
		return;
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	if (flags == kTextureColor) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
	} else if (flags == kTextureDepth) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::construct(GLuint tex)
{
	destruct();
	m_texture = tex;
}

void Texture2D::destruct()
{
	glDeleteTextures(1, &m_texture);
}

void Texture2D::bind()
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Texture2D::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
