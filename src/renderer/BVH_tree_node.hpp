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
#include <functional>
#include "axis_aligned_bounding_box.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/shader_memory_buffer.hpp"
#include "memory/writable.hpp"

class BVHTreeNode : public Writable {
public:
  BVHTreeNode(bool isLeaf, bool isRoot, Pixel address);
  BVHTreeNode(bool isLeaf, bool isRoot, bool isLink, Pixel address);
  ~BVHTreeNode();

  void reInit(bool isLeaf, bool isRoot, bool isLink, Pixel address);

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
  BVHTreeNode * getLink();
  size_t getLayer();

  bool isLeaf();
  bool isRoot();
  bool isLink();
  bool hasChildren();
  bool hasLink();
  int anyFree();
  

  void setPos(sf::Glsl::Vec3 pos);
  void setBound(sf::Glsl::Vec3 bound);
  void setChild(size_t childIndex, BVHTreeNode * child);
  void removeChild(size_t childIndex);
  void setParent(BVHTreeNode * parent, size_t parentIndex);
  void setLink(BVHTreeNode * link);
  void removeLink();
  void setLayer(size_t layer);

  void setOverflowCallbackOn(std::function<void(BVHTreeNode*)> callback, size_t triggerLayer);

  void updateParams(sf::Uint8 * &dataArray);

  void destroyAllChildren();
private:
  static constexpr int _paramsSize = 16;
  bool _hasChildren = false;
  bool _isRoot;
  bool _isLeaf;
  bool _isLink;

  sf::Glsl::Vec3 _pos;
  sf::Glsl::Vec3 _bound;
  Pixel _address;

  std::vector<Pixel> _params;

  BVHTreeNode * _parent;
  size_t _parentIndex;
  size_t _layer = 0;
  BVHTreeNode * _children[8] = {NULL};
  BVHTreeNode * _link = NULL;

  sf::Uint8 * _writeData;

  std::function<void(BVHTreeNode*)> _onOverflow;
  size_t _triggerLayer = 0;
};


#endif