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

AxisAlignedBoundingBox BVHTree::resizeNodeFromChildren(BVHTreeNode * node) {
  if (!node->hasChildren()) {
    return AxisAlignedBoundingBox(sf::Glsl::Vec3(0, 0, 0), sf::Glsl::Vec3(0, 0, 0));
  }

  AxisAlignedBoundingBox childBoxes[8];
  size_t numChildren = 0;

  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      childBoxes[numChildren] = AxisAlignedBoundingBox(node->getChild(i)->getPos(), node->getChild(i)->getBound());
      numChildren+=1;
    }
  }

  AxisAlignedBoundingBox collection = childBoxes[0];
  for (int i = 1; i < numChildren; i++) {
      collection = addToBox(collection, childBoxes[i]);
      //std::cout << "X: " << collection.pos.x << " Y: " << collection.pos.y << " Z: " << collection.pos.z << std::endl;
      //std::cout << "XSize: " << collection.bound.x << " YSize: " << collection.bound.y << " ZSize: " << collection.bound.z << std::endl;
  }

  return collection;
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

  if (node->hasChildren()) {
    throw std::invalid_argument("Error, attempted to destroy a node with children.");
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
      if (!parent->hasParent()) {
        throw std::logic_error("Tried to travel to upper node, but there was no parent and the node was not root!");
      }
      currentNode = parent;
      parent = parent->getParent();
    } else {
      currentNode = parent;
      break;
    }
  }

  while (true) {
    //std::cout << "Adding up bounding boxes!" << std::endl;
    //now, we must add the bounding boxes of the node's remaining children
    AxisAlignedBoundingBox collection = resizeNodeFromChildren(currentNode);
    //std::cout << "X: " << collection.pos.x << " Y: " << collection.pos.y << " Z: " << collection.pos.z << std::endl;
    bool canBreak = collection==AxisAlignedBoundingBox(currentNode->getPos(), currentNode->getBound());
    currentNode->setPos(collection.pos);
    currentNode->setBound(collection.bound);
    currentNode->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(currentNode->getAddress().pointerIndex(), _writeBuffer);

    if (currentNode->isRoot() || canBreak) {
      break;
    } else {
      if (!currentNode->hasParent()) {
        throw std::logic_error("Error, current node does not have a parent but is not root!");
      }
      currentNode = currentNode->getParent();
    }
  }
}

void BVHTree::updateNode(BVHTreeNode * node) {
  if (!node->hasParent()) {
    throw std::invalid_argument("Error, this node does not have a parent node.");
  }

  BVHTreeNode * currentNode = node;
  BVHTreeNode * parent = node->getParent();

  AxisAlignedBoundingBox currentBox = AxisAlignedBoundingBox(currentNode->getPos(), currentNode->getBound());
  AxisAlignedBoundingBox parentBox = AxisAlignedBoundingBox(parent->getPos(), parent->getBound());


  //if the new bounding box was already bounded by it's parent's bounding box, we can safely return
  if (boundedByBox(parentBox, currentBox)) {
    return;
  }

  //if the parent is the root, simply resize the root node and move on
  if (parent->isRoot()) {
    AxisAlignedBoundingBox collection = resizeNodeFromChildren(parent);
    parent->setPos(collection.pos);
    parent->setBound(collection.bound);
    parent->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(parent->getAddress().pointerIndex(), _writeBuffer);
    return;
  }

  if (!parent->hasParent()) {
    throw std::invalid_argument("Error, this parent node does not have a parent node despite not being root!");
  }

  //now we want to see if we can resize the parent node by checking against it's parent
  BVHTreeNode * upperParent = parent->getParent();

  AxisAlignedBoundingBox upperParentBox = AxisAlignedBoundingBox(upperParent->getPos(), upperParent->getBound());
  AxisAlignedBoundingBox testParentBox = resizeNodeFromChildren(parent);

  //if the changed box is bounded by the upper parent, we can simply resize
  //and move on
  if (boundedByBox(upperParentBox, testParentBox)) {
    parent->setPos(testParentBox.pos);
    parent->setBound(testParentBox.bound);
    parent->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(parent->getAddress().pointerIndex(), _writeBuffer);
    return;
  }
  
  //if the box octant of the changed box is the same, we can simply add a 
  //recursion of this function to update the parent
  if (getBoxOctant(upperParentBox, testParentBox.pos)==getBoxOctant(upperParentBox, parentBox.pos)) {
    parent->setPos(testParentBox.pos);
    parent->setBound(testParentBox.bound);
    parent->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(parent->getAddress().pointerIndex(), _writeBuffer);
    updateNode(parent);
    return;
  }

  //now, we get to the most potentially expensive case, wherein we must
  //remove the node from it's parent, delete the parent if it is now empty,
  //and readd the node from the root
  parent->removeChild(currentNode->getParentIndex());
  if (parent->hasChildren()) {
    while (true) {
      AxisAlignedBoundingBox collection = resizeNodeFromChildren(parent);
      bool canBreak = collection==AxisAlignedBoundingBox(parent->getPos(), parent->getBound());
      parent->setPos(collection.pos);
      parent->setBound(collection.bound);
      parent->updateParams(_writeBuffer);
      _memoryBuffer.writeItem(parent->getAddress().pointerIndex(), _writeBuffer);

      if (parent->isRoot() || canBreak) {
        break;
      } else {
        if (!parent->hasParent()) {
          throw std::logic_error("Error, current node does not have a parent but is not root!");
        }
        parent = parent->getParent();
      }
    }
  } else {
    destroyNode(parent);
  }
  
  //simply readd the current node to the tree from the root
  addItemToRoot(currentNode);
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
    retData += std::to_string(x.first) + " " + x.second + "\n";
  }
  std::cout << "X Pos: " << _root->getPos().x << " Y Pos: " << _root->getPos().y << " Z Pos: " << _root->getPos().z << std::endl;
  std::cout << "X Size: " << _root->getBound().x << " Y Size: " << _root->getBound().y << " Z Size: " << _root->getBound().z << std::endl;
  return retData;
}
