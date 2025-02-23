#include "transform.hpp"

void Transform::setOffset(Vector3 offset) {
  _offset = offset;

  updateChildrenOffsetSignal();
}

void Transform::setRotationOrigin(Vector3 rotationOrigin) {
  _rotationOrigin = rotationOrigin;

  updateChildrenRotationOriginSignal();
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
    return _offset+_parentOffset;
  }
  return _offset;
}

Vector3 Transform::getWorldRotationOrigin() {
  if (hasParent()) {
    return _parentRotationOrigin + (_parentOrientation * Quaternion::fromVector3(_rotationOrigin - _parentRotationOrigin) * _parentOrientation.conjugate()).toVector3();
  }
  return _rotationOrigin;
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

  //the child's new local rotation origin
  child->_rotationOrigin = getWorldRotationOrigin() + (getWorldOrientation().conjugate() * Quaternion::fromVector3(child->getWorldRotationOrigin() - getWorldRotationOrigin()) * getWorldOrientation()).toVector3();
  
  //the child's new local orientation
  child->_orientation = getWorldOrientation().conjugate() * child->getWorldOrientation();

  child->_parentOffset = getWorldOffset();
  child->_parentRotationOrigin = getWorldRotationOrigin();
  child->_parentOrientation = getWorldOrientation();

  child->updateChildrenOffsetSignal();
  child->updateChildrenRotationOriginSignal();
  child->updateChildrenOrientationSignal();

  child->_parent = this;

  _children.insert(child);
}

void Transform::detach() {
  if (!hasParent()) {
    throw std::logic_error("Error! Attempted to detach a transform when the transform had no parent!");
  }
  _offset = getWorldOffset();
  _rotationOrigin = getWorldRotationOrigin();
  _orientation = getWorldOrientation();

  _parent->_children.erase(this);
  _parent = NULL;

  updateChildrenOffsetSignal();
  updateChildrenRotationOriginSignal();
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

void Transform::updateChildrenOffsetSignal() {
  if (hasChildren()) {
    for (const auto& elem : _children) {
      elem->_parentOffset = getWorldOffset();
      elem->updateChildrenOffsetSignal();
    }
  }
}

void Transform::updateChildrenRotationOriginSignal() {
  if (hasChildren()) {
    for (const auto& elem : _children) {
      elem->_parentRotationOrigin = getWorldRotationOrigin();
      elem->updateChildrenRotationOriginSignal();
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