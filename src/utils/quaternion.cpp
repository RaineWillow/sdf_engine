#include "quaternion.hpp"

Quaternion::Quaternion() {
  _x = 0;
  _y = 0;
  _z = 0;
  _w = 1;
}

Quaternion::Quaternion(float x, float y, float z, float w) {
  _x = x;
  _y = y;
  _z = z;
  _w = w;
}

Quaternion::Quaternion(Vector3 axis, float angle) {
  setAxisAngle(axis, angle);
}

Quaternion::~Quaternion() {

}

void Quaternion::setX(float x) {
  _x = x;
}

void Quaternion::setY(float y) {
  _y = y;
}

void Quaternion::setZ(float z) {
  _z = z;
}

void Quaternion::setW(float w) {
  _w = w;
}

void Quaternion::setAxisAngle(Vector3 axis, float angle) {
  Vector3 imaginary = axis.normalize() * std::sin(angle / 2.0);
  float real = std::cos(angle / 2.0);

  _x = imaginary.getX();
  _y = imaginary.getY();
  _z = imaginary.getZ();
  _w = real;
}

float Quaternion::getX() const {
  return _x;
}

float Quaternion::getY() const {
  return _y;
}

float Quaternion::getZ() const {
  return _z;
}

float Quaternion::getW() const {
  return _w;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const {
  return Quaternion(
    _w*rhs._x + _x*rhs._w + _y*rhs._z - _z*rhs._y,
    _w*rhs._y - _x*rhs._z + _y*rhs._w + _z*rhs._x,
    _w*rhs._z + _x*rhs._y - _y*rhs._x + _z*rhs._w,
    _w*rhs._w - _x*rhs._x - _y*rhs._y - _z*rhs._z    
  );
}

Quaternion Quaternion::normalize() const {
  float norm = std::sqrt(_x*_x + _y*_y + _z*_z + _w*_w);
  if (norm==0.0) {
    throw std::runtime_error("Error, cannot normalize a zero quaternion!");
  }
  float invNorm = 1.0/norm;
  return Quaternion(_x*invNorm, _y*invNorm, _z*invNorm, _w*invNorm);
}

Quaternion Quaternion::inverse() const {
  float normSquare = _x*_x + _y*_y + _z*_z + _w*_w;
  if (normSquare==0.0) {
    throw std::runtime_error("Cannot invert a quaternion with a zero norm!");
  }
  return Quaternion(-_x/normSquare, -_y/normSquare, -_z/normSquare, _w/normSquare);
}

Quaternion Quaternion::conjugate() const {
  return Quaternion(-_x, -_y, -_z, _w);
}

Quaternion Quaternion::fromVector3(const Vector3& in) {
  return Quaternion(in.getX(), in.getY(), in.getZ(), 0.0);
}

Quaternion Quaternion::fromGlslVec4(const sf::Glsl::Vec4& in) {
  return Quaternion(in.x, in.y, in.z, in.w);
}

Vector3 Quaternion::toVector3() const {
  return Vector3(_x, _y, _z);
}

sf::Glsl::Vec4 Quaternion::toGlslVec4() const {
  return sf::Glsl::Vec4(_x, _y, _z, _w);
}