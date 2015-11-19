#pragma once

#include <glm/glm.hpp>

#include <map>
#include <string>

namespace ge2 {

class Cubemap;
class Shader;
class Texture2D;

class Material
{
public:
	enum DepthFunc {
		kDepthFuncLess,
		kDepthFuncLEqual,
		kDepthFuncEqual,
		kDepthFuncGEqual,
		kDepthFuncGreater,
		kDepthFuncNotEqual,
		kDepthFuncNever,
		kDepthFuncAlways
	};

	enum CullFace {
		kCullFaceFront,
		kCullFaceBack,
		kCullFaceFrontAndBack
	};

	Material() {}
	Material(Shader *shader) : m_shader{shader} {}

	Shader *shader() const { return m_shader; }

	glm::vec3 ambientColor() const { return m_ambient; }
	glm::vec3 diffuseColor() const { return m_diffuse; }
	glm::vec3 emissionColor() const { return m_emission; }
	glm::vec3 specular() const { return m_specular; }
	float shininess() const { return m_shininess; }

	void setAmbientColor(const glm::vec3 &color) { m_ambient = color; }
	void setDiffuseColor(const glm::vec3 &color) { m_diffuse = color; }
	void setEmissionColor(const glm::vec3 &color) { m_emission = color; }
	void setSpecularColor(const glm::vec3 &color) { m_specular = color; }
	void setShininess(float strength) { m_shininess = strength; }

	CullFace cullFace() const { return m_cullFace; }
	DepthFunc depthFunc() const { return m_depthFunc; }
	bool depthTest() const { return m_depthTest; }
	bool faceCulling() const { return m_faceCulling; }

	void setCullFace(CullFace face) { m_cullFace = face; }
	void setDepthFunc(DepthFunc depthFunc) { m_depthFunc = depthFunc; }
	void setDepthTest(bool depthTest) { m_depthTest = depthTest; }
	void setFaceCulling(bool faceCulling) { m_faceCulling = faceCulling; }

	void setUniform(const std::string &name, bool b) { m_intUniforms[name] = b; }
	void setUniform(const std::string &name, Cubemap *cubemap) { m_cubemapUniforms[name] = cubemap; }
	void setUniform(const std::string &name, float f) { m_floatUniforms[name] = f; }
	void setUniform(const std::string &name, int i) { m_intUniforms[name] = i; }
	void setUniform(const std::string &name, glm::mat4 mat4) { m_mat4Uniforms[name] = mat4; }
	void setUniform(const std::string &name, Texture2D *tex2D) { m_tex2DUniforms[name] = tex2D; }
	void setUniform(const std::string &name, glm::vec2 vec2) { m_vec2Uniforms[name] = vec2; }
	void setUniform(const std::string &name, glm::vec3 vec3) { m_vec3Uniforms[name] = vec3; }
	void setUniform(const std::string &name, glm::vec4 vec4) { m_vec4Uniforms[name] = vec4; }

	int bind();
	void unbind();

protected:
	void setShader(Shader *shader) { m_shader = shader; }

private:
	typedef std::map<std::string, Cubemap *>   CubemapMap;
	typedef std::map<std::string, float>       FloatMap;
	typedef std::map<std::string, int>         IntMap;
	typedef std::map<std::string, glm::mat4>   Mat4Map;
	typedef std::map<std::string, Texture2D *> Tex2DMap;
	typedef std::map<std::string, glm::vec2>   Vec2Map;
	typedef std::map<std::string, glm::vec3>   Vec3Map;
	typedef std::map<std::string, glm::vec4>   Vec4Map;

	Shader    *m_shader = nullptr;

	glm::vec3  m_ambient = glm::vec3{1.0f};
	glm::vec3  m_diffuse = glm::vec3{1.0f};
	glm::vec3  m_emission = glm::vec3{0.0f};
	glm::vec3  m_specular = glm::vec3{0.0f};
	float      m_shininess{1.0f};

	CullFace   m_cullFace = kCullFaceBack;
	DepthFunc  m_depthFunc = kDepthFuncLess;
	bool       m_depthTest = true;
	bool       m_faceCulling = true;
	CubemapMap m_cubemapUniforms;
	FloatMap   m_floatUniforms;
	IntMap     m_intUniforms;
	Mat4Map    m_mat4Uniforms;
	Tex2DMap   m_tex2DUniforms;
	Vec2Map    m_vec2Uniforms;
	Vec3Map    m_vec3Uniforms;
	Vec4Map    m_vec4Uniforms;
};

} // namespace ge2
