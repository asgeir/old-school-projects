#pragma once

#include "gecommon.h"
#include "gerenderer.h"

#include "gl_core_3_2.h"
#include <glm/glm.hpp>

class RenderState;
class Texture2D;

class Quad : public Renderable
{
public:
	Quad();
	~Quad();

	void construct(int size);
	void construct(int width, int height);
	void construct(int width, int height, const glm::vec3 &color);
	void construct(int width, int height, Texture2D *texture);
	void construct(int width, int height,
		const glm::vec3 &topLeftColor, const glm::vec3 &topRightColor,
		const glm::vec3 &bottomLeftColor, const glm::vec3 &bottomRightColor);
	void destruct();

	virtual PrimitiveType primitiveType() const { return kPrimitiveTypeTriangles; }
	virtual int numVertices() const override { return 6; }
	virtual void bind(RenderState *renderState) override;
	virtual void unbind(RenderState *renderState) override;

private:
	Quad(const Quad &other);
	Quad &operator=(const Quad &rhs);

	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	Texture2D *textureObject = nullptr;
};
