#include "gl_core_4_3.h"
#include "SDL.h"
#include "SDL_keyboard.h"
#include "SDL_opengl.h"
#include "SDL_scancode.h"
#include "SDL_timer.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "loadshaders_4_3.h"

#include <cmath>
#include <iostream>
#include <stdint.h>

namespace {

const float PI = 2*3.1415926;
const float TWO_PI = 2*PI;

constexpr GLubyte *bufferOffset(size_t bytes)
{
    return static_cast<GLubyte *>(0) + bytes;
}

constexpr float degToRad(float angle)
{
    return angle * PI / 180;
}

constexpr float radToDeg(float angle)
{
    return angle * 180 / PI;
}

const int kWindowWidth = 800;
const int kWindowHeight = 600;

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, ColorBuffer, IndexBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor = 1 };
enum Uniform_IDs { mModelView = 0, mProjection = 1 };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

uint32_t oldTime = 0;
uint32_t currentTime = 0;

const GLuint NumVertices = 8;
const GLuint NumIndices = 36;

Eigen::Matrix4f cameraTransform;

void initCube()
{
    glGenVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Triangles]);

    Eigen::Vector3f vertices[NumVertices] = {
        { -0.5,  0.5,  0.5 },
        {  0.5,  0.5,  0.5 },
        {  0.5,  0.5, -0.5 },
        { -0.5,  0.5, -0.5 },

        { -0.5, -0.5,  0.5 },
        {  0.5, -0.5,  0.5 },
        {  0.5, -0.5, -0.5 },
        { -0.5, -0.5, -0.5 }
    };

    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
    glEnableVertexAttribArray(vPosition);

    struct { float r, g, b; } colors[NumVertices] = {
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },

        { 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 0.0 }
    };

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ColorBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
    glEnableVertexAttribArray(vColor);

    GLubyte indices[NumIndices] = {
        // top
        0, 1, 3,
        3, 1, 2,

        // front
        4, 5, 0,
        0, 5, 1,

        // bottom
        5, 4, 6,
        6, 4, 7,

        // back
        6, 7, 2,
        2, 7, 3,

        // left side
        7, 4, 3,
        3, 4, 0,

        // right side
        5, 6, 1,
        1, 6, 2
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[IndexBuffer]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void init()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "../tests/persp/persp_4_3.vert" },
        { GL_FRAGMENT_SHADER, "../tests/persp/persp_4_3.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = loadShaders(shaders);
    glUseProgram(program);

    initCube();

    cameraTransform << 1, 0, 0,  0,
                       0, 1, 0,  0,
                       0, 0, 1, -1,
                       0, 0, 0,  1;
}

Eigen::Matrix4f createPerspectiveProjection(float fov, float width, float height, float zNear, float zFar)
{
    float h = cos(0.5f * fov) / sin(0.5f * fov);
    float w = h * height / width;
    float p = -(zFar+zNear) / (zFar-zNear);
    float q = -(2*zFar*zNear) / (zFar-zNear);

    Eigen::Matrix4f result;
    result << w, 0,  0, 0,
              0, h,  0, 0,
              0, 0,  p, q,
              0, 0, -1, 0;

    return result;
}

const Eigen::Matrix4f kProjection = createPerspectiveProjection(degToRad(40), kWindowWidth, kWindowHeight, 0.1f, 100);
float rotationAngle = 0;

const float cameraSpeed = 1.5f;

void moveCamera(float deltaTime)
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    Eigen::Vector3f direction = Eigen::Vector3f::Zero();
    if (state[SDL_SCANCODE_W]) {
        direction += Eigen::Vector3f::UnitZ();
    } else if (state[SDL_SCANCODE_S]) {
        direction -= Eigen::Vector3f::UnitZ();
    } else if (state[SDL_SCANCODE_A]) {
        direction += Eigen::Vector3f::UnitX();
    } else if (state[SDL_SCANCODE_D]) {
        direction -= Eigen::Vector3f::UnitX();
    }

    if (direction.squaredNorm()) {
        direction.normalize();
        cameraTransform.block<3, 1>(0, 3) += direction * cameraSpeed * deltaTime;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    oldTime = currentTime;
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - oldTime) / 1000.0f;

    moveCamera(deltaTime);

    Eigen::Matrix4f modelTransform;
    modelTransform << 1, 0, 0,  cos(rotationAngle),
                      0, 1, 0,  sin(rotationAngle),
                      0, 0, 1, -2,
                      0, 0, 0,  1;

    rotationAngle = fmod(rotationAngle + (deltaTime * (PI / 15.0f)), TWO_PI);
    Eigen::Matrix4f modelRotation = Eigen::Matrix4f::Identity();
    modelRotation.topLeftCorner<3, 3>() = Eigen::Matrix3f(
        Eigen::AngleAxisf(rotationAngle, Eigen::Vector3f::UnitX()) *
        Eigen::AngleAxisf(rotationAngle, Eigen::Vector3f::UnitY()));

    Eigen::Matrix4f modelView = cameraTransform * modelTransform * modelRotation;
    glUniformMatrix4fv(mModelView, 1, GL_FALSE, modelView.data());
    glUniformMatrix4fv(mProjection, 1, GL_FALSE, kProjection.data());

    glBindVertexArray(VAOs[Triangles]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[IndexBuffer]);
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_BYTE, bufferOffset(0));
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
        "Prespective Projection Test",
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
