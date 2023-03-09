// Termm--Winter 2023

#include <glm/ext.hpp>

#include "A4.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "PhongMaterial.hpp"
#include <array>

#define EPSILON 0.0001

using namespace std;
using namespace glm;

// Useless now as hit detection methods for primitives have been defined
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
	SceneNode *scene,
	const glm::vec3 & eye,
	const glm::vec3 & ambient,
	const std::list<Light *> & lights
	) {
	// if (hitSphere(vec3(0, 0, 750), 10, ray)) {
	// 	return vec3(1.0, 0.0, 0.0);
	// }

	HitRecord record, shadowRecord;

	if (scene->hit(ray, EPSILON, std::numeric_limits<float>::max(), record)) {
		vec3 color(0.2*ambient);
		// Perform Blinn-Phong shading for each lightsource
		if (record.material->m_materialType == MaterialType::PhongMaterial) {

			for (const Light * light: lights) {

				vec3 p = ray.pointAtParameter(record.t);

				vec3 pAdjusted = p + (light->position - p) * EPSILON;

				Ray shadowRay(pAdjusted, light->position - pAdjusted);

				if (!scene->hit(shadowRay, 0, std::numeric_limits<float>::infinity(), shadowRecord)) {
					vec3 l = normalize(shadowRay.getDirection());
					vec3 n = normalize(record.normal);

					PhongMaterial *phongMaterial = static_cast<PhongMaterial *>(record.material);

					vec3 kd = phongMaterial->getKd();
					vec3 ks = phongMaterial->getKs();
					double shininess = phongMaterial->getShininess();
					
					vec3 v = normalize(eye - ray.pointAtParameter(record.t));

					vec3 h = normalize(l + normalize(-ray.getDirection()));

					color += kd * light->colour * glm::max((float)0.0, dot(n, l)) +
						     ks * light->colour * glm::pow(dot(n, h), shininess);

				}
			}
		}

		return color;
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

	#ifdef ENABLE_RANDOM_SAMPLING
	size_t ns = 16;
	#else
	size_t ns = 0;
	#endif

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
