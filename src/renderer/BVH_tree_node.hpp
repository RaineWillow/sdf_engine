#pragma once
#ifndef BVH_Tree_Node_hpp
#define BVH_Tree_Node_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "shapes.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/shader_memory_buffer.hpp"

template <typename T>
class BVHTreeNode {
public:
  BVHTreeNode(int itemSize, bool leaf) {
    if (itemSize < 16) {
      throw std::invalid_argument("Error, item size was smaller than the minimum requirement for a BVHTree Node.");
    }
    if (!leaf) {
      for (int i = 0; i < itemSize; i++) {
        Pixel defaultParam;
        _params.push_back(defaultParam);
      }
      _params[0].toInt(0);
    }
    _isLeaf = leaf;
    dataArray = new sf::Uint8[itemSize*4];
    _itemSize = itemSize;
    updateAllParams();
  }

  ~BVHTreeNode() {
    delete[] dataArray;
  }

  void create(int memoryBufferIndex) {
    _memoryBufferIndex = memoryBufferIndex;
  }

  bool isLeaf() {
    return _isLeaf;
  }

  void setRoot(bool isRoot) {
    _isRoot = isRoot;
  }

  bool getRoot() {
    return _isRoot;
  }

  int getMemoryBufferIndex() {
    return _memoryBufferIndex;
  }

  bool hasChildren() {
    return _hasChildren;
  }

  BVHTreeNode<T> * getParent() {
    if (_parent==NULL) {
      throw std::logic_error("Tried to a parent node that doesn't exist!");
    }

    return _parent;
  }

  BVHTreeNode<T> * getChild(size_t childIndex) {
    if (_isLeaf) {
      throw std::invalid_argument("Node is a leaf node, has no children!");
    }

    if (!_hasChildren) {
      throw std::invalid_argument("Node has no children!");
    }

    if (childIndex > 7) {
      throw std::invalid_argument("Child index " + std::to_string(childIndex) + " is out of bounds!");
    }

    if (_children[childIndex]==NULL) {
      throw std::invalid_argument("Child index " + std::to_string(childIndex) + "is NULL");
    }

    return _children[childIndex];
  }

  T * getLeafData() {
    if (!_isLeaf) {
      throw std::logic_error("Tried to get leaf data from a node which is not a leaf!");
    }

    return _leaf;
  }

  sf::Glsl::Vec3 getPos() {
    return _pos;
  }

  sf::Glsl::Vec3 getBox() {
    return _box;
  }

  void setParent(BVHTreeNode<T> * parent) {
    if (_isRoot) {
      throw std::logic_error("Cannot set the parent of the root node!");
    }

    if (parent==NULL) {
      throw std::invalid_argument("Cannot set a null object as the parent of this node!");
    }

    _parent = parent;
  }

  void setChild(size_t childIndex, BVHTreeNode<T> * child) {
    if (_isLeaf) {
      throw std::logic_error("Node is a leaf node, cannot add children!");
    }

    if (childIndex > 7) {
      throw std::invalid_argument("Child index " + std::to_string(childIndex) + " is out of bounds!");
    }

    if (child==NULL) {
      throw std::invalid_argument("Attempted to pass a NULL child (Did you mean to delete the child node?)");
    }

    if (child->getMemoryBufferIndex()==-1) {
      throw std::invalid_argument("Attempted to pass a child that has not been created, it must be allocated with an id from a ShaderMemoryBuffer!");
    }

    _hasChildren = true;
    _children[childIndex] = child;
    _params[1].toBool(_hasChildren);
    _params[8+childIndex].toPointer(_children[childIndex]->getMemoryBufferIndex(), 1);
  }

  void setLeafData(T * leaf) {
    if (!_isLeaf) {
      throw std::logic_error("Tried to set leaf data on a node which was not a leaf!");
    }

    _leaf = leaf;
    _pos = _leaf->getPos();
    _box = _leaf->getBox();
  }

  void updateLeafData() {
    if (!_isLeaf) {
      throw std::logic_error("Cannot refresh the leaf data of a node which is not a leaf!");
    }

    if (_leaf==NULL) {
      throw std::logic_error("Cannot refresh the leaf data of a node which has not been assigned leaf data!");
    }

    _pos = _leaf->getPos();
    _box = _leaf->getBox();
  }

  void destroyChild(size_t childIndex) {
    if (_isLeaf) {
      throw std::logic_error("Cannot destroy the child of a leaf node, which has no children!");
    }

    if (!_hasChildren) {
      throw std::logic_error("Error, node has no children!");
    }

    if (childIndex > 7) {
      throw std::invalid_argument("Child index " + std::to_string(childIndex) + " is out of bounds!");
    }

    if (_children[childIndex]==NULL) {
      throw std::invalid_argument("Attempted to destroy a child node which was already NULL!");
    }

    delete _children[childIndex];
    _children[childIndex] = NULL;
    _params[8+childIndex].toPointer(0, 0);

    for (size_t i = 0; i < 8; i++) {
      if (_children[i] != NULL) {
        _hasChildren = false;
      }
    }

    _params[1].toBool(_hasChildren);
  }

  void setPos(sf::Glsl::Vec3 pos) {
    if (_isLeaf) {
      throw std::logic_error("Cannot set the position of a leaf node, set the position using the leaf data!");
    }

    _pos = pos;

    _params[2].toNum(_pos.x);
    _params[3].toNum(_pos.y);
    _params[4].toNum(_pos.z);
  }

  void setBox(sf::Glsl::Vec3 box) {
    if (_isLeaf) {
      throw std::logic_error("Cannot set the box size of a leaf node, set the box size using the leaf data!");
    }

    _box = box;

    _params[5] = _box.x;
    _params[6] = _box.y;
    _params[7] = _box.z;
  }

  sf::Uint8 * dataArray;

  void updateAllParams() {
    if (_isLeaf) {
      if (_leaf==NULL) {
        throw std::logic_error("Cannot update params, leaf data not set!");
      }
      _leaf->updateParams(dataArray);
    } else {
      for (int i = 0; i < _params.size(); i++) {
        _params[i].writeToArray(i, dataArray, _itemSize);
      }
    }
  }

private:
  int _itemSize;
  bool _isRoot = false;
  BVHTreeNode<T> * _parent;

  bool _hasChildren = false;
  BVHTreeNode<T> * _children[8] = {};
  bool _isLeaf;
  T * _leaf;
  
  sf::Glsl::Vec3 _box;
  sf::Glsl::Vec3 _pos;

  std::vector<Pixel> _params;
  int _memoryBufferIndex=-1;
};

#endif