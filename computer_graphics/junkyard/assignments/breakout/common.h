#pragma once

#include "geshaderprogram.h"
#include "gerenderer.h"
#include "gerenderstate.h"

extern ShaderProgram simpleVertexUnlitShaderProgram;
extern ShaderProgram ballVertexUnlitShaderProgram;
extern RenderState basicRenderState;
extern RenderState ballRenderState;
extern Renderer renderer;
extern Camera camera;

void initializeCommon();
