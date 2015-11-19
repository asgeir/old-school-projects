#pragma once

#include "genode.h"

#include <vector>

class Camera;
class Renderable;
class RenderState;

typedef std::vector<Renderable *> RenderList;

class Renderable : public Node
{
public:
	enum PrimitiveType
	{
		kPrimitiveTypeTriangles
	};

	virtual PrimitiveType primitiveType() const = 0;
	virtual int numVertices() const = 0;
	virtual void bind(RenderState *renderState) = 0;
	virtual void unbind(RenderState *renderState) = 0;

	void render();
};

class Renderer
{
public:
	Camera *camera() const { return m_camera; }
	void setCamera(Camera *camera) { m_camera = camera; }

	void render(const RenderList &renderables, RenderState *renderState);

private:
	Camera *m_camera = nullptr;
};
