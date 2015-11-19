#pragma once

#include "gl_core_3_2.h"

#include <string>

namespace ge2 {

class Material;

class FullscreenQuad
{
public:
	FullscreenQuad();
	FullscreenQuad(Material *material);
	FullscreenQuad(FullscreenQuad &&rhs);
	~FullscreenQuad();

	FullscreenQuad &operator=(FullscreenQuad rhs);

	static const std::string &defaultVertexShader();

	Material *material() const;

	void setMaterial(Material *material);

	void draw();

private:
	void construct();
	void swap(FullscreenQuad &other);

	Material *m_material = nullptr;

	GLuint m_indexBuffer = 0;
	GLuint m_textureCoordinatesBuffer = 0;
	GLuint m_vertexArray = 0;
	GLuint m_vertexBuffer = 0;
	GLuint m_vertexNormalsBuffer = 0;
};

} // namespace ge2
