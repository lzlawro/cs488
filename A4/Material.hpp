// Termm--Fall 2020

#pragma once

enum class MaterialType {
  Material,
  PhongMaterial
};

class Material {
public:
  virtual ~Material();
  MaterialType m_materialType;

protected:
  Material();
};
