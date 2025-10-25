#pragma once
#ifndef Axis_Aligned_Bounding_Box_hpp
#define Axis_Aligned_Bounding_Box_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <algorithm>
#include <stdexcept>
#include "../utils/fuzzy_float_compares.hpp"

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

  float volume() {
    return 8.0*(float)bound.x*(float)bound.y*(float)bound.z;
  }

  bool operator==(const AxisAlignedBoundingBox& rhs) const {
    return (fuzzyEquals(pos.x, rhs.pos.x) && fuzzyEquals(pos.y, rhs.pos.y) && fuzzyEquals(pos.z, rhs.pos.z)) 
      && (fuzzyEquals(bound.x, rhs.bound.x) && fuzzyEquals(bound.y, rhs.bound.y) && fuzzyEquals(bound.z, rhs.bound.z));
  }
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

  AxisAlignedBoundingBox retData;
  retData.bound = sf::Glsl::Vec3((maxVals.x-minVals.x)/2.0, (maxVals.y-minVals.y)/2.0, (maxVals.z-minVals.z)/2.0);
  retData.pos = sf::Glsl::Vec3((maxVals.x+minVals.x)/2.0, (maxVals.y+minVals.y)/2.0, (maxVals.z+minVals.z)/2.0);

  return retData;
}

inline bool pointInsideAABB(AxisAlignedBoundingBox container, sf::Glsl::Vec3 point) {
  sf::Glsl::Vec3 minVals(container.pos.x-container.bound.x, container.pos.y-container.bound.y, container.pos.z-container.bound.z);
  sf::Glsl::Vec3 maxVals(container.pos.x+container.bound.x, container.pos.y+container.bound.y, container.pos.z+container.bound.z);
  return !(fuzzyLT(point.x, minVals.x) || fuzzyGT(point.x, maxVals.x) || fuzzyLT(point.y, minVals.y) || fuzzyGT(point.y, maxVals.y) || fuzzyLT(point.z, minVals.z) || fuzzyGT(point.z, maxVals.z));
}

inline int getBoxOctant(AxisAlignedBoundingBox container, sf::Glsl::Vec3 point) {
  sf::Glsl::Vec3 minVals(container.pos.x-container.bound.x, container.pos.y-container.bound.y, container.pos.z-container.bound.z);
  sf::Glsl::Vec3 maxVals(container.pos.x+container.bound.x, container.pos.y+container.bound.y, container.pos.z+container.bound.z);

  if (fuzzyLT(point.x, minVals.x) || fuzzyGT(point.x, maxVals.x) || fuzzyLT(point.y, minVals.y) || fuzzyGT(point.y, maxVals.y) || fuzzyLT(point.z, minVals.z) || fuzzyGT(point.z, maxVals.z)) {
    throw std::invalid_argument("Error, point is outside of the bounding container!");
  }

  int data = 0;

  if (fuzzyGTE(point.x, container.pos.x)) {
    data = 1;
  }

  if (fuzzyGTE(point.y, container.pos.y)) {
    data+=2;
  }

  if (fuzzyGTE(point.z, container.pos.z)) {
    data+=4;
  }

  return data;
}

inline AxisAlignedBoundingBox getOctantBox(AxisAlignedBoundingBox container, sf::Glsl::Vec3 point) {
  sf::Glsl::Vec3 minVals(container.pos.x-container.bound.x, container.pos.y-container.bound.y, container.pos.z-container.bound.z);
  sf::Glsl::Vec3 maxVals(container.pos.x+container.bound.x, container.pos.y+container.bound.y, container.pos.z+container.bound.z);

  if (fuzzyLT(point.x, minVals.x) || fuzzyGT(point.x, maxVals.x) || fuzzyLT(point.y, minVals.y) || fuzzyGT(point.y, maxVals.y) || fuzzyLT(point.z, minVals.z) || fuzzyGT(point.z, maxVals.z)) {
    throw std::invalid_argument("Error, point is outside of the bounding container!");
  }


  sf::Glsl::Vec3 retMin = minVals;
  sf::Glsl::Vec3 retMax = container.pos;

  if (fuzzyGTE(point.x, container.pos.x)) {
    retMin.x = container.pos.x;
    retMax.x = maxVals.x;
  }

  if (fuzzyGTE(point.y, container.pos.y)) {
    retMin.y = container.pos.y;
    retMax.y = maxVals.y;
  }

  if (fuzzyGTE(point.z, container.pos.z)) {
    retMin.z = container.pos.z;
    retMax.z = maxVals.z;
  }
  return fromMinMax(retMin, retMax);
}

inline bool boundedByBox(AxisAlignedBoundingBox container, AxisAlignedBoundingBox tester) {
  sf::Glsl::Vec3 minValsContainer(container.pos.x-container.bound.x, container.pos.y-container.bound.y, container.pos.z-container.bound.z);
  sf::Glsl::Vec3 maxValsContainer(container.pos.x+container.bound.x, container.pos.y+container.bound.y, container.pos.z+container.bound.z);
  sf::Glsl::Vec3 minValsTester(tester.pos.x-tester.bound.x, tester.pos.y-tester.bound.y, tester.pos.z-tester.bound.z);
  sf::Glsl::Vec3 maxValsTester(tester.pos.x+tester.bound.x, tester.pos.y+tester.bound.y, tester.pos.z+tester.bound.z);

  return (fuzzyGTE(minValsTester.x, minValsContainer.x) && fuzzyGTE(minValsTester.y, minValsContainer.y) && fuzzyGTE(minValsTester.z, minValsContainer.z)) 
    && (fuzzyLTE(maxValsTester.x, maxValsContainer.x) && fuzzyLTE(maxValsTester.y, maxValsContainer.y) && fuzzyLTE(maxValsTester.z, maxValsContainer.z));
}

inline bool isBoxIntersecting(AxisAlignedBoundingBox box1, AxisAlignedBoundingBox box2) {
  sf::Glsl::Vec3 minValsBox1(box1.pos.x-box1.bound.x, box1.pos.y-box1.bound.y, box1.pos.z-box1.bound.z);
  sf::Glsl::Vec3 maxValsBox1(box1.pos.x+box1.bound.x, box1.pos.y+box1.bound.y, box1.pos.z+box1.bound.z);
  sf::Glsl::Vec3 minValsBox2(box2.pos.x-box2.bound.x, box2.pos.y-box2.bound.y, box2.pos.z-box2.bound.z);
  sf::Glsl::Vec3 maxValsBox2(box2.pos.x+box2.bound.x, box2.pos.y+box2.bound.y, box2.pos.z+box2.bound.z);


  return (fuzzyGTE(maxValsBox1.x, minValsBox2.x) && fuzzyLTE(minValsBox1.x, maxValsBox2.x))
    && (fuzzyGTE(maxValsBox1.y, minValsBox2.y) && fuzzyLTE(minValsBox1.y, maxValsBox2.y))
    && (fuzzyGTE(maxValsBox1.z, minValsBox2.z) && fuzzyLTE(minValsBox1.z, maxValsBox2.z));
}


inline AxisAlignedBoundingBox getIntersectingBox(AxisAlignedBoundingBox box1, AxisAlignedBoundingBox box2) {
  if (!isBoxIntersecting(box1, box2)) {
    return {sf::Glsl::Vec3(0, 0, 0), sf::Glsl::Vec3(0, 0, 0)};
  }

  sf::Glsl::Vec3 minValsBox1(box1.pos.x-box1.bound.x, box1.pos.y-box1.bound.y, box1.pos.z-box1.bound.z);
  sf::Glsl::Vec3 maxValsBox1(box1.pos.x+box1.bound.x, box1.pos.y+box1.bound.y, box1.pos.z+box1.bound.z);
  sf::Glsl::Vec3 minValsBox2(box2.pos.x-box2.bound.x, box2.pos.y-box2.bound.y, box2.pos.z-box2.bound.z);
  sf::Glsl::Vec3 maxValsBox2(box2.pos.x+box2.bound.x, box2.pos.y+box2.bound.y, box2.pos.z+box2.bound.z);


  sf::Glsl::Vec3 retMin(std::max(minValsBox1.x, minValsBox2.x)
    , std::max(minValsBox1.y, minValsBox2.y)
    , std::max(minValsBox1.z, minValsBox2.z));
  sf::Glsl::Vec3 retMax(std::min(maxValsBox1.x, maxValsBox2.x)
    , std::min(maxValsBox1.y, maxValsBox2.y)
    , std::min(maxValsBox1.z, maxValsBox2.z));
  
  return fromMinMax(retMin, retMax);
}


/*
  Function which determines if some container AABB should subdivide into smaller 
  octants based on how much it overlaps octants other than it's own octant. 
  Subtracts the volume which overlaps the octant of the AABB and the tester
  from each other, then divides by the overall volume of the container and returns
  true if less than the threshold
*/
inline bool parentShouldSubdivide(AxisAlignedBoundingBox container, AxisAlignedBoundingBox tester, float threshold) {
  if (!boundedByBox(container, tester)) {
    throw std::invalid_argument("Error, tried to subdivide test a tester box which doesn't fit in it's container!");
  }

  AxisAlignedBoundingBox octantBox = getOctantBox(container, tester.pos);
  AxisAlignedBoundingBox intersectingOctantBox = getIntersectingBox(tester, octantBox);

  float containerVolume = container.volume();
  float testVolume = tester.volume()-intersectingOctantBox.volume();

  return fuzzyLT(testVolume/octantBox.volume(), threshold);
}


#endif