#include "quad.h"
#include "texture2d.h"

#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr GLubyte *bufferOffset(size_t bytes)
{
	return static_cast<GLubyte *>(0) + bytes;
}

}

Quad::Quad()
	: vertexArrayObject(0)
	, vertexBufferObject(0)
{
}

Quad::~Quad()
{
	destruct();
}

void Quad::construct(int size)
{
	construct(size, size);
}

void Quad::construct(int width, int height)
{
	construct(width, height, glm::vec3{1.0f, 1.0f, 1.0f});
}

void Quad::construct(int width, int height, const glm::vec3 &color)
{
	construct(width, height, color, color, color, color);
}

void Quad::construct(int width, int height, Texture2D *texture)
{
	construct(width, height, glm::vec3{1.0f, 1.0f, 1.0f});
	if (vertexBufferObject) {
		textureObject = texture;
	}
}

struct vertex
{
	GLfloat x, y, z;
	GLfloat r, g, b;
	GLfloat u, v;
};

void Quad::construct(int width, int height,
	const glm::vec3 &topLeftColor, const glm::vec3 &topRightColor,
	const glm::vec3 &bottomLeftColor, const glm::vec3 &bottomRightColor)
{
	destruct();

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	vertex vertices[] = {
		{
			width * -0.5f, height * -0.5f, 0.0f,
			bottomLeftColor.r, bottomLeftColor.g, bottomLeftColor.b,
			0.0f, 0.0f
		},
		{
			width * 0.5f, height * -0.5f, 0.0f,
			bottomRightColor.r, bottomRightColor.g, bottomRightColor.b,
			1.0f, 0.0f
		},
		{
			width * -0.5f, height * 0.5f, 0.0f,
			topLeftColor.r, topLeftColor.g, topLeftColor.b,
			0.0f, 1.0f
		},
		{
			width * -0.5f, height * 0.5f, 0.0f,
			topLeftColor.r, topLeftColor.g, topLeftColor.b,
			0.0f, 1.0f
		},
		{
			width * 0.5f, height * -0.5f, 0.0f,
			bottomRightColor.r, bottomRightColor.g, bottomRightColor.b,
			1.0f, 0.0f
		},
		{
			width * 0.5f, height * 0.5f, 0.0f,
			topRightColor.r, topRightColor.g, topRightColor.b,
			1.0f, 1.0f
		}
	};

	glGenBuffers(1, &vertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void Quad::destruct()
{
	if (vertexBufferObject) {
		glDeleteBuffers(1, &vertexBufferObject);
	}
	if (vertexArrayObject) {
		glDeleteVertexArrays(1, &vertexArrayObject);
	}
	textureObject = nullptr;
}

void Quad::draw(const ProgramIndices &indices, const glm::mat4 &modelViewProjection)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	if (indices.vertexIndex >= 0) {
		glVertexAttribPointer(indices.vertexIndex, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(0));
		glEnableVertexAttribArray(indices.vertexIndex);
	}

	if (indices.colorIndex >= 0) {
		glVertexAttribPointer(indices.colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(sizeof(GLfloat) * 3));
		glEnableVertexAttribArray(indices.colorIndex);
	}

	if (indices.uvIndex >= 0) {
		glVertexAttribPointer(indices.uvIndex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(sizeof(GLfloat) * 6));
		glEnableVertexAttribArray(indices.uvIndex);
	}

	const int textureSamplerIndex = 0;
	if (textureObject && indices.textureIndex >= 0) {
		glActiveTexture(GL_TEXTURE0 + textureSamplerIndex);
		textureObject->bind();
		glUniform1i(indices.textureIndex, textureSamplerIndex);
	}

	glUniformMatrix4fv(indices.modelViewProjectionIndex, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
	glDrawArrays(GL_TRIANGLES, 0, Quad::NumVertices);

	if (textureObject && indices.textureIndex >= 0) {
		glActiveTexture(GL_TEXTURE0 + textureSamplerIndex);
		textureObject->unbind();
		glActiveTexture(GL_TEXTURE0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
