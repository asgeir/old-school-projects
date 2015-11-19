#include "ge2material.h"
#include "ge2cubemap.h"
#include "ge2shader.h"
#include "ge2texture2d.h"

#include "gl_core_3_2.h"

using namespace ge2;

int Material::bind()
{
	if (!m_shader) {
		return 0;
	}

	if (m_faceCulling) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (m_depthTest) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	switch (m_depthFunc) {
	case kDepthFuncLess:
		glDepthFunc(GL_LESS);
		break;

	case kDepthFuncLEqual:
		glDepthFunc(GL_LEQUAL);
		break;

	case kDepthFuncEqual:
		glDepthFunc(GL_EQUAL);
		break;

	case kDepthFuncGEqual:
		glDepthFunc(GL_GEQUAL);
		break;

	case kDepthFuncGreater:
		glDepthFunc(GL_GREATER);
		break;

	case kDepthFuncNotEqual:
		glDepthFunc(GL_NOTEQUAL);
		break;

	case kDepthFuncNever:
		glDepthFunc(GL_NEVER);
		break;

	case kDepthFuncAlways:
		glDepthFunc(GL_ALWAYS);
		break;

	default:
		break;
	}

	switch (m_cullFace) {
	case kCullFaceFront:
		glCullFace(GL_FRONT);
		break;

	case kCullFaceBack:
		glCullFace(GL_BACK);
		break;

	case kCullFaceFrontAndBack:
		glCullFace(GL_FRONT_AND_BACK);
		break;

	default:
		break;
	}

	m_shader->bind();

	int nextTextureUnit = 0;
	for (auto it : m_tex2DUniforms) {
		if (it.second) {
			int textureUnit = nextTextureUnit++;
			glActiveTexture(GL_TEXTURE0 + textureUnit);

			it.second->bind();

			m_shader->setUniform(it.first, textureUnit);
		}
	}
	glActiveTexture(GL_TEXTURE0);

	for (auto it : m_cubemapUniforms) {
		if (it.second) {
			int textureUnit = nextTextureUnit++;
			glActiveTexture(GL_TEXTURE0 + textureUnit);

			it.second->bind();

			m_shader->setUniform(it.first, textureUnit);
		}
	}
	glActiveTexture(GL_TEXTURE0);

	for (auto it : m_floatUniforms) {
		m_shader->setUniform(it.first, it.second);
	}
	for (auto it : m_intUniforms) {
		m_shader->setUniform(it.first, it.second);
	}
	for (auto it : m_mat4Uniforms) {
		m_shader->setUniform(it.first, it.second);
	}
	for (auto it : m_vec2Uniforms) {
		m_shader->setUniform(it.first, it.second);
	}
	for (auto it : m_vec3Uniforms) {
		m_shader->setUniform(it.first, it.second);
	}
	for (auto it : m_vec4Uniforms) {
		m_shader->setUniform(it.first, it.second);
	}

	m_shader->setUniform("ge_materialProperties.ambient", m_ambient);
	m_shader->setUniform("ge_materialProperties.diffuse", m_diffuse);
	m_shader->setUniform("ge_materialProperties.emission", m_emission);
	m_shader->setUniform("ge_materialProperties.shininess", m_shininess);
	m_shader->setUniform("ge_materialProperties.specular", m_specular);

	return nextTextureUnit;
}

void Material::unbind()
{
	if (!m_shader) {
		return;
	}

	for (size_t i = 0; i < m_tex2DUniforms.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);

	m_shader->unbind();
}
