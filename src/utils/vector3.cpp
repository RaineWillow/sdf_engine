#include "vector3.hpp"

Vector3::Vector3() {
  _x = 0;
  _y = 0;
  _z = 0;
}

Vector3::Vector3(float x, float y, float z) {
  _x = x;
  _y = y;
  _z = z;
}

Vector3::~Vector3() {

}

void Vector3::setX(float x) {
  _x = x;
}

void Vector3::setY(float y) {
  _y = y;
}

void Vector3::setZ(float z) {
  _z = z;
}

float Vector3::getX() const {
  return _x;
}

float Vector3::getY() const {
  return _y;
}

float Vector3::getZ() const {
  return _z;
}

Vector3 Vector3::operator+(const Vector3& rhs) const {
  return Vector3(_x + rhs._x, _y + rhs._y, _z + rhs._z);
}

Vector3 Vector3::operator-(const Vector3& rhs) const {
  return Vector3(_x - rhs._x, _y - rhs._y, _z - rhs._z);
}

Vector3 Vector3::operator*(const Vector3& rhs) const {
  return Vector3(_x * rhs._x, _y * rhs._y, _z * rhs._z);
}

Vector3 Vector3::operator/(const Vector3& rhs) const {
  if (rhs._x == 0.0 || rhs._y == 0.0 || rhs._z == 0.0) {
    throw std::runtime_error("Division by zero in Vector3/Vector3 division operation!");
  }
  return Vector3(_x / rhs._x, _y / rhs._y, _z / rhs._z);
}

Vector3 Vector3::operator+(float rhs) const {
  return Vector3(_x + rhs, _y + rhs, _z + rhs);
}

Vector3 Vector3::operator-(float rhs) const {
  return Vector3(_x - rhs, _y - rhs, _z - rhs);
}

Vector3 Vector3::operator*(float rhs) const {
  return Vector3(_x * rhs, _y * rhs, _z * rhs);
}

Vector3 Vector3::operator-() const {
  return Vector3(-_x, -_y, -_z);
}

Vector3 Vector3::operator/(float rhs) const {
  if (rhs == 0) {
    throw std::runtime_error("Division by zero in Vector3/Float division operation!");
  }
  return Vector3(_x / rhs, _y / rhs, _z / rhs);
}

float Vector3::length() const {
  return std::sqrt(_x*_x + _y*_y + _z*_z);
}

Vector3 Vector3::normalize() const {
  float magnitude = length();
  if (magnitude == 0) {
    throw std::invalid_argument("Error, attempted to normalize vector but it's magnitude was zero!");
  }
  return Vector3(_x/magnitude, _y/magnitude, _z/magnitude);
}

Vector3 Vector3::cross(const Vector3& rhs) const {
  return Vector3(
    _y*rhs._z - _z*rhs._y,
    _z*rhs._x - _x*rhs._z,
    _x*rhs._y - _y*rhs._x
  );
}

Vector3 Vector3::fromGlslVec3(const sf::Glsl::Vec3& in) {
  return Vector3(in.x, in.y, in.z);
}

sf::Glsl::Vec3 Vector3::toGlslVec3() const {
  return sf::Glsl::Vec3(_x, _y, _z);
}
