#include "compositorapp.h"

using namespace ge2;

namespace {

const std::string kTestMaterialVertexShader = R"(
	#version 150

	in vec4 ge_position;
	in vec2 ge_textureCoordinates;

	uniform mat4 ge_modelViewProjection;

	out vec2 textureCoordinates;

	void main()
	{
		gl_Position = ge_modelViewProjection * ge_position;
		textureCoordinates = ge_textureCoordinates;
	}
)";

const std::string kTestMaterialFragmentShader = R"(
	#version 150

	in vec2 textureCoordinates;

	out vec4 ge_fragmentColor;
	out vec4 ge_fragmentGlow;

	void main()
	{
		ge_fragmentColor = vec4(textureCoordinates * 2.0, 0.0, 1.0);

		if (textureCoordinates.x < 0.01 || textureCoordinates.x > 0.99 || textureCoordinates.y < 0.01 || textureCoordinates.y > 0.99) {
			ge_fragmentGlow = vec4(0.0, 0.0, 8.0, 1.0);
		} else {
			ge_fragmentGlow = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
)";

const std::string kSphereMaterialVertexShader = R"(
	#version 150

	in vec4 ge_position;
	in vec3 ge_normal;

	uniform mat4 ge_modelViewProjection;
	uniform mat3 ge_normalMatrix;

	out vec3 normal;

	void main()
	{
		gl_Position = ge_modelViewProjection * ge_position;
		normal = ge_normal;//normalize(ge_normalMatrix * ge_normal);
	}
)";

const std::string kSphereMaterialFragmentShader = R"(
	#version 150

	in vec3 normal;

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = vec4(vec3(0.5) + normal * 0.5, 1.0);
	}
)";

}

CompositorApplication::CompositorApplication(int argc, char *argv[])
{
	geRenderer->setTitle("GL Engine 2 - Compositor Test");

	geResourceMgr->setAssetDirectory("../assets/compositortest");

	if (argc >= 2) {
		geResourceMgr->setAssetDirectory(argv[1]);
	}

	Shader *shader = geResourceMgr->loadShaderFromStrings(
		"default_shader",
		kTestMaterialVertexShader,
		kTestMaterialFragmentShader,
		{ });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}
	Material *material = geResourceMgr->createMaterial("default_material", shader);

	Shader *sphereShader = geResourceMgr->loadShaderFromStrings(
		"sphere_shader",
		kSphereMaterialVertexShader,
		kSphereMaterialFragmentShader,
		{ });
	if (sphereShader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << sphereShader->errorString() << std::endl;
	}
	Material *sphereMaterial = geResourceMgr->createMaterial("sphere_material", sphereShader);

	m_antiAliasingEffect = new ge2::AntiAliasingEffect;
	m_bloomEffect = new ge2::BloomEffect;
	m_glowEffect = new ge2::GlowEffect;
	m_tonemapEffect = new ge2::TonemapEffect;
	m_compositorEffects.push_back(m_antiAliasingEffect);
	m_compositorEffects.push_back(m_glowEffect);
	m_compositorEffects.push_back(m_bloomEffect);
	m_compositorEffects.push_back(m_tonemapEffect);

	m_compositor = new Compositor;
	m_compositor->construct(geRenderer->width(), geRenderer->height());

	m_camera = new DebugCamera;
	m_scene = new Node;

	Mesh *cubeMesh = geResourceMgr->createCube("cube", 1.0f);
	cubeMesh->setMaterial(material);
	cubeMesh->construct();

	m_cubeNode = new Node;
	m_cubeNode->setMeshList({ cubeMesh });
	m_scene->addChild(m_cubeNode);

	Mesh *sphereMesh = geResourceMgr->createSphere("sphere", 0.5f);
	sphereMesh->setMaterial(sphereMaterial);
	sphereMesh->construct();

	Node *sphereNode = new Node;
	sphereNode->setPosition(glm::vec3{0.0f, -2.0f, 0.0f});
	sphereNode->setMeshList({ sphereMesh });
	m_scene->addChild(sphereNode);
}

CompositorApplication::~CompositorApplication()
{
	delete m_scene;
	delete m_compositor;
	delete m_camera;
	delete m_bloomEffect;
	delete m_glowEffect;
}

void CompositorApplication::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		SDL_Event quitEvent = { SDL_QUIT };
		SDL_PushEvent(&quitEvent);
	}
}

void CompositorApplication::update()
{
	m_camera->update();

	m_cubeNode->setRotation(glm::rotate(m_cubeNode->rotation(), Time::deltaTime() * (GE_PI / 4.0f), glm::vec3{1.0f, 1.0f, 0.0f}));

	RenderableList renderables;
	NodeTreeVisitor treeVisitor;
	treeVisitor.visitNodeTree(m_scene, {
		[&renderables] (Node *node, const glm::mat4 &modelMatrix) {
			if (node->meshCount()) {
				renderables.push_back({ modelMatrix, node });
			}
		}
	});

	LightInfoList lights;
	geRenderer->setActiveCameraAndLights(m_camera->camera(), lights);

	m_compositor->bindInputFramebuffer();

	geRenderer->clear();
	geRenderer->render(renderables);

	m_compositor->unbindInputFramebuffer();

	m_compositor->compose(m_compositorEffects);
}
