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
#include "memory/memory_pixel.hpp"
#include "BVH_tree_node.hpp"

class Shape {
public:

  Shape() {
    for (int i = 0; i < 48; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }

    _params[0].toInt(1);
  }

  bool isModel() {
    return _isModel;
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

  sf::Glsl::Vec3 getBox() {
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

  BVHTreeNode<Shape> * getBVHTreeNode() {
    if (_BVHTreeNode == NULL) {
      throw std::logic_error("Cannot get the BVH Tree Node of a shape that has not had it's node set!");
    }
    return _BVHTreeNode;
  }

  void setModel(std::string modelId) {
    _isModel = true;
    _modelId = modelId;
    _params[1].toIBool(_isModel, _objectId);
  }

  void setPos(sf::Glsl::Vec3 pos) {
    _pos = pos;
    _params[3].toNum(_pos.x);
    _params[4].toNum(_pos.y);
    _params[5].toNum(_pos.z);
  }

  void setQRot(sf::Glsl::Vec4 qRot) {
    _qRot = qRot;
    _params[6].toHighFloat(_qRot.x);
    _params[7].toHighFloat(_qRot.y);
    _params[8].toHighFloat(_qRot.z);
    _params[9].toHighFloat(_qRot.w);
  }

  void setBoundRadius(double radius) {
    _boundRadius = radius;
    _params[10].toNum(_boundRadius);
    setBox(sf::Glsl::Vec3(_boundRadius, _boundRadius, _boundRadius));
  }

  void setBox(sf::Glsl::Vec3 boundingBox) {
    _box = boundingBox;
    _params[11].toNum(_box.x);
    _params[12].toNum(_box.y);
    _params[13].toNum(_box.z);
  }

  void setAmbient(sf::Glsl::Vec3 ambient) {
    _aColor = ambient;
    _params[14].toColor(_aColor);
  }

  void setDiffuse(sf::Glsl::Vec3 diffuse) {
    _dColor = diffuse;
    _params[15].toColor(_aColor);
  }

  void setSpecular(sf::Glsl::Vec3 specular) {
    _sColor = specular;
    _params[16].toColor(_aColor);
  }

  void setShine(double shine) {
    _shine = shine;
    _params[17].toHighFloat(_shine);
  }

  void setOpacity(double opacity) {
    _opacity = opacity;
    _params[18].toHighFloat(_opacity);
  }

  void setIOR(double IOR) {
    _IOR = IOR;
    _params[19].toHighFloat(_opacity);
  }

  void setEmit(sf::Glsl::Vec4 emit) {
    _emit = emit;
    _params[20].to4Color(emit);
  }

  void setBVHTreeNode(BVHTreeNode<Shape> * node) {
    _BVHTreeNode = node;
  }

  void destroy() {
    _destroyed = true;
  }

  bool destroyed() {
    return _destroyed;
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

  void setObjectId(int id) {
    _objectId = id;
    _params[1].toIBool(_isModel, _objectId);
  }

  std::vector<Pixel> _params;

  BVHTreeNode<Shape> * _BVHTreeNode = NULL;
};


class Sphere : public Shape {
public:

  Sphere() : Shape() {
    this->setObjectId(0);
  }
private:

};

#endif /* end of include guard: Shapes_hpp */
