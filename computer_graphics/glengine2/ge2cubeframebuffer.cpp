#include "ge2cubeframebuffer.h"
#include "ge2camera.h"
#include "ge2cubemap.h"

using namespace ge2;

namespace {

// The camera up vector must be -y because opengl textures have (0,0) in bottom-left corner
// instead of in top-left corner
const glm::quat kPositiveXRotation = glm::rotate(glm::rotate(glm::quat{}, -GE_PI/2.0f, kUnitVectorY), GE_PI, kUnitVectorZ);
const glm::quat kNegativeXRotation = glm::rotate(glm::rotate(glm::quat{}, GE_PI/2.0f, kUnitVectorY), GE_PI, kUnitVectorZ);
const glm::quat kPositiveYRotation = glm::rotate(glm::quat{}, GE_PI/2.0f, kUnitVectorX);
const glm::quat kNegativeYRotation = glm::rotate(glm::quat{}, -GE_PI/2.0f, kUnitVectorX);
const glm::quat kPositiveZRotation = glm::rotate(glm::rotate(glm::quat{}, GE_PI, kUnitVectorY), GE_PI, kUnitVectorZ);
const glm::quat kNegativeZRotation = glm::rotate(glm::quat{}, GE_PI, kUnitVectorZ);

}

CubeFramebuffer::CubeFramebuffer(CubeFramebuffer &&rhs)
{
	swap(rhs);
}

CubeFramebuffer::~CubeFramebuffer()
{
	destruct();
}

CubeFramebuffer &CubeFramebuffer::operator=(CubeFramebuffer rhs)
{
	swap(rhs);
	return *this;
}

void CubeFramebuffer::construct(int size, bool depthBufferEnabled, const std::vector<FragmentBuffer> &enabledFragmentBuffers)
{
	destruct();

	if (size <= 0 || (!depthBufferEnabled && enabledFragmentBuffers.empty())) {
		return;
	}

	m_size = size;

	// Thank you!!!
	// http://the-witness.net/news/2012/02/seamless-cube-map-filtering/
	float fov = 2.0f * glm::atan((float)size / ((float)size - 0.5f));
	m_camera = new PerspectiveCamera{fov, 1.0f, 0.001f, 1000.0f};

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	if (depthBufferEnabled) {
		m_depthBuffer = new Cubemap;
		m_depthBuffer->construct(size, kTextureDepth);
	}

	if (!enabledFragmentBuffers.empty()) {
		GLenum drawBuffers[(int)FragmentBuffer::NumBuffers];
		for (int i = 0; i < (int)FragmentBuffer::NumBuffers; ++i) {
			drawBuffers[i] = GL_NONE;
		}

		for (FragmentBuffer buf : enabledFragmentBuffers) {
			m_colorBuffers[(int)buf] = new Cubemap;
			m_colorBuffers[(int)buf]->construct(size, kTextureColor|kTextureFloatingPoint);

			drawBuffers[(int)buf] = GL_COLOR_ATTACHMENT0 + (int)buf;
		}

		glDrawBuffers((GLsizei)FragmentBuffer::NumBuffers, drawBuffers);
	} else {
		glDrawBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeFramebuffer::destruct()
{
	delete m_depthBuffer;
	m_depthBuffer = nullptr;

	delete m_camera;
	m_camera = nullptr;

	for (int i = 0; i < (int)FragmentBuffer::NumBuffers; ++i) {
		delete m_colorBuffers[i];
		m_colorBuffers[i] = nullptr;
	}

	glDeleteFramebuffers(1, &m_frameBuffer);

	m_size = -1;

	m_storedViewport[0] = m_storedViewport[1] = m_storedViewport[2] = m_storedViewport[3] = 0;
}

PerspectiveCamera *CubeFramebuffer::camera(CubeDirection direction)
{
	if (!m_camera) {
		return nullptr;
	}

	switch (direction) {
	case CubeDirection::PositiveX:
		m_camera->setRotation(kPositiveXRotation);
		return m_camera;
	case CubeDirection::NegativeX:
		m_camera->setRotation(kNegativeXRotation);
		return m_camera;
	case CubeDirection::PositiveY:
		m_camera->setRotation(kPositiveYRotation);
		return m_camera;
	case CubeDirection::NegativeY:
		m_camera->setRotation(kNegativeYRotation);
		return m_camera;
	case CubeDirection::PositiveZ:
		m_camera->setRotation(kPositiveZRotation);
		return m_camera;
	case CubeDirection::NegativeZ:
		m_camera->setRotation(kNegativeZRotation);
		return m_camera;
	default:
		return nullptr;
		break;
	}

	return nullptr;
}

float CubeFramebuffer::far() const
{
	if (!m_camera) {
		return 0.0f;
	}
	return m_camera->far();
}

float CubeFramebuffer::near() const
{
	if (!m_camera) {
		return 0.0f;
	}
	return m_camera->near();
}

glm::vec3 CubeFramebuffer::position()
{
	if (!m_camera) {
		return glm::vec3{0.0f};
	}
	return m_camera->position();
}

void CubeFramebuffer::setFar(float far)
{
	if (!m_camera) {
		return;
	}
	m_camera->setFar(far);
}

void CubeFramebuffer::setNear(float near)
{
	if (!m_camera) {
		return;
	}
	m_camera->setNear(near);
}

void CubeFramebuffer::setPosition(const glm::vec3 &position)
{
	if (!m_camera) {
		return;
	}
	m_camera->setPosition(position);
}

void CubeFramebuffer::bind(CubeDirection direction)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glGetIntegerv(GL_VIEWPORT, m_storedViewport);
	glViewport(0, 0, m_size, m_size);

	if (m_depthBuffer) {
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLint)direction,
			m_depthBuffer->handle(),
			0
		);
	}

	for (int i = 0; i < (int)FragmentBuffer::NumBuffers; ++i) {
		if (m_colorBuffers[i]) {
			glFramebufferTexture2D(
				GL_DRAW_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + i,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLint)direction,
				m_colorBuffers[i]->handle(),
				0
			);
		}
	}
}

void CubeFramebuffer::unbind()
{
	glViewport(m_storedViewport[0], m_storedViewport[1], m_storedViewport[2], m_storedViewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeFramebuffer::update(CubeFramebufferRenderFunction renderFunction)
{
	for (GLint dir = (GLint)CubeDirection::PositiveX; dir < (GLint)CubeDirection::NumDirections; ++dir) {
		bind((CubeDirection)dir);
		renderFunction(camera((CubeDirection)dir));
		unbind();
	}
}

void CubeFramebuffer::swap(CubeFramebuffer &other)
{
	std::swap(m_camera, other.m_camera);
	std::swap(m_frameBuffer, other.m_frameBuffer);
	std::swap(m_size, other.m_size);
	std::swap(m_depthBuffer, other.m_depthBuffer);
	std::swap(m_colorBuffers, other.m_colorBuffers);
	std::swap(m_storedViewport, other.m_storedViewport);
}
