#include "common.h"
#include "gecommon.h"
#include "gecamera.h"

#include <iostream>

ShaderProgram simpleVertexUnlitShaderProgram;
ShaderProgram ballVertexUnlitShaderProgram;
RenderState basicRenderState;
RenderState ballRenderState;
Renderer renderer;
Camera camera;

namespace {

const char kVertexShader[] = R"(
	#version 150

	in vec4 vertexPosition;
	in vec3 vertexColor;
	in vec2 textureCoordinates;
	uniform mat4 modelViewProjection;

	out vec4 color;
	out vec2 uv;

	void main()
	{
		gl_Position = modelViewProjection * vertexPosition;
		color = vec4(vertexColor, 1.0);
		uv = textureCoordinates;
	}
)";

const char kFragmentShader[] = R"(
	#version 150

	in vec4 color;
	out vec4 fragColor;

	void main()
	{
		fragColor = color;
	}
)";

const char kBallFragmentShader[] = R"(
	#version 150

	in vec4 color;
	in vec2 uv;
	out vec4 fragColor;

	void main()
	{
		vec2 middleOffset = uv - vec2(0.5f, 0.5f);
		float col = smoothstep(0.4f, 0.5f, sqrt(dot(middleOffset, middleOffset)));
		fragColor = color * (1 - col);
	}
)";

} // namespace

void initializeCommon()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	camera.constructOrthographic(0.0f, kWindowWidth, 0.0f, kWindowHeight, -1.0f, 1.0f);
	renderer.setCamera(&camera);

	simpleVertexUnlitShaderProgram = ShaderProgram::loadFromString("simpleVertexUnlitShaderProgram", kVertexShader, kFragmentShader);
	if (!simpleVertexUnlitShaderProgram.link()) {
		// Unable to link shaders
		std::cerr << simpleVertexUnlitShaderProgram.vertexShader().errorString() << std::endl << std::endl;
		std::cerr << simpleVertexUnlitShaderProgram.fragmentShader().errorString() << std::endl;
	}

	ballVertexUnlitShaderProgram = ShaderProgram::loadFromString("ballVertexUnlitShaderProgram", kVertexShader, kBallFragmentShader);
	if (!ballVertexUnlitShaderProgram.link()) {
		// Unable to link shaders
		std::cerr << ballVertexUnlitShaderProgram.vertexShader().errorString() << std::endl << std::endl;
		std::cerr << ballVertexUnlitShaderProgram.fragmentShader().errorString() << std::endl;
	}

	basicRenderState.setClearFlags(kClearFlagDepth);
	basicRenderState.setShaderProgram(&simpleVertexUnlitShaderProgram);

	ballRenderState.setClearFlags(kClearFlagColor | kClearFlagDepth);
	ballRenderState.setShaderProgram(&ballVertexUnlitShaderProgram);
}
