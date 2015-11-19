#pragma once

#include "ge2.h"

#include <array>

class GameCamera;

const size_t kMazeWidth = 25;
const size_t kMazeHeight = 25;

enum class MazeCellType
{
	Wall,
	Floor,

	Start,
	Target
};

struct MazeCell
{
	MazeCellType type;

	bool visited;
};

typedef std::array<MazeCell, kMazeWidth> MazeRow;
typedef std::array<MazeRow, kMazeHeight> MazeGrid;

class GameState : public ge2::State
{
public:
	GameState();
	~GameState();

	void initialize();

	virtual void onTransitionIn(ge2::StateManager *manager, void *userdata = nullptr) override;
	virtual void *onTransitionOut() override;

	virtual void handleEvent(const SDL_Event &event) override;
	virtual void update() override;

private:
	void setupMaterials();
	void setupMeshes();
	void setupScene();

	ge2::Node *createFloor(int x, int y);
	ge2::Node *createTarget(int x, int y);
	ge2::Node *createWall(int x, int y);

	bool checkCollisions();

	ge2::StateManager *m_stateManager = nullptr;
	GameCamera        *m_camera = nullptr;
	ge2::Compositor   *m_compositor = nullptr;
	ge2::Node         *m_scene = nullptr;

	ge2::CompositorEffectList  m_compositorEffects;
	ge2::AntiAliasingEffect   *m_antiAliasingEffect = nullptr;
	ge2::BloomEffect          *m_bloomEffect = nullptr;
	ge2::TonemapEffect        *m_tonemapEffect = nullptr;

	ge2::Node  *m_mazeNode = nullptr;
	MazeGrid    m_maze;
	ge2::Node  *m_targetNode = nullptr;

	ge2::Material   *m_floorMaterial = nullptr;
	ge2::Mesh       *m_floorMesh = nullptr;
	ge2::Material   *m_targetMaterial = nullptr;
	ge2::MeshList    m_targetMesh;
	ge2::Material   *m_wallMaterial = nullptr;
	ge2::Mesh       *m_wallMesh = nullptr;

	ge2::SpotLight        m_flashlight;
	ge2::PointLight       m_targetLight;
	ge2::DirectionalLight m_worldLight;
};
