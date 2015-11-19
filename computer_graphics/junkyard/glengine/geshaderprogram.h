#pragma once

#include "geshader.h"

#include "gl_core_3_2.h"

#include <map>

enum {
	kShaderParamVertexPosition,
	kShaderParamVertexColor,
	kShaderParamTextureCoordinate,
	kShaderParamModelViewProjection,
	kShaderParamTextureData0
};

typedef std::map<int, GLint> ParamIndices;

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const ShaderProgram &other) = delete;
	ShaderProgram(ShaderProgram &&rvalue);
	~ShaderProgram();

	ShaderProgram &operator=(ShaderProgram rhs);

	static ShaderProgram loadFromString(const char *name, const char *vertexShader, const char *fragmentShader);

	ParamIndices paramIndices();

	Shader &fragmentShader() { return m_fragmentShader; }
	Shader &vertexShader() { return m_vertexShader; }

	void setVertexShader(Shader shader);
	void setFragmentShader(Shader shader);

	bool bind();
	void unbind();
	bool link();

private:
	void swap(ShaderProgram &other);
	void populateParamIndices();
	void locateShaderAttribute(int id, const char *variableName);
	void locateShaderUniform(int id, const char *variableName);
	void formatProgramInfoLog();

	GLuint m_programId = 0;
	char *m_errorString = nullptr;
	ParamIndices m_paramIndices;

	Shader m_vertexShader;
	Shader m_fragmentShader;
};
