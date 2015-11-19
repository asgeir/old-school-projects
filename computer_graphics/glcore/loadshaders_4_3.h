#pragma once

#include "gl_core_4_3.h"

GLuint loadShaderFromFile(GLenum shaderType, const char *path);
GLuint loadShaderFromString(GLenum shaderType, const char *shader, const char *errorIdentifier);
GLuint linkShader(GLuint vertexShaderID, GLuint fragmentShaderID,
    GLuint geometryShaderID, GLenum inGeomType, GLenum outGeomType, GLuint maxOutVerts);

typedef struct {
    GLenum type;
    const char *path;
} ShaderInfo;

GLuint loadShaders(const ShaderInfo shaders[]);
