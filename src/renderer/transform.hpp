#pragma once
#ifndef Transform_hpp
#define Transform_hpp

#include "../utils/vector3.hpp"
#include "../utils/quaternion.hpp"
#include <unordered_set>
#include <stdexcept>

class Transform {
public:
  void setOffset(Vector3 offset);
  void setRotationOrigin(Vector3 rotationOrigin);
  void setOrientation(Quaternion orientation);

  Vector3 getOffset();
  Vector3 getRotationOrigin();
  Quaternion getOrientation();

  Vector3 getWorldOffset();
  Vector3 getWorldRotationOrigin();
  Quaternion getWorldOrientation();

  void addChild(Transform * child);
  void detach();
  bool hasParent();
  bool hasChildren();
  Transform * getParent();
private:
  //local transformation values
  Vector3 _offset;
  Vector3 _rotationOrigin;
  Quaternion _orientation;
  //parent transformation values
  Vector3 _parentOffset;
  Vector3 _parentRotationOrigin;
  Quaternion _parentOrientation;

  //list of children 
  std::unordered_set<Transform*> _children;
  Transform * _parent = NULL;

  void updateChildrenOffsetSignal();
  void updateChildrenRotationOriginSignal();
  void updateChildrenOrientationSignal();
};

#endif