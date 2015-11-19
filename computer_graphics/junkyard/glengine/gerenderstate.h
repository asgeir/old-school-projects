#pragma once

#include <glm/glm.hpp>

#include <map>

class ShaderProgram;
class Texture2d;

typedef std::map<int, glm::mat4> UniformMap;

enum
{
	kClearFlagNone  = 0,
	kClearFlagColor = 1,
	kClearFlagDepth = 2,
	kClearFlagMask  = 3
};

class RenderState
{
public:
	RenderState();
	~RenderState();

	glm::vec4 clearColor() const;
	int clearFlags() const;
	ShaderProgram *shaderProgram() const;

	void setClearColor(glm::vec4 color);
	void setClearFlags(int clearFlags);
	void setShaderProgram(ShaderProgram *program);

	void setShaderUniform(int uniformId, glm::mat4 matrix);
	void unsetShaderUniform(int uniformId);

	void bindProgram();
	void unbindProgram();

	void bind();
	void unbind();

private:
	RenderState(const RenderState &other);
	RenderState &operator=(const RenderState &rhs);

	glm::vec4 m_clearColor = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
	int m_clearFlags = 0;
	ShaderProgram *m_shaderProgram = nullptr;
	UniformMap m_uniforms;
};
