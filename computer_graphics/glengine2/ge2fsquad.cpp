#include "ge2fsquad.h"
#include "ge2geometry.h"
#include "ge2material.h"
#include "ge2resourcemgr.h"
#include "ge2shader.h"

using namespace ge2;

namespace {

const VertexList kFullscreenQuadVertices{
	glm::vec3{-1.0, -1.0, 0.0f},
	glm::vec3{ 1.0, -1.0, 0.0f},
	glm::vec3{-1.0,  1.0, 0.0f},
	glm::vec3{ 1.0,  1.0, 0.0f},
};

const IndexList kFullscreenQuadIndices{0, 1, 2, 2, 1, 3};

const std::string kFullscreenQuadVertexShader{R"(
	#version 150

	in vec4 ge_position;

	out vec2 textureCoordinates;

	void main()
	{
		gl_Position = ge_position;

		textureCoordinates = vec2((ge_position / 2.0) + 0.5);
	}
)"};

}

FullscreenQuad::FullscreenQuad()
	: m_material{nullptr}
{
	construct();
}

FullscreenQuad::FullscreenQuad(Material *material)
	: m_material{material}
{
	construct();
}

FullscreenQuad::FullscreenQuad(FullscreenQuad &&rhs)
{
	swap(rhs);
}

FullscreenQuad::~FullscreenQuad()
{
	glDeleteBuffers(1, &m_indexBuffer);
	glDeleteVertexArrays(1, &m_vertexArray);
	glDeleteBuffers(1, &m_vertexBuffer);
}

FullscreenQuad &FullscreenQuad::operator=(FullscreenQuad rhs)
{
	swap(rhs);
	return *this;
}

const std::string &FullscreenQuad::defaultVertexShader()
{
	return kFullscreenQuadVertexShader;
}

Material *FullscreenQuad::material() const
{
	return m_material;
}

void FullscreenQuad::setMaterial(Material *material)
{
	m_material = material;
}

void FullscreenQuad::draw()
{
	glBindVertexArray(m_vertexArray);

	glDrawElements(GL_TRIANGLES, kFullscreenQuadIndices.size(), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}

void FullscreenQuad::construct()
{
	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);

	// Fill vertex position data
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, kFullscreenQuadVertices.size() * sizeof(glm::vec3), kFullscreenQuadVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer((GLint)ShaderAttribute::VertexPosition, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
	glEnableVertexAttribArray((GLint)ShaderAttribute::VertexPosition);

	// Fill index buffer
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, kFullscreenQuadIndices.size() * sizeof(uint32_t), kFullscreenQuadIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void FullscreenQuad::swap(FullscreenQuad &other)
{
	std::swap(m_material, other.m_material);
	std::swap(m_indexBuffer, other.m_indexBuffer);
	std::swap(m_textureCoordinatesBuffer, other.m_textureCoordinatesBuffer);
	std::swap(m_vertexArray, other.m_vertexArray);
	std::swap(m_vertexBuffer, other.m_vertexBuffer);
	std::swap(m_vertexNormalsBuffer, other.m_vertexNormalsBuffer);
}
