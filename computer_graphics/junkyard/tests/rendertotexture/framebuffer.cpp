#include "framebuffer.h"
#include "texture2d.h"

FrameBuffer::FrameBuffer()
{
}

FrameBuffer::~FrameBuffer()
{
}

#include <iostream>

void FrameBuffer::construct(int width, int height, int flags)
{
	destruct();

	if (!(flags & kFrameBufferFlagsMask) || (width <= 0) || (height <= 0)) {
		return;
	}

	m_width = width;
	m_height = height;

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	if (flags & kFrameBufferColor) {
		m_colorBuffer = new Texture2D;
		m_colorBuffer->construct(width, height);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer->handle(), 0);

		// Fragment shader output at location 0 will be output to color_attachment0
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
	}

	if (flags & kFrameBufferDepth) {
		m_depthBuffer = new Texture2D;
		m_depthBuffer->construct(width, height, kTextureDepth);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer->handle(), 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::destruct()
{
	delete m_depthBuffer;
	m_depthBuffer = nullptr;

	delete m_colorBuffer;
	m_colorBuffer = nullptr;

	glDeleteFramebuffers(1, &m_frameBuffer);

	m_width = 0;
	m_height = 0;
}

Texture2D *FrameBuffer::depthBuffer() const
{
	return m_depthBuffer;
}

Texture2D *FrameBuffer::colorBuffer() const
{
	return m_colorBuffer;
}

void FrameBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glGetIntegerv(GL_VIEWPORT, m_storedViewport);
	glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::unbind()
{
	glViewport(m_storedViewport[0], m_storedViewport[1], m_storedViewport[2], m_storedViewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
