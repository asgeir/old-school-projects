#include "gl_core_3_2.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "gecommon.h"
#include "getime.h"

#include <iostream>

extern void geInitApplication();
extern void geHandleEvent(const SDL_Event &event);
extern void geDisplayFrame();

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
		kApplicationTitle,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		kWindowWidth, kWindowHeight,
		SDL_WINDOW_OPENGL
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
	geInitApplication();

	SDL_Event event;
	bool keepRunning = true;
	while (keepRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				keepRunning = false;
			}
			geHandleEvent(event);
		}

		Time::update();

		// Render scene
		geDisplayFrame();

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
