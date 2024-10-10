#pragma once
#ifndef Writable_hpp
#define Writable_hpp
#include "memory_pixel.hpp"
#include "SFML/Graphics.hpp"

class Writable {
public:
  virtual void setAddress(Pixel address) = 0;
  virtual Pixel getAddress() = 0;

  virtual int getParamsSize() = 0;

  virtual void updateParams(sf::Uint8 * &dataArray) = 0;
};

#endif