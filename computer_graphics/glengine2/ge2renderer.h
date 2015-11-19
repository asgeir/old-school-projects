#pragma once

#include "ge2common.h"

#include <glm/glm.hpp>
#include <SDL_video.h>

#include <array>
#include <cstddef>
#include <vector>

namespace ge2 {

class Camera;
class CubeFramebuffer;
class Framebuffer;
class Material;
class Mesh;
class Node;

const int kRendererMaxLights = 10;

struct LightProperties
{
	uint32_t  isEnabled{0};
	uint32_t  isLocal{0};
	uint32_t  isSpot{0};
	int32_t   shadowId{0};
	glm::vec4 ambient{0.0f};
	glm::vec4 color{0.0f};
	glm::vec4 position{0.0f};
	glm::vec4 halfVector{0.0f};
	glm::vec3 coneDirection{0.0f};
	float     spotCosCutoff{0.0f};
	float     spotExponent{0.0f};
	float     radius{0.0f};
	float     cutoff{0.0f};
	uint32_t _padding2{0};
};
static_assert(sizeof(LightProperties) == 112, "LightProperties is not packed");
static_assert(offsetof(LightProperties, isEnabled)     ==   0, "isEnabled is at an incorrect offset");
static_assert(offsetof(LightProperties, isLocal)       ==   4, "isLocal is at an incorrect offset");
static_assert(offsetof(LightProperties, isSpot)        ==   8, "isSpot is at an incorrect offset");
static_assert(offsetof(LightProperties, shadowId)      ==  12, "shadowId is at an incorrect offset");
static_assert(offsetof(LightProperties, ambient)       ==  16, "ambient is at an incorrect offset");
static_assert(offsetof(LightProperties, color)         ==  32, "color is at an incorrect offset");
static_assert(offsetof(LightProperties, position)      ==  48, "position is at an incorrect offset");
static_assert(offsetof(LightProperties, halfVector)    ==  64, "halfVector is at an incorrect offset");
static_assert(offsetof(LightProperties, coneDirection) ==  80, "coneDirection is at an incorrect offset");
static_assert(offsetof(LightProperties, spotCosCutoff) ==  92, "spotCosCutoff is at an incorrect offset");
static_assert(offsetof(LightProperties, spotExponent)  ==  96, "spotExponent is at an incorrect offset");
static_assert(offsetof(LightProperties, radius)        == 100, "radius is at an incorrect offset");
static_assert(offsetof(LightProperties, cutoff)        == 104, "cutoff is at an incorrect offset");

class Light
{
public:
	enum class Type
	{
		Directional,
		Point,
		Spot
	};

	virtual ~Light() {}

	glm::vec3 ambientColor() const { return m_ambientColor; }
	bool      castsShadows() const { return m_castsShadows; }
	glm::vec3 color() const { return m_color; }
	bool      enabled() const { return m_enabled; }

	void setAmbientColor(const glm::vec3 &color) { m_ambientColor = color; }
	void setCastsShadows(bool castsShadows) { m_castsShadows = castsShadows; }
	void setColor(const glm::vec3 &color) { m_color = color; }
	void setEnabled(bool enabled) { m_enabled = enabled; }

	virtual void populateLightProperties(LightProperties &properties) = 0;
	virtual Type lightType() const = 0;

private:
	glm::vec3 m_ambientColor{0.0f};
	bool      m_castsShadows{false};
	glm::vec3 m_color{0.0f};
	bool      m_enabled{true};
};

class DirectionalLight : public Light
{
public:
	glm::vec3 direction() const { return m_direction; }

	glm::vec2 depth() const { return m_depth; }
	glm::vec2 horizontal() const { return m_horizontal; }
	glm::vec2 vertical() const { return m_vertical; }

	void setDirection(const glm::vec3 &direction) { m_direction = direction; }

	void setDepth(const glm::vec2 &depth) { m_depth = depth; }
	void setHorizontal(const glm::vec2 &horizontal) { m_horizontal = horizontal; }
	void setVertical(const glm::vec2 &vertical) { m_vertical = vertical; }

	virtual void populateLightProperties(LightProperties &properties) override;
	virtual Type lightType() const override { return Type::Directional; }

private:
	glm::vec3 m_direction{0.0f};

	glm::vec2 m_depth{-10.0f, 10.0f};
	glm::vec2 m_horizontal{-10.0f, 10.0f};
	glm::vec2 m_vertical{-10.0f, 10.0f};
};

class PointLight : public Light
{
public:
	float cutoff() const { return m_cutoff; }
	float radius() const { return m_radius; }

	void setCutoff(float cutoff) { m_cutoff = cutoff; }
	void setRadius(float radius) { m_radius = radius; }

	virtual void populateLightProperties(LightProperties &properties) override;
	virtual Type lightType() const override { return Type::Point; }

private:
	float     m_cutoff{0.001f};
	float     m_radius{1.0f};
};

class SpotLight : public Light
{
public:
	float angle() const { return m_angle; }
	float cutoff() const { return m_cutoff; }
	float exponent() const { return m_exponent; }
	float length() const { return m_length; }

	void setAngle(float angle) { m_angle = angle; }
	void setCutoff(float cutoff) { m_cutoff = cutoff; }
	void setExponent(float exponent) { m_exponent = exponent; }
	void setLength(float length) { m_length = length; }

	virtual void populateLightProperties(LightProperties &properties) override;
	virtual Type lightType() const override { return Type::Spot; }

private:
	float     m_angle = degToRad(45);
	float     m_cutoff = 0.001;
	float     m_exponent = 320.0f;
	float     m_length = 1.0f;
};

struct LightInfo
{
	LightInfo(const glm::mat4 &mm, Light *l) : modelMatrix{mm}, light{l} {}
	glm::mat4 modelMatrix;
	Light *light = nullptr;
	LightProperties *props = nullptr;
};

typedef std::vector<LightInfo> LightInfoList;

struct Renderable
{
	Renderable(const glm::mat4 &mm, Node *n, bool cs = true) : modelMatrix{mm}, node{n}, castsShadows{cs} {}

	glm::mat4 modelMatrix;
	Node *node;
	bool castsShadows;
};

typedef std::vector<Renderable> RenderableList;

class Renderer
{
public:
	enum class ClearBits : uint16_t
	{
		Color,
		Depth,
		Stencil
	};

	enum
	{
		kClearFlagNone    = 0,
		kClearFlagColor   = (1 << (uint16_t)ClearBits::Color),
		kClearFlagDepth   = (1 << (uint16_t)ClearBits::Depth),
		kClearFlagStencil = (1 << (uint16_t)ClearBits::Stencil),
		kClearFlagAll     = kClearFlagColor|kClearFlagDepth|kClearFlagStencil
	};

	Renderer(SDL_Window *window);
	~Renderer();

	int height() { return m_windowHeight; }
	int width() { return m_windowWidth; }
	SDL_Window *window() { return m_window; }

	Camera *activeCamera();
	glm::vec4 clearColorValue() { return m_clearColor; }
	float clearDepthValue() { return m_clearDepth; }
	int clearStencilValue() { return m_clearStencil; }
	float specularStrength() { return m_specularStrength; }

	void setActiveCameraAndLights(Camera *camera, LightInfoList &lights);
	void setClearColorValue(const glm::vec4 &color);
	void setClearDepthValue(float value);
	void setClearStencilValue(int value);
	void setSpecularStrength(float strength);
	void setTitle(const char *title);

	void resize(int width, int height);

	void clear(int clearFlags = kClearFlagAll);
	void render(RenderableList &renderables, bool isShadowPass = false, Material *overrideMaterial = nullptr);
	void updateShadowMaps(RenderableList &renderables);

private:
	struct ShadowData
	{
		Framebuffer     *shadowMap = nullptr;
		CubeFramebuffer *cubeShadowMap = nullptr;
		float            shadowBias = 0.0f;
		float            shadowFarPlane = 0.0f;
		glm::mat4        lightViewProjectionMatrix{1.0f};
	};

	static const size_t kNumShadowMaps = 2;

	typedef std::array<LightProperties, kRendererMaxLights> LightArray;
	typedef std::array<ShadowData, kNumShadowMaps> ShadowDataArray;

	SDL_Window           *m_window = nullptr;
	int                   m_windowHeight = 0;
	int                   m_windowWidth = 0;
	unsigned int          m_lightsBufferObject = 0;

	Camera               *m_camera = nullptr;
	LightInfoList         m_activeLights;
	LightArray            m_lightProperties;
	glm::vec4             m_clearColor{0.0f, 0.0f, 0.0f, 1.0f};
	float                 m_clearDepth{1.0f};
	int                   m_clearStencil{0};
	float                 m_specularStrength{1.0f};

	Material             *m_shadowMapMaterial = nullptr;
	ShadowDataArray       m_shadowData;
};

} // namespace ge2
