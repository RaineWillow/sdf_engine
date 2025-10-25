#include "BVH_tree_node.hpp"

BVHTreeNode::BVHTreeNode(bool isLeaf, bool isRoot, Pixel address) {
  _isLeaf = isLeaf;
  _isRoot = isRoot;
  _isLink = false;
  _address = address;
  for (int i = 0; i < _paramsSize; i++) {
    Pixel defaultParam;
    _params.push_back(defaultParam);
  }
  _parent = NULL;
  _pos = sf::Glsl::Vec3(0, 0, 0);
  _bound = sf::Glsl::Vec3(0, 0, 0);
  _writeData = new sf::Uint8[_paramsSize*4];
}

BVHTreeNode::BVHTreeNode(bool isLeaf, bool isRoot, bool isLink, Pixel address) {
  _isLeaf = isLeaf;
  _isRoot = isRoot;
  _isLink = isLink;
  _address = address;
  for (int i = 0; i < _paramsSize; i++) {
    Pixel defaultParam;
    _params.push_back(defaultParam);
  }
  _parent = NULL;
  _pos = sf::Glsl::Vec3(0, 0, 0);
  _bound = sf::Glsl::Vec3(0, 0, 0);
  _writeData = new sf::Uint8[_paramsSize*4];
}

void BVHTreeNode::reInit(bool isLeaf, bool isRoot, bool isLink, Pixel address) {
  _isLeaf = isLeaf;
  _isRoot = isRoot;
  _isLink = isLink;
  _address = address;
  _parent = NULL;
  _parentIndex = 0;
  _layer = 0;
  _pos = sf::Glsl::Vec3(0, 0, 0);
  _bound = sf::Glsl::Vec3(0, 0, 0);
  _hasChildren = false;
  for (int i = 0; i < 8; i++) {
    _children[i] = NULL;
  }
  _link = NULL;
  _onOverflow = nullptr;
  _triggerLayer = 0;
  for (int i = 0; i < _paramsSize; i++) {
    Pixel defaultParam;
    _params[i] = defaultParam;
  }
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

void BVHTreeNode::setAddress(Pixel address) {
  _address = address;
}

Pixel BVHTreeNode::getAddress() {
  return _address;
}

int BVHTreeNode::getParamsSize() {
  return _paramsSize;
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

BVHTreeNode * BVHTreeNode::getLink() {
  if (isLeaf()) {
    throw std::logic_error("Error, leaf nodes cannot have links!");
  }
  if (!hasLink()) {
    throw std::logic_error("Error, attempted to get the link of a node without a link!");
  }

  return _link;
}

size_t BVHTreeNode::getLayer() {
  return _layer;
}

bool BVHTreeNode::isLeaf() {
  return _isLeaf;
}

bool BVHTreeNode::isRoot() {
  return _isRoot;
}

bool BVHTreeNode::isLink() {
  return _isLink;
}

bool BVHTreeNode::hasChildren() {
  return _hasChildren;
}

bool BVHTreeNode::hasLink() {
  return _link != NULL;
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
  if (_isLink && !child->isLeaf()) {
    throw std::logic_error("Error, cannot add a non-leaf to a link node as a standard child!");
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
  if (_parent!=NULL && !_isLink) {
    setLayer(_parent->getLayer()+1);
  } else if (_isLink) {
    setLayer(0);
  }
  
}

void BVHTreeNode::setLink(BVHTreeNode * link) {
  if (_isLeaf) {
    throw std::logic_error("Error, cannot set the link of a leaf, a leaf cannot have children!");
  }
  if (!link->isLink()) {
    throw std::invalid_argument("Error, cannot set the link to a node which is not itself a link!");
  }
  if (hasLink()) {
    throw std::logic_error("Error, cannot set a link to a node which already has one! (try removing, or adding to the linked list)");
  }

  _link = link;
  _link->setParent(this, 9);
  _params[15] = _link->getAddress();
}

void BVHTreeNode::removeLink() {
  if (!hasLink()) {
    throw std::logic_error("Error, cannot remove a link that doesn't exist!");
  }

  _link->setParent(NULL, 0);
  _link = NULL;
  _params[15].toPointer(0, 0);
}

void BVHTreeNode::setLayer(size_t layer) {
  _layer = layer;
  for (size_t i = 0; i < 8; i++) {
    if (hasChild(i)) {
      _children[i]->setLayer(layer+1);
    }
  }

  if (_isLeaf && _layer >= _triggerLayer && _onOverflow && hasParent()) {
    _onOverflow(this);
  }
}

void BVHTreeNode::setOverflowCallbackOn(std::function<void(BVHTreeNode*)> callback, size_t triggerLayer) {
  if (!_isLeaf) {
    throw std::logic_error("Error, cannot give overflow callback to non-leaf node.");
  }
  _triggerLayer = triggerLayer;
  _onOverflow = callback;
}

void BVHTreeNode::updateParams(sf::Uint8 * &dataArray) {
  
  for (int i = 0; i < _paramsSize; i++) {
    _params[i].writeToArray(i, _writeData, _paramsSize);
  }

  
  dataArray = _writeData;
}

void BVHTreeNode::destroyAllChildren() {
  for (size_t i = 0; i < 8; i++) {
    if (hasChild(i)) {
      if (_children[i]->hasChildren() || _children[i]->hasLink()) {
        _children[i]->destroyAllChildren();
      }

      delete _children[i];
      _children[i] = NULL;
    }
  }
  if (hasLink()) {
    _link->destroyAllChildren();
    delete _link;
  }
}
