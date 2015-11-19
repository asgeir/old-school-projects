#include "gl_core_3_2.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "loadshaders_3_2.h"

#include <cmath>
#include <iostream>
#include <stdint.h>

namespace {

constexpr GLubyte *bufferOffset(size_t bytes)
{
    return static_cast<GLubyte *>(0) + bytes;
}

const char kVertexShader[] = R"(
#version 150

in vec4 vPosition;
in vec3 vColor;
uniform mat4 mModelViewProjection;

out vec4 fColor;

void main()
{
    gl_Position = mModelViewProjection * vPosition;
    fColor = vec4(vColor, 1.0);
}
)";
const char kFragmentShader[] = R"(
#version 150

in vec4 fColor;
out vec4 fragColor;

void main()
{
    fragColor = fColor;
}
)";

const int kWindowWidth = 800;
const int kWindowHeight = 600;

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, ColorBuffer, NumBuffers };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices = 6;

GLint vPositionIndex = -1;
GLint vColorIndex = -1;
GLint mModelViewProjectionIndex = -1;

const float squareWidth = 100.0f;
const float squareSpeed = 2 * squareWidth;
Eigen::Matrix4f squareTransform;
Eigen::Vector2f squareDirection;

void initSquare()
{
    glGenVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Triangles]);

    Eigen::Vector3f vertices[NumVertices] = {
        { -0.5, -0.5, 0.0 },
        {  0.5, -0.5, 0.0 },
        { -0.5,  0.5, 0.0 },
        { -0.5,  0.5, 0.0 },
        {  0.5, -0.5, 0.0 },
        {  0.5,  0.5, 0.0 }
    };

    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(vPositionIndex, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
    glEnableVertexAttribArray(vPositionIndex);

    struct { float r, g, b; } colors[NumVertices] = {
        { 0.0, 0.0, 1.0 },
        { 0.0, 0.0, 1.0 },
        { 0.0, 0.0, 1.0 },
        { 0.0, 0.0, 1.0 },
        { 0.0, 0.0, 1.0 },
        { 0.0, 0.0, 1.0 }
    };

    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ColorBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glVertexAttribPointer(vColorIndex, 3, GL_FLOAT, GL_FALSE, 0, bufferOffset(0));
    glEnableVertexAttribArray(vColorIndex);

    squareDirection = Eigen::Vector2f::Zero();
    squareTransform << squareWidth,           0, 0, (kWindowWidth/2),
                                 0, squareWidth, 0, (kWindowHeight/2),
                                 0,           0, 1, 0,
                                 0,           0, 0, 1;
}

void init()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint vertexShader = loadShaderFromString(GL_VERTEX_SHADER, kVertexShader, "kVertexShader");
    GLuint fragmentShader = loadShaderFromString(GL_FRAGMENT_SHADER, kFragmentShader, "kFragmentShader");

    GLuint program = linkShader(vertexShader, fragmentShader);
    glUseProgram(program);

    vPositionIndex = glGetAttribLocation(program, "vPosition");
    vColorIndex = glGetAttribLocation(program, "vColor");
    mModelViewProjectionIndex = glGetUniformLocation(program, "mModelViewProjection");

    initSquare();
}

Eigen::Matrix4f createOrthographicProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float a = 2.0 / (right - left);
    float b = 2.0 / (top - bottom);
    float c = 2.0 / (zFar - zNear);
    float d = -(right + left) / (right - left);
    float e = -(top + bottom) / (top - bottom);
    float f = -(zFar + zNear) / (zFar - zNear);

    Eigen::Matrix4f result;
    result << a, 0, 0, d,
              0, b, 0, e,
              0, 0, c, f,
              0, 0, 0, 1;

    return result;
}

const Eigen::Matrix4f kProjection = createOrthographicProjection(0, kWindowWidth, 0, kWindowHeight, -1, 1);

uint32_t oldTime = 0;
uint32_t currentTime = 0;

void updateSquare()
{
    oldTime = currentTime;
    currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - oldTime) / 1000.0f;

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    squareDirection = Eigen::Vector2f::Zero();
    if (state[SDL_SCANCODE_UP]) {
        squareDirection += Eigen::Vector2f::UnitY();
    }
    if (state[SDL_SCANCODE_DOWN]) {
        squareDirection -= Eigen::Vector2f::UnitY();
    }
    if (state[SDL_SCANCODE_LEFT]) {
        squareDirection -= Eigen::Vector2f::UnitX();
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        squareDirection += Eigen::Vector2f::UnitX();
    }

    if (squareDirection.squaredNorm()) {
        squareDirection.normalize();

        auto pos = squareTransform.topRightCorner<2, 1>();
        if (pos(0) <= (squareWidth/2) && squareDirection(0) < 0){
            squareDirection(0) = 0;
            pos(0) = (squareWidth/2);
        }
        if (pos(0) >= (kWindowWidth-(squareWidth/2)) && squareDirection(0) > 0) {
            squareDirection(0) = 0;
            pos(0) = (kWindowWidth-(squareWidth/2));
        }
        if (pos(1) <= (squareWidth/2) && squareDirection(1) < 0) {
            squareDirection(1) = 0;
            pos(1) = (squareWidth/2);
        }
        if (pos(1) >= (kWindowHeight-(squareWidth/2)) && squareDirection(1) > 0) {
            squareDirection(1) = 0;
            pos(1) = (kWindowHeight-(squareWidth/2));
        }

        if (squareDirection.squaredNorm()) {
            squareDirection.normalize();
            pos += squareDirection * squareSpeed * deltaTime;
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    updateSquare();

    Eigen::Matrix4f modelViewProjection = kProjection * squareTransform;
    glUniformMatrix4fv(mModelViewProjectionIndex, 1, GL_FALSE, modelViewProjection.data());

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an OpenGL capable window
    SDL_Window *window = SDL_CreateWindow(
        "Assignment 1.2",
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
