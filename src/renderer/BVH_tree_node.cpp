#include "BVH_tree_node.hpp"

BVHTreeNode::BVHTreeNode(bool isLeaf, bool isRoot, Pixel address) {
  _isLeaf = isLeaf;
  _isRoot = isRoot;
  _address = address;
  for (int i = 0; i < 15; i++) {
    Pixel defaultParam;
    _params.push_back(defaultParam);
  }

  _pos = sf::Glsl::Vec3(0, 0, 0);
  _bound = sf::Glsl::Vec3(0, 0, 0);
  _writeData = new sf::Uint8[15*4];
}

BVHTreeNode::~BVHTreeNode() {
  delete[] _writeData;
}

sf::Glsl::Vec3 BVHTreeNode::getPos() {
  return _pos;
}

sf::Glsl::Vec3 BVHTreeNode::getBound() {
  return _bound;
}

Pixel BVHTreeNode::getAddress() {
  return _address;
}

bool BVHTreeNode::hasChild(size_t childIndex) {
  if (childIndex > 7) {
    throw std::invalid_argument("Error, attempted to check a child greater than 7.");
  }
  return _children[childIndex] != NULL;
}

BVHTreeNode * BVHTreeNode::getChild(size_t childIndex) {
  if (childIndex > 7) {
    throw std::invalid_argument("Error, attempted to get a child greater than 7.");
  }
  if (!hasChild(childIndex)) {
    throw std::invalid_argument("Error, attempted to get a child that may not exist! Possible segfault.");
  }
  return _children[childIndex];
}

bool BVHTreeNode::hasParent() {
  return _parent != NULL;
}

BVHTreeNode * BVHTreeNode::getParent() {
  if (!hasParent()) {
    throw std::logic_error("Error, attempted to get the parent of a node that does not exist! Possible segfault.");
  }
  return _parent;
}

size_t BVHTreeNode::getParentIndex() {
  if (!hasParent()) {
    throw std::logic_error("Error, node does not have a parent!");
  }
  return _parentIndex;
}

bool BVHTreeNode::isLeaf() {
  return _isLeaf;
}

bool BVHTreeNode::isRoot() {
  return _isRoot;
}

bool BVHTreeNode::hasChildren() {
  return _hasChildren;
}

int BVHTreeNode::anyFree() {
  for (int i = 0; i < 8; i++) {
    if (!hasChild(i)) {
      return i;
    }
  }
  return -1;
}

void BVHTreeNode::setPos(sf::Glsl::Vec3 pos) {
  _pos = pos;
  _params[1].toNum(_pos.x);
  _params[2].toNum(_pos.y);
  _params[3].toNum(_pos.z);
}

void BVHTreeNode::setBound(sf::Glsl::Vec3 bound) {
  _bound = bound;
  _params[4].toNum(bound.x);
  _params[5].toNum(bound.y);
  _params[6].toNum(bound.z);
}

void BVHTreeNode::setChild(size_t childIndex, BVHTreeNode * child) {
  if (_isLeaf) {
    throw std::logic_error("Error, cannot set the child of a leaf."); 
  }
  if (childIndex > 7) {
    throw std::invalid_argument("Error, attempted to set a child greater than 7.");
  }
  if (hasChild(childIndex)) {
    throw std::logic_error("Error, cannot set a child to a location that already exists! Loc:" + std::to_string(childIndex));
  }

  _children[childIndex] = child;
  _children[childIndex]->setParent(this, childIndex);
  _params[7+childIndex] = child->getAddress();
  _hasChildren = true;
  _params[0].toBool(_hasChildren);
}

void BVHTreeNode::removeChild(size_t childIndex) {
  if (_isLeaf) {
    throw std::logic_error("Error, cannot destroy the child of a leaf, which does not exist."); 
  }
  if (childIndex > 7) {
    throw std::invalid_argument("Error, attempted to destroy a child greater than 7.");
  }
  if (!hasChild(childIndex)) {
    throw std::logic_error("Error, cannot destroy a child that doesn't exist!");
  }

  _children[childIndex]->setParent(NULL, 0);
  _children[childIndex] = NULL;
  _params[7+childIndex].toPointer(0, 0);
  for (int i = 0; i < 8; i++) {
    if (hasChild(i)) {
      _hasChildren=true;
      break;
    } else {
      _hasChildren=false;
    }
  }
  
  
  _params[0].toBool(_hasChildren);
}

void BVHTreeNode::setParent(BVHTreeNode * parent, size_t parentIndex) {
  if (_isRoot) {
    throw std::logic_error("Cannot set the parent of the root node!");
  }

  _parent = parent;
  _parentIndex = parentIndex;
}

void BVHTreeNode::updateParams(sf::Uint8 * &dataArray) {
  
  for (int i = 0; i < _params.size(); i++) {
    _params[i].writeToArray(i, _writeData, 15);
  }

  
  dataArray = _writeData;
}

void BVHTreeNode::destroyAllChildren() {
  for (size_t i = 0; i < 8; i++) {
    if (hasChild(i)) {
      if (_children[i]->hasChildren()) {
        _children[i]->destroyAllChildren();
      }

      delete _children[i];
      _children[i] = NULL;
    }
  }
}
