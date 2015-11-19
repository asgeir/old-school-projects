#include "ge2compositor.h"
#include "ge2common.h"
#include "ge2framebuffer.h"
#include "ge2fsquad.h"
#include "ge2material.h"
#include "ge2resourcemgr.h"
#include "ge2shader.h"
#include "ge2texture2d.h"

#include <iostream>

using namespace ge2;

namespace {

const std::string kAdditiveBlendShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D colorData1;
	uniform sampler2D colorData2;

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = texture(colorData1, textureCoordinates) + texture(colorData2, textureCoordinates);
	}
)";

const std::string kDisplayShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D colorData;

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = texture(colorData, textureCoordinates);
	}
)";

// http://fabiensanglard.net/lightScattering/index.php
const std::string kCrepuscularRaysShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform float exposure;
	uniform float decay;
	uniform float density;
	uniform float weight;
	uniform vec2 lightPositionOnScreen;
	uniform sampler2D lightStencil;

	out vec4 ge_fragmentColor;

	const int NUM_SAMPLES = 25;

	void main()
	{
		vec2 deltaTextCoord = vec2(textureCoordinates - lightPositionOnScreen);
		vec2 sampleCoordinates = textureCoordinates;
		deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
		float illuminationDecay = 1.0;

		for(int i = 0; i < NUM_SAMPLES ; i++)
		{
			sampleCoordinates -= deltaTextCoord;
			vec4 sample = texture(lightStencil, sampleCoordinates);

			sample *= illuminationDecay * weight;

			ge_fragmentColor += sample;

			illuminationDecay *= decay;
		}

		ge_fragmentColor *= exposure;
	}
)";

const std::string kDrawTextureShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D colorData;

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = texture(colorData, textureCoordinates);
	}
)";

// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling
const std::string kGaussianBlurLinearSamplingShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform bool horizontalBlur;
	uniform sampler2D colorData;
	uniform vec2 screenSize;

	out vec4 ge_fragmentColor;

	const float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
	const float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

	void main(void)
	{
		ge_fragmentColor = texture(colorData, textureCoordinates) * weight[0];
		for (int i = 1; i < 3; ++i) {
			vec2 samplePos = vec2(0.0, 0.0);
			if (horizontalBlur) {
				samplePos.x = offset[i] / screenSize.x;
			} else {
				samplePos.y = offset[i] / screenSize.y;
			}
			ge_fragmentColor += texture(colorData, textureCoordinates + samplePos) * weight[i];
			ge_fragmentColor += texture(colorData, textureCoordinates - samplePos) * weight[i];
		}
	}
)";

}

Compositor::Compositor(Compositor &&rhs)
{
	swap(rhs);
}

Compositor::~Compositor()
{
}

Compositor &Compositor::operator=(Compositor rhs)
{
	swap(rhs);
	return *this;
}

void Compositor::construct(int width, int height)
{
	destruct();

	m_inputFramebuffer = new Framebuffer;
	m_inputFramebuffer->construct(width, height, true, { FragmentBuffer::Color, FragmentBuffer::Glow, FragmentBuffer::CrepuscularRays });

	m_fullBuffer.front = new Framebuffer;
	m_fullBuffer.back = new Framebuffer;
	m_quarterBuffer.front = new Framebuffer;
	m_quarterBuffer.back = new Framebuffer;

	m_fullBuffer.front->construct(width, height, false, { FragmentBuffer::Color });
	m_fullBuffer.back->construct(width, height, false, { FragmentBuffer::Color });
	m_quarterBuffer.front->construct(width/2, height/2, false, { FragmentBuffer::Color });
	m_quarterBuffer.back->construct(width/2, height/2, false, { FragmentBuffer::Color });

	m_displayMaterial = geResourceMgr->compositorMaterial("_ge_internal_compositor_display");
	if (!m_displayMaterial) {
		m_displayMaterial = geResourceMgr->loadCompositorMaterialFromString(
			"_ge_internal_compositor_display",
			kDisplayShader,
			{ "colorData" });
	}

	m_displayQuad = new FullscreenQuad(m_displayMaterial);

	m_additiveBlendEffect = geResourceMgr->compositorMaterial("std::additiveBlend");
	if (!m_additiveBlendEffect) {
		m_additiveBlendEffect = geResourceMgr->loadCompositorMaterialFromString(
			"std::additiveBlend",
			kAdditiveBlendShader,
			{ "colorData1", "colorData2" });
	}

	m_crepuscularRaysEffect = geResourceMgr->compositorMaterial("std::crepuscularRays");
	if (!m_crepuscularRaysEffect) {
		m_crepuscularRaysEffect = geResourceMgr->loadCompositorMaterialFromString(
			"std::crepuscularRays",
			kCrepuscularRaysShader,
			{ "decay", "density", "exposure", "lightPositionOnScreen", "lightStencil", "weight" });
	}

	m_drawTextureEffect = geResourceMgr->compositorMaterial("std::drawTexture");
	if (!m_drawTextureEffect) {
		m_drawTextureEffect = geResourceMgr->loadCompositorMaterialFromString(
			"std::drawTexture",
			kDrawTextureShader,
			{ "colorData" });
	}

	m_gaussianBlurEffect = geResourceMgr->compositorMaterial("std::gaussianBlurLinearSampling");
	if (!m_gaussianBlurEffect) {
		m_gaussianBlurEffect = geResourceMgr->loadCompositorMaterialFromString(
			"std::gaussianBlurLinearSampling",
			kGaussianBlurLinearSamplingShader,
			{ "horizontalBlur", "colorData", "screenSize" });
	}
}

void Compositor::destruct()
{
	delete m_inputFramebuffer;
	m_inputFramebuffer = nullptr;

	delete m_fullBuffer.front; m_fullBuffer.front = nullptr;
	delete m_fullBuffer.back; m_fullBuffer.back = nullptr;
	delete m_quarterBuffer.front; m_quarterBuffer.front = nullptr;
	delete m_quarterBuffer.back; m_quarterBuffer.back = nullptr;

	delete m_displayQuad;
	m_displayQuad = nullptr;

	m_displayMaterial = nullptr;
}

void Compositor::bindInputFramebuffer()
{
	m_inputFramebuffer->bind();
}

void Compositor::unbindInputFramebuffer()
{
	m_inputFramebuffer->unbind();
}

void Compositor::compose(const CompositorEffectList &effects)
{
	bool isFirstEffect = true;
	for (auto effect : effects) {
		effect->compose(this, isFirstEffect);
		isFirstEffect = false;
	}

	clear();

	m_displayMaterial->setUniform("colorData", buffer(CompositorBuffer::Full));
	m_displayMaterial->bind();

	m_displayQuad->draw();

	m_displayMaterial->unbind();
}

Texture2D *Compositor::inputColorBuffer(FragmentBuffer buffer)
{
	return m_inputFramebuffer->colorBuffer(buffer);
}

Texture2D *Compositor::inputDepthBuffer()
{
	return m_inputFramebuffer->depthBuffer();
}

Texture2D *Compositor::buffer(CompositorBuffer buf)
{
	switch (buf) {
	case CompositorBuffer::Full:
		return m_fullBuffer.back->colorBuffer(FragmentBuffer::Color);
	case CompositorBuffer::Quarter:
		return m_quarterBuffer.back->colorBuffer(FragmentBuffer::Color);
	default:
		std::cerr << "Compositor::buffer - Unhandled compositor buffer" << std::endl;
		return nullptr;
	}
}

void Compositor::drawEffect(CompositorBuffer buffer, Material *material)
{
	bindBuffer(buffer);

	material->bind();
	m_displayQuad->draw();
	material->unbind();

	unbindBuffer(buffer);
}

void Compositor::additiveBlendFilter(CompositorBuffer buf, Texture2D *tex1, Texture2D *tex2)
{
	m_additiveBlendEffect->setUniform("colorData1", tex1);
	m_additiveBlendEffect->setUniform("colorData2", tex2);
	drawEffect(buf, m_additiveBlendEffect);
}

void Compositor::drawCrepuscularRaysFilter(const glm::vec2 &screenSpacePosition, Texture2D *lightStencil, float decay, float density, float exposure, float weight)
{
	drawTextureFilter(CompositorBuffer::Quarter, lightStencil);
	gaussianBlurFilter(CompositorBuffer::Quarter);

	m_crepuscularRaysEffect->setUniform("decay", decay);
	m_crepuscularRaysEffect->setUniform("density", density);
	m_crepuscularRaysEffect->setUniform("exposure", exposure);
	m_crepuscularRaysEffect->setUniform("lightPositionOnScreen", screenSpacePosition);
	m_crepuscularRaysEffect->setUniform("lightStencil", buffer(CompositorBuffer::Quarter));
	m_crepuscularRaysEffect->setUniform("weight", weight);
	drawEffect(CompositorBuffer::Quarter, m_crepuscularRaysEffect);

	additiveBlendFilter(CompositorBuffer::Full, buffer(CompositorBuffer::Full), buffer(CompositorBuffer::Quarter));
}

void Compositor::drawTextureFilter(CompositorBuffer buf, Texture2D *data)
{
	m_drawTextureEffect->setUniform("colorData", data);
	drawEffect(buf, m_drawTextureEffect);
}

void Compositor::gaussianBlurFilter(CompositorBuffer buf)
{
	glm::vec2 screenSize{(float)buffer(buf)->width(), (float)buffer(buf)->height()};
	m_gaussianBlurEffect->setUniform("screenSize", screenSize);

	m_gaussianBlurEffect->setUniform("horizontalBlur", true);
	m_gaussianBlurEffect->setUniform("colorData", buffer(buf));
	drawEffect(buf, m_gaussianBlurEffect);

	m_gaussianBlurEffect->setUniform("horizontalBlur", false);
	m_gaussianBlurEffect->setUniform("colorData", buffer(buf));
	drawEffect(buf, m_gaussianBlurEffect);
}

void Compositor::clear()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

void Compositor::bindBuffer(CompositorBuffer buffer)
{
	switch (buffer) {
	case CompositorBuffer::Full:
		m_fullBuffer.front->bind();
		break;
	case CompositorBuffer::Quarter:
		m_quarterBuffer.front->bind();
		break;
	default:
		std::cerr << "Compositor::bindBuffer - Unhandled compositor buffer" << std::endl;
		break;
	}

	clear();
}

void Compositor::unbindBuffer(CompositorBuffer buffer)
{
	switch (buffer) {
	case CompositorBuffer::Full:
		m_fullBuffer.front->unbind();
		std::swap(m_fullBuffer.front, m_fullBuffer.back);
		break;
	case CompositorBuffer::Quarter:
		m_quarterBuffer.front->unbind();
		std::swap(m_quarterBuffer.front, m_quarterBuffer.back);
		break;
	default:
		std::cerr << "Compositor::bindBuffer - Unhandled compositor buffer" << std::endl;
		break;
	}
}

void Compositor::swap(Compositor &rhs)
{
	std::swap(m_inputFramebuffer, rhs.m_inputFramebuffer);
	std::swap(m_fullBuffer, rhs.m_fullBuffer);
	std::swap(m_quarterBuffer, rhs.m_quarterBuffer);
	std::swap(m_displayMaterial, rhs.m_displayMaterial);
	std::swap(m_displayQuad, rhs.m_displayQuad);
}
