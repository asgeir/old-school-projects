#pragma once

#include "ge2common.h"

#include <vector>

namespace ge2 {

class Compositor;
class CompositorEffect;
class Framebuffer;
class FullscreenQuad;
class Material;
class Shader;
class Texture2D;

typedef std::vector<CompositorEffect *> CompositorEffectList;

enum class CompositorBuffer
{
	Full,
	Quarter
};

class CrepuscularEffectSettings
{

};

class Compositor
{
	Compositor(const Compositor &other) = delete;

public:
	Compositor() = default;
	Compositor(Compositor &&rhs);
	~Compositor();

	Compositor &operator=(Compositor rhs);

	void construct(int width, int height);
	void destruct();

	void bindInputFramebuffer();
	void unbindInputFramebuffer();

	void compose(const CompositorEffectList &effects);

	Texture2D *inputColorBuffer(FragmentBuffer buffer);
	Texture2D *inputDepthBuffer();
	Texture2D *buffer(CompositorBuffer buf);

	void drawEffect(CompositorBuffer buffer, Material *material);

	void additiveBlendFilter(CompositorBuffer buf, Texture2D *tex1, Texture2D *tex2);
	void drawCrepuscularRaysFilter(const glm::vec2 &screenSpacePosition, Texture2D *lightStencil, float decay, float density, float exposure, float weight);
	void drawTextureFilter(CompositorBuffer buf, Texture2D *data);
	void gaussianBlurFilter(CompositorBuffer buf);

private:
	void clear();
	void bindBuffer(CompositorBuffer buffer);
	void unbindBuffer(CompositorBuffer buffer);

	struct BufferPair
	{
		Framebuffer *front = nullptr;
		Framebuffer *back = nullptr;
	};

	void swap(Compositor &rhs);

	Framebuffer    *m_inputFramebuffer = nullptr;
	BufferPair      m_fullBuffer;
	BufferPair      m_quarterBuffer;

	Material       *m_displayMaterial = nullptr;
	FullscreenQuad *m_displayQuad = nullptr;

	Material       *m_additiveBlendEffect = nullptr;
	Material       *m_crepuscularRaysEffect = nullptr;
	Material       *m_drawTextureEffect = nullptr;
	Material       *m_gaussianBlurEffect = nullptr;
};

class CompositorEffect
{
public:
	virtual ~CompositorEffect() {}

	virtual void compose(Compositor *compositor, bool firstInChain) = 0;
};

} // namespace ge2
