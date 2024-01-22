#pragma once
#ifndef Axis_Aligned_Bounding_Box_hpp
#define Axis_Aligned_Bounding_Box_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"

struct AxisAlignedBoundingBox {
  AxisAlignedBoundingBox() {
    pos = sf::Glsl::Vec3(0, 0, 0);
    bound = sf::Glsl::Vec3(0, 0, 0);
  }
  AxisAlignedBoundingBox(sf::Glsl::Vec3 p, sf::Glsl::Vec3 b) {
    pos=p;
    bound=b;
  }
  
  sf::Glsl::Vec3 pos;
  sf::Glsl::Vec3 bound;
};

inline AxisAlignedBoundingBox fromMinMax(sf::Glsl::Vec3 minVals, sf::Glsl::Vec3 maxVals) {
  AxisAlignedBoundingBox retData;
  retData.bound = sf::Glsl::Vec3((maxVals.x-minVals.x)/2.0, (maxVals.y-minVals.y)/2.0, (maxVals.z-minVals.z)/2.0);
  retData.pos = sf::Glsl::Vec3((maxVals.x+minVals.x)/2.0, (maxVals.y+minVals.y)/2.0, (maxVals.z+minVals.z)/2.0);

  return retData;
}

inline AxisAlignedBoundingBox addToBox(AxisAlignedBoundingBox container, AxisAlignedBoundingBox item) {
  sf::Glsl::Vec3 minVals(std::min(container.pos.x-container.bound.x, item.pos.x-item.bound.x), std::min(container.pos.y-container.bound.y, item.pos.y-item.bound.y), std::min(container.pos.z-container.bound.z, item.pos.z-item.bound.z));
  sf::Glsl::Vec3 maxVals(std::max(container.pos.x+container.bound.x, item.pos.x+item.bound.x), std::max(container.pos.y+container.bound.y, item.pos.y+item.bound.y), std::max(container.pos.z+container.bound.z, item.pos.z+item.bound.z));
  //std::cout << "MinX: " << minVals.x << " MinY: " << minVals.y << std::endl;
  //std::cout << "MaxX: " << maxVals.x << " MaxY: " << maxVals.y << std::endl;

  AxisAlignedBoundingBox retData;
  retData.bound = sf::Glsl::Vec3((maxVals.x-minVals.x)/2.0, (maxVals.y-minVals.y)/2.0, (maxVals.z-minVals.z)/2.0);
  retData.pos = sf::Glsl::Vec3((maxVals.x+minVals.x)/2.0, (maxVals.y+minVals.y)/2.0, (maxVals.z+minVals.z)/2.0);

  return retData;
}

inline int getBoxOctant(AxisAlignedBoundingBox container, sf::Glsl::Vec3 point) {
  sf::Glsl::Vec3 minVals(container.pos.x-container.bound.x, container.pos.y-container.bound.y, container.pos.z-container.bound.z);
  sf::Glsl::Vec3 maxVals(container.pos.x+container.bound.x, container.pos.y+container.bound.y, container.pos.z+container.bound.z);

  if (point.x < minVals.x || point.x > maxVals.x || point.y < minVals.y || point.y > maxVals.y || point.z < minVals.z || point.z > maxVals.z) {
    throw std::invalid_argument("Error, point is outside of the bounding container!");
  }

  int data = 0;

  if (point.x >= minVals.x+(container.bound.x)) {
    data = 1;
  }

  if (point.y >= minVals.y+(container.bound.y)) {
    data+=2;
  }

  if (point.z >= minVals.z+(container.bound.z)) {
    data+=4;
  }

  return data;
}

#endif