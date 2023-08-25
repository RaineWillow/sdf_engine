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

class Shape {
public:

  Shape() {
    for (int i = 0; i < 48; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }
  }

  bool isModel() {
    return _isModel;
  }

  void setAddress(Pixel address) {
    _address = address;
  }

  Pixel getAddress() {
    return _address;
  }

  std::string getModelId() {
    return _modelId;
  }

  sf::Glsl::Vec3 getPos() {
    return _pos;
  }

  sf::Glsl::Vec4 getRot() {
    return _qRot;
  }

  double getBoundRadius() {
    return _boundRadius;
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

  void setModel(std::string modelId) {
    _isModel = true;
    _modelId = modelId;
    _params[0].toIBool(_isModel, _objectId);
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

  void setBoundRadius(double radius) {
    _boundRadius = radius+0.5;
    _params[9].toNum(_boundRadius);
    setBound(sf::Glsl::Vec3(_boundRadius, _boundRadius, _boundRadius));
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

  void updateParams(sf::Uint8 * dataArray) {
    for (int i = 0; i < _params.size(); i++) {
      _params[i].writeToArray(i, dataArray, 48);
    }
  }
protected:
  bool _destroyed = false;

  int _objectId;
  bool _isModel = false;
  std::string _modelId;


  sf::Glsl::Vec3 _pos;
  sf::Glsl::Vec4 _qRot;
  double _boundRadius = 0.0;
  sf::Glsl::Vec3 _box;
  sf::Glsl::Vec3 _aColor;
  sf::Glsl::Vec3 _dColor;
  sf::Glsl::Vec3 _sColor;
  double _shine;
  double _opacity;
  double _IOR;
  sf::Glsl::Vec4 _emit;
  float k = 0.5;

  void setObjectId(int id) {
    _objectId = id;
    _params[0].toIBool(_isModel, _objectId);
  }

  Pixel _address;
  std::vector<Pixel> _params;

  BVHTreeNode * _BVHTreeNode = NULL;
};


class Sphere : public Shape {
public:

  Sphere() : Shape() {
    this->setObjectId(0);
  }

  void setRadius(double radius) {
    _radius = radius;
    this->_params[28].toNum(_radius);
    this->setBoundRadius(_radius);
  }
private:
  double _radius;
};

#endif /* end of include guard: Shapes_hpp */
