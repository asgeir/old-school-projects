#include "testapplication.h"

#include "ge2camera.h"
#include "ge2debugcamera.h"
#include "ge2material.h"
#include "ge2mesh.h"
#include "ge2renderer.h"
#include "ge2resourcemgr.h"
#include "ge2shader.h"
#include "ge2time.h"

#include <cstdlib>

using namespace ge2;

namespace {

const std::string kVertexShaderFile = "default.vs";
const std::string kFragmentShaderFile = "textured_fragment_light.fs";
const std::string kAnimatedFragmentShaderFile = "red_swirls.fs";
const std::string kCylinderAnimatedFragmentShaderFile = "red_swirls_cylinder.fs";

}

#include "ge2geometry.h"

TestApplication::TestApplication(int argc, char *argv[])
{
	geRenderer->setTitle("GL Engine 2 - Test Application");

	geResourceMgr->setAssetDirectory("../assets/ge2test");

	if (argc >= 2) {
		geResourceMgr->setAssetDirectory(argv[1]);
	}

	m_meshList = geResourceMgr->loadMeshListFromFile("phone?", "Silly_Phone.obj");
	m_texture = geResourceMgr->loadTexture2DFromFile("blue_tile", "blue_tile.png");

	Shader *shader = geResourceMgr->loadShaderFromFiles(
		"default_shader",
		kVertexShaderFile,
		kFragmentShaderFile,
		{ "textureData", "textureGamma" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}
	Material *material = geResourceMgr->createMaterial("default_material", shader);
	material->setUniform("textureData", m_texture);
	material->setUniform("textureGamma", 1.0f);
	material->setShininess(0.0f);

	Shader *animatedShader = geResourceMgr->loadShaderFromFiles(
		"animated_shader",
		kVertexShaderFile,
		kAnimatedFragmentShaderFile,
		{ "time" });
	if (animatedShader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << animatedShader->errorString() << std::endl;
	}
	m_animatedMaterial = geResourceMgr->createMaterial("animated_material", animatedShader);

	Shader *cylinderAnimatedShader = geResourceMgr->loadShaderFromFiles(
		"cylinder_animated_shader",
		kVertexShaderFile,
		kCylinderAnimatedFragmentShaderFile,
		{ "time" });
	if (cylinderAnimatedShader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << cylinderAnimatedShader->errorString() << std::endl;
	}
	m_cylinderAnimatedMaterial = geResourceMgr->createMaterial("cylinder_animated_material", cylinderAnimatedShader);

	m_scene = new Node;
	m_camera = new DebugCamera;

	Mesh *cubeMesh = geResourceMgr->createCube("cube", 1.0f);
	cubeMesh->setMaterial(m_animatedMaterial);
	cubeMesh->construct();

	Mesh *quadMesh = geResourceMgr->createQuad("quad", 1.0f, 1.0f);
	quadMesh->setMaterial(material);
	quadMesh->construct();

	m_scene->setMeshList({ quadMesh });
	m_scene->setPosition(glm::vec3{0.0f, 0.0f, 0.0f});

	Node *quadNode = new Node;
	quadNode->setMeshList({ quadMesh });
	quadNode->setPosition(glm::vec3{-1.0f, 1.0f, 0.0f});
	quadNode->setRotation(glm::rotate(quadNode->rotation(), GE_PI / 4.0f, glm::vec3{0.0f, 0.0f, 1.0f}));
	m_scene->addChild(quadNode);

	m_cubeNode = new Node;
	m_cubeNode->setMeshList({ cubeMesh });
	m_cubeNode->setPosition(glm::vec3{1.0f, 1.0f, 1.0f});
	m_scene->addChild(m_cubeNode);

	if (!m_meshList.empty()) {
		m_meshList[0]->setMaterial(m_animatedMaterial);
		m_meshList[1]->setMaterial(m_animatedMaterial);
		m_meshList[2]->destruct();

		Node *meshListNode = new Node;
		meshListNode->setMeshList(m_meshList);
		meshListNode->setPosition(glm::vec3{0.0f, 0.0f, 2.0f});
		meshListNode->setScale(glm::vec3{0.01});
		m_scene->addChild(meshListNode);
	}

	Mesh *cylinderMesh = geResourceMgr->createCylinder("cylinder", 0.5f, 1.0f, 16);
	cylinderMesh->setMaterial(m_cylinderAnimatedMaterial);
	cylinderMesh->construct();

	Node *cylinderNode = new Node;
	cylinderNode->setMeshList({ cylinderMesh });
	cylinderNode->setPosition(glm::vec3{-1.0f, 0.0f, 1.0f});
	m_scene->addChild(cylinderNode);

	m_whitePointLight.setAmbientColor(glm::vec3{0.2f});
	m_whitePointLight.setColor(glm::vec3{1.0f});
	m_whitePointLight.setCutoff(0.001f);
	m_whitePointLight.setRadius(2.0f);

	Node *whiteLightNode = new Node;
	whiteLightNode->setLight(&m_whitePointLight);
	whiteLightNode->setPosition(glm::vec3{0.0f, 0.0f, 0.5f});
	m_scene->addChild(whiteLightNode);

	m_redPointLight.setAmbientColor(glm::vec3{0.2f});
	m_redPointLight.setColor(glm::vec3{1.0f, 0.0f, 0.0f});
	m_redPointLight.setCutoff(0.001f);
	m_redPointLight.setRadius(2.0f);

	Node *redLightNode = new Node;
	redLightNode->setLight(&m_redPointLight);
	redLightNode->setPosition(glm::vec3{-1.0f, 1.0f, 0.5f});
	m_scene->addChild(redLightNode);
}

TestApplication::~TestApplication()
{
	delete m_scene;
	delete m_camera;
}

void TestApplication::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		SDL_Event quitEvent = { SDL_QUIT };
		SDL_PushEvent(&quitEvent);
	}
}

void TestApplication::update()
{
	m_camera->update();

	m_cubeNode->setRotation(glm::rotate(m_cubeNode->rotation(), Time::deltaTime() * (GE_PI / 4.0f), glm::vec3{1.0f, 1.0f, 0.0f}));
	m_animatedMaterial->setUniform("time", Time::totalSeconds());
	m_cylinderAnimatedMaterial->setUniform("time", Time::totalSeconds());

	LightInfoList lights;
	RenderableList renderables;
	NodeTreeVisitor treeVisitor;
	treeVisitor.visitNodeTree(m_scene, {
		[&renderables, &lights] (Node *node, const glm::mat4 &modelMatrix) {
			if (node->meshCount()) {
				renderables.push_back({ modelMatrix, node });
			}
			if (node->light()) {
				lights.push_back({ modelMatrix, node->light() });
			}
		}
	});

	geRenderer->setActiveCameraAndLights(m_camera->camera(), lights);

	geRenderer->clear();
	geRenderer->render(renderables);
}
