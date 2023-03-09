// Termm--Fall 2020

#include "SceneNode.hpp"
#include "GeometryNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
#include <stack>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	invtrans(mat4()),
	m_nodeId(nodeInstanceCount++)
{

}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = glm::inverse(m);
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
	return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
	return invtrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
	children.push_back(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	set_transform( rot_matrix * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	set_transform( glm::scale(amount) * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
}

// Useless
//---------------------------------------------------------------------------------------
void hitDfs(
	const SceneNode *node,
	const Ray &ray, 
	float t_min, 
	float t_max, 
	HitRecord &record,
	bool &hit_anything,
	double &closest_so_far,
	glm::mat4 &M
	) 
{
	
	if (node == nullptr) return;

	M = node->get_transform() * M;

	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

		vec3 a = ray.getOrigin();
		vec3 b = ray.getDirection();
		vec4 transformedOrigin = glm::inverse(M) * vec4(a.x, a.y, a.z, 1.0);
		vec4 transformedDirection = glm::inverse(M) * vec4(b.x, b.y, b.z, 0.0);

		Ray transformedRay(
			vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z),
			vec3(transformedDirection.x, transformedDirection.y, transformedDirection.z)
		);

		HitRecord tempRecord;

		if (geometryNode->m_primitive->hit(transformedRay, t_min, closest_so_far, tempRecord)) {
			hit_anything = true;
			tempRecord.material = geometryNode->m_material;
			vec4 transformedNormal = 
				glm::transpose(glm::inverse(M)) *
				vec4(
					tempRecord.normal.x,
					tempRecord.normal.y,
					tempRecord.normal.z,
					0.0
				);
			tempRecord.normal = vec3(
				transformedNormal.x, transformedNormal.y, transformedNormal.z
			);
			closest_so_far = tempRecord.t;
			record = tempRecord;
		}
	}

	for (const SceneNode *child: node->children) {
		hitDfs(child, ray, t_min, t_max, record, hit_anything, closest_so_far, M);
	}

	M = node->get_inverse() * M;

}

//---------------------------------------------------------------------------------------
bool SceneNode::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const {

	vec3 a = ray.getOrigin();
	vec3 b = ray.getDirection();
	vec4 transformedOrigin = invtrans * vec4(a.x, a.y, a.z, 1.0);
	vec4 transformedDirection = invtrans * vec4(b.x, b.y, b.z, 0.0);

	Ray transformedRay(
		vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z),
		vec3(transformedDirection.x, transformedDirection.y, transformedDirection.z)
	);

	HitRecord tempRecord;
	bool hitAnything = false;
	double closestSoFar = t_max;

	for (const SceneNode *child: children) {
		// if (child->m_nodeType != NodeType::GeometryNode)
		// 	continue;

		// const GeometryNode *geometryNode = static_cast<const GeometryNode *>(child);

		// HitRecord tempRecord;

		// if (geometryNode->m_primitive->hit(ray, t_min, closestSoFar, tempRecord)) {
		// 	hitAnything = true;
		// 	tempRecord.material = geometryNode->m_material;
		// 	closestSoFar = tempRecord.t;
		// 	record = tempRecord;
		// }
		if (child->hit(transformedRay, t_min, closestSoFar, tempRecord)) {
			hitAnything = true;
			closestSoFar = tempRecord.t;
			record = tempRecord;
		}
	}

	if (hitAnything) {
		vec4 transformedNormal = 
				glm::transpose(invtrans) *
				vec4(
					record.normal.x,
					record.normal.y,
					record.normal.z,
					0.0
				);
			record.normal = vec3(
				transformedNormal.x, transformedNormal.y, transformedNormal.z
			);
	}

	return hitAnything;
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;

	os << "]\n";
	return os;
}
