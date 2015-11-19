#pragma once

#include "gl_core_3_2.h"

class Texture2D;

enum {
	kFrameBufferColor     = 1,
	kFrameBufferDepth     = 2,
	kFrameBufferFlagsMask = 3
};

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	void construct(int width, int height, int flags = kFrameBufferColor);
	void destruct();

	Texture2D *depthBuffer() const;
	Texture2D *colorBuffer() const;

	void bind();
	void unbind();

private:
	GLuint m_frameBuffer = 0;
	int m_width = 0;
	int m_height = 0;
	Texture2D *m_colorBuffer = nullptr;
	Texture2D *m_depthBuffer = nullptr;

	GLint m_storedViewport[4] = {};
};
