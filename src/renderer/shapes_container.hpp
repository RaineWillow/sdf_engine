#pragma once
#ifndef Shapes_Container_hpp
#define Shapes_Container_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <string>
#include <utility>
#include <map>
#include "memory/memory_pixel.hpp"
#include "memory/shader_memory_buffer.hpp"
#include "shapes.hpp"

class ShapesContainer {
public:
  ShapesContainer(int memoryBufferId);
  ~ShapesContainer();

  void addShape(Shape * shape);
  void destroyShape(Shape * shape);
  void updateShape(Shape * shape);

  void bind(sf::Shader & shader, std::string bufferName);

  void update();

  void render(sf::RenderTexture & renderTarget);
private:
  ShaderMemoryBuffer _memoryBuffer;
  sf::Uint8 * _writeBuffer;
};
#endif