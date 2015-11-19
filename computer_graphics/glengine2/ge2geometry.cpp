#include "ge2geometry.h"
#include "ge2common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <iostream>
#include <map>

using namespace ge2;

namespace {

// http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
class IcoSphereCreator
{
public:
	VertexList vertices(float scale)
	{
		VertexList vl;
		for (auto p : m_vertices) {
			vl.push_back(p.position * scale);
		}
		return std::move(vl);
	}

	IndexList indices()
	{
		IndexList il;
		for (auto s : m_surfaces) {
			il.push_back(s.v1);
			il.push_back(s.v2);
			il.push_back(s.v3);
		}
		return std::move(il);
	}

	VertexList normals()
	{
		for (auto s : m_surfaces) {
			s.calculateNormal(m_vertices);
		}
		VertexList nl;
		for (auto v : m_vertices) {
			nl.push_back(v.normal());
		}
		return std::move(nl);
	}

	void create(size_t recursionLevel=3)
	{
		// create 12 vertices of a icosahedron
		float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;

		addVertex(glm::vec3{-1.0f,     t,  0.0f});
		addVertex(glm::vec3{ 1.0f,     t,  0.0f});
		addVertex(glm::vec3{-1.0f,    -t,  0.0f});
		addVertex(glm::vec3{ 1.0f,    -t,  0.0f});

		addVertex(glm::vec3{ 0.0f, -1.0f,     t});
		addVertex(glm::vec3{ 0.0f,  1.0f,     t});
		addVertex(glm::vec3{ 0.0f, -1.0f,    -t});
		addVertex(glm::vec3{ 0.0f,  1.0f,    -t});

		addVertex(glm::vec3{    t,  0.0f, -1.0f});
		addVertex(glm::vec3{    t,  0.0f,  1.0f});
		addVertex(glm::vec3{   -t,  0.0f, -1.0f});
		addVertex(glm::vec3{   -t,  0.0f,  1.0f});

		// create 20 triangles of the icosahedron

		// 5 faces around point 0
		m_surfaces.push_back(Surface{0, 11,  5});
		m_surfaces.push_back(Surface{0,  5,  1});
		m_surfaces.push_back(Surface{0,  1,  7});
		m_surfaces.push_back(Surface{0,  7, 10});
		m_surfaces.push_back(Surface{0, 10, 11});

		// 5 adjacent faces
		m_surfaces.push_back(Surface{ 1,  5, 9});
		m_surfaces.push_back(Surface{ 5, 11, 4});
		m_surfaces.push_back(Surface{11, 10, 2});
		m_surfaces.push_back(Surface{10,  7, 6});
		m_surfaces.push_back(Surface{ 7,  1, 8});

		// 5 faces around point 3
		m_surfaces.push_back(Surface{3, 9, 4});
		m_surfaces.push_back(Surface{3, 4, 2});
		m_surfaces.push_back(Surface{3, 2, 6});
		m_surfaces.push_back(Surface{3, 6, 8});
		m_surfaces.push_back(Surface{3, 8, 9});

		// 5 adjacent faces
		m_surfaces.push_back(Surface{4, 9,  5});
		m_surfaces.push_back(Surface{2, 4, 11});
		m_surfaces.push_back(Surface{6, 2, 10});
		m_surfaces.push_back(Surface{8, 6,  7});
		m_surfaces.push_back(Surface{9, 8,  1});

		for (size_t i = 0; i < recursionLevel; ++i) {
			std::vector<Surface> newSurfaces;
			for (auto s : m_surfaces) {
				// replace triangle by 4 triangles
				size_t a = getMiddlePoint(s.v1, s.v2);
				size_t b = getMiddlePoint(s.v2, s.v3);
				size_t c = getMiddlePoint(s.v3, s.v1);

				newSurfaces.push_back(Surface{s.v1, a, c});
				newSurfaces.push_back(Surface{s.v2, b, a});
				newSurfaces.push_back(Surface{s.v3, c, b});
				newSurfaces.push_back(Surface{   a, b, c});
			}
			m_surfaces = newSurfaces;
		}
	}

private:
	struct Point3D
	{
		Point3D(glm::vec3 pos) : position{pos} {}

		glm::vec3 position{0.0f};
		VertexList normals;

		glm::vec3 normal() const
		{
			glm::vec3 average{0.0f};
			for (auto n : normals) {
				average += n;
			}

			if (glm::dot(average, average)) {
				average = glm::normalize(average);
			}

			return average;
		}
	};

	typedef std::vector<Point3D> Point3DList;

	struct Surface
	{
		Surface(size_t a, size_t b, size_t c) : v1{a}, v2{b}, v3{c} {}

		void calculateNormal(Point3DList &vertices) const
		{
			glm::vec3 ab = vertices[v2].position - vertices[v1].position;
			glm::vec3 ac = vertices[v3].position - vertices[v1].position;
			glm::vec3 n = glm::cross(ab, ac);

			if (glm::dot(n, n)) {
				n = glm::normalize(n);
			}

			vertices[v1].normals.push_back(n);
			vertices[v2].normals.push_back(n);
			vertices[v3].normals.push_back(n);
		}

		size_t v1 = 0;
		size_t v2 = 0;
		size_t v3 = 0;
	};

	size_t addVertex(glm::vec3 v)
	{
		m_vertices.push_back(Point3D{glm::normalize(v)});
		return m_index++;
	}

	size_t getMiddlePoint(size_t p1, size_t p2)
	{
		uint64_t key = (((uint64_t)std::min(p1, p2)) << 32) | ((uint64_t)std::max(p1, p2));

		auto it = m_middlePointIndexCache.find(key);
		if (it != m_middlePointIndexCache.end()) {
			return it->second;
		}

		const Point3D &point1 = m_vertices[p1];
		const Point3D &point2 = m_vertices[p2];
		glm::vec3 middle{
			(point1.position[0] + point2.position[0]) / 2.0,
			(point1.position[1] + point2.position[1]) / 2.0,
			(point1.position[2] + point2.position[2]) / 2.0
		};

		size_t i = addVertex(middle);
		m_middlePointIndexCache[key] = i;

		return i;
	}

	Point3DList                	m_vertices;
	std::vector<Surface>        m_surfaces;
	size_t                     	m_index = 0;
	std::map<uint64_t, size_t> m_middlePointIndexCache;
};

}

Geometry::Geometry(VertexList vertices, IndexList indices, UVList uvs, VertexList normals)
	: m_indices(indices)
	, m_vertices(vertices)
	, m_normals(normals)
	, m_uvs(uvs)
{
}

Geometry::Geometry(VertexList vertices, IndexList indices, UVList uvs, VertexList normals, PrimitiveType type)
	: m_indices(indices)
	, m_vertices(vertices)
	, m_normals(normals)
	, m_uvs(uvs)
	, m_primitiveType(type)
{
}

Geometry *Geometry::createCube(float size)
{
	Geometry *cube = new Geometry;
	cube->setPrimitiveType(kGeometryTriangles);

	float halfSize = size / 2.0f;
	cube->setVertices({
		// top
		glm::vec3{-halfSize,  halfSize,  halfSize},
		glm::vec3{ halfSize,  halfSize,  halfSize},
		glm::vec3{ halfSize,  halfSize, -halfSize},
		glm::vec3{-halfSize,  halfSize, -halfSize},

		// front
		glm::vec3{-halfSize, -halfSize,  halfSize},
		glm::vec3{ halfSize, -halfSize,  halfSize},
		glm::vec3{ halfSize,  halfSize,  halfSize},
		glm::vec3{-halfSize,  halfSize,  halfSize},

		// bottom
		glm::vec3{-halfSize, -halfSize, -halfSize},
		glm::vec3{ halfSize, -halfSize, -halfSize},
		glm::vec3{ halfSize, -halfSize,  halfSize},
		glm::vec3{-halfSize, -halfSize,  halfSize},

		// back
		glm::vec3{ halfSize, -halfSize, -halfSize},
		glm::vec3{-halfSize, -halfSize, -halfSize},
		glm::vec3{-halfSize,  halfSize, -halfSize},
		glm::vec3{ halfSize,  halfSize, -halfSize},

		// left side
		glm::vec3{-halfSize, -halfSize, -halfSize},
		glm::vec3{-halfSize, -halfSize,  halfSize},
		glm::vec3{-halfSize,  halfSize,  halfSize},
		glm::vec3{-halfSize,  halfSize, -halfSize},

		// right side
		glm::vec3{ halfSize, -halfSize,  halfSize},
		glm::vec3{ halfSize, -halfSize, -halfSize},
		glm::vec3{ halfSize,  halfSize, -halfSize},
		glm::vec3{ halfSize,  halfSize,  halfSize}
	});

	cube->setNormals({
		// top
		glm::vec3{ 0.0,  1.0,  0.0},
		glm::vec3{ 0.0,  1.0,  0.0},
		glm::vec3{ 0.0,  1.0,  0.0},
		glm::vec3{ 0.0,  1.0,  0.0},

		// front
		glm::vec3{ 0.0,  0.0,  1.0},
		glm::vec3{ 0.0,  0.0,  1.0},
		glm::vec3{ 0.0,  0.0,  1.0},
		glm::vec3{ 0.0,  0.0,  1.0},

		// bottom
		glm::vec3{ 0.0, -1.0,  0.0},
		glm::vec3{ 0.0, -1.0,  0.0},
		glm::vec3{ 0.0, -1.0,  0.0},
		glm::vec3{ 0.0, -1.0,  0.0},

		// back
		glm::vec3{ 0.0,  0.0, -1.0},
		glm::vec3{ 0.0,  0.0, -1.0},
		glm::vec3{ 0.0,  0.0, -1.0},
		glm::vec3{ 0.0,  0.0, -1.0},

		// left side
		glm::vec3{-1.0,  0.0, 0.0},
		glm::vec3{-1.0,  0.0, 0.0},
		glm::vec3{-1.0,  0.0, 0.0},
		glm::vec3{-1.0,  0.0, 0.0},

		// right side
		glm::vec3{ 1.0,  0.0, 0.0},
		glm::vec3{ 1.0,  0.0, 0.0},
		glm::vec3{ 1.0,  0.0, 0.0},
		glm::vec3{ 1.0,  0.0, 0.0},
	});

	cube->setUVs({
		// top
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f},

		// front
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f},

		// bottom
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f},

		// back
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f},

		// left side
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f},

		// right side
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{0.0f, 1.0f}
	});

	cube->setIndices({
		// top
		0, 1, 3,
		3, 1, 2,

		// front
		4, 5, 7,
		7, 5, 6,

		// bottom
		 8, 9, 11,
		11, 9, 10,

		// back
		12, 13, 15,
		15, 13, 14,

		// left side
		16, 17, 19,
		19, 17, 18,

		// right side
		20, 21, 23,
		23, 21, 22
	});

	return cube;
}

Geometry *Geometry::createCylinder(float radius, float height, int sides)
{
	Geometry *cylinder = new Geometry;
	cylinder->setPrimitiveType(kGeometryTriangles);

	if (radius <= 0 || height <= 0 || sides < 3) {
		return cylinder;
	}

	int edges = sides - 1;
	float halfHeight = height / 2.0f;
	float rotationAngle = 2.0f * GE_PI / edges;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4{1.0f}, rotationAngle, kUnitVectorY);
	glm::vec4 curPoint = radius * glm::vec4{1.0f, 0.0f, 0.0f, 0.0f};

	VertexList vertices(4*sides+2, glm::vec3{0.0f});
	VertexList normals(4*sides+2, glm::vec3{0.0f});
	UVList uvs(4*sides+2, glm::vec2{0.0f});
	for (int i = 0; i < sides; ++i) {
		vertices[i]         = glm::vec3{curPoint} + glm::vec3{0.0f, -halfHeight, 0.0f};
		vertices[sides+i]   = glm::vec3{curPoint} + glm::vec3{0.0f,  halfHeight, 0.0f};
		vertices[2*sides+i] = vertices[i];
		vertices[3*sides+i] = vertices[sides+i];

		normals[i] = normals[sides+i] = glm::normalize(glm::vec3(curPoint));
		normals[2*sides+i] = glm::vec3{0.0f, -1.0f, 0.0f};
		normals[3*sides+i] = glm::vec3{0.0f,  1.0f, 0.0f};

		uvs[i]         = glm::vec2{i * (1.0f / (float)edges), 0.0f};
		uvs[sides+i]   = glm::vec2{i * (1.0f / (float)edges), 1.0f};
		uvs[2*sides+i] = uvs[3*sides+i] = glm::vec2{0.5f} + 0.5f*glm::normalize(glm::vec2{curPoint[0], curPoint[2]});

		curPoint = rotationMatrix * curPoint;
	}
	vertices[4*sides]   = glm::vec3{0.0f, -halfHeight, 0.0f};
	vertices[4*sides+1] = glm::vec3{0.0f,  halfHeight, 0.0f};
	normals[4*sides]   = glm::vec3{0.0f, -1.0f, 0.0f};
	normals[4*sides+1] = glm::vec3{0.0f,  1.0f, 0.0f};
	uvs[4*sides] = uvs[4*sides+1] = glm::vec2{0.5f, 0.5f};

	int bottomIndex = 0;
	int topIndex = 0;
	IndexList indices;
	for (int i = 0; i < sides; ++i) {
		indices.push_back(sides+topIndex); topIndex = (topIndex + 1) % sides;
		indices.push_back(   bottomIndex);
		indices.push_back(sides+topIndex);
		indices.push_back(sides+topIndex);
		indices.push_back(   bottomIndex); bottomIndex = (bottomIndex + 1) % sides;
		indices.push_back(   bottomIndex);
	}

	bottomIndex = 0;
	topIndex = 0;
	for (int i = 0; i < sides; ++i) {
		indices.push_back(4*sides);
		indices.push_back(2*sides+((bottomIndex+1)%sides));
		indices.push_back(2*sides+bottomIndex); bottomIndex = (bottomIndex + 1) % sides;
		indices.push_back(4*sides+1);
		indices.push_back(3*sides+topIndex); topIndex = (topIndex + 1) % sides;
		indices.push_back(3*sides+topIndex);
	}

	cylinder->setIndices(indices);
	cylinder->setUVs(uvs);
	cylinder->setVertices(vertices);
	cylinder->setNormals(normals);

	return cylinder;
}

Geometry *Geometry::createQuad(float width, float height)
{
	Geometry *quad = new Geometry;
	quad->setPrimitiveType(kGeometryTriangles);

	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	quad->setVertices({
		glm::vec3{-halfWidth, -halfHeight, 0.0f},
		glm::vec3{ halfWidth, -halfHeight, 0.0f},
		glm::vec3{-halfWidth,  halfHeight, 0.0f},
		glm::vec3{ halfWidth,  halfHeight, 0.0f},
	});

	quad->setNormals({
		glm::vec3{0.0f, 0.0f, 1.0f},
		glm::vec3{0.0f, 0.0f, 1.0f},
		glm::vec3{0.0f, 0.0f, 1.0f},
		glm::vec3{0.0f, 0.0f, 1.0f}
	});

	quad->setUVs({
		glm::vec2{0.0f, 0.0f},
		glm::vec2{1.0f, 0.0f},
		glm::vec2{0.0f, 1.0f},
		glm::vec2{1.0f, 1.0f}
	});

	quad->setIndices({ 0, 1, 2, 2, 1, 3 });

	return quad;
}

Geometry *Geometry::createSphere(float radius, int subdivisions)
{
	Geometry *sphere = new Geometry;
	sphere->setPrimitiveType(kGeometryTriangles);

	// https://www.opengl.org/wiki/Texturing_a_Sphere
	IcoSphereCreator sphereCreator;
	sphereCreator.create(subdivisions);

	sphere->setVertices(sphereCreator.vertices(radius));
	sphere->setIndices(sphereCreator.indices());
	sphere->setNormals(sphereCreator.normals());

	return sphere;
}

IndexList Geometry::indices() const
{
	return m_indices;
}

size_t Geometry::indexCount() const
{
	return m_indices.size();
}

Geometry::PrimitiveType Geometry::primitiveType() const
{
	return m_primitiveType;
}

VertexList Geometry::normals() const
{
	return m_normals;
}

size_t Geometry::normalCount() const
{
	return m_normals.size();
}

UVList Geometry::uvs() const
{
	return m_uvs;
}

size_t Geometry::uvCount() const
{
	return m_uvs.size();
}

VertexList Geometry::vertices() const
{
	return m_vertices;
}

size_t Geometry::vertexCount() const
{
	return m_vertices.size();
}

void Geometry::setIndices(IndexList indices)
{
	m_indices = indices;
}

void Geometry::setPrimitiveType(PrimitiveType type)
{
	m_primitiveType = type;
}

void Geometry::setNormals(VertexList normals)
{
	m_normals = normals;
}

void Geometry::setUVs(UVList uvs)
{
	m_uvs = uvs;
}

void Geometry::setVertices(VertexList vertices)
{
	m_vertices = vertices;
}
