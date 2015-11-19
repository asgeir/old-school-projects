#include "geshaderprogram.h"
#include "geshader.h"

#include <utility>

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::ShaderProgram(ShaderProgram &&rvalue)
{
	swap(rvalue);
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_programId);
	delete[] m_errorString;
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram rhs)
{
	swap(rhs);
	return *this;
}

ShaderProgram ShaderProgram::loadFromString(const char *name, const char *vertexShader, const char *fragmentShader)
{
	ShaderProgram program;
	program.setVertexShader(Shader::loadFromString(vertexShader, name, kShaderTypeVertex));
	program.setFragmentShader(Shader::loadFromString(fragmentShader, name, kShaderTypeFragment));
	return program;
}

ParamIndices ShaderProgram::paramIndices()
{
	return m_paramIndices;
}

void ShaderProgram::setVertexShader(Shader shader)
{
	if (m_programId) {
		glDeleteProgram(m_programId);
		m_programId = 0;
	}
	m_vertexShader = std::move(shader);
}

void ShaderProgram::setFragmentShader(Shader shader)
{
	if (m_programId) {
		glDeleteProgram(m_programId);
		m_programId = 0;
	}
	m_fragmentShader = std::move(shader);
}

bool ShaderProgram::bind()
{
	glUseProgram(m_programId);
	return m_programId != 0;
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

bool ShaderProgram::link()
{
	if (m_programId) {
		glDeleteProgram(m_programId);
		m_programId = 0;
		delete[] m_errorString;
		m_errorString = nullptr;
	}

	if (m_vertexShader.errorString() || m_fragmentShader.errorString()) {
		return false;
	}

	m_programId = glCreateProgram();
	glAttachShader(m_programId, m_vertexShader.shader());
	glAttachShader(m_programId, m_fragmentShader.shader());
	glLinkProgram(m_programId);

	GLint status;
	glGetProgramiv(m_programId, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		formatProgramInfoLog();
		glDeleteProgram(m_programId);
		m_programId = 0;
		return false;
	}

	populateParamIndices();

	return true;
}

void ShaderProgram::swap(ShaderProgram &other)
{
	std::swap(m_programId, other.m_programId);
	std::swap(m_errorString, other.m_errorString);
	std::swap(m_paramIndices, other.m_paramIndices);
	std::swap(m_vertexShader, other.m_vertexShader);
	std::swap(m_fragmentShader, other.m_fragmentShader);
}

void ShaderProgram::populateParamIndices()
{
	m_paramIndices.clear();
	locateShaderAttribute(kShaderParamVertexPosition, "vertexPosition");
	locateShaderAttribute(kShaderParamVertexColor, "vertexColor");
	locateShaderAttribute(kShaderParamTextureCoordinate, "textureCoordinates");
	locateShaderUniform(kShaderParamModelViewProjection, "modelViewProjection");
	locateShaderUniform(kShaderParamTextureData0, "textureSampler0");
}

void ShaderProgram::locateShaderAttribute(int id, const char *variableName)
{
	int index = -1;
	index = glGetAttribLocation(m_programId, variableName);
	if (index >= 0) {
		m_paramIndices[id] = index;
	}
}

void ShaderProgram::locateShaderUniform(int id, const char *variableName)
{
	int index = -1;
	index = glGetUniformLocation(m_programId, variableName);
	if (index >= 0) {
		m_paramIndices[id] = index;
	}
}

void ShaderProgram::formatProgramInfoLog()
{
	delete[] m_errorString;
	m_errorString = nullptr;

	int infologLength = 0;
	glGetShaderiv(m_programId, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		m_errorString = new char[infologLength];

		int charsWritten  = 0;
		glGetProgramInfoLog(m_programId, infologLength, &charsWritten, m_errorString);
	}
}
