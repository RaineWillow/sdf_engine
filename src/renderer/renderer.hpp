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

class Renderer {
public:
  Renderer(int width, int height);
  ~Renderer();

  void addShape(Shape * shape);
  void destroyShape(Shape * shape);

  void updateTest(int newNum);

  void update();
  const sf::Texture & render();
private:
  int _width;
  int _height;
  sf::Shader _marcher;
  sf::VertexArray _marchDrawable;
  sf::RenderTexture _context;

  std::vector<Shape*> _shapes;

  bool shapesDestroyed = false;

  int maxShapes = 10000;

  sf::Texture _memoryBuffer;

  sf::Uint8 * _memoryBufferUpdateArray;
};

#endif
