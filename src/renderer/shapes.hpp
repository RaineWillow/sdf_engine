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

class Shape : public Writable {
public:

  Shape() {
    _paramsSize = 52;
    for (int i = 0; i < _paramsSize; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }
    setK(0);

    _writeData = new sf::Uint8[_paramsSize*4];
    _matName = "";
  }

  ~Shape() {
    delete[] _writeData;
  }

  bool isPrimative() {
    return _isPrimative;
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

  sf::Glsl::Vec3 getPos() {
    sf::Glsl::Vec3 pos(_offset.x+_center.x, _offset.y+_center.y, _offset.z+_center.z);
    return pos;
  }

  sf::Glsl::Vec3 getRotCenter() {
    return _cRot;
  }

  sf::Glsl::Vec3 getCenter() {
    return _center;
  }

  sf::Glsl::Vec4 getRot() {
    return _qRot;
  }

  sf::Glsl::Vec3 getBound() {
    return _box;
  }

  BVHTreeNode * getBVHTreeNode() {
    if (_BVHTreeNode == NULL) {
      throw std::logic_error("Cannot get the BVH Tree Node of a shape that has not had it's node set!");
    }
    return _BVHTreeNode;
  }

  void setPrimative(bool isPrimative) {
    _isPrimative = isPrimative;
    _params[0].toIBool(_isPrimative, _objectId);
  }

  void setK(float K) {
    _K = K;
    _params[1].toNum(_K);
  }

  void setOffset(sf::Glsl::Vec3 offset) {
    _offset = offset;
    _params[2].toNum(_offset.x);
    _params[3].toNum(_offset.y);
    _params[4].toNum(_offset.z);
  }

  void setRot(sf::Glsl::Vec4 qRot) {
    _qRot = qRot;
    _params[5].toNum(_qRot.x);
    _params[6].toNum(_qRot.y);
    _params[7].toNum(_qRot.z);
    _params[8].toNum(_qRot.w);
  }

  void setRotCenter(sf::Glsl::Vec3 cRot) {
    _cRot = cRot;
    _params[9].toNum(_cRot.x);
    _params[10].toNum(_cRot.y);
    _params[11].toNum(_cRot.z);
  }

  void setCenter(sf::Glsl::Vec3 center) {
    _center = center;
    _params[12].toNum(_center.x);
    _params[13].toNum(_center.y);
    _params[14].toNum(_center.z);
  }

  void setBound(sf::Glsl::Vec3 boundingBox) {
    _box = boundingBox;
    _params[15].toNum(_box.x);
    _params[16].toNum(_box.y);
    _params[17].toNum(_box.z);
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

  void updateBoundingBox() {
    updateMaxMinFromCenter();

    //calculate based on global transform list

    //calculate based on local transform

    sf::Glsl::Vec3 _transformedMin = _minBound;
    sf::Glsl::Vec3 _transformedMax = _maxBound;
    AxisAlignedBoundingBox bound = fromMinMax(_transformedMin, _transformedMax);
    bound.bound.x += _K;
    bound.bound.y += _K;
    bound.bound.z += _K;
    setBound(bound.bound);
    setCenter(bound.pos);
  }

  void updateParams(sf::Uint8 * &dataArray) {
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
protected:
  bool _destroyed = false;

  int _objectId;
  bool _isPrimative = false;

  float _K = 0.8;
  sf::Glsl::Vec3 _offset;
  sf::Glsl::Vec4 _qRot;
  sf::Glsl::Vec3 _cRot;
  sf::Glsl::Vec3 _center;
  sf::Glsl::Vec3 _box;

  std::string _matName;

  sf::Uint8 * _writeData;

  //computations for making sure the center of the shape is always translated back
  //to the origin
  sf::Glsl::Vec3 _maxBound;
  sf::Glsl::Vec3 _minBound;

  void setObjectId(int id) {
    _objectId = id;
    _params[0].toIBool(_isPrimative, _objectId);
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
    this->setPrimative(true);
  }

  void setRadius(double radius) {
    _radius = radius;
    this->_params[26].toNum(_radius);
    this->updateBoundingBox();
  }

  void updateMaxMinFromCenter() {
    this->_maxBound = sf::Glsl::Vec3(_radius, _radius, _radius);
    this->_minBound = sf::Glsl::Vec3(-_radius, -_radius, -_radius);
  }
private:
  double _radius;
};

class Box : public Shape {
public:
  Box() : Shape() {
    this->setObjectId(1);
    this->setPrimative(true);
  }

  void setSize(sf::Glsl::Vec3 size) {
    _size = size;
    this->_params[26].toNum(_size.x);
    this->_params[27].toNum(_size.y);
    this->_params[28].toNum(_size.z);
    this->updateBoundingBox();
  }

  void updateMaxMinFromCenter() {
    this->_maxBound = sf::Glsl::Vec3(_size.x, _size.y, _size.z);
    this->_minBound = sf::Glsl::Vec3(-_size.x, -_size.y, -_size.z);
  }

private:
  sf::Glsl::Vec3 _size;
};

#endif /* end of include guard: Shapes_hpp */
