#include "gl_core_3_2.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "ge2common.h"
#include "ge2application.h"
#include "ge2renderer.h"
#include "ge2resourcemgr.h"
#include "ge2time.h"

#include <iostream>

extern ge2::Application *geConstructApplication(int argc, char *argv[]);

ge2::Renderer *ge2::geRenderer = nullptr;
ge2::ResourceManager *ge2::geResourceMgr = nullptr;

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
		"GL Engine 2",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600,
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

	ge2::geResourceMgr = new ge2::ResourceManager;
	ge2::geRenderer = new ge2::Renderer(window);
	ge2::Application *app = geConstructApplication(argc, argv);

	SDL_Event event;
	bool keepRunning = true;
	while (keepRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				keepRunning = false;
			}
			app->handleEvent(event);
		}

		ge2::Time::update();

		app->update();

		// Swap buffers
		SDL_GL_SwapWindow(window);
	}

	delete app;
	app = nullptr;

	delete ge2::geRenderer;
	ge2::geRenderer = nullptr;

	delete ge2::geResourceMgr;
	ge2::geResourceMgr = nullptr;

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(glcontext);

	// Done! Close the window, clean-up and exit the program.
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
