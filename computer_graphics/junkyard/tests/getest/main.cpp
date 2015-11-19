#include "gecamera.h"
#include "gecommon.h"
#include "gequad.h"
#include "gerenderer.h"
#include "gerenderstate.h"
#include "geshader.h"
#include "geshaderprogram.h"

#include <glm/gtx/transform.hpp>

#include "SDL_events.h"

#include <iostream>

extern const char * const kApplicationTitle = "GLEngine Test";
extern const int kWindowWidth = 800;
extern const int kWindowHeight = 600;

namespace {

const char * const kSimpleVertexShader = R"(
	#version 150

	in vec4 vertexPosition;
	in vec3 vertexColor;
	uniform mat4 modelViewProjection;

	out vec4 color;

	void main()
	{
		gl_Position = modelViewProjection * vertexPosition;
		color = vec4(vertexColor, 1.0);
	}
)";

const char * const kVertexUnlitFragmentShader = R"(
	#version 150

	in vec4 color;
	out vec4 fragColor;

	void main()
	{
		fragColor = color;
	}
)";

ShaderProgram simpleVertexUnlitShaderProgram;
RenderState basicRenderState;
Renderer basicRenderer;
Camera camera;

Quad simpleQuad;

} // namespace

void geInitApplication()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	camera.constructOrthographic(0.0f, kWindowWidth, 0.0f, kWindowHeight, -1.0f, 1.0f);
	basicRenderer.setCamera(&camera);

	simpleQuad.construct(100, 100, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	simpleQuad.setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(kWindowWidth / 2.0f, kWindowHeight / 2.0f, 0.0f)));

	simpleVertexUnlitShaderProgram = ShaderProgram::loadFromString("simpleVertexUnlitShaderProgram", kSimpleVertexShader, kVertexUnlitFragmentShader);
	if (!simpleVertexUnlitShaderProgram.link()) {
		// Unable to link shaders
		std::cerr << simpleVertexUnlitShaderProgram.vertexShader().errorString() << std::endl << std::endl;
		std::cerr << simpleVertexUnlitShaderProgram.fragmentShader().errorString() << std::endl;
	}

	basicRenderState.setClearFlags(kClearFlagColor | kClearFlagDepth);
	basicRenderState.setShaderProgram(&simpleVertexUnlitShaderProgram);
}

void geHandleEvent(const SDL_Event &event)
{
}

void geDisplayFrame()
{
	RenderList renderables{&simpleQuad};
	basicRenderer.render(renderables, &basicRenderState);
}
