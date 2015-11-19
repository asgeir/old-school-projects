#include "ge2mesh.h"
#include "ge2common.h"
#include "ge2geometry.h"
#include "ge2material.h"
#include "ge2shader.h"

using namespace ge2;

Mesh::Mesh(Geometry *geometry, Material *material)
	: m_geometry(geometry)
	, m_material(material)
{
}

Mesh::Mesh(Mesh &&rhs)
{
	swap(rhs);
}

Mesh::~Mesh()
{
	destruct();
}

Mesh &Mesh::operator=(Mesh rhs)
{
	swap(rhs);
	return *this;
}

Geometry *Mesh::geometry() const
{
	return m_geometry;
}

Material *Mesh::material() const
{
	return m_material;
}

void Mesh::setGeometry(Geometry *geometry)
{
	m_geometry = geometry;
	m_dirty = true;
}

void Mesh::setMaterial(Material *material)
{
	m_material = material;
}

void Mesh::construct()
{
	if (!m_material || !m_geometry) {
		return;
	}

	destruct();

	glGenVertexArrays(1, &m_vertexArray);
	glBindVertexArray(m_vertexArray);

	// Fill vertex position data
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

	VertexList vertices = m_geometry->vertices();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer((GLint)ShaderAttribute::VertexPosition, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
	glEnableVertexAttribArray((GLint)ShaderAttribute::VertexPosition);

	// Fill UV vertex data
	if (m_geometry->uvCount()) {
		glGenBuffers(1, &m_textureCoordinatesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_textureCoordinatesBuffer);

		UVList uvs = m_geometry->uvs();
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer((GLint)ShaderAttribute::VertexTextureCoordinates, 2, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
		glEnableVertexAttribArray((GLint)ShaderAttribute::VertexTextureCoordinates);
	}

	// Fill vertex normal data
	if (m_geometry->normalCount()) {
		glGenBuffers(1, &m_vertexNormalsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexNormalsBuffer);

		VertexList normals = m_geometry->normals();
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

		glVertexAttribPointer((GLint)ShaderAttribute::VertexNormals, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
		glEnableVertexAttribArray((GLint)ShaderAttribute::VertexNormals);
	}

	// Fill index buffer
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

	IndexList indices = m_geometry->indices();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	m_dirty = false;
}

void Mesh::destruct()
{
	glDeleteBuffers(1, &m_indexBuffer);
	glDeleteVertexArrays(1, &m_vertexArray);
	glDeleteBuffers(1, &m_vertexBuffer);

	m_indexBuffer = 0;
	m_vertexArray = 0;
	m_vertexBuffer = 0;

	m_dirty = true;
}

void Mesh::draw()
{
	if (m_dirty || !m_vertexArray || !m_geometry) {
		return;
	}

	glBindVertexArray(m_vertexArray);

	GLenum primitiveType;
	switch (m_geometry->primitiveType()) {
	case Geometry::kGeometryTriangles:
		primitiveType = GL_TRIANGLES;
		break;

	default:
		primitiveType = GL_TRIANGLES;
		break;
	}

	glDrawElements(primitiveType, m_geometry->indexCount(), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}

void Mesh::swap(Mesh &other)
{
	std::swap(m_geometry, other.m_geometry);
	std::swap(m_material, other.m_material);
	std::swap(m_dirty, other.m_dirty);
	std::swap(m_indexBuffer, other.m_indexBuffer);
	std::swap(m_textureCoordinatesBuffer, other.m_textureCoordinatesBuffer);
	std::swap(m_vertexArray, other.m_vertexArray);
	std::swap(m_vertexBuffer, other.m_vertexBuffer);
	std::swap(m_vertexNormalsBuffer, other.m_vertexNormalsBuffer);
}
