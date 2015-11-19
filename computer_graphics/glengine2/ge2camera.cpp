#include "ge2camera.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace ge2;

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar)
	: m_projection{glm::ortho(left, right, bottom, top, zNear, zFar)}
	, m_left{left}
	, m_right{right}
	, m_bottom{bottom}
	, m_top{top}
	, m_zNear{zNear}
	, m_zFar{zFar}
{
}

float OrthographicCamera::left() const
{
	return m_left;
}

float OrthographicCamera::right() const
{
	return m_right;
}

float OrthographicCamera::bottom() const
{
	return m_bottom;
}

float OrthographicCamera::top() const
{
	return m_top;
}

float OrthographicCamera::zNear() const
{
	return m_zNear;
}

float OrthographicCamera::zFar() const
{
	return m_zFar;
}

void OrthographicCamera::setLeft(float left)
{
	m_left = left;
	m_dirty = true;
}

void OrthographicCamera::setRight(float right)
{
	m_right = right;
	m_dirty = true;
}

void OrthographicCamera::setBottom(float bottom)
{
	m_bottom = bottom;
	m_dirty = true;
}

void OrthographicCamera::setTop(float top)
{
	m_top = top;
	m_dirty = true;
}

void OrthographicCamera::setZNear(float zNear)
{
	m_zNear = zNear;
	m_dirty = true;
}

void OrthographicCamera::setZFar(float zFar)
{
	m_zFar = zFar;
	m_dirty = true;
}

glm::mat4 OrthographicCamera::projectionMatrix()
{
	if (m_dirty) {
		m_projection = glm::ortho(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
		m_dirty = false;
	}
	return m_projection;
}

glm::mat4 OrthographicCamera::viewMatrix()
{
	return glm::inverse(transform());
}

glm::mat4 OrthographicCamera::viewProjectionMatrix()
{
	return projectionMatrix() * viewMatrix();
}

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float near, float far)
	: m_projection{glm::perspective(fov, aspect, near, far)}
	, m_useAspect{true}
	, m_fov{fov}
	, m_aspect{aspect}
	, m_near{near}
	, m_far{far}
{
}

PerspectiveCamera::PerspectiveCamera(float fov, float width, float height, float near, float far)
	: m_projection{glm::perspectiveFov(fov, width, height, near, far)}
	, m_useAspect{false}
	, m_fov{fov}
	, m_width{width}
	, m_height{height}
	, m_near{near}
	, m_far{far}
{
}

float PerspectiveCamera::aspect() const
{
	return m_aspect;
}

float PerspectiveCamera::fov() const
{
	return m_fov;
}

float PerspectiveCamera::width() const
{
	return m_width;
}

float PerspectiveCamera::height() const
{
	return m_height;
}

float PerspectiveCamera::near() const
{
	return m_near;
}

float PerspectiveCamera::far() const
{
	return m_far;
}

bool PerspectiveCamera::useAspect() const
{
	return m_useAspect;
}

void PerspectiveCamera::setAspect(float aspect)
{
	m_aspect = aspect;
	m_dirty = true;
}

void PerspectiveCamera::setFov(float fov)
{
	m_fov = fov;
	m_dirty = true;
}

void PerspectiveCamera::setWidth(float width)
{
	m_width = width;
	m_dirty = true;
}

void PerspectiveCamera::setHeight(float height)
{
	m_height = height;
	m_dirty = true;
}

void PerspectiveCamera::setNear(float near)
{
	m_near = near;
	m_dirty = true;
}

void PerspectiveCamera::setFar(float far)
{
	m_far = far;
	m_dirty = true;
}

void PerspectiveCamera::setUseAspect(bool useAspect)
{
	m_useAspect = useAspect;
	m_dirty = true;
}

glm::mat4 PerspectiveCamera::projectionMatrix()
{
	if (m_dirty) {
		if (m_useAspect) {
			m_projection = glm::perspective(m_fov, m_aspect, m_near, m_far);
		} else {
			m_projection = glm::perspectiveFov(m_fov, m_width, m_height, m_near, m_far);
		}
		m_dirty = false;
	}
	return m_projection;
}

glm::mat4 PerspectiveCamera::viewMatrix()
{
	return glm::inverse(transform());
}

glm::mat4 PerspectiveCamera::viewProjectionMatrix()
{
	return projectionMatrix() * viewMatrix();
}
