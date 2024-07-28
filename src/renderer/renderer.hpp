#pragma once
#ifndef Renderer_hpp
#define Renderer_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "shapes.hpp"
#include <vector>
#include "SFML/OpenGL.hpp"
#include <iostream>
#include "shapes_container.hpp"
#include "BVH_tree.hpp"

class Renderer {
public:
  Renderer(int width, int height);
  ~Renderer();

  void addShape(Shape * shape);
  void destroyShape(Shape * shape);
  void updateShape(Shape * shape);

  void update();

  std::string outTree() {
    return _BVHUnion.drawTree();
  }
  const sf::Texture & render();

private:

  int _width;
  int _height;
  sf::Shader _marcher;
  sf::VertexArray _marchDrawable;
  sf::RenderTexture _context;
  int numShapes = 0;

  bool shapesDestroyed = false;

  int maxShapes = 10000;
  ShapesContainer _shapesContainer;
  BVHTree _BVHUnion;
};

#endif
