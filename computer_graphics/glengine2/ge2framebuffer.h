#pragma once

#include "ge2common.h"

#include "gl_core_3_2.h"

#include <vector>

namespace ge2 {

class Texture2D;

class Framebuffer
{
	Framebuffer(const Framebuffer &other) = delete;

public:
	Framebuffer() = default;
	Framebuffer(Framebuffer &&rhs);
	~Framebuffer();

	Framebuffer &operator=(Framebuffer rhs);

	Texture2D *colorBuffer(FragmentBuffer buffer) { return m_colorBuffers[(GLint)buffer]; }
	bool colorBufferEnabled(FragmentBuffer buffer) const { return m_colorBuffers[(GLint)buffer] != nullptr; }
	Texture2D *depthBuffer() { return m_depthBuffer; }
	bool depthBufferEnabled() const { return m_depthBuffer != nullptr; }
	int height() const { return m_height; }
	int width() const { return m_width; }

	void construct(int width, int height, bool depthBufferEnabled, const std::vector<FragmentBuffer> &enabledFragmentBuffers);
	void destruct();

	void bind();
	void unbind();

private:
	void swap(Framebuffer &other);

	GLuint                    m_frameBuffer = 0;
	int                       m_width = -1;
	int                       m_height = -1;
	Texture2D                *m_depthBuffer = nullptr;
	std::vector<Texture2D *>  m_colorBuffers{(size_t)FragmentBuffer::NumBuffers, nullptr};

	GLint                     m_storedViewport[4] = {};
};

} // namespace ge2
