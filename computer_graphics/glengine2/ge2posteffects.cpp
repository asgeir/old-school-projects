#include "ge2posteffects.h"

#include "ge2camera.h"
#include "ge2material.h"
#include "ge2resourcemgr.h"
#include "ge2texture2d.h"

namespace {

// http://www.glge.org/demos/fxaa/
const std::string kFullscreenAntiAliasingShader = R"(
	#version 150

	uniform sampler2D colorData;
	uniform vec2 inverseResolution;

	out vec4 ge_fragmentColor;

	#define FXAA_REDUCE_MIN (1.0/128.0)
	#define FXAA_REDUCE_MUL (1.0/8.0)
	#define FXAA_SPAN_MAX    8.0

	void main()
	{
		vec3 rgbNW = texture(colorData, (gl_FragCoord.xy + vec2(-1.0,-1.0)) * inverseResolution).xyz;
		vec3 rgbNE = texture(colorData, (gl_FragCoord.xy + vec2( 1.0,-1.0)) * inverseResolution).xyz;
		vec3 rgbSW = texture(colorData, (gl_FragCoord.xy + vec2(-1.0, 1.0)) * inverseResolution).xyz;
		vec3 rgbSE = texture(colorData, (gl_FragCoord.xy + vec2( 1.0, 1.0)) * inverseResolution).xyz;
		vec3 rgbM  = texture(colorData,  gl_FragCoord.xy                    * inverseResolution).xyz;
		vec3 luma = vec3(0.299, 0.587, 0.114);

		float lumaNW = dot(rgbNW, luma);
		float lumaNE = dot(rgbNE, luma);
		float lumaSW = dot(rgbSW, luma);
		float lumaSE = dot(rgbSE, luma);
		float lumaM  = dot(rgbM,  luma);
		float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
		float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

		vec2 dir;
		dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
		dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

		float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

		float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

		dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseResolution;

		vec3 rgbA = 0.5 * (
			texture(colorData, gl_FragCoord.xy * inverseResolution + dir * (1.0/3.0 - 0.5)).xyz +
			texture(colorData, gl_FragCoord.xy * inverseResolution + dir * (2.0/3.0 - 0.5)).xyz);

		vec3 rgbB = rgbA * 0.5 + 0.25 * (
			texture(colorData, gl_FragCoord.xy * inverseResolution + dir * -0.5).xyz +
			texture(colorData, gl_FragCoord.xy * inverseResolution + dir *  0.5).xyz);

		float lumaB = dot(rgbB, luma);

		if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
			ge_fragmentColor = vec4(rgbA, 1.0);
		} else {
			ge_fragmentColor = vec4(rgbB, 1.0);
		}
	}

)";

const std::string kHighPassFilterShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D colorData;
	uniform vec3 whitePoint;

	out vec4 ge_fragmentColor;

	void main()
	{
		vec3 color = texture(colorData, textureCoordinates).rgb;
		vec3 luma = vec3(0.2126, 0.7152, 0.0722);

		if (dot(color, luma) > dot(whitePoint, luma)) {
			ge_fragmentColor = vec4(color, 1.0);
		} else {
			ge_fragmentColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
)";

// http://frictionalgames.blogspot.com/2012/09/tech-feature-hdr-lightning.html
// http://maddieman.wordpress.com/2009/06/23/gamma-correction-and-linear-colour-space-simplified/
const std::string kTonemapShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D colorData;
	uniform float exposure;
	uniform float finalGamma;
	uniform vec3 whitePoint;

	out vec4 ge_fragmentColor;

	#define gammaCorrection(color, gamma) vec3(pow(color.r, 1.0 / gamma), pow(color.g, 1.0 / gamma), pow(color.b, 1.0 / gamma))

	vec3 uncharted2Tonemap(vec3 x)
	{
		float A = 0.15;
		float B = 0.50;
		float C = 0.10;
		float D = 0.20;
		float E = 0.02;
		float F = 0.30;

		return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
	}

	void main()
	{
		vec3 color = texture(colorData, textureCoordinates).rgb;
		color = uncharted2Tonemap(color * exposure) / uncharted2Tonemap(whitePoint);
		ge_fragmentColor = vec4(gammaCorrection(color, finalGamma), 1.0);
	}
)";

}

using namespace ge2;

AntiAliasingEffect::AntiAliasingEffect()
{
	m_antiAliasingFilter = geResourceMgr->compositorMaterial("std::anti-aliasing::anti-aliasing");
	if (!m_antiAliasingFilter) {
		m_antiAliasingFilter = geResourceMgr->loadCompositorMaterialFromString(
			"std::anti-aliasing::anti-aliasing",
			kFullscreenAntiAliasingShader,
			{ "colorData", "inverseResolution" });
	}
}

void AntiAliasingEffect::compose(Compositor *compositor, bool firstInChain)
{
	Texture2D *input = nullptr;
	if (firstInChain) {
		input = compositor->inputColorBuffer(FragmentBuffer::Color);
	} else {
		input = compositor->buffer(CompositorBuffer::Full);
	}

	m_antiAliasingFilter->setUniform("colorData", input);
	m_antiAliasingFilter->setUniform("inverseResolution", glm::vec2{
		1.0f / (float)compositor->buffer(CompositorBuffer::Full)->width(),
		1.0f / (float)compositor->buffer(CompositorBuffer::Full)->height()});

	compositor->drawEffect(CompositorBuffer::Full, m_antiAliasingFilter);
}

BloomEffect::BloomEffect(glm::vec3 whitePoint)
	: m_whitePoint{whitePoint}
{
	m_highPassFilter = geResourceMgr->compositorMaterial("std::bloom::high_pass");
	if (!m_highPassFilter) {
		m_highPassFilter = geResourceMgr->loadCompositorMaterialFromString(
			"std::bloom::high_pass",
			kHighPassFilterShader,
			{ "colorData", "whitePoint" });
	}
}

void BloomEffect::compose(Compositor *compositor, bool firstInChain)
{
	Texture2D *input = nullptr;
	if (firstInChain) {
		input = compositor->inputColorBuffer(FragmentBuffer::Color);
	} else {
		input = compositor->buffer(CompositorBuffer::Full);
	}

	m_highPassFilter->setUniform("colorData", input);
	m_highPassFilter->setUniform("whitePoint", m_whitePoint);

	compositor->drawEffect(CompositorBuffer::Quarter, m_highPassFilter);
	compositor->gaussianBlurFilter(CompositorBuffer::Quarter);
	compositor->gaussianBlurFilter(CompositorBuffer::Quarter);

	compositor->additiveBlendFilter(CompositorBuffer::Full, input, compositor->buffer(CompositorBuffer::Quarter));
}

CrepuscularRaysEffect::CrepuscularRaysEffect()
{
}

void CrepuscularRaysEffect::compose(Compositor *compositor, bool firstInChain)
{
	if (firstInChain) {
		compositor->drawTextureFilter(CompositorBuffer::Full, compositor->inputColorBuffer(FragmentBuffer::Color));
	}

	if (!m_camera || !m_lightNode || !m_lightStencil) {
		return;
	}

	glm::vec4 screenSpacePosition = m_camera->viewProjectionMatrix() * glm::vec4(m_lightNode->position(), 1.0f);
	screenSpacePosition = screenSpacePosition / screenSpacePosition[3];

	compositor->drawCrepuscularRaysFilter(0.5f * glm::vec2(screenSpacePosition) + glm::vec2{0.5}, m_lightStencil, m_decay, m_density, m_exposure, m_weight);
}

GlowEffect::GlowEffect()
{
}

void GlowEffect::compose(Compositor *compositor, bool firstInChain)
{
	compositor->drawTextureFilter(CompositorBuffer::Quarter, compositor->inputColorBuffer(FragmentBuffer::Glow));
	compositor->gaussianBlurFilter(CompositorBuffer::Quarter);
	compositor->gaussianBlurFilter(CompositorBuffer::Quarter);

	Texture2D *input = nullptr;
	if (firstInChain) {
		input = compositor->inputColorBuffer(FragmentBuffer::Color);
	} else {
		input = compositor->buffer(CompositorBuffer::Full);
	}

	compositor->additiveBlendFilter(CompositorBuffer::Full, input, compositor->buffer(CompositorBuffer::Quarter));
}

TonemapEffect::TonemapEffect(float exposure, float outputGamma, glm::vec3 whitePoint)
	: m_exposure{exposure}
	, m_outputGamma{outputGamma}
	, m_whitePoint{whitePoint}
{
	m_tonemapEffect = geResourceMgr->compositorMaterial("std::tonemap::tonemap");
	if (!m_tonemapEffect) {
		m_tonemapEffect = geResourceMgr->loadCompositorMaterialFromString(
			"std::tonemap::tonemap",
			kTonemapShader,
			{ "colorData", "exposure", "finalGamma", "whitePoint" });
	}
}

void TonemapEffect::compose(Compositor *compositor, bool firstInChain)
{
	Texture2D *input = nullptr;
	if (firstInChain) {
		input = compositor->inputColorBuffer(FragmentBuffer::Color);
	} else {
		input = compositor->buffer(CompositorBuffer::Full);
	}

	m_tonemapEffect->setUniform("colorData", input);
	m_tonemapEffect->setUniform("exposure", m_exposure);
	m_tonemapEffect->setUniform("finalGamma", m_outputGamma);
	m_tonemapEffect->setUniform("whitePoint", m_whitePoint);

	compositor->drawEffect(CompositorBuffer::Full, m_tonemapEffect);
}
