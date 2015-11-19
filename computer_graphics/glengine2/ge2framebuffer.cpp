#include "ge2framebuffer.h"
#include "ge2texture2d.h"

using namespace ge2;

Framebuffer::Framebuffer(Framebuffer &&rhs)
{
	swap(rhs);
}

Framebuffer::~Framebuffer()
{
	destruct();
}

Framebuffer &Framebuffer::operator=(Framebuffer rhs)
{
	swap(rhs);
	return *this;
}

void Framebuffer::construct(int width, int height, bool depthBufferEnabled, const std::vector<FragmentBuffer> &enabledFragmentBuffers)
{
	destruct();

	if (width <= 0 || height <= 0 || (!depthBufferEnabled && enabledFragmentBuffers.empty())) {
		return;
	}

	m_width = width;
	m_height = height;

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	if (depthBufferEnabled) {
		m_depthBuffer = new Texture2D;
		m_depthBuffer->construct(width, height, kTextureDepth);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer->handle(), 0);
	}

	if (!enabledFragmentBuffers.empty()) {
		GLenum drawBuffers[(int)FragmentBuffer::NumBuffers];
		for (int i = 0; i < (int)FragmentBuffer::NumBuffers; ++i) {
			drawBuffers[i] = GL_NONE;
		}

		for (FragmentBuffer buf : enabledFragmentBuffers) {
			m_colorBuffers[(int)buf] = new Texture2D;
			m_colorBuffers[(int)buf]->construct(width, height, kTextureColor|kTextureFloatingPoint|kTextureClampUvs);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (int)buf, GL_TEXTURE_2D, m_colorBuffers[(int)buf]->handle(), 0);
			drawBuffers[(int)buf] = GL_COLOR_ATTACHMENT0 + (int)buf;
		}

		glDrawBuffers((GLsizei)FragmentBuffer::NumBuffers, drawBuffers);
	} else {
		glDrawBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destruct()
{
	delete m_depthBuffer;
	m_depthBuffer = nullptr;

	for (int i = 0; i < (int)FragmentBuffer::NumBuffers; ++i) {
		delete m_colorBuffers[i];
		m_colorBuffers[i] = nullptr;
	}

	glDeleteFramebuffers(1, &m_frameBuffer);

	m_width = -1;
	m_height = -1;

	m_storedViewport[0] = m_storedViewport[1] = m_storedViewport[2] = m_storedViewport[3] = 0;
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glGetIntegerv(GL_VIEWPORT, m_storedViewport);
	glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind()
{
	glViewport(m_storedViewport[0], m_storedViewport[1], m_storedViewport[2], m_storedViewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::swap(Framebuffer &other)
{
	std::swap(m_frameBuffer, other.m_frameBuffer);
	std::swap(m_width, other.m_width);
	std::swap(m_height, other.m_height);
	std::swap(m_depthBuffer, other.m_depthBuffer);
	std::swap(m_colorBuffers, other.m_colorBuffers);
	std::swap(m_storedViewport, other.m_storedViewport);
}
