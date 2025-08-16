#pragma once
#ifndef Shapes_hpp
#define Shapes_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <string>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "BVH_tree_node.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/writable.hpp"
#include "axis_aligned_bounding_box.hpp"
#include "transform.hpp"

class Shape : public Writable {
public:

  Shape() {
    _paramsSize = 52;
    for (int i = 0; i < _paramsSize; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }
    setK(0.0);
    _params[8].toNum(1.0);
    _writeData = new sf::Uint8[_paramsSize*4];
    _matName = "";
  }

  ~Shape() {
    delete[] _writeData;
  }

  bool isPrimitive() {
    return _isPrimitive;
  }

  void setAddress(Pixel address) {
    _address = address;
  }

  int getParamsSize() {
    return _paramsSize;
  }

  Pixel getAddress() {
    return _address;
  }

  float getK() {
    return _K;
  }

  sf::Glsl::Vec3 getPos() {;
    return (transform.getWorldOffset()+_center).toGlslVec3();
  }

  sf::Glsl::Vec3 getRotCenter() {
    return transform.getRotationOrigin().toGlslVec3();
  }

  sf::Glsl::Vec3 getCenter() {
    return _center.toGlslVec3();
  }

  sf::Glsl::Vec4 getRot() {
    return transform.getWorldOrientation().toGlslVec4();
  }

  sf::Glsl::Vec3 getBound() {
    return _box.toGlslVec3();
  }

  BVHTreeNode * getBVHTreeNode() {
    if (_BVHTreeNode == NULL) {
      throw std::logic_error("Cannot get the BVH Tree Node of a shape that has not had it's node set!");
    }
    return _BVHTreeNode;
  }

  void setPrimitive(bool isPrimitive) {
    _isPrimitive = isPrimitive;
    _params[0].toIBool(_isPrimitive, _objectId);
  }

  void setK(float K) {
    _K = K;
    _params[1].toNum(_K);
  }

  void setOffset(sf::Glsl::Vec3 offset) {
    _params[2].toNum(offset.x);
    _params[3].toNum(offset.y);
    _params[4].toNum(offset.z);
  }

  void setRot(sf::Glsl::Vec4 qRot) {
    _params[5].toNum(qRot.x);
    _params[6].toNum(qRot.y);
    _params[7].toNum(qRot.z);
    _params[8].toNum(qRot.w);
  }

  void setRotCenter(sf::Glsl::Vec3 cRot) {
    _params[9].toNum(cRot.x);
    _params[10].toNum(cRot.y);
    _params[11].toNum(cRot.z);
  }

  void setCenter(sf::Glsl::Vec3 center) {
    _params[12].toNum(center.x);
    _params[13].toNum(center.y);
    _params[14].toNum(center.z);
  }

  void setBound(sf::Glsl::Vec3 boundingBox) {
    _params[15].toNum(boundingBox.x);
    _params[16].toNum(boundingBox.y);
    _params[17].toNum(boundingBox.z);
  }

  void setScale(float scale) {
    _params[18].toNum(scale);
  }

  void setMaterial(Pixel matAddress, std::string matName) {
    _matName = matName;
    _params[25] = matAddress;
  }

  std::string getMaterial() {
    return _matName;
  }

  void destroy() {
    _destroyed = true;
  }

  bool destroyed() {
    return _destroyed;
  }

  void setBVHTreeNode(BVHTreeNode * node) {
    _BVHTreeNode = node;
  }

  bool inBVHTree() {
    return _BVHTreeNode != NULL;
  }

  virtual void updateMaxMinFromCenter()=0;

  void updateTransform() {
    setOffset(transform.getWorldOffset().toGlslVec3());
    setRotCenter(transform.getRotationOrigin().toGlslVec3());
    setRot(transform.getWorldOrientation().toGlslVec4());
    //setScale(transform.getWorldScale());
  }

  void updateBoundingBox() {
    updateMaxMinFromCenter();

    //calculate based on global transform list

    //calculate based on local transform

    AABBTransform aabb = transform.getTransformedAABB(Vector3::fromGlslVec3(_maxBound));

    sf::Glsl::Vec3 _transformedMin = aabb.min.toGlslVec3();
    sf::Glsl::Vec3 _transformedMax = aabb.max.toGlslVec3();
    AxisAlignedBoundingBox bound = fromMinMax(_transformedMin, _transformedMax);
    //std::cout << "X: " << bound.pos.x << " Y: " << bound.pos.y << " Z: " << bound.pos.z << std::endl;
    bound.bound.x += _K;
    bound.bound.y += _K;
    bound.bound.z += _K;
    setBound(bound.bound);
    setCenter(bound.pos);
    _center = Vector3::fromGlslVec3(bound.pos);
    _box = Vector3::fromGlslVec3(bound.bound);
  }

  void debugPrint() {
    //std::cout << "X: " << _center.x << " Y: " << _center.y << " Z: " << _center.z << std::endl;
  }

  void updateParams(sf::Uint8 * &dataArray) {
    updateTransform();
    updateBoundingBox();
    for (int i = 0; i < _paramsSize; i++) {
      _params[i].writeToArray(i, _writeData, _paramsSize);
    }
    dataArray = _writeData;
  }

  void printParams() {
    for (int i = 0; i < _paramsSize; i++) {
      std::cout << _params[i].fromNum() << std::endl;
    }
  }

  Transform transform;

protected:
  bool _destroyed = false;

  int _objectId;
  bool _isPrimitive = false;

  float _K = 0.0;

  std::string _matName;

  sf::Uint8 * _writeData;

  Vector3 _center;
  Vector3 _box;

  //computations for making sure the center of the shape is always translated back
  //to the origin
  sf::Glsl::Vec3 _maxBound;
  sf::Glsl::Vec3 _minBound;

  void setObjectId(int id) {
    _objectId = id;
    _params[0].toIBool(_isPrimitive, _objectId);
  }

  int _paramsSize;

  Pixel _address;
  std::vector<Pixel> _params;

  BVHTreeNode * _BVHTreeNode = NULL;
};


class Sphere : public Shape {
public:

  Sphere() : Shape() {
    this->setObjectId(0);
    this->setPrimitive(true);
  }

  void setRadius(float radius) {
    _radius = radius;
    this->_params[26].toNum(_radius);
  }

  void updateMaxMinFromCenter() {
    this->_maxBound = sf::Glsl::Vec3(_radius, _radius, _radius);
    this->_minBound = sf::Glsl::Vec3(-_radius, -_radius, -_radius);
  }
private:
  float _radius;
};

class Box : public Shape {
public:
  Box() : Shape() {
    this->setObjectId(1);
    this->setPrimitive(true);
  }

  void setSize(sf::Glsl::Vec3 size) {
    _size = size;
    this->_params[26].toNum(_size.x);
    this->_params[27].toNum(_size.y);
    this->_params[28].toNum(_size.z);
  }
 
  sf::Glsl::Vec3 getSize() {
    return _size;
  }

  void updateMaxMinFromCenter() {
    this->_maxBound = sf::Glsl::Vec3(_size.x, _size.y, _size.z);
    this->_minBound = sf::Glsl::Vec3(-_size.x, -_size.y, -_size.z);
  }

private:
  sf::Glsl::Vec3 _size;
};

#endif /* end of include guard: Shapes_hpp */
