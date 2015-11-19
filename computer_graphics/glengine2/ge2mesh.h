#pragma once

#include "gl_core_3_2.h"

#include <string>

namespace ge2 {

class Geometry;
class Material;

class Mesh
{
public:
	Mesh(Geometry *geometry, Material *material);
	Mesh(Mesh &&rhs);
	~Mesh();

	Mesh &operator=(Mesh rhs);

	Geometry *geometry() const;
	Material *material() const;

	void setGeometry(Geometry *geometry);
	void setMaterial(Material *material);

	void construct();
	void destruct();

	void draw();

private:
	Mesh() = default;

	void swap(Mesh &other);

	Geometry *m_geometry = nullptr;
	Material *m_material = nullptr;
	bool m_dirty = true;

	GLuint m_indexBuffer = 0;
	GLuint m_textureCoordinatesBuffer = 0;
	GLuint m_vertexArray = 0;
	GLuint m_vertexBuffer = 0;
	GLuint m_vertexNormalsBuffer = 0;
};

} // namespace ge2
