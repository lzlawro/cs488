// Termm-Fall 2020

#include "JointNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
	m_angle_x = 0.0f;
	m_angle_y = 0.0f;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
}

void JointNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			// m_joint_x.min <= m_angle_x + angle <= m_joint_x.max
			angle = clamp(angle, (float)(m_joint_x.min - m_angle_x), (float)(m_joint_x.max - m_angle_x));
			m_angle_x += angle;
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
			angle = clamp(angle, (float)(m_joint_y.min - m_angle_y), (float)(m_joint_y.max - m_angle_y));
			m_angle_y += angle;
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	trans = rot_matrix * trans;
}
