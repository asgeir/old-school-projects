#include "cubemapapp.h"

namespace {

const std::string kReflectiveVertexShader = R"(
	#version 150

	in vec4 ge_position;
	in vec3 ge_normal;

	uniform mat4 ge_modelViewProjection;
	uniform mat3 ge_normalMatrix;

	out vec3 normal;

	void main()
	{
		gl_Position = ge_modelViewProjection * ge_position;
		normal = normalize(ge_normalMatrix * ge_normal);
	}
)";

const std::string kReflectiveFragmentShader = R"(
	#version 150
	#ge_include "standard/lighting.fs"

	//in vec3 normal;

	uniform samplerCube envMap;
	uniform mat3 ge_viewMatrixLinearInverse;

	out vec4 ge_fragmentColor;

	void main()
	{
		vec3 reflected = normalize(reflect(vec3(0.0, 0.0, -1.0), normal));
		reflected = ge_viewMatrixLinearInverse * reflected;
		ge_fragmentColor = texture(envMap, reflected) + lighting(vec4(0.2));
	}
)";

const std::string kRefractiveFragmentShader = R"(
	#version 150

	in vec3 normal;

	uniform float outsideRefractiveIndex = 1.0;
	uniform float objectRefractiveIndex = 1.5;
	uniform samplerCube envMap;
	uniform mat3 ge_viewMatrixLinearInverse;

	out vec4 ge_fragmentColor;

	void main()
	{
		vec3 reflected = normalize(refract(vec3(0.0, 0.0, -1.0), normal, outsideRefractiveIndex / objectRefractiveIndex));
		reflected = ge_viewMatrixLinearInverse * reflected;
		ge_fragmentColor = texture(envMap, reflected);
	}
)";

}

using namespace ge2;

CubemapApplication::CubemapApplication(int argc, char *argv[])
{
	geRenderer->setTitle("GL Engine 2 - Cubemap Test");

	geResourceMgr->setAssetDirectory("../assets");

	if (argc >= 2) {
		geResourceMgr->setAssetDirectory(argv[1]);
	}

	Shader *shader = geResourceMgr->loadShaderFromFiles(
		"default_shader",
		"standard/default.vs",
		"ge2test/colored_fragment_light_shadow.fs",
		{ });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}
	Material *greyMaterial = geResourceMgr->createMaterial("grey_material", shader);
	greyMaterial->setDiffuseColor(glm::vec3{0.8f});

	Material *redMaterial = geResourceMgr->createMaterial("red_material", shader);
	redMaterial->setDiffuseColor(glm::vec3{1.0f, 0.0f, 0.0f});
	redMaterial->setSpecularColor(glm::vec3{0.5f, 0.0f, 0.0f});
	redMaterial->setShininess(50.0f);

	Material *greenMaterial = geResourceMgr->createMaterial("green_material", shader);
	greenMaterial->setDiffuseColor(glm::vec3{0.0f, 1.0f, 0.0f});
	greenMaterial->setSpecularColor(glm::vec3{0.0f, 0.5f, 0.0f});
	greenMaterial->setShininess(50.0f);

	Material *blueMaterial = geResourceMgr->createMaterial("blue_material", shader);
	blueMaterial->setDiffuseColor(glm::vec3{0.0f, 0.0f, 1.0f});
	blueMaterial->setSpecularColor(glm::vec3{0.0f, 0.0f, 0.5f});
	blueMaterial->setShininess(50.0f);

	shader = geResourceMgr->loadShaderFromStrings(
		"reflective_shader",
		kReflectiveVertexShader,
		kReflectiveFragmentShader,
		{ "envMap" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}
	m_reflectiveMaterial = geResourceMgr->createMaterial("reflective_material", shader);
	m_reflectiveMaterial->setSpecularColor(glm::vec3{1.0f});
	m_reflectiveMaterial->setShininess(25.0f);

	shader = geResourceMgr->loadShaderFromStrings(
		"refractive_shader",
		kReflectiveVertexShader,
		kRefractiveFragmentShader,
		{ "envMap", "objectRefractiveIndex", "outsideRefractiveIndex" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}
	m_refractiveMaterial = geResourceMgr->createMaterial("refractive_material", shader);

	m_environmentMap = new CubeFramebuffer;
	m_environmentMap->construct(512, true, { FragmentBuffer::Color });
	m_reflectiveMaterial->setUniform("envMap", m_environmentMap->colorBuffer(FragmentBuffer::Color));
	m_refractiveMaterial->setUniform("envMap", m_environmentMap->colorBuffer(FragmentBuffer::Color));

	m_camera = new DebugCamera;
	m_scene = new Node;

	// grey sphere

	Mesh *sphereMesh = geResourceMgr->createSphere("sphere", 0.5f, 3);
	sphereMesh->setMaterial(greyMaterial);
	sphereMesh->construct();

	Node *sphereNode = new Node;
	sphereNode->setMeshList({ sphereMesh });
	m_scene->addChild(sphereNode);

	// red sphere

	sphereMesh = geResourceMgr->createSphere("red_sphere", 0.5f, 3);
	sphereMesh->setMaterial(redMaterial);
	sphereMesh->construct();

	sphereNode = new Node;
	sphereNode->setMeshList({ sphereMesh });
	sphereNode->setPosition(glm::vec3{0.0f, 0.0f, -4.0f});
	m_scene->addChild(sphereNode);

	// green sphere

	sphereMesh = geResourceMgr->createSphere("green_sphere", 0.5f, 3);
	sphereMesh->setMaterial(greenMaterial);
	sphereMesh->construct();

	sphereNode = new Node;
	sphereNode->setMeshList({ sphereMesh });
	sphereNode->setPosition(glm::rotate(glm::quat{}, degToRad(132), kUnitVectorY) * glm::vec3{0.0f, 0.0f, -4.0f});
	m_scene->addChild(sphereNode);

	// blue sphere

	sphereMesh = geResourceMgr->createSphere("blue_sphere", 0.5f, 3);
	sphereMesh->setMaterial(blueMaterial);
	sphereMesh->construct();

	sphereNode = new Node;
	sphereNode->setMeshList({ sphereMesh });
	sphereNode->setPosition(glm::rotate(glm::quat{}, degToRad(-132), kUnitVectorY) * glm::vec3{0.0f, 0.0f, -4.0f});
	m_scene->addChild(sphereNode);

	// ground

	Mesh *planeMesh = geResourceMgr->createQuad("ground", 50, 50);
	planeMesh->setMaterial(greyMaterial);
	planeMesh->construct();

	Node *groundNode = new Node;
	groundNode->setMeshList({ planeMesh });
	groundNode->setPosition(glm::vec3{0.0f, -1.0f, 0.0f});
	groundNode->setRotation(glm::rotate(glm::quat{}, degToRad(-90), kUnitVectorX));
	m_scene->addChild(groundNode);

	// reflective/refractive sphere

	m_reflectiveRefractiveSphere = geResourceMgr->createSphere("reflective/refractive_sphere", 0.5f, 3);
	m_reflectiveRefractiveSphere->setMaterial(m_reflectiveMaterial);
	m_reflectiveRefractiveSphere->construct();

	m_rotationNode = new Node;
	m_scene->addChild(m_rotationNode);

	m_reflectiveNode = new Node;
	m_reflectiveNode->setPosition(glm::vec3{2.0f, 0.0f, 0.0f});
	m_reflectiveNode->setMeshList({ m_reflectiveRefractiveSphere });
	m_rotationNode->addChild(m_reflectiveNode);

	// light

	Node *lightNode = new Node;
	DirectionalLight *light = new DirectionalLight;
	light->setAmbientColor(glm::vec3{0.15f});
	light->setColor(glm::vec3{1.0f});
	light->setDirection(glm::vec3{-1.0f});
	light->setCastsShadows(true);
	light->setDepth(glm::vec2{-30.0f, 30.0f});
	light->setHorizontal(glm::vec2{-35.0f, 35.0f});
	light->setVertical(glm::vec2{-35.0f, 35.0f});
	lightNode->setLight(light);
	lightNode->setPosition(glm::vec3{4.0f});
	m_scene->addChild(lightNode);

	// compositor

	m_antiAliasingEffect = new AntiAliasingEffect;
	m_bloomEffect = new BloomEffect;
	m_tonemapEffect = new TonemapEffect{1.0f, 1.0f, glm::vec3{1.0f}};
	m_compositorEffects.push_back(m_antiAliasingEffect);
	m_compositorEffects.push_back(m_bloomEffect);
	m_compositorEffects.push_back(m_tonemapEffect);

	m_compositor = new Compositor;
	m_compositor->construct(geRenderer->width(), geRenderer->height());
}

CubemapApplication::~CubemapApplication()
{
	delete m_compositor;
	delete m_antiAliasingEffect;
	delete m_bloomEffect;
	delete m_tonemapEffect;
	delete m_scene;
	delete m_camera;
}

void CubemapApplication::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	if (event.type == SDL_KEYUP) {
		const SDL_KeyboardEvent *evt = (const SDL_KeyboardEvent *)(&event);
		if (evt->keysym.scancode == SDL_SCANCODE_ESCAPE) {
			SDL_Event quitEvent = { SDL_QUIT };
			SDL_PushEvent(&quitEvent);
		} else if (evt->keysym.scancode == SDL_SCANCODE_1) {
			m_reflectiveRefractiveSphere->setMaterial(m_refractiveMaterial);
		} else if (evt->keysym.scancode == SDL_SCANCODE_2) {
			m_reflectiveRefractiveSphere->setMaterial(m_reflectiveMaterial);
		}
	}
}

void CubemapApplication::update()
{
	m_camera->update();

	m_rotationNode->setRotation(glm::rotate(m_rotationNode->rotation(), Time::deltaTime() * GE_PI / 8.0f, kUnitVectorY));

	LightInfoList lights;
	RenderableList renderables;
	NodeTreeVisitor treeVisitor;
	treeVisitor.visitNodeTree(m_scene, {
		[this, &renderables, &lights] (Node *node, const glm::mat4 &modelMatrix) {
			if (node->meshCount()) {
				renderables.push_back({ modelMatrix, node, node != this->m_reflectiveNode });
			}
			if (node->light()) {
				lights.push_back({ modelMatrix, node->light() });
			}
		}
	});

	m_environmentMap->setPosition(m_reflectiveNode->worldPosition());
	m_environmentMap->update(
		[&renderables, &lights] (PerspectiveCamera *faceCamera) {
			geRenderer->setActiveCameraAndLights(faceCamera, lights);

			geRenderer->clear();
			geRenderer->render(renderables);
		}
	);

	geRenderer->setActiveCameraAndLights(m_camera->camera(), lights);
	geRenderer->updateShadowMaps(renderables);

	m_compositor->bindInputFramebuffer();

	geRenderer->clear();
	geRenderer->render(renderables);

	m_compositor->unbindInputFramebuffer();

	m_compositor->compose(m_compositorEffects);
}
