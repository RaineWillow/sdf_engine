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
    return;
  }

  //we did not have any free node positions. We need to get the octant of the new item
  //based on it's position.
  int itemOctant = getBoxOctant(newBox, item->getPos());

  //we will first check if there is a bounding volume that is not a leaf which we use to
  //add the child to
  for (int i = 0; i < 8; i++) {
    if (node->hasChild(i)) {
      BVHTreeNode * testChild = node->getChild(i);
      //if the child is a leaf, we should skip this node, as leaf nodes cannot have
      //children
      if (testChild->isLeaf()) {
        continue;
      }
      //the child was not a leaf, so lets see if there is a bounding volume already in 
      //the octant of the current box which we can attach our new item to. this is 
      //common if the bounding volume of the node did not change, even after adding the 
      //item
      if (getBoxOctant(newBox, testChild->getPos()) == itemOctant) {
        node->updateParams(_writeBuffer);
        _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);
        addItemFromNode(testChild, item);
        return;
      }
    }
  }

  //finally, we arrive at the most expensive case, wherein we construct new bounding
  //volumes for all children by collapsing them into bounding volumes corresponding to
  //their octant

  //this octmap keeps track of each node we need to move, as well as it's octant.
  std::map<int, std::vector<BVHTreeNode*>> octMap;

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

  //now, for each entry in the octmap, we must create a new bounding volume, recursively
  //add each child to it, and then set it to the first free node in the current node. we
  //don't really have to worry about overflow here, since there can only be a maximum of
  //8 items
  for (auto const& x : octMap) {
    Pixel newAddress = _memoryBuffer.newItem();
    BVHTreeNode * newBoundingVolume = new BVHTreeNode(false, false, newAddress);
    for (int i = 0; i < x.second.size(); i++) {
      addItemFromNode(newBoundingVolume, x.second[i]);
    }
    newBoundingVolume->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(newBoundingVolume->getAddress().pointerIndex(), _writeBuffer);
    node->setChild(node->anyFree(), newBoundingVolume);
  }

  //write our finalized changes to this node before continuing
  node->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(node->getAddress().pointerIndex(), _writeBuffer);

  //lastly, we deal with our item. if there is a free space, it will just end up in this
  //bounding volume, but if there isn't a free space (odd case where every item was in a 
  //different octant) it will find it's octant and be moved there. either way, the
  //previous steps will have created a space for it somewhere.
  addItemFromNode(node, item);
}