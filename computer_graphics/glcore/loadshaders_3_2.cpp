#include "gl_core_3_2.h"

#include "loadshaders_3_2.h"

#include <cstdio>

static void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        infoLog = new char[infologLength];
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        fprintf(stderr, "%s\n", infoLog);
        delete[] infoLog;
    }
}


static void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        infoLog = new char[infologLength];
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        fprintf(stderr, "%s\n", infoLog);
        delete[] infoLog;
    }
}

GLuint loadShaderFromString(GLenum shaderType, const char *shader, const char *errorIdentifier)
{
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, (const GLchar **)&shader, NULL);
    glCompileShader(shaderID);

    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        if (errorIdentifier) {
            fprintf(stderr, "Shader %s failed to compile:\n", errorIdentifier);
        } else {
            fprintf(stderr, "Unnamed shader failed to compile:\n");
        }
        printShaderInfoLog(shaderID);
        glDeleteShader(shaderID);
        shaderID = 0;
    }

    return shaderID;
}

GLuint loadShaderFromFile(GLenum shaderType, const char *path)
{
    GLuint shaderID;
    char *text = NULL;

    FILE *shaderFile = fopen(path, "r");
    if (shaderFile == NULL) {
        fprintf(stderr, "Unable to load shader: couldn't open file %s\n", path);
        return 0;
    }

    // Get the length of the file.
    fseek(shaderFile, 0, SEEK_END);
    size_t length = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    text = new char[length + 1];
    fread(text, sizeof(const char), length, shaderFile);
    text[length] = '\0';

    fclose(shaderFile);

    shaderID = loadShaderFromString(shaderType, text, path);

    delete[] text;
    return shaderID;
}

GLuint linkShader(GLuint vertexShaderID, GLuint fragmentShaderID)
{
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    GLint status;
    glGetProgramiv(programID, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        fprintf(stderr, "Shader program failed to link.\n");
        printProgramInfoLog(programID);
        glDeleteProgram(programID);
        programID = 0;
    }

    return programID;
}

GLuint loadShaders(const ShaderInfo shaders[])
{
    if (!shaders) {
        return 0;
    }

    GLuint programID = glCreateProgram();
    size_t i = 0;
    while (shaders[i].type != GL_NONE && shaders[i].path) {
        GLuint shaderID = loadShaderFromFile(shaders[i].type, shaders[i].path);
        if (!shaderID) {
            glDeleteProgram(programID);
            return 0;
        }

        glAttachShader(programID, shaderID);
        glDeleteShader(shaderID);
        ++i;
    }
    glLinkProgram(programID);

    GLint status;
    glGetProgramiv(programID, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        fprintf(stderr, "Shader program failed to link.\n");
        printProgramInfoLog(programID);
        glDeleteProgram(programID);
        programID = 0;
    }

    return programID;
}
