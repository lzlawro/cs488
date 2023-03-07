// Termm--Fall 2020

#include "Primitive.hpp"

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
    return false;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const
{
    return false;
}
