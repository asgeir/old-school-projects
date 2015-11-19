#pragma once

#include "gl_core_3_2.h"

GLuint loadShaderFromFile(GLenum shaderType, const char *path);
GLuint loadShaderFromString(GLenum shaderType, const char *shader, const char *errorIdentifier);
GLuint linkShader(GLuint vertexShaderID, GLuint fragmentShaderID);

typedef struct {
    GLenum type;
    const char *path;
} ShaderInfo;

GLuint loadShaders(const ShaderInfo shaders[]);
