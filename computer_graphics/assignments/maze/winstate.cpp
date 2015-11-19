#include "winstate.h"

#include "ge2fsquad.h"
#include "ge2material.h"
#include "ge2renderer.h"
#include "ge2resourcemgr.h"
#include "ge2shader.h"
#include "ge2time.h"

#include <iostream>
#include <string>

using namespace ge2;

namespace {

const std::string kSimpleTextureFragmentShader = R"(
	#version 150

	in vec2 textureCoordinates;

	uniform sampler2D textureData;

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = texture(textureData, textureCoordinates);
	}
)";

}

WinState::WinState()
{
}

WinState::~WinState()
{
	delete m_fullscreenQuad;
}

void WinState::initialize()
{
	m_fullscreenQuad = new FullscreenQuad;

	m_winTexture = geResourceMgr->loadTexture2DFromFile("win_screen_texture", "maze/transporting.png");
	Shader *shader = geResourceMgr->loadShaderFromStrings(
		"win_screen_shader",
		FullscreenQuad::defaultVertexShader(),
		kSimpleTextureFragmentShader,
		{ "textureData" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error in win screen material" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}

	m_fullscreenQuadMaterial = geResourceMgr->createMaterial("win_sceeen_material", shader);
	m_fullscreenQuadMaterial->setUniform("textureData", m_winTexture);
}

void WinState::onTransitionIn(StateManager *manager, void *userdata)
{
	m_stateManager = manager;
	m_timeRemaining = 3.0f;
}

void *WinState::onTransitionOut()
{
	return nullptr;
}

void WinState::handleEvent(const SDL_Event &event)
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		SDL_Event quitEvent = { SDL_QUIT };
		SDL_PushEvent(&quitEvent);
	} else if (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_SPACE]) {
		m_stateManager->transition();
	}
}

void WinState::update()
{
	geRenderer->clear();

	m_fullscreenQuadMaterial->bind();
	m_fullscreenQuad->draw();
	m_fullscreenQuadMaterial->unbind();

	m_timeRemaining -= Time::deltaTime();
	if (m_timeRemaining <= 0.0f) {
		m_stateManager->transition();
	}
}
