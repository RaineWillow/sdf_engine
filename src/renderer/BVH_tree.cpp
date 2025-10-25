#include "BVH_tree.hpp"

BVHTree::BVHTree(int memoryBufferId) : _memoryBuffer(64, 2000, 16, memoryBufferId), 
_keepTracker(sf::Glsl::Vec3(-1, -1, -1), sf::Glsl::Vec3(-1, -1, -1)){
  
  //_writeBuffer = new sf::Uint8[15*4];
  Pixel _rootAddress = _memoryBuffer.newItem();
  _root = _nodePool.aquire(false, true, _rootAddress);
  _root->updateParams(_writeBuffer);

  _memoryBuffer.writeItem(_root->getAddress().pointerIndex(), _writeBuffer);
}

BVHTree::~BVHTree() {
  //delete[] _writeBuffer;
  _root->destroyAllChildren();
  delete _root;
  _root = NULL;
}

void BVHTree::bind(sf::Shader & shader, std::string bufferName) {
  shader.setUniform("rootPointer", _root->getAddress().asVec4());
  _memoryBuffer.bind(shader, bufferName);
}

AxisAlignedBoundingBox BVHTree::resizeNodeFromChildren(BVHTreeNode * node) {
  if (!node->hasChildren()) {
    std::cout << "Returned empty Bound!" << std::endl;
    return AxisAlignedBoundingBox(sf::Glsl::Vec3(0, 0, 0), sf::Glsl::Vec3(0, 0, 0));
  }

  AxisAlignedBoundingBox collection = AxisAlignedBoundingBox(sf::Glsl::Vec3(0, 0, 0), sf::Glsl::Vec3(0, 0, 0));
  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      collection = addToBox(collection, AxisAlignedBoundingBox(node->getChild(i)->getPos(), node->getChild(i)->getBound()));
    }
  }

  return collection;
}

void BVHTree::addItemToLink(BVHTreeNode * link, BVHTreeNode * item) {
  if (!link->isLink()) {
    throw std::invalid_argument("Error, attempted to link an item but the link container was something other than a link!");
  }

  if (!item->isLeaf()) {
    throw std::invalid_argument("Error, attempted to add an item to a link when it wasn't a leaf! Only leaf nodes can be added to a link!");
  }

  //there are no children present in the link container, just add it to 0
  if (!link->hasChildren()) {
    link->setChild(0, item);
    link->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(link->getAddress().pointerIndex(), _writeBuffer);
    return;
  }

  //there is a free location in the link container, add it to the free loc
  int freeChild = link->anyFree();
  if (freeChild != -1) {
    link->setChild(freeChild, item);
    link->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(link->getAddress().pointerIndex(), _writeBuffer);
    return;
  }

  //there was no free location, we must step down to the next link container
  if (!link->hasLink()) {
    Pixel linkAddress = _memoryBuffer.newItem();
    BVHTreeNode * newLink = _nodePool.aquire(false, false, true, linkAddress);
      
    link->setLink(newLink);
    _memoryBuffer.writeItem(newLink->getAddress().pointerIndex(), _writeBuffer);
  }

  link->getLink()->setPos(link->getPos());
  link->getLink()->setBound(link->getBound());

  addItemToLink(link->getLink(), item);
  _memoryBuffer.writeItem(link->getAddress().pointerIndex(), _writeBuffer);

  return;
}


void BVHTree::addItemFromNode(BVHTreeNode * node, BVHTreeNode * item) {
  //if the node does not have any children, simply set this node as the first child and 
  //set our bounding box accordingly.
  if (!node->hasChildren()) {
    node->setPos(item->getPos());
    node->setBound(item->getBound());
    node->setChild(0, item);
    node->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
    //std::cout << "Set item to position zero" << std::endl;
    return;
  }

  //this node does have children, so we have to create a new bounding box for our node
  AxisAlignedBoundingBox newBox = addToBox(
    AxisAlignedBoundingBox(node->getPos(), node->getBound()), 
    AxisAlignedBoundingBox(item->getPos(), item->getBound())
  );
  node->setPos(newBox.pos);
  node->setBound(newBox.bound);
  

  //we first check if the node has any free positions. if it does, we can just put our
  //item in the first free position and end.
  int freeChild = node->anyFree();
  if (freeChild != -1) {
    node->setChild(freeChild, item);
    node->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
    //std::cout << "Set item to free: " << freeChild << std::endl;
    return;
  }

  //We didn't have any free positions, but we may not want to subdivide. Lets
  //see if we can add the item to a node's linked list
  /*
  std::cout << std::setprecision(10);
  std::cout << "Node: " << node->getPos().x << " " << node->getPos().y << " " << node->getPos().z << " : "
   << node->getBound().x << " " << node->getBound().y << " " << node->getBound().z << " " << std::endl;
  std::cout << "Item: " << item->getPos().x << " " << item->getPos().y << " " << item->getPos().z << " : "
   << item->getBound().x << " " << item->getBound().y << " " << item->getBound().z << " " << std::endl;
  std::cout << "Bounded: " << boundedByBox(AxisAlignedBoundingBox(node->getPos(), node->getBound()), 
    AxisAlignedBoundingBox(item->getPos(), item->getBound())) << std::endl;
  */
  
  if ((!parentShouldSubdivide(
    AxisAlignedBoundingBox(node->getPos(), node->getBound()), 
    AxisAlignedBoundingBox(item->getPos(), item->getBound()), 
    _subdivideThreshold) || node->getLayer() >= 18) && item->isLeaf()) {
    

    //we now know that we can in fact use a link node here. Lets create it!
    if (!node->hasLink()) {
      Pixel linkAddress = _memoryBuffer.newItem();
      BVHTreeNode * newLink = _nodePool.aquire(false, false, true, linkAddress);
      
      node->setLink(newLink);
      _memoryBuffer.writeItem(newLink->getAddress().pointerIndex(), _writeBuffer);
    }

    node->getLink()->setPos(node->getPos());
    node->getLink()->setBound(node->getBound());

    addItemToLink(node->getLink(), item);

    _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
    return;
  }

  //we did not have any free node positions and we must subdivide. 
  //We need to get the octant of the new item based on it's position.
  int itemOctant = getBoxOctant(newBox, item->getPos());

  std::vector<int> childBoxIndexes;

  

  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      BVHTreeNode * testChild = node->getChild(i);
      if (testChild->isLeaf()) {
        childBoxIndexes.push_back(i);
        //std::cout << "Skipping leaf: " << i << std::endl;
      }
    }
  }

  //now, we check to see if any of the leaves of this node can be put into a bounding
  //volume, freeing up space for our new node.

  //first, we find all of the leaf nodes, and construct a octMap for them, mapping
  //their octant to the map
  std::unordered_map<int, std::vector<BVHTreeNode*>> leafOctMap;
  for (int i = 0; i < childBoxIndexes.size(); i++) {
    int testedOctant = getBoxOctant(newBox, node->getChild(childBoxIndexes[i])->getPos());
    leafOctMap[testedOctant].push_back(node->getChild(childBoxIndexes[i]));
  }

  //now, we try to find a set of leaves that can be collapsed into their own bounding
  //volume node, freeing up space for our new node
  for (auto const& x : leafOctMap) {
    if (x.second.size() > 1) {
      Pixel newAddress = _memoryBuffer.newItem();
      BVHTreeNode * newBoundingVolume = _nodePool.aquire(false, false, newAddress);
      for (int i = 0; i < x.second.size(); i++) {
        node->removeChild(x.second[i]->getParentIndex());
        addItemFromNode(newBoundingVolume, x.second[i]);
      }
      newBoundingVolume->updateParams(_writeBuffer);
      _memoryBuffer.writeItem(newBoundingVolume->getAddress().pointerIndex(), _writeBuffer);
      node->setChild(node->anyFree(), newBoundingVolume);

      node->updateParams(_writeBuffer);
      _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
      addItemFromNode(node, item);
      return;
    }
  }

  //we will first check if there is a bounding volume that is not a leaf which we use to
  //add the child to
  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      BVHTreeNode * testChild = node->getChild(i);
      //if the child is a leaf, we should skip this node, as leaf nodes cannot have
      //children
      if (testChild->isLeaf()) {
        //childBoxIndexes.push_back(i);
        //std::cout << "Skipping leaf: " << i << std::endl;
        continue;
      }
      //the child was not a leaf, so lets see if there is a bounding volume already in 
      //the octant of the current box which we can attach our new item to. this is 
      //common if the bounding volume of the node did not change, even after adding the 
      //item
      int childOctantTest = getBoxOctant(newBox, testChild->getPos());
      //std::cout << "Octant " << childOctantTest << " testing against " << itemOctant << std::endl;
      //std::cout << "Test Pos: x:" << testChild->getPos().x << " y: " << testChild->getPos().y << " z " << testChild->getPos().z << " child pos: x" << item->getPos().x << " y: " << item->getPos().y << " z " << item->getPos().z << std::endl;
      if (childOctantTest == itemOctant) {
        //std::cout << "Writing to item " << i << std::endl;
        node->updateParams(_writeBuffer);
        _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
        addItemFromNode(testChild, item);
        return;
      }
    }
  }
  //std::cout << "Building new bounding volume layer" << std::endl;

  //finally, we arrive at the most expensive case, wherein we construct new bounding
  //volumes for all children by collapsing them into bounding volumes corresponding to
  //their octant

  //this octmap keeps track of each node we need to move, as well as it's octant.
  std::unordered_map<int, std::vector<BVHTreeNode*>> octMap;

  //we loop through each child and remove it from the node, while adding it to the
  //octmap
  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      BVHTreeNode * currentChild = node->getChild(i);
      int currentOctant = getBoxOctant(newBox, currentChild->getPos());
      octMap[currentOctant].push_back(currentChild);
      node->removeChild(i);
    }
  }

  //now, for each entry in the octmap, we must create a new bounding volume,
  //add each child to it, and then set it to the first free node in the current node. we
  //don't really have to worry about overflow here, since there can only be a maximum of
  //8 items

  //std::cout << "Num Bounding volumes: " << octMap.size() << std::endl;
  for (auto const& x : octMap) {
    Pixel newAddress = _memoryBuffer.newItem();
    BVHTreeNode * newBoundingVolume = _nodePool.aquire(false, false, newAddress);
    for (int i = 0; i < x.second.size(); i++) {
      //newBoundingVolume->setChild(newBoundingVolume->anyFree(), x.second[i]);
      addItemFromNode(newBoundingVolume, x.second[i]);
    }
    newBoundingVolume->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(newBoundingVolume->getAddress().pointerIndex(), _writeBuffer);
    size_t index = node->anyFree();
    //std::cout << "writing new bounding volume to index " << index << " with size: " << x.second.size() << std::endl;
    node->setChild(index, newBoundingVolume);
  }

  //write our finalized changes to this node before continuing
  node->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);

  //lastly, we deal with our item. if there is a free space, it will just end up in this
  //bounding volume, but if there isn't a free space (odd case where every item was in a 
  //different octant) it will find it's octant and be moved there. either way, the
  //previous steps will have created a space for it somewhere.
  this->addItemFromNode(node, item);
}

void BVHTree::addItemToRoot(BVHTreeNode * item) {
  this->addItemFromNode(_root, item);
}

void BVHTree::addLeafToRoot(BVHTreeNode * leafNode) {
  sf::Glsl::Vec3 pos = leafNode->getPos();
  sf::Glsl::Vec3 bound = leafNode->getBound();

  if (!_hasAnyBoxes) {
    _keepTracker.pos = pos;
    _keepTracker.bound = bound;
    _hasAnyBoxes = true;
  } else {
    AxisAlignedBoundingBox addBox(pos, bound);
    _keepTracker = addToBox(_keepTracker, addBox);
  }

  int octant = getBoxOctant(_keepTracker, pos);
  int manhattanDistance = abs(_keepTracker.pos.x-pos.x) + abs(_keepTracker.pos.y-pos.y) + abs(_keepTracker.pos.z-pos.z);

  int insertLoc = 0;

  //find insert position

  /*
  for (int i = 0; i < _updates[octant].size(); i++) {
    if (_updates[octant][i].distance >= manhattanDistance) {
        insertLoc = i+1;
    } else {
        break;
    }
  }
*/
  SizeOrder newSize(leafNode, manhattanDistance);
  _updates[octant].push_back(newSize);

/*
  if (insertLoc == _updates[octant].size()) {
    _updates[octant].push_back(newSize);
  } else {
    _updates[octant].insert(_updates[octant].begin() + insertLoc, newSize);
  }
*/
}

void BVHTree::destroyAllChildrenOfNode(BVHTreeNode * node) {
  for (size_t i = 0; i < 8; i++) {
    if (node->hasChild(i)) {

      BVHTreeNode * child = node->getChild(i);
      if (child->hasChildren() || child->hasLink()) {
        destroyAllChildrenOfNode(child);
      }

      _memoryBuffer.freeItem(child->getAddress().pointerIndex());
      node->removeChild(i);
      _nodePool.freeItem(child);
    }
  }

  if (node->hasLink()) {
    BVHTreeNode * link = node->getLink();
    destroyAllChildrenOfNode(link);
    _memoryBuffer.freeItem(link->getAddress().pointerIndex());
    node->removeLink();
    _nodePool.freeItem(link);
  }
}

void BVHTree::resetTree() {
    for (int i = 0; i < _leafNodes.size(); i++) {
        BVHTreeNode * itemParent = _leafNodes[i]->getParent();
        size_t parentIndex = _leafNodes[i]->getParentIndex();
        itemParent->removeChild(parentIndex);

        addLeafToRoot(_leafNodes[i]);
    }
    _leafNodes.clear();

    destroyAllChildrenOfNode(_root);
    _root->setPos(sf::Glsl::Vec3(0, 0, 0));
    _root->setBound(sf::Glsl::Vec3(0, 0, 0));

    _root->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(_root->getAddress().pointerIndex(), _writeBuffer);

    _keepTracker.pos = _root->getPos();
    _keepTracker.bound = _root->getBound();
    _hasAnyBoxes = _root->hasChildren();
}

BVHTreeNode * BVHTree::addLeaf(Pixel address, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound) {
  BVHTreeNode * leafNode = _nodePool.aquire(true, false, address);
  leafNode->setPos(pos);
  leafNode->setBound(bound);

  addLeafToRoot(leafNode);
  
  //this->addItemToRoot(leafNode);
  return leafNode;
}

void BVHTree::updateLeaf(BVHTreeNode * node, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound) {
  node->setPos(pos);
  node->setBound(bound);
}

void BVHTree::destroyLeaf(BVHTreeNode * node) {
  if (!node->isLeaf()) {
    throw std::invalid_argument("Error, attempted to destroy a node which was not a leaf!");
  }

  auto it = std::find(_leafNodes.begin(), _leafNodes.end(), node);

  if (it==_leafNodes.end()) {
    throw std::invalid_argument("Error, attempted to destroy a node that either doesn't exist in the tree or hasn't loaded yet! (If recently added, it might be in staging! Must call full update before removing!)");
  }

  if (node->hasParent()) {
    node->getParent()->removeChild(node->getParentIndex());
  }

  _memoryBuffer.freeItem(node->getAddress().pointerIndex());

  _nodePool.freeItem(node);
}

bool BVHTree::hasLeaf(BVHTreeNode * node) {
  if (!node->isLeaf()) {
    throw std::invalid_argument("Error, attempted to destroy a node which was not a leaf!");
  }

  return std::find(_leafNodes.begin(), _leafNodes.end(), node)!=_leafNodes.end();
}

void BVHTree::recurseTree(BVHTreeNode* nextItem, int layer, std::map<int, std::string> & layerData) {
  std::string data = "";
  if (nextItem->isLeaf()) {
    data += "L ";
  } else if (nextItem->isRoot()) {
    data += "R ";
  } else {
    data += "B ";
  }
  layerData[layer] += data;
  for (int i = 0; i < 8; i++) {
    if (nextItem->hasChild(i)) {
      this->recurseTree(nextItem->getChild(i), layer+1, layerData);
    }
  }
}

std::string BVHTree::drawTree() {
  std::string retData = "";
  std::map<int, std::string> layerData;
  this->recurseTree(_root, 0, layerData);
  for (auto const& x : layerData) {
    retData += std::to_string(x.first) + " " + x.second + "\n";
  }

  retData += std::to_string(_leafNodes.size());
  //std::cout << "X Pos: " << _root->getPos().x << " Y Pos: " << _root->getPos().y << " Z Pos: " << _root->getPos().z << std::endl;
  //std::cout << "X Size: " << _root->getBound().x << " Y Size: " << _root->getBound().y << " Z Size: " << _root->getBound().z << std::endl;
  return retData;
}

void BVHTree::update() {
  resetTree();

  int updateIter[8] = {0};

  bool iterating = hasItemUpdates();

  if (iterating) {
    for (int i = 0; i < 8; i++) {
      std::sort(_updates[i].begin(), _updates[i].end(), [](SizeOrder a, SizeOrder b){return a.distance>b.distance;});
    }
    
  }

  bool iterateMode = true;

  while (iterating) {
    iterating = false;
    for (int i = 0; i < 8; i++) {
        if (_updates[i].size() > updateIter[i]) {

            if (iterateMode) {
                this->addItemToRoot(_updates[i][updateIter[i]].item);
                _leafNodes.push_back(_updates[i][updateIter[i]].item);
                updateIter[i] += 1;
            } else {
                this->addItemToRoot(_updates[i].back().item);
                _leafNodes.push_back(_updates[i].back().item);
                _updates[i].pop_back();
            }
        }

        iterating = iterating || (_updates[i].size() > updateIter[i]);
    }

    iterateMode = !iterateMode;
  }

  /*
  if (hasItemUpdates()) {
    std::cout << this->drawTree() << std::endl;
  }

  */

  
  for (int i = 0; i < 8; i++) {
    _updates[i].clear();
  }
  _memoryBuffer.update();
}
