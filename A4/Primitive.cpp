// Termm--Fall 2020

#include <bits/stdc++.h>
#include "Primitive.hpp"
#include "polyroots.hpp"

#define EPSILON 0.0001

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

NonhierCuboid::~NonhierCuboid()
{
}

// Axis-aligned bounding box
// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms/18459#18459
bool NonhierCuboid::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    const glm::vec3 minCorner = m_pos;
    const glm::vec3 maxCorner = m_pos + m_dimensions;
    const glm::vec3 directionFraction = 1.0f / ray.getDirection();

    float t1 = (minCorner.x - ray.getOrigin().x) * directionFraction.x;
    float t2 = (maxCorner.x - ray.getOrigin().x) * directionFraction.x;
    float t3 = (minCorner.y - ray.getOrigin().y) * directionFraction.y;
    float t4 = (maxCorner.y - ray.getOrigin().y) * directionFraction.y;
    float t5 = (minCorner.z - ray.getOrigin().z) * directionFraction.z;
    float t6 = (maxCorner.z - ray.getOrigin().z) * directionFraction.z;

    float tMin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
    float tMax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    if (tMax < 0 || tMax < tMin) return false;

    // Intersection case
    record.t = (tMin < 0 && tMax > 0) ? tMax : tMin;
    // record.t = tMin;
    record.p = ray.pointAtParameter(record.t);

    if (glm::abs(record.p.x - minCorner.x) < EPSILON) record.normal = glm::vec3(-1.0f, 0, 0);
    else if (glm::abs(record.p.x - maxCorner.x) < EPSILON) record.normal = glm::vec3(+1.0f, 0, 0);
    else if (glm::abs(record.p.y - minCorner.y) < EPSILON) record.normal = glm::vec3(0, -1.0f, 0);
    else if (glm::abs(record.p.y - maxCorner.y) < EPSILON) record.normal = glm::vec3(0, 1.0f, 0);
    else if (glm::abs(record.p.z - minCorner.z) < EPSILON) record.normal = glm::vec3(0, 0, -1.0f);
    else if (glm::abs(record.p.z - maxCorner.z) < EPSILON) record.normal = glm::vec3(0, 0, 1.0f);
    
    return true;

    // return false;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    NonhierCuboid boxCuboid(m_pos, glm::vec3(m_size, m_size, m_size));

    return boxCuboid.hit(ray, t_min, t_max, record);
}
