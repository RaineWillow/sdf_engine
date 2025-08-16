#pragma once
#ifndef Transform_hpp
#define Transform_hpp

#include "../utils/vector3.hpp"
#include "../utils/quaternion.hpp"
#include <unordered_set>
#include <stdexcept>

struct AABBTransform {
  Vector3 max;
  Vector3 min;
};

class Transform {
public:
  void setOffset(Vector3 offset);
  void setRotationOrigin(Vector3 rotationOrigin);
  void setOrientation(Quaternion orientation);
  //void setScale(float scale);

  Vector3 getOffset();
  Vector3 getRotationOrigin();
  Quaternion getOrientation();
  //float getScale();

  Vector3 getWorldOffset();
  Quaternion getWorldOrientation();
  //float getWorldScale();

  void addChild(Transform * child);
  void detach();
  bool hasParent();
  bool hasChildren();
  Transform * getParent();

  AABBTransform getTransformedAABB(Vector3 max);
private:
  //local transformation values
  Vector3 _offset;
  Vector3 _rotationOrigin;
  Quaternion _orientation;
  float _scale = 1.0;
  //parent transformation values
  Vector3 _parentOffset;
  Quaternion _parentOrientation;
  float _parentScale = 1.0;

  //scaling factor
  

  //list of children 
  std::unordered_set<Transform*> _children;
  Transform * _parent = NULL;

  void updateChildrenOffsetSignal();
  void updateChildrenOrientationSignal();
  void updateChildrenScaleSignal();
};

#endif