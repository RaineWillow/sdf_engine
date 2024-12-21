#pragma once
#ifndef Material_hpp
#define Material_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/writable.hpp"
#include <vector>
#include <string>

class Material : public Writable {
public:
  Material(std::string name);
  ~Material();

  std::string getName();

  void setAddress(Pixel address);
  Pixel getAddress();

  int getParamsSize();

  void updateParams(sf::Uint8 * &dataArray);

  void setAlbedo(sf::Glsl::Vec3 ambient);
  void setMetallic(float metallic);
  void setShine(float shine);
  void setReflectivity(float reflectivity);
  void setOpacity(float opacity);
  void setIOR(float IOR);

  sf::Glsl::Vec3 getAlbedo();
  float getMetallic();
  float getShine();
  float getReflectivity();
  float getOpacity();
  float getIOR();

private:

  std::string _name;
  sf::Uint8 * _writeData;
  Pixel _address;
  std::vector<Pixel> _params;
  int _paramsSize;

  sf::Glsl::Vec3 _albedo;
  float _metallic;
  float _shine;
  float _reflectivity;
  float _opacity;
  float _IOR;
};

#endif