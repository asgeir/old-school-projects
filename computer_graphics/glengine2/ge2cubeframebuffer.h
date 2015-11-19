#pragma once

#include "ge2common.h"

#include "gl_core_3_2.h"

#include <functional>
#include <vector>

namespace ge2 {

class Cubemap;
class PerspectiveCamera;

typedef std::function<void(PerspectiveCamera *camera)> CubeFramebufferRenderFunction;

class CubeFramebuffer
{
	CubeFramebuffer(const CubeFramebuffer &other) = delete;

public:
	CubeFramebuffer() = default;
	CubeFramebuffer(CubeFramebuffer &&rhs);
	~CubeFramebuffer();

	CubeFramebuffer &operator=(CubeFramebuffer rhs);

	Cubemap *colorBuffer(FragmentBuffer buffer) { return m_colorBuffers[(GLint)buffer]; }
	bool colorBufferEnabled(FragmentBuffer buffer) const { return m_colorBuffers[(GLint)buffer] != nullptr; }
	Cubemap *depthBuffer() { return m_depthBuffer; }
	bool depthBufferEnabled() const { return m_depthBuffer != nullptr; }
	int size() const { return m_size; }

	void construct(int size, bool depthBufferEnabled, const std::vector<FragmentBuffer> &enabledFragmentBuffers);
	void destruct();

	PerspectiveCamera *camera(CubeDirection direction);
	float far() const;
	float near() const;
	glm::vec3 position();

	void setFar(float far);
	void setNear(float near);
	void setPosition(const glm::vec3 &position);

	void update(CubeFramebufferRenderFunction renderFunction);

private:
	void bind(CubeDirection direction);
	void unbind();

	void swap(CubeFramebuffer &other);

	PerspectiveCamera      *m_camera = nullptr;
	GLuint                  m_frameBuffer = 0;
	int                     m_size = -1;
	Cubemap                *m_depthBuffer = nullptr;
	std::vector<Cubemap *>  m_colorBuffers{(size_t)FragmentBuffer::NumBuffers, nullptr};

	GLint                   m_storedViewport[4] = {};
};

} // namespace ge2
