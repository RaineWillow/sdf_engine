#include "light.hpp"

Light::Light() {
  _paramsSize = 52;

  for (int i = 0; i < _paramsSize; i++) {
    Pixel defaultParam;
    _params.push_back(defaultParam);
  }

  _writeData = new sf::Uint8[_paramsSize*4];
}

Light::~Light() {
  delete[] _writeData;
}

void Light::setAddress(Pixel address) {
  _address = address;
}

Pixel Light::getAddress() {
  return _address;
}

int Light::getParamsSize() {
  return _paramsSize;
}

void Light::updateParams(sf::Uint8 * &dataArray) {
  for (int i = 0; i < _paramsSize; i++) {
    _params[i].writeToArray(i, _writeData, _paramsSize);
  }

  dataArray = _writeData;
}

void Light::setPosition(sf::Glsl::Vec3 position) {
  _position = position;
  _params[0].toNum(position.x);
  _params[1].toNum(position.y);
  _params[2].toNum(position.z);
}

void Light::setColor(sf::Glsl::Vec3 color) {
  _color = color;
  _params[3].toColor(_color);
}

void Light::setIntensity(float intensity) {
  _intensity = intensity;
  _params[4].toNum(_intensity);
}

void Light::setAttenuation(sf::Glsl::Vec3 attenuation) {
  _attenuation = attenuation;
  _params[5].toNum(attenuation.x);
  _params[6].toNum(attenuation.y);
  _params[7].toNum(attenuation.z);
}

void Light::setNext(Pixel address) {
  _next = address;
  _params[8] = address;
}

sf::Glsl::Vec3 Light::getPosition() {
  return _position;
}

sf::Glsl::Vec3 Light::getColor() {
  return _color;
}

float Light::getIntensity() {
  return _intensity;
}

sf::Glsl::Vec3 Light::getAttenuation() {
  return _attenuation;
}

Pixel Light::getNext() {
  return _next;
}
