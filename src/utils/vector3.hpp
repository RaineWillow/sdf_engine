#pragma once
#ifndef Vector3_hpp
#define Vector3_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <stdexcept>
#include <cmath>

class Vector3 {
public:
  Vector3();
  Vector3(float x, float y, float z);
  ~Vector3();

  void setX(float x);
  void setY(float y);
  void setZ(float z);

  float getX() const;
  float getY() const;
  float getZ() const;

  Vector3 operator+(const Vector3& rhs) const;
  Vector3 operator-(const Vector3& rhs) const;
  Vector3 operator*(const Vector3& rhs) const;
  Vector3 operator/(const Vector3& rhs) const;

  Vector3 operator+(float rhs) const;
  Vector3 operator-(float rhs) const;
  Vector3 operator*(float rhs) const;
  Vector3 operator/(float rhs) const;

  Vector3 operator-() const;

  float length() const;
  Vector3 normalize() const;
  Vector3 abs() const;
  Vector3 cross(const Vector3& rhs) const;

  static Vector3 fromGlslVec3(const sf::Glsl::Vec3& in);

  sf::Glsl::Vec3 toGlslVec3() const;

private:
  float _x;
  float _y;
  float _z;
};

#endif