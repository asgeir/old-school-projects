#pragma once

#include "ge2common.h"

namespace ge2 {

class Cubemap
{
public:
	Cubemap();
	~Cubemap();

	void construct(int size);
	void construct(int size, int flags);
	void destruct();

	GLuint handle() const { return m_cubemap; }
	int size() const { return m_size; }

	void bind();
	void unbind();

private:
	GLuint m_cubemap = 0;
	int    m_size = 0;
};

}
