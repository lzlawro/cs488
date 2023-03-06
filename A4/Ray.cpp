#include "Ray.hpp"

using namespace std;
using namespace glm;

Ray::Ray() {}

Ray::Ray(const glm::vec3 &a, const glm::vec3 &b) { A = a; B = b; }

vec3 Ray::getOrigin() const { return A; }

vec3 Ray::getDirection() const { return B; }

vec3 Ray::pointAtParameter(float t) const { return A + t*B; }
