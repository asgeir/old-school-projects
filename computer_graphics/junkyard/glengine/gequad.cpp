#include "gequad.h"
#include "gerenderstate.h"
#include "geshaderprogram.h"
#include "getexture2d.h"

#include <glm/gtc/type_ptr.hpp>

Quad::Quad()
	: Renderable()
	, vertexArrayObject(0)
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

	glBindVertexArray(0);
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

void Quad::bind(RenderState *renderState)
{
	ShaderProgram *shaderProgram = renderState->shaderProgram();
	if (!shaderProgram) {
		return;
	}
	ParamIndices paramIndices = shaderProgram->paramIndices();

	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	if (paramIndices.find(kShaderParamVertexPosition) != paramIndices.end()) {
		glVertexAttribPointer(paramIndices[kShaderParamVertexPosition],
			3, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(0));
		glEnableVertexAttribArray(paramIndices[kShaderParamVertexPosition]);
	}

	if (paramIndices.find(kShaderParamVertexColor) != paramIndices.end()) {
		glVertexAttribPointer(paramIndices[kShaderParamVertexColor],
			3, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(sizeof(GLfloat) * 3));
		glEnableVertexAttribArray(paramIndices[kShaderParamVertexColor]);
	}

	if (paramIndices.find(kShaderParamTextureCoordinate) != paramIndices.end()) {
		glVertexAttribPointer(paramIndices[kShaderParamTextureCoordinate],
			2, GL_FLOAT, GL_FALSE, sizeof(vertex), bufferOffset(sizeof(GLfloat) * 6));
		glEnableVertexAttribArray(paramIndices[kShaderParamTextureCoordinate]);
	}

	if (textureObject && paramIndices.find(kShaderParamTextureData0) != paramIndices.end()) {
		glActiveTexture(GL_TEXTURE0);
		textureObject->bind();
		glUniform1i(paramIndices[kShaderParamTextureData0], 0);
	}
}

void Quad::unbind(RenderState *renderState)
{
	ShaderProgram *shaderProgram = renderState->shaderProgram();
	if (!shaderProgram) {
		return;
	}
	ParamIndices paramIndices = shaderProgram->paramIndices();

	if (paramIndices.find(kShaderParamVertexPosition) != paramIndices.end()) {
		glDisableVertexAttribArray(paramIndices[kShaderParamVertexPosition]);
	}

	if (paramIndices.find(kShaderParamVertexColor) != paramIndices.end()) {
		glDisableVertexAttribArray(paramIndices[kShaderParamVertexColor]);
	}

	if (paramIndices.find(kShaderParamTextureCoordinate) != paramIndices.end()) {
		glDisableVertexAttribArray(paramIndices[kShaderParamTextureCoordinate]);
	}

	if (textureObject && paramIndices.find(kShaderParamTextureData0) != paramIndices.end()) {
		glActiveTexture(GL_TEXTURE0);
		textureObject->unbind();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
