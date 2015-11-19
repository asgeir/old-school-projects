#pragma once

#include "ge2common.h"

#include "gl_core_3_2.h"

#include <string>

namespace ge2 {

class Texture2D
{
public:
	Texture2D();
	~Texture2D();

	void construct(int width, int height);
	void construct(int width, int height, int flags);
	void construct(const std::string &fileName);
	void destruct();

	GLuint handle() const { return m_texture; }
	int width() const { return m_width; }
	int height() const { return m_height; }

	void bind();
	void unbind();

private:
	GLuint m_texture = 0;
	int    m_width = 0;
	int    m_height = 0;
};

} // namespace ge2
