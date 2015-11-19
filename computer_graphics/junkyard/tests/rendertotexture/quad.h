#pragma once

#include "common.h"
#include "gl_core_3_2.h"
#include <glm/glm.hpp>

class Texture2D;

class Quad
{
public:
    static const int NumVertices = 6;

    Quad();
    ~Quad();

    void construct(int size);
    void construct(int width, int height);
    void construct(int width, int height, const glm::vec3 &color);
    void construct(int width, int height, Texture2D *texture);
    void construct(int width, int height,
        const glm::vec3 &topLeftColor, const glm::vec3 &topRightColor,
        const glm::vec3 &bottomLeftColor, const glm::vec3 &bottomRightColor);
    void destruct();

    void draw(const ProgramIndices &indices, const glm::mat4 &modelViewProjection);

private:
    Quad(const Quad &other);
    Quad &operator=(const Quad &rhs);

    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    Texture2D *textureObject = nullptr;
};
