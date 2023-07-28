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


struct Pixel {
  sf::Uint8 r = 0;
  sf::Uint8 g = 0;
  sf::Uint8 b = 0;
  sf::Uint8 a = 0;

  void toInt(int value) {
    if (value > 8388607 || value < -8388608) {
      throw std::invalid_argument("Int value: Received value outside of range: " + std::to_string(value));
    }

    int setVal = value;
    if (setVal < 0) {
      setVal += 16777216;
    }

    r = setVal/(256*256);
    setVal -= r*256*256;

    g = setVal/256;
    setVal -= g*256;

    b = setVal;
  }


  void toNum(double value) {
    if (value > 32767.0 || value < -32768.0) {
      throw std::invalid_argument("Num value: Received value outside of range: " + std::to_string(value));
    }

    int setVal = value;
    int setMantissa = 65535*std::abs(value-setVal);

    //handle int
    if (setVal < 0) {
      setVal += 65536;
    }

    r = setVal/256;
    setVal -= r*256;

    g = setVal;

    //handle mantissa
    b = setMantissa/256;
    setMantissa -= b*256;

    a = setMantissa;
  }

  void toHighFloat(double value) {
    if (value > 127 || value < -128) {
      throw std::invalid_argument("HighFloat value: Received value outside of range: " + std::to_string(value));
    }

    int setVal = value;
    int setMantissa = 16777215*std::abs(value-setVal);

    //handle int
    if (setVal < 0) {
      setVal += 256;
    }

    r = setVal;

    //handle mantissa

    g = setMantissa/(256*256);
    setMantissa -= g*256*256;

    b = setMantissa/256;
    setMantissa -= b*256;

    a = setMantissa;
  }

  void toDInt(int value1, int value2) {
    if (value1 > 32767.0 || value1 < -32768.0) {
      throw std::invalid_argument("DInt value 1: Received value outside of range: " + std::to_string(value1));
    }

    if (value2 > 32767.0 || value2 < -32768.0) {
      throw std::invalid_argument("DInt value 2: Received value outside of range: " + std::to_string(value2));
    }

    int setVal1 = value1;
    int setVal2 = value2;

    //handle int
    if (setVal1 < 0) {
      setVal1 += 65536;
    }

    r = setVal1/256;
    setVal1 -= r*256;

    g = setVal1;

    //handle int
    if (setVal2 < 0) {
      setVal2 += 65536;
    }

    b = setVal2/256;
    setVal2 -= b*256;

    a = setVal2;
  }

  void toIBool(bool truth, int value) {
    if (value > 8388607 || value < -8388608) {
      throw std::invalid_argument("IBool value: Received value outside of range: " + std::to_string(value));
    }

    int setVal = value;
    if (setVal < 0) {
      setVal += 16777216;
    }

    r = setVal/(256*256);
    setVal -= r*256*256;

    g = setVal/256;
    setVal -= g*256;

    b = setVal;

    a = (truth) ? 255 : 0;
  }

  void toBool(bool truth) {
    r = (truth) ? 255 : 0;
  }

  void toColor(sf::Glsl::Vec3 color) {
    r = color.x*255;
    g = color.y*255;
    b = color.z*255;
  }

  void apply(int shapeIndex, int paramIndex, sf::Uint8 * updateArray, int size) {
    int index = (shapeIndex*96+paramIndex)*4;

    if (index+3 > size) {
      throw std::invalid_argument("Applying pixel to update array, index was greater than the size of the array!");
    }

    updateArray[index] = r;
    updateArray[index+1] = g;
    updateArray[index+2] = b;
    updateArray[index+3] = a;
  }
};

class Shape {
public:

  Shape() {
    for (int i = 0; i < 96; i++) {
      Pixel defaultParam;
      _params.push_back(defaultParam);
    }
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

  double getRadius() {
    return _radius;
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

  void setModel(std::string modelId) {
    _isModel = true;
    _modelId = modelId;
    _params[0].toIBool(_isModel, _objectId);
  }

  void setPos(sf::Glsl::Vec3 pos) {
    _pos = pos;
    _requiresUpdate = true;
    _params[2].toNum(_pos.x);
    _params[3].toNum(_pos.y);
    _params[4].toNum(_pos.z);
  }

  void setQRot(sf::Glsl::Vec4 qRot) {
    _qRot = qRot;
    _requiresUpdate = true;
    _params[5].toHighFloat(_qRot.x);
    _params[6].toHighFloat(_qRot.y);
    _params[7].toHighFloat(_qRot.z);
    _params[8].toHighFloat(_qRot.w);
  }

  void setRadius(double radius) {
    _radius = radius;
    _requiresUpdate = true;
    _params[9].toNum(_radius);
  }

  void setBox(sf::Glsl::Vec3 boundingBox) {
    _box = boundingBox;
    _requiresUpdate = true;
    _params[10].toNum(_box.x);
    _params[11].toNum(_box.y);
    _params[12].toNum(_box.z);
  }

  void setAmbient(sf::Glsl::Vec3 ambient) {
    _aColor = ambient;
    _requiresUpdate = true;
    _params[13].toColor(_aColor);
  }

  void setDiffuse(sf::Glsl::Vec3 diffuse) {
    _dColor = diffuse;
    _requiresUpdate = true;
    _params[14].toColor(_aColor);
  }

  void setSpecular(sf::Glsl::Vec3 specular) {
    _sColor = specular;
    _requiresUpdate = true;
    _params[15].toColor(_aColor);
  }

  void setShine(double shine) {
    _shine = shine;
    _requiresUpdate = true;
    _params[16].toHighFloat(_shine);
  }

  void setOpacity(double opacity) {
    _opacity = opacity;
    _requiresUpdate = true;
    _params[17].toHighFloat(_opacity);
  }

  void setIOR(double IOR) {
    _IOR = IOR;
    _requiresUpdate = true;
    _params[18].toHighFloat(_opacity);
  }

  void destroy() {
    _destroyed = true;
  }

  bool destroyed() {
    return _destroyed;
  }

  bool requiresUpdate() {
    if (_requiresUpdate) {
      _requiresUpdate = false;
      return true;
    } else {
      return false;
    }
  }

  void apply(int shapeIndex, sf::Uint8 * updateArray, int size, int modelIndex, int numItems) {
    _params[1].toDInt(modelIndex, numItems);

    for (int i = 0; i < _params.size(); i++) {
      _params[i].apply(shapeIndex, i, updateArray, size);
    }
  }
protected:
  bool _destroyed = false;
  bool _requiresUpdate = false;

  int _objectId;
  bool _isModel = false;
  std::string _modelId;


  sf::Glsl::Vec3 _pos;
  sf::Glsl::Vec4 _qRot;
  double _radius = 0.0;
  sf::Glsl::Vec3 _box;
  sf::Glsl::Vec3 _aColor;
  sf::Glsl::Vec3 _dColor;
  sf::Glsl::Vec3 _sColor;
  double _shine;
  double _opacity;
  double _IOR;

  void setObjectId(int id) {
    _objectId = id;
    _params[0].toIBool(_isModel, _objectId);
  }

  std::vector<Pixel> _params;
};


class Sphere : public Shape {
public:

  Sphere() : Shape() {
    this->setObjectId(0);
  }
private:

};

#endif /* end of include guard: Shapes_hpp */
