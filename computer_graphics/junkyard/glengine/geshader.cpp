#include "geshader.h"

#include <cstdio>
#include <cstring>
#include <utility>

Shader::Shader()
{
}

Shader::Shader(Shader &&rvalue)
{
	swap(rvalue);
}

Shader::~Shader()
{
	unload();
}

Shader &Shader::operator=(Shader rvalue)
{
	swap(rvalue);
	return *this;
}

Shader Shader::loadFromFile(const char *fileName, ShaderType type)
{
	FILE *shaderFile = fopen(fileName, "r");
	if (shaderFile == NULL) {
		Shader shader;
		shader.m_errorString = strdup("Unable to load shader: couldn't open file");
		return std::move(shader);
	}

	// Get the length of the file.
	fseek(shaderFile, 0, SEEK_END);
	size_t length = ftell(shaderFile);
	fseek(shaderFile, 0, SEEK_SET);

	char *text = new char[length + 1];
	fread(text, sizeof(const char), length, shaderFile);
	text[length] = '\0';

	Shader shader = loadFromString(text, fileName, type);
	delete[] text;

	return std::move(shader);
}

Shader Shader::loadFromString(const char *shader, const char *name, ShaderType type)
{
	Shader s;
	s.setName(name);
	s.setShaderSource(shader);
	s.setType(type);
	s.compile();
	return s;
}

const char *Shader::name() const
{
	return m_name;
}

const char *Shader::shaderSource() const
{
	return m_shaderSource;
}

GLuint Shader::shader() const
{
	return m_shader;
}

ShaderType Shader::type() const
{
	return m_type;
}

void Shader::setName(const char *name)
{
	if (m_shader) {
		return;
	}
	if (m_name) {
		delete m_name;
	}
	m_name = strdup(name);
}

void Shader::setShaderSource(const char *shader)
{
	if (m_shader) {
		return;
	}
	if (m_shaderSource) {
		delete m_shaderSource;
	}
	m_shaderSource = strdup(shader);
}

void Shader::setType(ShaderType type)
{
	if (m_shader) {
		return;
	}
	m_type = type;
}

GLuint Shader::compile()
{
	GLenum shaderType = 0;
	if (m_type == kShaderTypeFragment) shaderType = GL_FRAGMENT_SHADER;
	if (m_type == kShaderTypeVertex) shaderType = GL_VERTEX_SHADER;

	m_shader = glCreateShader(shaderType);
	glShaderSource(m_shader, 1, (const GLchar **)&m_shaderSource, NULL);
	glCompileShader(m_shader);

	GLint status;
	glGetShaderiv(m_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		formatShaderInfoLog();
		glDeleteShader(m_shader);
		m_shader = 0;
	}

	return m_shader;
}

void Shader::unload()
{
	delete m_name;
	delete m_shaderSource;
	delete[] m_errorString;
	if (m_shader) {
		glDeleteShader(m_shader);
		m_shader = 0;
	}
}

const char *Shader::errorString()
{
	return m_errorString;
}

void Shader::formatShaderInfoLog()
{
	if (m_errorString) {
		delete[] m_errorString;
	}

	int infologLength = 0;
	glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		m_errorString = new char[infologLength];

		int charsWritten = 0;
		glGetShaderInfoLog(m_shader, infologLength, &charsWritten, m_errorString);
	}
}

void Shader::swap(Shader &other)
{
	std::swap(m_name, other.m_name);
	std::swap(m_shaderSource, other.m_shaderSource);
	std::swap(m_type, other.m_type);
	std::swap(m_shader, other.m_shader);
	std::swap(m_errorString, other.m_errorString);
}
