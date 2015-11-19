#pragma once

#include "ge2node.h"

namespace ge2 {

class Camera : public Node
{
public:
	virtual glm::mat4 projectionMatrix() = 0;
	virtual glm::mat4 viewMatrix() = 0;
	virtual glm::mat4 viewProjectionMatrix() = 0;
};

class OrthographicCamera : public Camera
{
public:
	OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar);

	float left() const;
	float right() const;
	float bottom() const;
	float top() const;
	float zNear() const;
	float zFar() const;

	void setLeft(float left);
	void setRight(float right);
	void setBottom(float bottom);
	void setTop(float top);
	void setZNear(float zNear);
	void setZFar(float zFar);

	virtual glm::mat4 projectionMatrix() override;
	virtual glm::mat4 viewMatrix() override;
	virtual glm::mat4 viewProjectionMatrix() override;

private:
	glm::mat4 m_projection{1.0f};
	bool m_dirty = false;

	float m_left = 0.0f;
	float m_right = 0.0f;
	float m_bottom = 0.0f;
	float m_top = 0.0f;
	float m_zNear = 0.0f;
	float m_zFar = 0.0f;
};

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(float fov, float aspect, float near, float far);
	PerspectiveCamera(float fov, float width, float height, float near, float far);

	float aspect() const;
	float fov() const;
	float width() const;
	float height() const;
	float near() const;
	float far() const;
	bool useAspect() const;

	void setAspect(float aspect);
	void setFov(float fov);
	void setWidth(float width);
	void setHeight(float height);
	void setNear(float near);
	void setFar(float far);
	void setUseAspect(bool useAspect);

	virtual glm::mat4 projectionMatrix() override;
	virtual glm::mat4 viewMatrix() override;
	virtual glm::mat4 viewProjectionMatrix() override;

private:
	glm::mat4 m_projection{1.0f};
	bool m_dirty = false;
	bool m_useAspect = false;

	float m_fov = 0.0f;
	float m_aspect = 0.0f;
	float m_width = 0.0f;
	float m_height = 0.0f;
	float m_near = 0.0f;
	float m_far = 0.0f;
};

} // namespace ge2
