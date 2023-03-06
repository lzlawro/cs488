#pragma once

#include <glm/glm.hpp>

class Ray {
    public:
        Ray();
        Ray(const glm::vec3 &a, const glm::vec3 &b);
        glm::vec3 getOrigin() const;
        glm::vec3 getDirection() const;
        glm::vec3 pointAtParameter(float t) const;

    private:
        glm::vec3 A;
        glm::vec3 B;
};
