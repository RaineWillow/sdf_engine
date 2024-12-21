#pragma once
#ifndef Memory_Pixel
#define Memory_Pixel
#include <stdexcept>
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <string>
#include <stdint.h>
#include <cstring>

struct Pixel {
  sf::Uint8 r = 0;
  sf::Uint8 g = 0;
  sf::Uint8 b = 0;
  sf::Uint8 a = 0;

  void toInt(int32_t value) {
    r = value >> 24;
    value = value & 0x00ffffff;
    g = value >> 16;
    value = value & 0x0000ffff;
    b = value >> 8;
    value = value & 0x000000ff;
    a = value;
  }


  void toNum(float value) {
    int32_t number;
    memcpy(&number, &value, sizeof(value));
    toInt(number);
  }

  void toDInt(int16_t value1, int16_t value2) {
    r = value1 >> 8;
    value1 = value1 & 0x00ff;
    g = value1;

    b= value2 >> 8;
    value2 = value2 & 0x00ff;
    a = value2;
  }

  void toIBool(bool truth, int32_t value) {
    if (value > 8388607 || value < -8388608) {
      throw std::invalid_argument("IBool value: Received value outside of range: " + std::to_string(value));
    }

    r = value >> 16;
    value = value & 0x0000ffff;
    g = value >> 8;
    value = value & 0x000000ff;
    b = value;

    a = (truth) ? 255 : 0;
  }

  void toBool(bool truth) {
    r = (truth) ? 255 : 0;
  }

  void toColor(sf::Glsl::Vec3 color) {
    r = color.x*255;
    g = color.y*255;
    b = color.z*255;
    a = 255;
  }

  void to4Color(sf::Glsl::Vec4 color) {
    r = color.x*255;
    g = color.y*255;
    b = color.z*255;
    a = color.w*255;
  }

  void to4Dat(uint8_t dat0, uint8_t dat1, uint8_t dat2, uint8_t dat3) {
    r = dat0;
    g = dat1;
    b = dat2;
    a = dat3;
  }

  void toPointer(uint32_t pointerIndex, uint8_t type) {
    if (pointerIndex > 16777215) {
      throw std::invalid_argument("Error, attempted to write pointer " + std::to_string(pointerIndex) + ", which is out of range.");
    }
    if (type > 255) {
      throw std::invalid_argument("Error, pointer type " + std::to_string(type) + " too high!");
    }

    r = pointerIndex >> 16;
    pointerIndex = pointerIndex & 0x0000ffff;
    g = pointerIndex >> 8;
    pointerIndex = pointerIndex & 0x000000ff;
    b = pointerIndex;

    a = type;
  }

  uint32_t pointerIndex() {
    return ((r << 16) + (g << 8) + b);
  }

  bool isNullPointer() {
    return a==0;
  }

  float fromNum() {
    uint32_t intNum = (r << 24) + (g << 16) + (b << 8) + a;
    float data;

    memcpy(&data, &intNum, sizeof(data));
    return data;
  }

  sf::Glsl::Vec4 asVec4() {
    return sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0);
  }

  void writeToArray(int paramIndex, sf::Uint8 * updateArray, int itemSize) {

    if (paramIndex >= itemSize) {
      throw std::invalid_argument("Error, attempted to write pixel to out of bounds area.");
    }

    int index = paramIndex*4;    

    updateArray[index] = r;
    updateArray[index+1] = g;
    updateArray[index+2] = b;
    updateArray[index+3] = a;
  }
};

#endif