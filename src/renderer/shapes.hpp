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
#include "axis_aligned_bounding_box.hpp"

class Shape {
public:

  Shape() {
    for (int i = 0; i < 52; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }
    setK(0);
    _center = sf::Glsl::Vec3(0, 0, 0);
  }

  bool isPrimative() {
    return _isPrimative;
  }

  void setAddress(Pixel address) {
    _address = address;
  }

  Pixel getAddress() {
    return _address;
  }

  float getK() {
    return _K;
  }

  sf::Glsl::Vec3 getPos() {
    return _pos;
  }

  sf::Glsl::Vec4 getRot() {
    return _qRot;
  }

  sf::Glsl::Vec3 getBound() {
    return _box;
  }

  sf::Glsl::Vec3 getAmbient() {
    return _aColor;
  }

  sf::Glsl::Vec3 getDiffuse() {
    return _dColor;
  }

  sf::Glsl::Vec3 getSpecular() {
    return _sColor;
  }

  double getShine() {
    return _shine;
  }

  double getOpacity() {
    return _opacity;
  }

  double getIOR() {
    return _IOR;
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
    _params[1].toHighFloat(_K);
  }

  void setPos(sf::Glsl::Vec3 pos) {
    _pos = pos;
    _params[2].toNum(_pos.x);
    _params[3].toNum(_pos.y);
    _params[4].toNum(_pos.z);
  }

  void setQRot(sf::Glsl::Vec4 qRot) {
    _qRot = qRot;
    _params[5].toHighFloat(_qRot.x);
    _params[6].toHighFloat(_qRot.y);
    _params[7].toHighFloat(_qRot.z);
    _params[8].toHighFloat(_qRot.w);
  }

  void setBound(sf::Glsl::Vec3 boundingBox) {
    _box = boundingBox;
    _params[10].toNum(_box.x);
    _params[11].toNum(_box.y);
    _params[12].toNum(_box.z);
  }

  void setAmbient(sf::Glsl::Vec3 ambient) {
    _aColor = ambient;
    _params[13].toColor(_aColor);
  }

  void setDiffuse(sf::Glsl::Vec3 diffuse) {
    _dColor = diffuse;
    _params[14].toColor(_aColor);
  }

  void setSpecular(sf::Glsl::Vec3 specular) {
    _sColor = specular;
    _params[15].toColor(_aColor);
  }

  void setShine(double shine) {
    _shine = shine;
    _params[16].toHighFloat(_shine);
  }

  void setOpacity(double opacity) {
    _opacity = opacity;
    _params[17].toHighFloat(_opacity);
  }

  void setIOR(double IOR) {
    _IOR = IOR;
    _params[18].toHighFloat(_opacity);
  }

  void setEmit(sf::Glsl::Vec4 emit) {
    _emit = emit;
    _params[19].to4Color(emit);
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

    //calculate based on transform
    AxisAlignedBoundingBox bound = fromMinMax(_minBound, _maxBound);
    bound.bound.x += _K;
    bound.bound.y += _K;
    bound.bound.z += _K;
    setBound(bound.bound);
    setPos(sf::Glsl::Vec3(_pos.x+bound.pos.x, _pos.y+bound.pos.y, _pos.z+bound.pos.z));
  }

  void updateParams(sf::Uint8 * dataArray) {
    for (int i = 0; i < _params.size(); i++) {
      _params[i].writeToArray(i, dataArray, 52);
    }
  }
protected:
  bool _destroyed = false;

  int _objectId;
  bool _isPrimative = false;

  float _K; 
  sf::Glsl::Vec3 _pos;
  sf::Glsl::Vec4 _qRot;
  sf::Glsl::Vec3 _box;
  sf::Glsl::Vec3 _aColor;
  sf::Glsl::Vec3 _dColor;
  sf::Glsl::Vec3 _sColor;

  double _shine;
  double _opacity;
  double _IOR;
  sf::Glsl::Vec4 _emit;
  float k = 0.5;

  //computations for making sure the center of the shape is always translated back
  //to the origin
  sf::Glsl::Vec3 _center;
  sf::Glsl::Vec3 _maxBound;
  sf::Glsl::Vec3 _minBound;

  void setObjectId(int id) {
    _objectId = id;
    _params[0].toIBool(_isPrimative, _objectId);
  }

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
    this->_params[35].toNum(_radius);
    this->updateBoundingBox();
  }

  void updateMaxMinFromCenter() {
    this->_maxBound = sf::Glsl::Vec3(_radius, _radius, _radius);
    this->_minBound = sf::Glsl::Vec3(-_radius, -_radius, -_radius);
  }
private:
  double _radius;
};

#endif /* end of include guard: Shapes_hpp */
