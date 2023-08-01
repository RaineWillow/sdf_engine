#pragma once
#ifndef BVH_Tree_Hpp
#define BVH_Tree_Hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "memory/shader_memory_buffer.hpp"
#include "BVH_tree_node.hpp"

template <typename T>
class BVHTree {
public:
  BVHTree(int itemSize, ShaderMemoryBuffer & memoryBuffer);
  ~BVHTree();

  void addItemFromNode(BVHTreeNode * node, BVHTreeNode * item);
  void addItemToRoot(BVHTreeNode * item);
  void destroyNode(BVHTreeNode * node);
  void updateNode(BVHTreeNode * item);


  void addItem(T * item);
  void destroyItem(T * item);

private:
  BVHTreeNode<T> * root;
  int _itemSize;
  ShaderMemoryBuffer & _memoryBuffer;
};

#endif