// Termm--Fall 2020

#include <bits/stdc++.h>
#include "Primitive.hpp"
#include "polyroots.hpp"

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

bool Sphere::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    return false;
}

Cube::~Cube()
{
}

bool Cube::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    return false;
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    glm::vec3 oc = ray.getOrigin() - m_pos;

	double A = glm::dot(ray.getDirection(), ray.getDirection());
	double B = 2.0f * glm::dot(oc, ray.getDirection());
	double C = dot(oc, oc) - m_radius*m_radius;

	// float discriminant = b*b - 4.0*a*c;
	// if (discriminant < 0) return -1.0;
	// return (-b - sqrt(discriminant)) / (2.0 * a);

	std::array<double, 2> roots;

	size_t numberOfRoots = quadraticRoots(A, B, C, roots.data());

	if (numberOfRoots > 0) {
        float temp = roots[0] < roots[1] ? roots[0] : roots[1];
        if (temp < t_max && temp > t_min) {
            record.t = temp;
            record.p = ray.pointAtParameter(record.t);
            record.normal = (record.p - m_pos) / static_cast<float>(m_radius);

            return true;
        }

        temp = roots[0] < roots[1] ? roots[1] : roots[0];
        if (temp < t_max && temp > t_min) {
            record.t = temp;
            record.p = ray.pointAtParameter(record.t);
            record.normal = (record.p - m_pos) / static_cast<float>(m_radius);

            return true;
        }
    }

	// return roots[0] < roots[1] ? roots[0] : roots[1];

    return false;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    return false;
}
