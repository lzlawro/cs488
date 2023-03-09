// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

bool Triangle::hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const {
	return false;
}

bool Mesh::hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const {
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
