#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace ge2 {

typedef std::vector<uint16_t> IndexList;
typedef std::vector<glm::vec2> UVList;
typedef std::vector<glm::vec3> VertexList;

class Geometry
{
public:
	enum PrimitiveType
	{
		kGeometryTriangles
	};

	Geometry() = default;
	Geometry(VertexList vertices, IndexList indices, UVList uvs, VertexList normals);
	Geometry(VertexList vertices, IndexList indices, UVList uvs, VertexList normals, PrimitiveType type);

	static Geometry *createCube(float size);
	static Geometry *createCylinder(float radius, float height, int sides = 8);
	static Geometry *createQuad(float width, float height);
	static Geometry *createSphere(float radius, int subdivisions = 2);

	IndexList indices() const;
	size_t indexCount() const;
	PrimitiveType primitiveType() const;
	VertexList normals() const;
	size_t normalCount() const;
	UVList uvs() const;
	size_t uvCount() const;
	VertexList vertices() const;
	size_t vertexCount() const;

	void setIndices(IndexList indices);
	void setPrimitiveType(PrimitiveType type);
	void setNormals(VertexList normals);
	void setUVs(UVList uvs);
	void setVertices(VertexList vertices);

private:
	IndexList     m_indices;
	VertexList    m_vertices;
	VertexList    m_normals;
	UVList        m_uvs;
	PrimitiveType m_primitiveType = kGeometryTriangles;
};

} // namespace ge2
