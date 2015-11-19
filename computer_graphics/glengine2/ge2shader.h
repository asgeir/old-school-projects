#pragma once

#include "ge2common.h"

#include "gl_core_3_2.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ge2 {

typedef std::map<std::string, GLint> IndexMap;

class Shader
{
public:
	Shader() = default;
	Shader(const Shader &other) = delete;
	Shader(Shader &&rvalue);
	~Shader();

	Shader &operator=(Shader rvalue);

	static Shader *loadFromString(
		const std::string &vertexShader,
		const std::string &fragmentShader,
		const StringList &uniforms);

	bool hasError() const;
	std::string errorString() const;

	bool hasAttribute(ShaderAttribute attribute) const;
	bool hasFragmentOutput(FragmentBuffer buffer) const;
	bool hasUniform(const std::string &name) const;

	GLint uniform(const std::string &name) const;

	void setUniform(const std::string &name, float f);
	void setUniform(const std::string &name, int i);
	void setUniform(const std::string &name, const glm::mat3 &mat3);
	void setUniform(const std::string &name, const glm::mat4 &mat4);
	void setUniform(const std::string &name, const glm::vec2 &vec2);
	void setUniform(const std::string &name, const glm::vec3 &vec3);
	void setUniform(const std::string &name, const glm::vec4 &vec4);

	void setUniformBlock(const std::string &name, int bindingPoint);

	void bind();
	void unbind();

private:
	void swap(Shader &other);
	void bindStandardLocations();
	void populateIndices(const StringList &uniforms);
	void setStandardUniformBlocks();
	std::string getShaderInfoLog(GLint shader);
	std::string getProgramInfoLog();
	GLint compileShader(GLenum shaderType, const std::string &source);

	GLuint      m_programId = 0;
	std::string m_errorString;
	IndexMap    m_uniforms;
	IndexMap    m_uniformBlocks;
};

} // namespace ge2
