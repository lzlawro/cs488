// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include "Ray.hpp"

struct HitRecord {
  float t;
  glm::vec3 p;
  glm::vec3 normal;
};

class Primitive {
public:
  virtual ~Primitive();
  virtual bool hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const = 0;
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  bool hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const;
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  bool hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  bool hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const;
  // virtual bool hit(const Ray &ray, float t_min, float t_max, HitRecord &record) const;

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual ~NonhierBox();
  bool hit(const Ray &ray, float tmin, float tmax, HitRecord &record) const;

private:
  glm::vec3 m_pos;
  double m_size;
};
