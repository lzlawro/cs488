// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

bool Mesh::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const {
	if (!m_boundingCuboid.hit(ray, t_min, t_max, record)) return false;

	#ifdef RENDER_BOUNDING_VOLUMES

	return true;

	#endif

	// TODO: if the option to render the bounding box is on, then do it here

	// If the ray hits the bounding volume, proceed to checking each individual triangle
	for (const Triangle &triangle: m_faces) {
		// Cramer's rule
		glm::vec3 a = m_vertices[triangle.v1];
		glm::vec3 b = m_vertices[triangle.v2];
		glm::vec3 c = m_vertices[triangle.v3];

		glm::vec3 col1 = glm::vec3(a - b);
		glm::vec3 col2 = glm::vec3(a - c);
		glm::vec3 col3 = ray.getDirection();
		glm::vec3 colRhs = glm::vec3(a - ray.getOrigin());

		glm::mat3x3 A = glm::mat3(col1, col2, col3);

		// glm::mat3 A = glm::transpose(glm::mat3(
		// 	a.x-b.x, a.x-c.x, ray.getDirection().x,
		// 	a.y-b.y, a.y-c.y, ray.getDirection().y,
		// 	a.z-b.z, a.z-c.z, ray.getDirection().z
		// ));

		// std::cout << glm::to_string(A) << std::endl;

		// std::cout << glm::to_string(glm::mat3(col1, col2, col3)) << std::endl;

		auto detOfA =  glm::determinant(A);

		auto t = glm::determinant(glm::mat3(col1, col2, colRhs)) / detOfA;
		if (t < t_min || t > t_max) continue; // continue is needed here instead of returning false

		auto gamma = glm::determinant(glm::mat3(col1, colRhs, col3)) / detOfA;
		if (gamma < 0 || gamma > 1) continue;

		auto beta = glm::determinant(glm::mat3(colRhs, col2, col3)) / detOfA;
		if (beta < 0 || beta > 1 - gamma) continue;

		// Hit case
		record.t = t;
		record.normal = glm::cross(col1, col2);
		return true;
	}

	// It is possible that the ray intersects the bounding box but still doesn't hit
	// any triangle. Or is it?
	return false;
}

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
	, m_boundingCuboid()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	std::cout << fname << ": " << m_vertices.size() << std::endl;

	// Determine the center position and dimensions of the bounding box
	if (m_vertices.size() > 0) {
		glm::vec3 minCorner(std::numeric_limits<float>::max());
		glm::vec3 maxCorner(-std::numeric_limits<float>::max());

		// std::cout << glm::to_string(maxCorner) << std::endl;

		for (const glm::vec3 &vertex: m_vertices) {
			for (int i = 0; i <= 2; i++) {
				if (vertex[i] < minCorner[i]) minCorner[i] = vertex[i];
				if (vertex[i] > maxCorner[i]) maxCorner[i] = vertex[i];
			}
		}

		// std::cout << m_vertices.size() << ", "+ glm::to_string(minCorner) + 
		//              ", " + glm::to_string(maxCorner) << std::endl;

		m_boundingCuboid.setPos(minCorner);
		m_boundingCuboid.setDimensions(maxCorner - minCorner);
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}
