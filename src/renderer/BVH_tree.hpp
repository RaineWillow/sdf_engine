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
#include <string>
#include <map>
#include <iostream>
#include <unordered_map>
#include "memory/shader_memory_buffer.hpp"
#include "BVH_tree_node.hpp"


class BVHTree {
public:
  BVHTree(int memoryBufferId);
  ~BVHTree();

  void bind(sf::Shader & shader, std::string bufferName);

  AxisAlignedBoundingBox resizeNodeFromChildren(BVHTreeNode * node);

  void addItemFromNode(BVHTreeNode * node, BVHTreeNode * item);
  void addItemToRoot(BVHTreeNode * item);
  void destroyNode(BVHTreeNode * node);
  void updateNode(BVHTreeNode * node);

  BVHTreeNode * addLeaf(Pixel address, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound);
  void updateLeaf(BVHTreeNode * node, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound);
  void recurseTree(BVHTreeNode * nextItem, int layer, std::map<int, std::string> & layerData);
  std::string drawTree();

  void update();

private:
  BVHTreeNode * _root;
  sf::Uint8 * _writeBuffer;
  ShaderMemoryBuffer _memoryBuffer;
};

#endif