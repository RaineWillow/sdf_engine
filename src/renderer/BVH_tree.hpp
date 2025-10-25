#pragma once
#ifndef BVH_Tree_Hpp
#define BVH_Tree_Hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <string>
#include <map>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <iomanip>
#include "memory/shader_memory_buffer.hpp"
#include "BVH_tree_node.hpp"

struct SizeOrder {
    BVHTreeNode * item;
    int distance;

    SizeOrder(BVHTreeNode * inItem, int inSize) {
        item = inItem;
        distance = inSize;
    }
};

class BVHTreeNodePool {
public:
  BVHTreeNodePool()=default;
  ~BVHTreeNodePool() {
    for (size_t i = 0; i < _pool.size(); i++) {
      delete _pool[i];
      _pool[i] = NULL;
    }
  }

  BVHTreeNode * aquire(bool isLeaf, bool isRoot, Pixel address) {
    BVHTreeNode * retVal;
    if (!_pool.empty()) {
      retVal = _pool.back();
      _pool.pop_back();
      retVal->reInit(isLeaf, isRoot, false, address);
    } else {
      retVal = new BVHTreeNode(isLeaf, isRoot, address);
    }

    return retVal;
  }

  BVHTreeNode * aquire(bool isLeaf, bool isRoot, bool isLink, Pixel address) {
    BVHTreeNode * retVal;
    if (!_pool.empty()) {
      retVal = _pool.back();
      _pool.pop_back();
      retVal->reInit(isLeaf, isRoot, isLink, address);
    } else {
      retVal = new BVHTreeNode(isLeaf, isRoot, isLink, address);
    }

    return retVal;
  }

  void freeItem(BVHTreeNode * item) {
    _pool.push_back(item);
  }
private:
  std::vector<BVHTreeNode*> _pool;
};

class BVHTree {
public:
  BVHTree(int memoryBufferId);
  ~BVHTree();

  void bind(sf::Shader & shader, std::string bufferName);

  AxisAlignedBoundingBox resizeNodeFromChildren(BVHTreeNode * node);

  void addItemToLink(BVHTreeNode * link, BVHTreeNode * item);
  void addItemFromNode(BVHTreeNode * node, BVHTreeNode * item);
  void addItemToRoot(BVHTreeNode * item);
  void addLeafToRoot(BVHTreeNode * leafNode);
  void destroyAllChildrenOfNode(BVHTreeNode * node);
  void resetTree();

  BVHTreeNode * addLeaf(Pixel address, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound);
  void updateLeaf(BVHTreeNode * node, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound);
  void destroyLeaf(BVHTreeNode * node);
  bool hasLeaf(BVHTreeNode * node);

  void recurseTree(BVHTreeNode * nextItem, int layer, std::map<int, std::string> & layerData);
  std::string drawTree();

  void update();

private:
  static constexpr float _subdivideThreshold = 0.05;
  BVHTreeNode * _root;
  sf::Uint8 * _writeBuffer;
  ShaderMemoryBuffer _memoryBuffer;

  BVHTreeNodePool _nodePool;

  AxisAlignedBoundingBox _keepTracker;
  bool _hasAnyBoxes = false;

  bool _resetFlag = false;

  std::vector<SizeOrder> _updates[8];

  std::vector<BVHTreeNode *> _leafNodes;

  bool hasItemUpdates() {
    for (int i = 0; i < 8; i++) {
        if (_updates[i].size() > 0) {
            return true;
        }
    }

    return false;
  }

  std::function<void(BVHTreeNode*)> _onOverflow;
  size_t _overflowThreshold = 18;
};

#endif