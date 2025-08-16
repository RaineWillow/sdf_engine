#include "camera_3d.hpp"

Camera3d::Camera3d() : _cameraPosition(0.0, 0.0, 0.0),
_forward(0.0, 0.0, -1.0), _up(0.0, 1.0, 0.0), _right(1.0, 0.0, 0.0) {}

Camera3d::Camera3d(Vector3 cameraPosition) : _cameraPosition(cameraPosition),
_forward(0.0, 0.0, -1.0), _up(0.0, 1.0, 0.0), _right(1.0, 0.0, 0.0) {}

void Camera3d::setPitchAndYaw(float pitch, float yaw) {
  Quaternion qYaw(Vector3(0.0, 1.0, 0.0), yaw);
  
  
  _orientation = qYaw * _orientation;
  _orientation.normalize();
  _right = (_orientation * Quaternion::fromVector3(Vector3(1.0, 0.0, 0.0)) * _orientation.conjugate()).toVector3();
  Quaternion qPitch(_right, pitch);
  _orientation = qPitch * _orientation;
  _orientation.normalize();

  _forward = (_orientation * Quaternion::fromVector3(Vector3(0.0, 0.0, -1.0)) * _orientation.conjugate()).toVector3();
  _up = (_orientation * Quaternion::fromVector3(Vector3(0.0, 1.0, 0.0)) * _orientation.conjugate()).toVector3();
  _right = (_orientation * Quaternion::fromVector3(Vector3(1.0, 0.0, 0.0)) * _orientation.conjugate()).toVector3();
}

void Camera3d::setRoll(float roll) {
  Quaternion qRoll(_forward, roll);

  _orientation = qRoll * _orientation;
  _orientation.normalize();

  _up = (_orientation * Quaternion::fromVector3(Vector3(0.0, 1.0, 0.0)) * _orientation.conjugate()).toVector3();
  _right = (_orientation * Quaternion::fromVector3(Vector3(1.0, 0.0, 0.0)) * _orientation.conjugate()).toVector3();
}

void Camera3d::setPosition(Vector3 position) {
  _cameraPosition = position;
}

Vector3 Camera3d::up() const {
  return _up;
}

Vector3 Camera3d::down() const {
  return -_up;
}

Vector3 Camera3d::forward() const {
  return _forward;
}

Vector3 Camera3d::backward() const {
  return -_forward;
}

Vector3 Camera3d::right() const {
  return _right;
}

Vector3 Camera3d::left() const {
  return -_right;
}

Vector3 Camera3d::getCameraPosition() const {
  return _cameraPosition;
}

sf::Glsl::Vec3 Camera3d::loadCameraPosition() const {
  return _cameraPosition.toGlslVec3();
}

sf::Glsl::Vec3 Camera3d::loadCameraForward() const {
  return _forward.toGlslVec3();
}

sf::Glsl::Vec3 Camera3d::loadCameraUp() const {
  return _up.toGlslVec3();
}

sf::Glsl::Vec3 Camera3d::loadCameraRight() const {
  return _right.toGlslVec3();
}


