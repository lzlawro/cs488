// Termm--Fall 2020

#include "GeometryNode.hpp"

#include <iostream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

//---------------------------------------------------------------------------------------
bool GeometryNode::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const {
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

	if (m_primitive->hit(transformedRay, t_min, t_max, tempRecord)) {
		tempRecord.material = m_material;
		hitAnything = true;
		closestSoFar = tempRecord.t;
		record = tempRecord;
	}

	// for (const SceneNode *child: children) {
	// 	if (child->hit(transformedRay, t_min, closestSoFar, tempRecord)) {
	// 		hitAnything = true;
	// 		closestSoFar = tempRecord.t;
	// 		record = tempRecord;
	// 	}
	// }

	HitRecord tempRecord_;

	if (SceneNode::hit(ray, t_min, closestSoFar, tempRecord_)) {
		hitAnything = true;
		record = tempRecord_;
		closestSoFar = tempRecord_.t;
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