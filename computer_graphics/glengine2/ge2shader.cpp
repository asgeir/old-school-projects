#include "ge2shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <utility>

using namespace ge2;

namespace {

const char * const kAttributeNames[] = {
	"ge_position",
	"ge_textureCoordinates",
	"ge_normal"
};

const char * const kFragmentBufferNames[] = {
	"ge_fragmentColor",
	"ge_fragmentGlow",
	"ge_fragmentCrepuscularRays"
};

const char * const kStandardUniforms[] = {
	"ge_materialProperties.ambient",
	"ge_materialProperties.diffuse",
	"ge_materialProperties.emission",
	"ge_materialProperties.shininess",
	"ge_materialProperties.specular",
	"ge_modelView",
	"ge_modelViewProjection",
	"ge_modelViewProjectionLight1",
	"ge_modelViewProjectionLight2",
	"ge_normalMatrix",
	"ge_oneOverShadowMapResolution",
	"ge_shadowBias1",
	"ge_shadowBias2",
	"ge_shadowCubeMap1",
	"ge_shadowCubeMap2",
	"ge_shadowFarPlane1",
	"ge_shadowFarPlane2",
	"ge_shadowMap1",
	"ge_shadowMap2",
	"ge_specularStrength",
	"ge_totalSeconds",
	"ge_viewMatrixLinear",
	"ge_viewMatrixLinearInverse"
};

const char * const kStandardUniformBlockNames[] = {
	"ge_Lights"
};

}

Shader::Shader(Shader &&rvalue)
{
	swap(rvalue);
}

Shader::~Shader()
{
	glDeleteProgram(m_programId);
}

Shader &Shader::operator=(Shader rvalue)
{
	swap(rvalue);
	return *this;
}

Shader *Shader::loadFromString(
	const std::string &vertexShader,
	const std::string &fragmentShader,
	const StringList &uniforms)
{
	Shader *shader = new Shader;
	GLint status;

	shader->m_programId = glCreateProgram();
	if (!shader->m_programId) {
		shader->m_errorString = "Unable to create program";
		return shader;
	}

	GLuint vertexShaderId = shader->compileShader(GL_VERTEX_SHADER, vertexShader);
	if (!vertexShaderId) {
		glDeleteProgram(shader->m_programId);
		shader->m_programId = 0;
		return shader;
	}
	glAttachShader(shader->m_programId, vertexShaderId);
	glDeleteShader(vertexShaderId);

	GLuint fragmentShaderId = shader->compileShader(GL_FRAGMENT_SHADER, fragmentShader);
	if (!fragmentShaderId) {
		glDeleteProgram(shader->m_programId);
		shader->m_programId = 0;
		return shader;
	}
	glAttachShader(shader->m_programId, fragmentShaderId);
	glDeleteShader(fragmentShaderId);

	shader->bindStandardLocations();
	glLinkProgram(shader->m_programId);

	glGetProgramiv(shader->m_programId, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		shader->m_errorString = shader->getProgramInfoLog();
		glDeleteProgram(shader->m_programId);
		shader->m_programId = 0;
		return shader;
	}

	shader->populateIndices(uniforms);
	shader->setStandardUniformBlocks();

	return shader;
}

bool Shader::hasError() const
{
	return m_programId == 0;
}

std::string Shader::errorString() const
{
	return m_errorString;
}

bool Shader::hasAttribute(ShaderAttribute attribute) const
{
	return glGetAttribLocation(m_programId, kAttributeNames[(GLint)attribute]) == (GLint)attribute;
}

bool Shader::hasFragmentOutput(FragmentBuffer buffer) const
{
	return glGetFragDataLocation(m_programId, kFragmentBufferNames[(GLint)buffer]) == (GLint)buffer;
}

bool Shader::hasUniform(const std::string &name) const
{
	return m_uniforms.find(name) != m_uniforms.end();
}

GLint Shader::uniform(const std::string &name) const
{
	auto it = m_uniforms.find(name);
	return (it != m_uniforms.end()) ? it->second : -1;
}

void Shader::setUniform(const std::string &name, float f)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniform1f(it->second, f);
	}
}

void Shader::setUniform(const std::string &name, int i)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniform1i(it->second, i);
	}
}

void Shader::setUniform(const std::string &name, const glm::mat3 &mat3)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniformMatrix3fv(it->second, 1, GL_FALSE, glm::value_ptr(mat3));
	}
}

void Shader::setUniform(const std::string &name, const glm::mat4 &mat4)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniformMatrix4fv(it->second, 1, GL_FALSE, glm::value_ptr(mat4));
	}
}

void Shader::setUniform(const std::string &name, const glm::vec2 &vec2)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniform2f(it->second, vec2[0], vec2[1]);
	}
}

void Shader::setUniform(const std::string &name, const glm::vec3 &vec3)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniform3f(it->second, vec3[0], vec3[1], vec3[2]);
	}
}

void Shader::setUniform(const std::string &name, const glm::vec4 &vec4)
{
	auto it = m_uniforms.find(name);
	if (it != m_uniforms.end()) {
		glUniform4f(it->second, vec4[0], vec4[1], vec4[2], vec4[3]);
	}
}

void Shader::setUniformBlock(const std::string &name, int bindingPoint)
{
	auto it = m_uniformBlocks.find(name);
	if (it != m_uniformBlocks.end()) {
		glUniformBlockBinding(m_programId, it->second, bindingPoint);
	}
}

void Shader::bind()
{
	glUseProgram(m_programId);
}

void Shader::unbind()
{
	glUseProgram(0);
}

void Shader::swap(Shader &other)
{
	std::swap(m_programId, other.m_programId);
	std::swap(m_errorString, other.m_errorString);
	std::swap(m_uniforms, other.m_uniforms);
}

void Shader::bindStandardLocations()
{
	glBindAttribLocation(m_programId, (GLint)ShaderAttribute::VertexPosition, kAttributeNames[(GLint)ShaderAttribute::VertexPosition]);
	glBindAttribLocation(m_programId, (GLint)ShaderAttribute::VertexTextureCoordinates, kAttributeNames[(GLint)ShaderAttribute::VertexTextureCoordinates]);
	glBindAttribLocation(m_programId, (GLint)ShaderAttribute::VertexNormals, kAttributeNames[(GLint)ShaderAttribute::VertexNormals]);

	glBindFragDataLocation(m_programId, (GLint)FragmentBuffer::Color, kFragmentBufferNames[(GLint)FragmentBuffer::Color]);
	glBindFragDataLocation(m_programId, (GLint)FragmentBuffer::Glow, kFragmentBufferNames[(GLint)FragmentBuffer::Glow]);
	glBindFragDataLocation(m_programId, (GLint)FragmentBuffer::CrepuscularRays, kFragmentBufferNames[(GLint)FragmentBuffer::CrepuscularRays]);
}

void Shader::populateIndices(const StringList &uniforms)
{
	bind();

	for (auto uniform : uniforms) {
		m_uniforms[uniform] = glGetUniformLocation(m_programId, uniform.c_str());
	}

	for (auto uniform : kStandardUniforms) {
		m_uniforms[uniform] = glGetUniformLocation(m_programId, uniform);
	}

	for (auto uniformBlock : kStandardUniformBlockNames) {
		m_uniformBlocks[uniformBlock] = glGetUniformBlockIndex(m_programId, uniformBlock);
	}

	unbind();
}

void Shader::setStandardUniformBlocks()
{
	setUniformBlock(kStandardUniformBlockNames[(GLint)StandardUniformBlocks::Lights], (GLint)StandardUniformBlocks::Lights);
}

std::string Shader::getShaderInfoLog(GLint shader)
{
	std::string infoLog;

	int infologLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		char *info = new char[infologLength];

		int charsWritten = 0;
		glGetShaderInfoLog(shader, infologLength, &charsWritten, info);

		infoLog = std::string(info, charsWritten);
		delete[] info;
	}

	return infoLog;
}

std::string Shader::getProgramInfoLog()
{
	std::string infoLog;

	int infologLength = 0;
	glGetShaderiv(m_programId, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		char *info = new char[infologLength];

		int charsWritten  = 0;
		glGetProgramInfoLog(m_programId, infologLength, &charsWritten, info);

		infoLog = std::string(info, charsWritten);
		delete[] info;
	}

	return infoLog;
}

GLint Shader::compileShader(GLenum shaderType, const std::string &source)
{
	std::string shaderName = (shaderType == GL_VERTEX_SHADER) ? "vertex" : "fragment";
	GLint status;

	GLuint shaderId = glCreateShader(shaderType);
	if (shaderId) {
		const GLchar *sourceArray[] = { source.c_str() };
		glShaderSource(shaderId, 1, sourceArray, NULL);
		glCompileShader(shaderId);
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			m_errorString = shaderName + " shader error: " + getShaderInfoLog(shaderId);
			glDeleteShader(shaderId);
			shaderId = 0;
		}
	} else {
		m_errorString = "Unable to create " + shaderName + " shader";
	}

	return shaderId;
}
