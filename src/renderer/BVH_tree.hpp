#pragma once
#ifndef BVH_Tree_Hpp
#define BVH_Tree_Hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <map>
#include "memory/shader_memory_buffer.hpp"
#include "BVH_tree_node.hpp"

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

inline AxisAlignedBoundingBox addToBox(AxisAlignedBoundingBox container, AxisAlignedBoundingBox item) {
  sf::Glsl::Vec3 minVals(std::min(container.pos.x-container.bound.x/2.0, item.pos.x-item.bound.x/2.0), std::min(container.pos.y-container.bound.y/2.0, item.pos.y-item.bound.y/2.0), std::min(container.pos.z-container.bound.z/2.0, item.pos.z-item.bound.z/2.0));
  sf::Glsl::Vec3 maxVals(std::max(container.pos.x+container.bound.x/2.0, item.pos.x+item.bound.x/2.0), std::max(container.pos.y+container.bound.y/2.0, item.pos.y+item.bound.y/2.0), std::max(container.pos.z+container.bound.z/2.0, item.pos.z+item.bound.z/2.0));

  AxisAlignedBoundingBox retData;
  retData.bound = sf::Glsl::Vec3(maxVals.x-minVals.x, maxVals.y-minVals.y, maxVals.z-minVals.z);
  retData.pos = sf::Glsl::Vec3(retData.bound.x/2+minVals.x, retData.bound.y/2+minVals.y, retData.bound.z/2+minVals.z);

  return retData;
}

inline int getBoxOctant(AxisAlignedBoundingBox container, sf::Glsl::Vec3 point) {
  sf::Glsl::Vec3 minVals(container.pos.x-container.bound.x/2.0, container.pos.y-container.bound.y/2.0, container.pos.z-container.bound.z/2.0);
  sf::Glsl::Vec3 maxVals(container.pos.x+container.bound.x/2.0, container.pos.y+container.bound.y/2.0, container.pos.z+container.bound.z/2.0);

  if (point.x < minVals.x || point.x > maxVals.x || point.y < minVals.y || point.y > maxVals.y || point.z < minVals.z || point.z > maxVals.z) {
    throw std::invalid_argument("Error, point is outside of the bounding container!");
  }

  int data = 0;

  if (point.x >= minVals.x+(container.bound.x/2.0)) {
    data = 1;
  }

  if (point.y >= minVals.y+(container.bound.y/2.0)) {
    data+=2;
  }

  if (point.z >= minVals.z+(container.bound.z/2.0)) {
    data+=4;
  }

  return data;
}

class BVHTree {
public:
  BVHTree(int memoryBufferId);
  ~BVHTree();

  void bind(sf::Shader & shader, std::string bufferName);

  void addItemFromNode(BVHTreeNode * node, BVHTreeNode * item);
  BVHTreeNode * addItemToRoot(BVHTreeNode * item);
  void destroyNode(BVHTreeNode * node);
  void updateNode(BVHTreeNode * node);

private:
  BVHTreeNode * _root;
  sf::Uint8 * _writeBuffer;
  ShaderMemoryBuffer _memoryBuffer;
};

#endif