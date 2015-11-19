#pragma once

#include "gl_core_3_2.h"

enum {
	kTextureColor     = 1,
	kTextureDepth     = 2,
	kTextureFlagsMask = 3
};

class Texture2D
{
public:
	Texture2D();
	~Texture2D();

	void construct(int width, int height);
	void construct(int width, int height, int flags);
	void destruct();

	GLuint handle() const { return m_texture; }

	void bind();
	void unbind();

private:
	GLuint m_texture = 0;
};
