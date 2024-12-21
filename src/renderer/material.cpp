#include "material.hpp"

Material::Material(std::string name) {
  _name = name;
  _paramsSize = 52;

  for (int i = 0; i < _paramsSize; i++) {
    Pixel defaultParam;
    _params.push_back(defaultParam);
  }

  _writeData = new sf::Uint8[_paramsSize*4];
}

Material::~Material() {
  delete[] _writeData;
}

std::string Material::getName() {
  return _name;
}

void Material::setAddress(Pixel address) {
  _address = address;
}

Pixel Material::getAddress() {
  return _address;
}

int Material::getParamsSize() {
  return _paramsSize;
}

void Material::updateParams(sf::Uint8 * &dataArray) {
  for (int i = 0; i < _paramsSize; i++) {
    _params[i].writeToArray(i, _writeData, _paramsSize);
  }

  dataArray = _writeData;
}

void Material::setAlbedo(sf::Glsl::Vec3 albedo) {
  _albedo = albedo;
  _params[0].toColor(_albedo);
}

void Material::setMetallic(float metallic) {
  _metallic = metallic;
  _params[1].toNum(_metallic);
}

void Material::setShine(float shine) {
  _shine = shine;
  _params[2].toNum(_shine);
}

void Material::setReflectivity(float reflectivity) {
  _reflectivity = reflectivity;
  _params[3].toNum(_reflectivity);
}

void Material::setOpacity(float opacity) {
  _opacity = opacity;
  _params[4].toNum(_opacity);
}

void Material::setIOR(float IOR) {
  _IOR = IOR;
  _params[5].toNum(_IOR);
}