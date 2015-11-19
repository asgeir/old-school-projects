#include "gerenderstate.h"
#include "geshaderprogram.h"
#include "getexture2d.h"

#include <glm/gtc/type_ptr.hpp>

RenderState::RenderState()
{
}

RenderState::~RenderState()
{
}

glm::vec4 RenderState::clearColor() const
{
	return m_clearColor;
}

int RenderState::clearFlags() const
{
	return m_clearFlags;
}

ShaderProgram *RenderState::shaderProgram() const
{
	return m_shaderProgram;
}

void RenderState::setClearColor(glm::vec4 color)
{
	m_clearColor = color;
}

void RenderState::setClearFlags(int clearFlags)
{
	m_clearFlags = clearFlags & kClearFlagMask;
}

void RenderState::setShaderProgram(ShaderProgram *program)
{
	m_shaderProgram = program;
}

void RenderState::setShaderUniform(int uniformId, glm::mat4 matrix)
{
	m_uniforms[uniformId] = matrix;
}

void RenderState::unsetShaderUniform(int uniformId)
{
	m_uniforms.erase(uniformId);
}

void RenderState::bindProgram()
{
	if (m_shaderProgram) {
		m_shaderProgram->bind();
	}
}

void RenderState::unbindProgram()
{
	if (m_shaderProgram) {
		m_shaderProgram->unbind();
	}
}

void RenderState::bind()
{
	glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	if (m_shaderProgram) {
		ParamIndices indices = m_shaderProgram->paramIndices();
		for (auto uniform : m_uniforms) {
			auto index = indices.find(uniform.first);
			if (index != indices.end()) {
				glUniformMatrix4fv((*index).second, 1, GL_FALSE, glm::value_ptr(uniform.second));
			}
		}
	}

}

void RenderState::unbind()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}
