#include "crepuscularapp.h"

using namespace ge2;

namespace {

const std::string kSimpleVertexShader = R"(
	#ge_include "standard/default.vs"
)";

const std::string kSimpleFragmentShader = R"(
	#version 150

	#ge_include "standard/lighting.fs"

	out vec4 ge_fragmentColor;
	out vec4 ge_fragmentCrepuscularRays;

	void main()
	{
		ge_fragmentColor = lighting(vec4(1.0));
		ge_fragmentCrepuscularRays = vec4(0.0, 0.0, 0.0, 1.0);
	}
)";

const std::string kLightSourceFragmentShader = R"(
	#version 150
	#ge_include "standard/noise4d.glsl"

	in vec3 normal;

	uniform vec3 lightColor;
	uniform float ge_totalSeconds;

	out vec4 ge_fragmentColor;
	out vec4 ge_fragmentCrepuscularRays;

	float oscillator(float scale, float offset)
	{
		return scale * sin((offset + ge_totalSeconds) / scale);
	}

	void main()
	{
		float noiseValue = 0.5 * snoise(vec4(2.0 * normal, oscillator(5.0, 13.0)));
		noiseValue = 0.25 * snoise(vec4(4.0 * normal + vec3(21.0, 15.0, -12.0), oscillator(11.0, 27.0))) + noiseValue;

		vec4 rgba = vec4(lightColor * (noiseValue * 0.5 + 0.5), 1.0);
		ge_fragmentColor = rgba;
		ge_fragmentCrepuscularRays = rgba;
	}
)";

const glm::vec3 kColorBlue{0.15f, 0.35f, 1.0f};
const glm::vec3 kColorOrange{1.0f, 0.5f, 0.0f};

}

CrepuscularApplication::CrepuscularApplication(int argc, char *argv[])
{
	geRenderer->setTitle("GL Engine 2 - Crepuscular Rays Test");

	geResourceMgr->setAssetDirectory("../assets");

	if (argc >= 2) {
		geResourceMgr->setAssetDirectory(argv[1]);
	}

	Shader *simpleShader = geResourceMgr->loadShaderFromStrings(
		"simple_shader",
		kSimpleVertexShader,
		kSimpleFragmentShader,
		{ });
	if (simpleShader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << simpleShader->errorString() << std::endl;
	}
	Material *simpleMaterial = geResourceMgr->createMaterial("simple_material", simpleShader);
	simpleMaterial->setDiffuseColor(glm::vec3{1.0f});

	Shader *lightSourceShader = geResourceMgr->loadShaderFromStrings(
		"light_source_shader",
		kSimpleVertexShader,
		kLightSourceFragmentShader,
		{ "lightColor" });
	if (lightSourceShader->hasError()) {
		std::cerr << "Light source shader compilation error" << std::endl;
		std::cerr << lightSourceShader->errorString() << std::endl;
	}
	Material *blueLightSourceMaterial = geResourceMgr->createMaterial("blue_light_source_material", lightSourceShader);
	Material *orangeLightSourceMaterial = geResourceMgr->createMaterial("orange_light_source_material", lightSourceShader);
	blueLightSourceMaterial->setUniform("lightColor", kColorBlue);
	orangeLightSourceMaterial->setUniform("lightColor", kColorOrange);

	int lightFramebufferHeight = geRenderer->height() / 2;
	int lightFramebufferWidth = geRenderer->width() / 2;

	m_blueLightFramebuffer = new ge2::Framebuffer;
	m_blueLightFramebuffer->construct(lightFramebufferWidth, lightFramebufferHeight, true, { FragmentBuffer::CrepuscularRays });

	m_orangeLightFramebuffer = new ge2::Framebuffer;
	m_orangeLightFramebuffer->construct(lightFramebufferWidth, lightFramebufferHeight, true, { FragmentBuffer::CrepuscularRays });

	m_antiAliasingEffect = new ge2::AntiAliasingEffect;
	m_blueCrepuscularRaysEffect = new ge2::CrepuscularRaysEffect;
	m_orangeCrepuscularRaysEffect = new ge2::CrepuscularRaysEffect;
	m_tonemapEffect = new ge2::TonemapEffect;
	m_compositorEffects.push_back(m_antiAliasingEffect);
	m_compositorEffects.push_back(m_blueCrepuscularRaysEffect);
	m_compositorEffects.push_back(m_orangeCrepuscularRaysEffect);
	m_compositorEffects.push_back(m_tonemapEffect);

	m_compositor = new Compositor;
	m_compositor->construct(geRenderer->width(), geRenderer->height());

	m_camera = new DebugCamera;
	m_scene = new Node;

	Mesh *cubeMesh = geResourceMgr->createCube("cube", 0.5f);
	cubeMesh->setMaterial(simpleMaterial);
	cubeMesh->construct();

	for (float x = -2.5; x < 3.5; ++x) {
		for (float y = -2.5f; y < 3.5f; ++y) {
			for (float z = -2.5; z < 3.5; ++z) {
				Node *node = new Node;
				node->setPosition(glm::vec3{x, y, z});
				node->setMeshList({ cubeMesh });
				m_scene->addChild(node);
			}
		}
	}

	// blue sphere

	Mesh *blueSphereMesh = geResourceMgr->createSphere("blue_sphere", 0.25f);
	blueSphereMesh->setMaterial(blueLightSourceMaterial);
	blueSphereMesh->construct();

	m_blueLightSource = new Node;
	m_blueLightSource->setMeshList({ blueSphereMesh });
	m_scene->addChild(m_blueLightSource);

	PointLight *bluePointLight = new PointLight;
	bluePointLight->setColor(kColorBlue);
	bluePointLight->setCastsShadows(true);
	m_blueLightSource->setLight(bluePointLight);

	m_blueCrepuscularRaysEffect->setCamera(m_camera->camera());
	m_blueCrepuscularRaysEffect->setLightNode(m_blueLightSource);
	m_blueCrepuscularRaysEffect->setLightStencil(m_blueLightFramebuffer->colorBuffer(FragmentBuffer::CrepuscularRays));

	// orange sphere

	Mesh *orangeSphereMesh = geResourceMgr->createSphere("orange_sphere", 0.25f);
	orangeSphereMesh->setMaterial(orangeLightSourceMaterial);
	orangeSphereMesh->construct();

	m_orangeLightSource = new Node;
	m_orangeLightSource->setMeshList({ orangeSphereMesh });
	m_scene->addChild(m_orangeLightSource);

	PointLight *orangePointLight = new PointLight;
	orangePointLight->setColor(kColorOrange);
	orangePointLight->setCastsShadows(true);
	m_orangeLightSource->setLight(orangePointLight);

	m_orangeCrepuscularRaysEffect->setCamera(m_camera->camera());
	m_orangeCrepuscularRaysEffect->setLightNode(m_orangeLightSource);
	m_orangeCrepuscularRaysEffect->setLightStencil(m_orangeLightFramebuffer->colorBuffer(FragmentBuffer::CrepuscularRays));
}

CrepuscularApplication::~CrepuscularApplication()
{
	delete m_scene;
	delete m_compositor;
	delete m_camera;
	delete m_antiAliasingEffect;
	delete m_blueCrepuscularRaysEffect;
	delete m_orangeCrepuscularRaysEffect;
	delete m_tonemapEffect;
	delete m_blueLightFramebuffer;
	delete m_orangeLightFramebuffer;
}

void CrepuscularApplication::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		SDL_Event quitEvent = { SDL_QUIT };
		SDL_PushEvent(&quitEvent);
	}
}

void CrepuscularApplication::update()
{
	m_camera->update();

	m_blueLightSource->setPosition(glm::vec3{3.0f, 0.0f, 0.0f} * glm::sin(0.25f * Time::totalSeconds()));
	m_orangeLightSource->setPosition(glm::vec3{0.0f, 3.0f, 0.0f} * glm::cos(0.25f * Time::totalSeconds()));

	LightInfoList lights;
	RenderableList renderables;
	NodeTreeVisitor treeVisitor;
	treeVisitor.visitNodeTree(m_scene, {
		[&renderables, &lights] (Node *node, const glm::mat4 &modelMatrix) {
			if (node->meshCount()) {
				renderables.push_back({ modelMatrix, node, node->light() == nullptr });
			}
			if (node->light()) {
				lights.push_back({ modelMatrix, node->light() });
			}
		}
	});

	geRenderer->setActiveCameraAndLights(m_camera->camera(), lights);
	geRenderer->updateShadowMaps(renderables);

	// Blue light crepuscular rays stencil pass

	m_blueLightFramebuffer->bind();

	m_orangeLightSource->setEnabled(false);

	geRenderer->clear();
	geRenderer->render(renderables);

	m_orangeLightSource->setEnabled(true);

	m_blueLightFramebuffer->unbind();

	// Orange light crepuscular rays stencil pass

	m_orangeLightFramebuffer->bind();

	m_blueLightSource->setEnabled(false);

	geRenderer->clear();
	geRenderer->render(renderables);

	m_blueLightSource->setEnabled(true);

	m_orangeLightFramebuffer->unbind();

	// Main render pass

	m_compositor->bindInputFramebuffer();

	geRenderer->clear();
	geRenderer->render(renderables);

	m_compositor->unbindInputFramebuffer();

	m_compositor->compose(m_compositorEffects);
}
