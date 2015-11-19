#include "gerenderer.h"
#include "gecamera.h"
#include "gerenderstate.h"
#include "geshaderprogram.h"

#include "gl_core_3_2.h"

void Renderable::render()
{
	switch (primitiveType()) {
	case kPrimitiveTypeTriangles:
		glDrawArrays(GL_TRIANGLES, 0, numVertices());
		break;

	default:
		break;
	}
}


void Renderer::render(const RenderList &renderables, RenderState *renderState)
{
	renderState->bindProgram();

	int clearFlags = renderState->clearFlags();
	GLbitfield glClearFlags = 0;
	if (clearFlags & kClearFlagColor) {
		glClearFlags |= GL_COLOR_BUFFER_BIT;
	}
	if (clearFlags & kClearFlagDepth) {
		glClearFlags |= GL_DEPTH_BUFFER_BIT;
	}
	glClear(glClearFlags);

	glm::mat4 viewProjection = m_camera->viewProjectionMatrix();
	for (auto renderable : renderables) {
		renderState->setShaderUniform(kShaderParamModelViewProjection, viewProjection * renderable->transform());
		renderState->bind();

		renderable->bind(renderState);
		renderable->render();
		renderable->unbind(renderState);

		renderState->unbind();
		renderState->unsetShaderUniform(kShaderParamModelViewProjection);
	}

	renderState->unbindProgram();
}
