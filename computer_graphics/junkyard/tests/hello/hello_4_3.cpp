#include "gl_core_4_3.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_timer.h"
#include <Eigen/Dense>

#include "loadshaders_4_3.h"

#include <cmath>
#include <iostream>
#include <stdint.h>

namespace {

constexpr GLubyte *bufferOffset(size_t bytes)
{
    return static_cast<GLubyte *>(0) + bytes;
}

const int kWindowWidth = 800;
const int kWindowHeight = 600;

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0, mTransform = 1 };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

float rotationAngle = 0;
uint32_t oldTime = 0;
uint32_t currentTime = 0;

const GLuint NumVertices = 6;

void init()
{
    glGenVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Triangles]);

    Eigen::Vector2f vertices[NumVertices] = {
        { -0.90, -0.90 },
        {  0.85, -0.90 },
        { -0.90,  0.85 },
        {  0.90, -0.85 },
        {  0.90,  0.90 },
        { -0.85,  0.90 }
    };

    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "../tests/hello/hello_4_3.vert" },
        { GL_FRAGMENT_SHADER, "../tests/hello/hello_4_3.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = loadShaders(shaders);
    glUseProgram(program);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
    glEnableVertexAttribArray(vPosition);
}

static const float PI = 2*3.1415926;
static const float TWO_PI = 2*PI;

// http://www.opengl.org/sdk/docs/man/xhtml/glBindFragDataLocation.xml
// http://www.opengl.org/sdk/docs/man/xhtml/glBindAttribLocation.xml

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    oldTime = currentTime;
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - oldTime) / 1000.0f;

    rotationAngle = fmod(rotationAngle + (deltaTime * (PI / 60.0f)), TWO_PI);

    Eigen::Matrix2f rotation;
    rotation << cos(rotationAngle), -sin(rotationAngle),
                sin(rotationAngle),  cos(rotationAngle);
    glUniformMatrix2fv(mTransform, 1, GL_FALSE, rotation.data());

    glBindVertexArray(VAOs[Triangles]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

} // namespace

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an OpenGL capable window
    SDL_Window *window = SDL_CreateWindow(
        "Hello World",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
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
        }

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
