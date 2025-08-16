#include "transform.hpp"

void Transform::setOffset(Vector3 offset) {
  _offset = offset;

  updateChildrenOffsetSignal();
}

void Transform::setRotationOrigin(Vector3 rotationOrigin) {
  _rotationOrigin = rotationOrigin;
}

void Transform::setOrientation(Quaternion orientation) {
  _orientation = orientation;

  updateChildrenOrientationSignal();
}

Vector3 Transform::getOffset() {
  return _offset;
}

Vector3 Transform::getRotationOrigin() {
  return _rotationOrigin;
}

Quaternion Transform::getOrientation() {
  return _orientation;
}

Vector3 Transform::getWorldOffset() {
  if (hasParent()) {
    return (_parentOrientation * Quaternion::fromVector3(_offset) * _parentOrientation.conjugate()).toVector3()+_parentOffset;
  }
  return _offset;
}

Quaternion Transform::getWorldOrientation() {
  if (hasParent()) {
    return _parentOrientation * _orientation;
  }
  return _orientation;
}

void Transform::addChild(Transform * child) {
  if (child->hasParent()) {
    throw std::invalid_argument("Error, transform already has a parent, and cannot have two!");
  }

  //the child's new local offset is just it's world offset minus the parent's world offset
  child->_offset = child->getWorldOffset() - getWorldOffset();

  //the child's new local orientation
  child->_orientation = getWorldOrientation().conjugate() * child->getWorldOrientation();

  child->_parentOffset = getWorldOffset();
  child->_parentOrientation = getWorldOrientation();

  child->updateChildrenOffsetSignal();
  child->updateChildrenOrientationSignal();

  child->_parent = this;

  _children.insert(child);
}

void Transform::detach() {
  if (!hasParent()) {
    throw std::logic_error("Error! Attempted to detach a transform when the transform had no parent!");
  }
  _offset = getWorldOffset();
  _orientation = getWorldOrientation();

  _parent->_children.erase(this);
  _parent = NULL;

  updateChildrenOffsetSignal();
  updateChildrenOrientationSignal();
}

bool Transform::hasParent() {
  return _parent != NULL;
}

bool Transform::hasChildren() {
  return _children.size() > 0;
}

Transform * Transform::getParent() {
  return _parent;
}

AABBTransform Transform::getTransformedAABB(Vector3 max) {
  //begin by getting the extants
  Vector3 right(max.getX(), 0.0, 0.0);
  Vector3 up(0.0, max.getY(), 0.0);
  Vector3 depth(0.0, 0.0, max.getZ());

  Quaternion orientation = getWorldOrientation();
  Vector3 rotCenter = getRotationOrigin();

  right = (orientation *  Quaternion::fromVector3(right) * orientation.conjugate()).toVector3();
  up = (orientation *  Quaternion::fromVector3(up) * orientation.conjugate()).toVector3();
  depth = (orientation *  Quaternion::fromVector3(depth) * orientation.conjugate()).toVector3();

  Vector3 centerPoint = rotCenter + (orientation * Quaternion::fromVector3(-rotCenter) * orientation.conjugate()).toVector3();

  Vector3 extantToMax = (right.abs()+up.abs()+depth.abs());

  return {Vector3(centerPoint+extantToMax), Vector3(centerPoint-extantToMax)};
}

void Transform::updateChildrenOffsetSignal() {
  if (hasChildren()) {
    for (const auto& elem : _children) {
      elem->_parentOffset = getWorldOffset();
      elem->updateChildrenOffsetSignal();
    }
  }
}

void Transform::updateChildrenOrientationSignal() {
  if (hasChildren()) {
    for (const auto& elem : _children) {
      elem->_parentOrientation = getWorldOrientation();
      elem->updateChildrenOrientationSignal();
    }
  }
}