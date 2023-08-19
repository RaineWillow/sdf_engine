#include "BVH_tree.hpp"

BVHTree::BVHTree(int memoryBufferId) : _memoryBuffer(250, 3500, 15, memoryBufferId){
  
  _writeBuffer = new sf::Uint8[15*4];
  Pixel _rootAddress = _memoryBuffer.newItem();
  _root = new BVHTreeNode(false, true, _rootAddress);
  _root->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(_root->getAddress().pointerIndex(), _writeBuffer);
}

BVHTree::~BVHTree() {
  delete[] _writeBuffer;
  _root->destroyAllChildren();
  delete _root;
  _root = NULL;
}

void BVHTree::bind(sf::Shader & shader, std::string bufferName) {
  _memoryBuffer.bind(shader, bufferName);
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

  //we did not have any free node positions. We need to get the octant of the new item
  //based on it's position.
  int itemOctant = getBoxOctant(newBox, item->getPos());

  std::vector<int> childBoxIndexes;
  //we will first check if there is a bounding volume that is not a leaf which we use to
  //add the child to
  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      BVHTreeNode * testChild = node->getChild(i);
      //if the child is a leaf, we should skip this node, as leaf nodes cannot have
      //children
      if (testChild->isLeaf()) {
        childBoxIndexes.push_back(i);
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

  std::unordered_map<int, std::vector<BVHTreeNode*>> leafOctMap;
  for (int i = 0; i < childBoxIndexes.size(); i++) {
    int testedOctant = getBoxOctant(newBox, node->getChild(childBoxIndexes[i])->getPos());
    leafOctMap[testedOctant].push_back(node->getChild(childBoxIndexes[i]));
  }

  for (auto const& x : leafOctMap) {
    if (x.second.size() > 1) {
      Pixel newAddress = _memoryBuffer.newItem();
      BVHTreeNode * newBoundingVolume = new BVHTreeNode(false, false, newAddress);
      for (int i = 0; i < x.second.size(); i++) {
        node->removeChild(x.second[i]->getParentIndex());
        addItemFromNode(newBoundingVolume, x.second[i]);
      }
      newBoundingVolume->updateParams(_writeBuffer);
      _memoryBuffer.writeItem(newBoundingVolume->getAddress().pointerIndex(), _writeBuffer);
      node->setChild(node->anyFree(), newBoundingVolume);

      node->updateParams(_writeBuffer);
      _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
      this->addItemFromNode(node, item);
      return;
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
    BVHTreeNode * newBoundingVolume = new BVHTreeNode(false, false, newAddress);
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

void BVHTree::destroyNode(BVHTreeNode * node) {
  if (!node->hasParent()) {
    throw std::invalid_argument("Error, this node does not have a parent node.");
  }
  BVHTreeNode * currentNode = node;
  BVHTreeNode * parent = node->getParent();

  //destroy item from parent, determine if parent needs to be deleted
  while (true) {
    if (!currentNode->isLeaf()) {
      _memoryBuffer.freeItem(currentNode->getAddress().pointerIndex());
    }
    parent->removeChild(currentNode->getParentIndex());
    BVHTreeNode * deleteNode = currentNode;
    delete deleteNode;
    deleteNode = NULL;
    if (!parent->hasChildren() && !parent->isRoot()) {
      if (!node->hasParent()) {
        throw std::logic_error("Tried to travel to upper node, but there was no parent, but the node was not root!");
      }
      currentNode = parent;
      parent = currentNode->getParent();
    } else {
      currentNode = parent;
      break;
    }
  }

  while (true) {
    //now, we must add the bounding boxes of the node's children
    std::vector<AxisAlignedBoundingBox> _childBoxes;
    for (int i = 0; i < 8; i++) {
      if (currentNode->hasChild(i)) {
        _childBoxes.push_back(AxisAlignedBoundingBox(currentNode->getChild(i)->getPos(), currentNode->getChild(i)->getBound()));
      }
    }
    AxisAlignedBoundingBox collection = _childBoxes[0];
    for (int i = 1; i < _childBoxes.size(); i++) {
      addToBox(collection, _childBoxes[i]);
    }
    currentNode->setPos(collection.pos);
    currentNode->setBound(collection.bound);

    currentNode->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(currentNode->getAddress().pointerIndex(), _writeBuffer);

    if (currentNode->isRoot()) {
      break;
    } else {
      if (!currentNode->hasParent()) {
        throw std::logic_error("Error, current node does not have a parent but is not root!");
      }
      currentNode = currentNode->getParent();
    }
  }
}

BVHTreeNode * BVHTree::addLeaf(Pixel address, sf::Glsl::Vec3 pos, sf::Glsl::Vec3 bound) {
  BVHTreeNode * leafNode = new BVHTreeNode(true, false, address);
  leafNode->setPos(pos);
  leafNode->setBound(bound);
  this->addItemToRoot(leafNode);
  return leafNode;
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
    retData += x.second + "\n";
  }
  std::cout << "X Pos: " << _root->getPos().x << " Y Pos: " << _root->getPos().y << " Z Pos: " << _root->getPos().z << std::endl;
  std::cout << "X Size: " << _root->getBound().x << " Y Size: " << _root->getBound().y << " Z Size: " << _root->getBound().z << std::endl;
  return retData;
}
