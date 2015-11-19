#include "gl_core_3_2.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include <glm/gtx/transform.hpp>

#include "common.h"
#include "framebuffer.h"
#include "quad.h"
#include "texture2d.h"
#include "time.h"
#include "loadshaders_3_2.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace {

const char kVertexShader[] = R"(
#version 150

in vec4 vertex;
in vec3 color;
in vec2 uv;
uniform mat4 modelViewProjection;

out vec4 vertexColor;
out vec2 textureCoord;

void main()
{
	gl_Position = modelViewProjection * vertex;
	vertexColor = vec4(color, 1.0);
	textureCoord = uv;
}
)";
const char kVertexUnlitFragmentShader[] = R"(
#version 150

in vec4 vertexColor;
out vec4 fragColor;

void main()
{
	fragColor = vertexColor;
}
)";
const char kTexturedUnlitFragmentShader[] = R"(
#version 150

in vec2 textureCoord;
uniform sampler2D texData;

out vec4 fragColor;

void main()
{
	fragColor = texture(texData, textureCoord);
}
)";

struct ShaderProgram
{
	GLuint programId = 0;
	ProgramIndices indices;
};

ShaderProgram vertexUnlitProgram;
ShaderProgram texturedUnlitProgram;

const glm::mat4 kPostprocessProjection =
	glm::ortho(0.0f, (float)kWindowWidth, 0.0f, (float)kWindowHeight, -1.0f, 1.0f) *
	glm::translate(glm::vec3((kWindowWidth/2), (kWindowHeight/2), 0));

FrameBuffer framebuffer;
Quad fullscreenQuad;

Quad quad;

ProgramIndices getProgramIndices(GLuint program)
{
	ProgramIndices indices;
	indices.vertexIndex = glGetAttribLocation(program, "vertex");
	indices.colorIndex = glGetAttribLocation(program, "color");
	indices.textureIndex = glGetUniformLocation(program, "texData");
	indices.uvIndex = glGetAttribLocation(program, "uv");
	indices.modelViewProjectionIndex = glGetUniformLocation(program, "modelViewProjection");

	return std::move(indices);
}

ShaderProgram loadProgram(const char *vertexShaderName, const char *vertexShader, const char *fragmentShaderName, const char *fragmentShader)
{
	ShaderProgram program;

	GLuint vs = loadShaderFromString(GL_VERTEX_SHADER, vertexShader, vertexShaderName);
	GLuint fs = loadShaderFromString(GL_FRAGMENT_SHADER, fragmentShader, fragmentShaderName);

	program.programId = linkShader(vs, fs);
	program.indices = getProgramIndices(program.programId);

	return std::move(program);
}

void init()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	vertexUnlitProgram = loadProgram("kVertexShader", kVertexShader, "kVertexUnlitFragmentShader", kVertexUnlitFragmentShader);
	texturedUnlitProgram = loadProgram("kVertexShader", kVertexShader, "kTexturedUnlitFragmentShader", kTexturedUnlitFragmentShader);

	framebuffer.construct(kWindowWidth, kWindowHeight, kFrameBufferColor | kFrameBufferDepth);
	fullscreenQuad.construct(kWindowWidth, kWindowHeight, framebuffer.colorBuffer());

	quad.construct(100, 100, glm::vec3{1.0f, 0.0f, 0.0f});
}

const glm::mat4 kProjection = glm::ortho(0.0f, (float)kWindowWidth, 0.0f, (float)kWindowHeight, -1.0f, 1.0f);

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(vertexUnlitProgram.programId);
	glm::mat4 modelViewProjection = kProjection * glm::translate(glm::vec3((kWindowWidth/2), (kWindowHeight/2), 0));
	quad.draw(vertexUnlitProgram.indices, modelViewProjection);
}

void renderPostprocessEffects()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(texturedUnlitProgram.programId);
	fullscreenQuad.draw(texturedUnlitProgram.indices, kPostprocessProjection);
}

void display()
{
	framebuffer.bind();
	renderScene();
	framebuffer.unbind();

	renderPostprocessEffects();
}

void dumpTexture(Texture2D *tex, const char *fileName)
{
	tex->bind();

	int size = 4 * sizeof(char) * kWindowHeight * kWindowWidth;
	char *buf = (char *)malloc(size);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buf);

	FILE *f = fopen(fileName, "wb");
	fwrite(buf, sizeof(char), size, f);
	fclose(f);

	free(buf);

	tex->unbind();
}

} // namespace

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Create an OpenGL capable window
	SDL_Window *window = SDL_CreateWindow(
		"Render to Texture",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		kWindowWidth, kWindowHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if (!window) {
		std::cerr << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	// Create an OpenGL context associated with the window.
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	if (!glcontext) {
		std::cerr << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	// Load extensions
	if (ogl_LoadFunctions() != ogl_LOAD_SUCCEEDED) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}

	// Initialize displayables
	init();

	SDL_Event event;
	bool keepRunning = true;
	while (keepRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				keepRunning = false;
			}
			if (event.type == SDL_KEYDOWN) {
				const Uint8 *state = SDL_GetKeyboardState(NULL);
				if (state[SDL_SCANCODE_F]) {
					dumpTexture(framebuffer.colorBuffer(), "framebuffer.data");
				} else if (state[SDL_SCANCODE_D]) {
					dumpTexture(framebuffer.depthBuffer(), "depthbuffer.data");
				}
			}
		}

		Time::update();

		// Render scene
		display();

		// Swap buffers
		SDL_GL_SwapWindow(window);
	}

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(glcontext);

	// Done! Close the window, clean-up and exit the program.
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
