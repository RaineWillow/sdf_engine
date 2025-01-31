#pragma once
#ifndef Camera_3d_hpp
#define Camera_3d_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <stdexcept>
#include <cmath>
#include "../utils/vector3.hpp"
#include "../utils/quaternion.hpp"

class Camera3d {
public:
  Camera3d();
  Camera3d(Vector3 cameraPosition);

  void setPitchAndYaw(float pitch, float yaw);
  void setRoll(float roll);
  void setPosition(Vector3 position);

  Vector3 up() const;
  Vector3 down() const;
  Vector3 forward() const;
  Vector3 backward() const;
  Vector3 right() const;
  Vector3 left() const;
  Vector3 getCameraPosition() const;

  sf::Glsl::Vec3 loadCameraPosition() const;
  sf::Glsl::Vec3 loadCameraForward() const;
  sf::Glsl::Vec3 loadCameraUp() const;
  sf::Glsl::Vec3 loadCameraRight() const;

private:
  Vector3 _cameraPosition;
  Vector3 _forward;
  Vector3 _up;
  Vector3 _right;

  Quaternion _orientation;
};

#endif