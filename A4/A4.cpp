// Termm--Winter 2023

#include <glm/ext.hpp>

#include "A4.hpp"

using namespace std;
using namespace glm;

vec3 rayColor(const Ray &r) {
	vec3 unitDirection = normalize(r.getDirection());

	float t = 0.5 * (unitDirection.y + 1.0);

	return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
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

	size_t h = image.height();
	size_t w = image.width();

	// Specify the parameters that define the camera
	vec3 lowerLeftCorner(-2.0, -1.0, -1.0);
	vec3 horizontal(4.0, 0.0, 0.0);
	vec3 vertical(0.0, 2.0, 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			float u = float(x) / float(w);
			float v = float(y) / float(h);

			Ray r(origin, lowerLeftCorner + u*horizontal + v*vertical);

			vec3 color = rayColor(r);

			// Red: 
			image(x, y, 0) = color.r;
			// Green: 
			image(x, y, 1) = color.g;
			// Blue: 
			image(x, y, 2) = color.b;
		}
	}

}
