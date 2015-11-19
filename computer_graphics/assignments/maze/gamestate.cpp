#include "gamestate.h"
#include "gamecamera.h"

#include <stack>

using namespace ge2;

namespace {

const char * const kVertexShaderFile = "ge2test/default.vs";
const char * const kDefaultTexturedFragmentShaderFile = "ge2test/textured_fragment_light.fs";
const char * const kRedSwirlsFragmentShaderFile = "ge2test/red_swirls.fs";

glm::vec3 gridCoordinatesToWorld(int x, int y)
{
	return glm::vec3{10.0f * x, 0.0f, 10.0f * y};
}

const glm::vec3 kCameraYAxisOffset{0.0f, 3.0f, 0.0f};
const glm::vec3 kFloorYAxisOffset{0.0f, -1.5f, 0.0f};
const glm::vec3 kTargetAxisOffset{0.0f, 1.0f, 0.0f};
const glm::vec3 kWallYAxisOffset{0.0f, 3.5f, 0.0f};

struct MazeCoordinate
{
	int x;
	int y;
};

typedef std::array<MazeCoordinate, 4> MazeNeighbourList;

MazeNeighbourList mazeCellNeighbours(const MazeGrid &grid, const MazeCoordinate &cell)
{
	MazeNeighbourList list = { {{ -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }} };

	if ((cell.y+2) <= (int)(kMazeHeight-2) && grid[cell.y+2][cell.x].visited == false) list[0] = { cell.x, cell.y+2 };
	if ((cell.x-2) >= 1 && grid[cell.y][cell.x-2].visited == false) list[1] = { cell.x-2, cell.y };
	if ((cell.y-2) >= 1 && grid[cell.y-2][cell.x].visited == false) list[2] = { cell.x, cell.y-2 };
	if ((cell.x+2) <= (int)(kMazeWidth-2) && grid[cell.y][cell.x+2].visited == false) list[3] = { cell.x+2, cell.y };

	return std::move(list);
}

MazeGrid createMaze()
{
	MazeGrid grid;

	for (size_t y = 0; y < grid.size(); ++y) {
		MazeRow &row = grid[y];
		for (size_t x = 0; x < row.size(); ++x) {
			row[x] = { MazeCellType::Wall, false };
		}
	}

	MazeCoordinate start = { (int)(1 + rand() % (kMazeWidth-2)), (int)(1 + rand() % (kMazeHeight-2)) };
	grid[start.y][start.x] = { MazeCellType::Floor, true };

	std::stack<MazeCoordinate> backtrackLog;

	MazeCoordinate current = start;
	while (true) {
		MazeNeighbourList neighborList = mazeCellNeighbours(grid, current);
		if (neighborList[0].x >= 0 || neighborList[1].x >= 0 || neighborList[2].x >= 0 || neighborList[3].x >= 0) {
			// choose neighbour
			int i = rand() % 4;
			while (neighborList[i].x < 0) i = rand() % 4;

			// mark walls
			grid[current.y][current.x+1].visited = true;
			grid[current.y][current.x-1].visited = true;
			grid[current.y+1][current.x].visited = true;
			grid[current.y-1][current.x].visited = true;

			backtrackLog.push(current);

			// remove wall between current and neighbour
			if (neighborList[i].x < current.x) grid[current.y][current.x-1].type = MazeCellType::Floor;
			if (neighborList[i].x > current.x) grid[current.y][current.x+1].type = MazeCellType::Floor;
			if (neighborList[i].y < current.y) grid[current.y-1][current.x].type = MazeCellType::Floor;
			if (neighborList[i].y > current.y) grid[current.y+1][current.x].type = MazeCellType::Floor;

			// make the neighbour the current cell
			current = neighborList[i];
			grid[current.y][current.x].visited = true;
			grid[current.y][current.x].type = MazeCellType::Floor;
		} else if (!backtrackLog.empty()) {
			current = backtrackLog.top();
			backtrackLog.pop();
		} else {
			break;
		}
	}

	grid[start.y][start.x].type = MazeCellType::Start;
	while (true) {
		size_t outX = rand() % kMazeWidth;
		size_t outY = rand() % kMazeHeight;

		if (((start.x-outX)*(start.x-outX)+(start.y-outY)*(start.y-outY)) >= (0.25f*(kMazeWidth+kMazeHeight)) && grid[outY][outX].type == MazeCellType::Floor) {
			grid[outY][outX].type = MazeCellType::Target;
			break;
		}
	}
	return std::move(grid);
}


// http://stackoverflow.com/a/402010/764349
struct CircleType
{
	float x;
	float y;
	float r;
};

struct RectType
{
	float x;
	float y;
	float width;
	float height;
};

bool intersects(CircleType circle, RectType rect)
{
	glm::vec2 circleDistance{abs(circle.x - rect.x), abs(circle.y - rect.y)};

	if (circleDistance[0] > (rect.width/2.0f + circle.r)) { return false; }
	if (circleDistance[1] > (rect.height/2.0f + circle.r)) { return false; }

	if (circleDistance[0] <= (rect.width/2.0f)) { return true; }
	if (circleDistance[1] <= (rect.height/2.0f)) { return true; }

	float cornerDistance_sq = pow((circleDistance[0] - rect.width/2.0f), 2) +
							  pow((circleDistance[1] - rect.height/2.0f), 2);

	return (cornerDistance_sq <= (circle.r * circle.r));
}

} // namespace

GameState::GameState()
{
}

GameState::~GameState()
{
	m_scene->removeChild(m_camera->camera());
	delete m_camera;
	delete m_scene;
	delete m_compositor;
	delete m_antiAliasingEffect;
	delete m_bloomEffect;
	delete m_tonemapEffect;
}

void GameState::initialize()
{
	m_camera = new GameCamera;
	m_camera->camera()->setLight(&m_flashlight);
	// m_camera->setVerticalMovementAllowed(true);

	m_scene = new Node;
	m_scene->addChild(m_camera->camera());

	Node *worldLightNode = new Node;
	worldLightNode->setLight(&m_worldLight);
	m_scene->addChild(worldLightNode);

	m_compositor = new Compositor;
	m_compositor->construct(geRenderer->width(), geRenderer->height());

	m_antiAliasingEffect = new ge2::AntiAliasingEffect;
	m_bloomEffect = new ge2::BloomEffect;
	m_tonemapEffect = new ge2::TonemapEffect;
	m_compositorEffects.push_back(m_antiAliasingEffect);
	m_compositorEffects.push_back(m_bloomEffect);
	m_compositorEffects.push_back(m_tonemapEffect);

	setupMaterials();
	setupMeshes();
}

void GameState::onTransitionIn(StateManager *manager, void *userdata)
{
	(void)userdata;
	m_stateManager = manager;

	m_mazeNode = new Node;
	m_scene->addChild(m_mazeNode);

	m_camera->setCollisionFlags(0);

	setupScene();
}

void *GameState::onTransitionOut()
{
	m_scene->removeChild(m_mazeNode);

	delete m_mazeNode;
	m_mazeNode = nullptr;

	m_targetNode = nullptr;

	return nullptr;
}

void GameState::handleEvent(const SDL_Event &event)
{
	m_camera->handleEvent(event);

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		SDL_Event quitEvent = { SDL_QUIT };
		SDL_PushEvent(&quitEvent);
	}
}

void GameState::update()
{
	if (!checkCollisions()) {
		// User has collided with exit node so continuing beyond this point will segfault
		return;
	}
	m_camera->update();

	m_targetMaterial->setUniform("time", Time::totalSeconds());

	m_targetNode->setRotation(glm::rotate(m_targetNode->rotation(), Time::deltaTime() * GE_PI/2.0f, kUnitVectorY));
	m_targetNode->setPosition(m_targetNode->position() + (float)cos(Time::totalSeconds()) * 0.02f * kUnitVectorY);

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

	m_compositor->bindInputFramebuffer();

	geRenderer->clear();
	geRenderer->render(renderables);

	m_compositor->unbindInputFramebuffer();

	m_compositor->compose(m_compositorEffects);
}

void GameState::setupMaterials()
{
	Texture2D *floorTexture = geResourceMgr->loadTexture2DFromFile("floor_texture", "maze/wood_floorboards.png");
	Shader *shader = geResourceMgr->loadShaderFromFiles(
		"default_textured_shader",
		kVertexShaderFile,
		kDefaultTexturedFragmentShaderFile,
		{ "textureData", "textureGamma" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error in floor material" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}

	m_floorMaterial = geResourceMgr->createMaterial("floor_material", shader);
	m_floorMaterial->setUniform("textureData", floorTexture);
	m_floorMaterial->setUniform("textureGamma", 1.0f / 2.2f);

	Texture2D *brickTexture = geResourceMgr->loadTexture2DFromFile("brickwork_texture", "maze/brickwork_texture.png");

	m_wallMaterial = geResourceMgr->createMaterial("wall_material", shader);
	m_wallMaterial->setUniform("textureData", brickTexture);
	m_wallMaterial->setUniform("textureGamma", 1.0f / 2.2f);

	shader = geResourceMgr->loadShaderFromFiles(
		"target_shader",
		kVertexShaderFile,
		kRedSwirlsFragmentShaderFile,
		{ "time" });
	if (shader->hasError()) {
		std::cerr << "Shader compilation error in red swirls material" << std::endl;
		std::cerr << shader->errorString() << std::endl;
	}

	m_targetMaterial = geResourceMgr->createMaterial("target_material", shader);
}

void GameState::setupMeshes()
{
	m_floorMesh = geResourceMgr->createQuad("floor", 10.0f, 10.0f);
	m_floorMesh->setMaterial(m_floorMaterial);
	m_floorMesh->construct();

	m_targetMesh = geResourceMgr->loadMeshListFromFile("phone", "ge2test/Silly_Phone.obj");
	m_targetMesh[0]->setMaterial(m_targetMaterial);
	m_targetMesh[1]->setMaterial(m_targetMaterial);
	m_targetMesh[2]->destruct();

	m_wallMesh = geResourceMgr->createCube("wall", 10.0f);
	m_wallMesh->setMaterial(m_wallMaterial);
	m_wallMesh->construct();
}

void GameState::setupScene()
{
	glm::vec3 targetCell{0.0f};

	m_maze = createMaze();
	for (size_t y = 0; y < m_maze.size(); ++y) {
		MazeRow &row = m_maze[y];
		for (size_t x = 0; x < row.size(); ++x) {
			if (row[x].type == MazeCellType::Floor) {
				createFloor(x, y);
			} else if (row[x].type == MazeCellType::Wall) {
				createWall(x, y);
			} else if (row[x].type == MazeCellType::Start) {
				m_camera->setPosition(gridCoordinatesToWorld(x, y) + kCameraYAxisOffset);
				createFloor(x, y);
			} else if (row[x].type == MazeCellType::Target) {
				targetCell = gridCoordinatesToWorld(x, y);
				createFloor(x, y);
				createTarget(x, y);
			}
		}
	}

	m_flashlight.setColor(glm::vec3{1.0f});
	m_flashlight.setAngle(degToRad(45));
	m_flashlight.setExponent(320.0f);
	m_flashlight.setLength(20.0f);
	m_flashlight.setCutoff(0.01f);

	m_targetLight.setColor(glm::vec3{0.98f, 0.0f, 0.0f});
	m_targetLight.setRadius(1.0f);
	m_targetLight.setCutoff(0.01f);

	m_worldLight.setAmbientColor(glm::vec3{0.01f});
	m_worldLight.setColor(glm::vec3{0.0f});
	m_worldLight.setDirection(glm::vec3{0.0f, -1.0f, 0.0f});
}

Node *GameState::createFloor(int x, int y)
{
	Node *node = new Node;
	node->setMeshList({ m_floorMesh });
	node->setPosition(gridCoordinatesToWorld(x, y) + kFloorYAxisOffset);
	node->setRotation(glm::rotate(node->rotation(), -GE_PI / 2.0f, glm::vec3{1.0f, 0.0f, 0.0f}));
	m_mazeNode->addChild(node);

	return node;
}

Node *GameState::createTarget(int x, int y)
{
	Node *node = new Node;
	node->setMeshList(m_targetMesh);
	node->setScale(glm::vec3{0.02f});
	node->setPosition(gridCoordinatesToWorld(x, y) + kTargetAxisOffset);
	node->setLight(&m_targetLight);
	m_mazeNode->addChild(node);
	m_targetNode = node;

	return node;
}

Node *GameState::createWall(int x, int y)
{
	Node *node = new Node;
	node->setMeshList({ m_wallMesh });
	node->setPosition(gridCoordinatesToWorld(x, y) + kWallYAxisOffset);
	m_mazeNode->addChild(node);

	return node;
}

bool GameState::checkCollisions()
{
	CircleType player = { m_camera->position()[0], m_camera->position()[2], 1.0f };
	for (size_t y = 0; y < m_maze.size(); ++y) {
		MazeRow &row = m_maze[y];
		for (size_t x = 0; x < row.size(); ++x) {
			glm::vec3 realWorldPos = gridCoordinatesToWorld(x, y);
			RectType cell = { realWorldPos[0], realWorldPos[2], 10.0f, 10.0f };
			if (intersects(player, cell)) {
				if (row[x].type == MazeCellType::Wall) {
					float deltaX = abs(cell.x - player.x);
					float deltaY = abs(cell.y - player.y);
					bool collisionX = (deltaX >= 4.8f) && (deltaX >= deltaY);
					bool collisionY = (deltaY >= 4.8f) && (deltaY >= deltaX);
					if (player.x > cell.x && collisionX) m_camera->setCollisionFlags(m_camera->collisionFlags() | (uint32_t)CollisionFlags::West);
					if (player.x < cell.x && collisionX) m_camera->setCollisionFlags(m_camera->collisionFlags() | (uint32_t)CollisionFlags::East);
					if (player.y > cell.y && collisionY) m_camera->setCollisionFlags(m_camera->collisionFlags() | (uint32_t)CollisionFlags::North);
					if (player.y < cell.y && collisionY) m_camera->setCollisionFlags(m_camera->collisionFlags() | (uint32_t)CollisionFlags::South);
				} else if (row[x].type == MazeCellType::Target) {
					m_stateManager->transition();
					return false;
				}
			}
		}
	}
	return true;
}
