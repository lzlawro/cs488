// Termm--Winter 2023

#include <glm/ext.hpp>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "PhongMaterial.hpp"
#include <array>

using namespace std;
using namespace glm;

float hitSphere(const vec3 &center, float radius, const Ray &ray) {
	vec3 oc = ray.getOrigin() - center;

	double A = dot(ray.getDirection(), ray.getDirection());
	double B = 2.0f * dot(oc, ray.getDirection());
	double C = dot(oc, oc) - radius*radius;

	// float discriminant = b*b - 4.0*a*c;

	// if (discriminant < 0) return -1.0;

	// return (-b - sqrt(discriminant)) / (2.0 * a);

	array<double, 2> roots;

	size_t numberOfRoots = quadraticRoots(A, B, C, roots.data());

	if (numberOfRoots == 0) return -1.0;

	return roots[0] < roots[1] ? roots[0] : roots[1];
}


vec3 rayColor(
	const Ray &ray, 
	SceneNode *root,
	const glm::vec3 & eye,
	const glm::vec3 & ambient,
	const std::list<Light *> & lights
	) {
	// if (hitSphere(vec3(0, 0, 750), 10, ray)) {
	// 	return vec3(1.0, 0.0, 0.0);
	// }

	HitRecord record, tempRecord;

	bool hitAnything = false;

	double closestSoFar = std::numeric_limits<float>::max();

	for (const SceneNode *node: root->children) {
		if (node->m_nodeType != NodeType::GeometryNode)
			continue;

		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

		if (geometryNode->m_primitive->hit(ray, 0.0001, closestSoFar, tempRecord)) {
			hitAnything = true;
			tempRecord.material = geometryNode->m_material;
			closestSoFar = tempRecord.t;
			record = tempRecord;
		}
	}

	if (hitAnything) {
		// Perform Blinn-Phong shading for each lightsource
		if (record.material->m_materialType == MaterialType::PhongMaterial) {
			vec3 L(0.1*ambient);

			for (const Light * light: lights) {
				PhongMaterial *phongMaterial = static_cast<PhongMaterial *>(tempRecord.material);

				vec3 kd = phongMaterial->getKd();
				vec3 ks = phongMaterial->getKs();
				double shininess = phongMaterial->getShininess();

				vec3 n = normalize(record.normal);
				vec3 l = normalize(light->position - ray.pointAtParameter(record.t));
				vec3 v = normalize(eye - ray.pointAtParameter(record.t));

				vec3 h = normalize(length(v)*l + length(l)*v);

				L += kd*light->colour*glm::max((float)0.0, dot(n, l)) +
					ks*light->colour*glm::pow(glm::max((float)0.0, dot(n, h)), shininess);
			}

			return L;
		}

		// return 0.5 * vec3(record.normal.x+1, record.normal.y+1, record.normal.z+1);
	}

	// float t = hitSphere(vec3(0, -1200, -500), 1000, ray);

	// if (t > 0.0) {
	// 	vec3 N = normalize(ray.pointAtParameter(t) - vec3(0,-1200,500));
	// 	return 0.5*vec3(N.x+1, N.y+1, N.z+1);
	// }

	// Background
	vec3 unitDirection = normalize(ray.getDirection());
	float t = (normalize(unitDirection).y + 0.5);
	return (1.0-t) * vec3(0.2, 0.2, 1.0) + (t) * vec3(0.0, 0.0, 0.0);

	// // Background
	// vec3 unitDirection = normalize(ray.getDirection());
	// float t = (unitDirection.y + 1.0) / 3.0;
	// // return (t) * vec3(0.0, 0.0, 0.0) + (1.0 - 2.0 * t) * vec3(0.0, 0.0, 0.0) + (1.0 - t) * vec3(0.0, 0.0, 1.0);
	// return (1.0-t) * vec3(1.0, 1.0, 1.0) + (t) * vec3(0.5, 0.7, 1.0);
	// // return (1.0-t) * vec3(0.5, 0.5, 0.1) + (t) * vec3(0.3, 0.3, 1.0);
}

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  

  std::cout << "W23: Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t ny = image.height();
	size_t nx = image.width();

	size_t ns = 16;

	// ny = 100;
	// nx = 200;

	// // Specify the (in world coordinate) parameters that define the camera
	// vec3 lowerLeftCorner(-2.0, -1.0, -1.0);
	// vec3 horizontal(4.0, 0.0, 0.0);
	// vec3 vertical(0.0, 2.0, 0.0);
	// vec3 origin(0.0, 0.0, 0.0);

	// Is view "Lookat"? or is the view already a vector?
	vec3 vz = normalize(view-eye);
	vec3 vx = normalize(cross(up, vz));
	vec3 vy = cross(vz, vx);
	float d = (ny / 2) / tan(radians(fovy / 2));

	vec3 lowerLeftCorner = vz*d - vx*((float)ny/2) - vy*((float)ny/2);

	for (uint y = 0; y < ny; ++y) {
		for (uint x = 0; x < nx; ++x) {
			// float u = float(x) / float(nx);
			// float v = float(ny-y) / float(ny);

			vec3 color(0.0, 0.0, 0.0);

			if (ns) {
				for (uint s = 0; s < ns; s++) {
					float u = float(x + drand48());
					float v = float(y + drand48());

					Ray ray(eye, lowerLeftCorner + (float)(nx-u)*vx + (float)(ny-v)*vy);
					vec3 p = ray.pointAtParameter(2.0);
					color += rayColor(ray, root, eye, ambient, lights);
				}

				color /= float(ns);
			} else {
				Ray ray(eye, lowerLeftCorner + (float)(nx-x)*vx + (float)(ny-y)*vy);
				vec3 p = ray.pointAtParameter(2.0);
				color = rayColor(ray, root, eye, ambient, lights);
			}

			// // Red: 
			// image(x, y, 0) = 0.5f;
			// // Green: 
			// image(x, y, 1) = 0.5f;
			// // Blue: 
			// image(x, y, 2) = 0.5f;

			// Red: 
			image(x, y, 0) = color.r;
			// Green: 
			image(x, y, 1) = color.g;
			// Blue: 
			image(x, y, 2) = color.b;
		}
	}

}
