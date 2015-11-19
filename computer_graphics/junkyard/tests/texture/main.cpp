#include "gl_core_3_2.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include <glm/gtx/transform.hpp>

#include "common.h"
#include "quad.h"
#include "texture2d.h"
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
	//fragColor = vec4(textureCoord, 0.0f, 1.0f);
	fragColor = texture(texData, textureCoord);
}
)";

struct ShaderProgram
{
	GLuint programId = 0;
	ProgramIndices indices;
};

ShaderProgram texturedUnlitProgram;

const glm::mat4 kPostprocessProjection =
	glm::ortho(0.0f, (float)kWindowWidth, 0.0f, (float)kWindowHeight, -1.0f, 1.0f) *
	glm::translate(glm::vec3((kWindowWidth/2), (kWindowHeight/2), 0));

Quad fullscreenQuad;
Texture2D texture;

GLuint loadBMP_custom(const char * imagepath)
{
        printf("Reading image %s\n", imagepath);

        // Data read from the header of the BMP file
        unsigned char header[54];
        unsigned int dataPos;
        unsigned int imageSize;
        unsigned int width, height;
        // Actual RGB data
        unsigned char * data;

        // Open the file
        FILE * file = fopen(imagepath,"rb");
        if (!file) {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); return 0;}

        // Read the header, i.e. the 54 first bytes

        // If less than 54 byes are read, problem
        if ( fread(header, 1, 54, file)!=54 ){
                printf("Not a correct BMP file\n");
                return 0;
        }
        // A BMP files always begins with "BM"
        if ( header[0]!='B' || header[1]!='M' ){
                printf("Not a correct BMP file\n");
                return 0;
        }
        // Make sure this is a 24bpp file
        if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
        if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

        // Read the information about the image
        dataPos    = *(int*)&(header[0x0A]);
        imageSize  = *(int*)&(header[0x22]);
        width      = *(int*)&(header[0x12]);
        height     = *(int*)&(header[0x16]);

        // Some BMP files are misformatted, guess missing information
        if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
        if (dataPos==0)      dataPos=54; // The BMP header is done that way

        // Create a buffer
        data = new unsigned char [imageSize];

        // Read the actual data from the file into the buffer
        fread(data,1,imageSize,file);

        // Everything is in memory now, the file wan be closed
        fclose (file);

        // Create one OpenGL texture
        GLuint textureID;
        glGenTextures(1, &textureID);

        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Give the image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

        // OpenGL has now copied the data. Free our own version
        delete [] data;

        // Poor filtering, or ...
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // ... nice trilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Return the ID of the texture we just created
        return textureID;
}

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

	texturedUnlitProgram = loadProgram("kVertexShader", kVertexShader, "kTexturedUnlitFragmentShader", kTexturedUnlitFragmentShader);

	texture.construct(loadBMP_custom("circle.bmp"));
	fullscreenQuad.construct(kWindowWidth, kWindowHeight, &texture);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(texturedUnlitProgram.programId);
	fullscreenQuad.draw(texturedUnlitProgram.indices, kPostprocessProjection);
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
