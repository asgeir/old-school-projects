#pragma once

#include "ge2common.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <functional>
#include <stack>
#include <vector>

namespace ge2 {

class Light;
class Mesh;
class Node;

typedef std::vector<Node *> NodeList;

typedef std::function<void(Node *node, const glm::mat4 &modelMatrix)> NodeListener;
typedef std::vector<NodeListener> NodeListenerList;

class Node
{
public:
	Node() = default;
	Node(const Node &other) = delete;
	Node(Node &&rhs) = delete;
	virtual ~Node();

	Node &operator=(const Node &other) = delete;
	Node &operator=(Node &&rhs) = delete;

	bool enabled() const;
	glm::vec3 position() const;
	glm::quat rotation() const;
	glm::vec3 scale() const;
	glm::mat4 transform();
	glm::vec3 worldPosition();

	void setEnabled(bool enabled);
	void setPosition(const glm::vec3 &position);
	void setRotation(const glm::quat &rotation);
	void setScale(const glm::vec3 &scale);
	void setTransform(glm::mat4 matrix);

	Node *parent() const;
	const NodeList &children() const;

	void addChild(Node *child);
	void removeChild(Node *child);

	Light *light() const;
	int meshCount() const;
	const MeshList &meshList() const;

	void setLight(Light *light);
	void setMeshList(MeshList list);

private:
	void setParent(Node *parent);

	bool      m_dirty = false;
	bool      m_enabled = true;
	glm::vec3 m_position{0.0f};
	glm::quat m_rotation;
	glm::vec3 m_scale{1.0f};
	glm::mat4 m_transform{1.0f};

	Node     *m_parent = nullptr;
	NodeList  m_children;

	Light    *m_light = nullptr;
	MeshList  m_meshList;
};

class NodeTreeVisitor
{
public:
	void visitNodeTree(Node *node, const NodeListenerList &listeners);

private:
	void pushMatrix(const glm::mat4 &matrix);
	void popMatrix();

	std::stack<glm::mat4> m_matrixStack;
	glm::mat4             m_modelMatrix{1.0f};
};

} // namespace ge2
