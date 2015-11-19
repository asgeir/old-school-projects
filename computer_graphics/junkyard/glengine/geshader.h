#pragma once

#include "gl_core_3_2.h"

enum ShaderType
{
	kShaderTypeInvalid,
	kShaderTypeVertex,
	kShaderTypeFragment
};

class Shader
{
public:
	Shader();
	Shader(const Shader &other) = delete;
	Shader(Shader &&rvalue);
	~Shader();

	Shader &operator=(Shader rvalue);

	static Shader loadFromFile(const char *fileName, ShaderType type);
	static Shader loadFromString(const char *shader, const char *name, ShaderType type);

	const char *name() const;
	const char *shaderSource() const;
	GLuint shader() const;
	ShaderType type() const;

	void setName(const char *name);
	void setShaderSource(const char *shader);
	void setType(ShaderType type);

	GLuint compile();
	void unload();
	const char *errorString();

private:
	void formatShaderInfoLog();
	void swap(Shader &other);

	char *m_name = nullptr;
	char *m_shaderSource = nullptr;
	ShaderType m_type = kShaderTypeInvalid;

	GLuint m_shader = 0;
	char *m_errorString = nullptr;
};
