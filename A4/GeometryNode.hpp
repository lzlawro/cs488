// Termm--Fall 2020

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );

	virtual bool hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const override;

	Material *m_material;
	Primitive *m_primitive;
};
