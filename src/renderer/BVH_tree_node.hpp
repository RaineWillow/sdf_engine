#pragma once
#ifndef BVH_Tree_Node_hpp
#define BVH_Tree_Node_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <iostream>
#include "axis_aligned_bounding_box.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/shader_memory_buffer.hpp"
#include "memory/writable.hpp"

class BVHTreeNode : public Writable {
public:
  BVHTreeNode(bool isLeaf, bool isRoot, Pixel address);
  ~BVHTreeNode();

  sf::Glsl::Vec3 getPos();
  sf::Glsl::Vec3 getBound();
  void setAddress(Pixel address);
  Pixel getAddress();

  int getParamsSize();

  bool hasChild(size_t childIndex);
  BVHTreeNode * getChild(size_t childIndex);
  bool hasParent();
  BVHTreeNode * getParent();
  size_t getParentIndex();

  bool isLeaf();
  bool isRoot();
  bool hasChildren();
  int anyFree();
  

  void setPos(sf::Glsl::Vec3 pos);
  void setBound(sf::Glsl::Vec3 bound);
  void setChild(size_t childIndex, BVHTreeNode * child);
  void removeChild(size_t childIndex);

  void setParent(BVHTreeNode * parent, size_t parentIndex);
  

  void updateParams(sf::Uint8 * &dataArray);

  void destroyAllChildren();
private:
  bool _hasChildren = false;
  bool _isRoot;
  bool _isLeaf;

  sf::Glsl::Vec3 _pos;
  sf::Glsl::Vec3 _bound;
  Pixel _address;

  std::vector<Pixel> _params;

  BVHTreeNode * _parent;
  size_t _parentIndex;
  BVHTreeNode * _children[8] = {NULL};

  sf::Uint8 * _writeData;
  int _paramsSize;
};


#endif