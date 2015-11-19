#include "shadowapp.h"

#include <iostream>

using namespace ge2;

ShadowApplication::ShadowApplication(int argc, char *argv[])
{
	geRenderer->setTitle("GL Engine 2 - Shadow Test");

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
	Material *material = geResourceMgr->createMaterial("default_material", shader);
	material->setDiffuseColor(glm::vec3{0.8f});
	material->setFaceCulling(false);

	m_camera = new DebugCamera;
	m_camera->setPosition(glm::vec3{6.0f, 2.0f, -6.0f});
	m_camera->setAzimuth(degToRad(135));
	m_camera->setAltitude(degToRad(-10));

	m_scene = new Node;

	Mesh *sphereMesh = geResourceMgr->createSphere("sphere", 0.5f, 3);
	sphereMesh->setMaterial(material);
	sphereMesh->construct();

	Node *sphereNode = new Node;
	sphereNode->setMeshList({ sphereMesh });
	m_scene->addChild(sphereNode);

	Mesh *quadMesh = geResourceMgr->createQuad("quad", 5.0f, 5.0f);
	quadMesh->setMaterial(material);
	quadMesh->construct();

	Node *floorNode = new Node;
	floorNode->setPosition(glm::vec3{0.0f, -2.5f, 0.0f});
	floorNode->setRotation(glm::rotate(glm::quat{}, -degToRad(90), kUnitVectorX));
	floorNode->setMeshList({ quadMesh });
	m_scene->addChild(floorNode);

	Node *wallNode = new Node;
	wallNode->setPosition(glm::vec3{0.0f, -2.0f, 2.5f});
	wallNode->setRotation(glm::rotate(glm::quat{}, degToRad(180), kUnitVectorX));
	wallNode->setMeshList({ quadMesh });
	m_scene->addChild(wallNode);

	Mesh *pillarMesh = geResourceMgr->createCylinder("pillar", 0.3f, 3.0f, 16);
	pillarMesh->setMaterial(material);
	pillarMesh->construct();

	Node *pillarNode = new Node;
	pillarNode->setPosition(glm::vec3{0.5f, -1.0f, 1.0f});
	pillarNode->setMeshList({ pillarMesh });
	m_scene->addChild(pillarNode);

	Node *directionalNode = new Node;
	m_directionalLight = new DirectionalLight;
	m_directionalLight->setAmbientColor(glm::vec3{0.15f});
	m_directionalLight->setColor(glm::vec3{1.0f, 0.0f, 0.0f});
	m_directionalLight->setDirection(glm::vec3{-0.5f, -1.0f, -0.8f});
	m_directionalLight->setCastsShadows(true);
	directionalNode->setLight(m_directionalLight);
	m_scene->addChild(directionalNode);

	Node *pointNode = new Node;
	m_pointLight = new PointLight;
	m_pointLight->setEnabled(false);
	m_pointLight->setAmbientColor(glm::vec3{0.15f});
	m_pointLight->setColor(glm::vec3{0.0f, 1.0f, 0.0f});
	m_pointLight->setCastsShadows(true);
	pointNode->setLight(m_pointLight);
	pointNode->setPosition(glm::vec3{-1.0f, -1.0, 0.5f});
	m_scene->addChild(pointNode);

	Node *spotNode = new Node;
	m_spotLight = new SpotLight;
	m_spotLight->setEnabled(false);
	m_spotLight->setAmbientColor(glm::vec3{0.15f});
	m_spotLight->setColor(glm::vec3{0.0f, 0.0f, 1.0f});
	m_spotLight->setLength(10.0f);
	m_spotLight->setExponent(100.0f);
	m_spotLight->setCastsShadows(true);
	spotNode->setLight(m_spotLight);
	spotNode->setPosition(glm::vec3{4.0f, 3.0, -1.0f});
	spotNode->setRotation(glm::rotate(glm::rotate(glm::quat{}, degToRad(115), kUnitVectorY), -degToRad(45), kUnitVectorX));
	m_scene->addChild(spotNode);
}

ShadowApplication::~ShadowApplication()
{
	delete m_scene;
	delete m_camera;
}

void ShadowApplication::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	if (event.type == SDL_KEYUP) {
		const SDL_KeyboardEvent *evt = (const SDL_KeyboardEvent *)(&event);
		if (evt->keysym.scancode == SDL_SCANCODE_ESCAPE) {
			SDL_Event quitEvent = { SDL_QUIT };
			SDL_PushEvent(&quitEvent);
		} else if (evt->keysym.scancode == SDL_SCANCODE_1) {
			m_directionalLight->setEnabled(!m_directionalLight->enabled());
		} else if (evt->keysym.scancode == SDL_SCANCODE_2) {
			m_spotLight->setEnabled(!m_spotLight->enabled());
		} else if (evt->keysym.scancode == SDL_SCANCODE_3) {
			m_pointLight->setEnabled(!m_pointLight->enabled());
		}
	}
}

void ShadowApplication::update()
{
	m_camera->update();

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
	geRenderer->updateShadowMaps(renderables);

	// If using compositor we probably need to sort out the lights beforehand
	// since they will be using framebuffers and that would mess up the
	// compositor setup

	geRenderer->clear();
	geRenderer->render(renderables);
}
