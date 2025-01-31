#pragma once
#ifndef Quaternion_hpp
#define Quaternion_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <stdexcept>
#include <cmath>
#include "vector3.hpp"

class Quaternion {
public:
  Quaternion();
  Quaternion(float x, float y, float z, float w);
  Quaternion(Vector3 axis, float angle);
  ~Quaternion();

  void setX(float x);
  void setY(float y);
  void setZ(float z);
  void setW(float w);

  void setAxisAngle(Vector3 axis, float angle);

  float getX() const;
  float getY() const;
  float getZ() const;
  float getW() const;

  Quaternion operator*(const Quaternion& rhs) const;

  Quaternion normalize() const;
  Quaternion inverse() const;
  Quaternion conjugate() const;

  static Quaternion fromVector3(const Vector3& in);
  static Quaternion fromGlslVec4(const sf::Glsl::Vec4& in);

  Vector3 toVector3() const;
  sf::Glsl::Vec4 toGlslVec4() const;

private:
  float _x;
  float _y;
  float _z;
  float _w;
};

#endif
