#include "ge2node.h"

#include <algorithm>

using namespace ge2;

Node::~Node()
{
	for (Node *node : m_children) {
		delete node;
	}
}

bool Node::enabled() const
{
	return m_enabled;
}

glm::vec3 Node::position() const
{
	return m_position;
}

glm::quat Node::rotation() const
{
	return m_rotation;
}

glm::vec3 Node::scale() const
{
	return m_scale;
}

glm::mat4 Node::transform()
{
	if (m_dirty) {
		m_transform = glm::translate(glm::mat4{1.0f}, m_position) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4{1.0f}, m_scale);
		m_dirty = false;
	}

	return m_transform;
}

glm::vec3 Node::worldPosition()
{
	glm::mat4 acc = transform();
	Node *n = parent();
	while (n) {
		acc = n->transform() * acc;
		n = n->parent();
	}

	return glm::vec3{acc[3]};
}

void Node::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void Node::setPosition(const glm::vec3 &position)
{
	m_position = position;
	m_dirty = true;
}

void Node::setRotation(const glm::quat &rotation)
{
	m_rotation = rotation;
	m_dirty = true;
}

void Node::setScale(const glm::vec3 &scale)
{
	m_scale = scale;
	m_dirty = true;
}

void Node::setTransform(glm::mat4 matrix)
{
	m_transform = matrix;
	m_dirty = false;
}

Node *Node::parent() const
{
	return m_parent;
}

const NodeList &Node::children() const
{
	return m_children;
}

void Node::addChild(Node *child)
{
	if (child->m_parent) {
		child->m_parent->removeChild(child);
	}

	m_children.push_back(child);
	child->setParent(this);
}

void Node::removeChild(Node *child)
{
	m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
	child->setParent(nullptr);
}

void Node::setParent(Node *parent)
{
	m_parent = parent;
}

Light *Node::light() const
{
	return m_light;
}

int Node::meshCount() const
{
	return m_meshList.size();
}

const MeshList &Node::meshList() const
{
	return m_meshList;
}

void Node::setLight(Light *light)
{
	m_light = light;
}

void Node::setMeshList(MeshList list)
{
	m_meshList = list;
}

void NodeTreeVisitor::visitNodeTree(Node *node, const NodeListenerList &listeners)
{
	if (!node) {
		return;
	}

	pushMatrix(node->transform());

	for (auto listener : listeners) {
		listener(node, m_modelMatrix);
	}

	for (Node *child : node->children()) {
		visitNodeTree(child, listeners);
	}

	popMatrix();
}

void NodeTreeVisitor::pushMatrix(const glm::mat4 &matrix)
{
	m_matrixStack.push(m_modelMatrix);
	m_modelMatrix = m_modelMatrix * matrix;
}

void NodeTreeVisitor::popMatrix()
{
	if (m_matrixStack.empty()) {
		return;
	}

	m_modelMatrix = m_matrixStack.top();
	m_matrixStack.pop();
}
