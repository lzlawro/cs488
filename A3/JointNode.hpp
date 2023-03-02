// Termm-Fall 2020

#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);

	void rotate(char axis, float angle) override;

	struct JointRange {
		double min, init, max;
	};


	JointRange m_joint_x, m_joint_y;

	float m_angle_x;
	float m_angle_y;

};
