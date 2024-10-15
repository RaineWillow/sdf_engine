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
#include <unordered_map>
#include "memory/memory_pixel.hpp"
#include "memory/shader_memory_buffer.hpp"
#include "shapes.hpp"
#include "material.hpp"

struct MaterialEntry {
  Material mat;
  std::vector<Shape*> shapeWithMat;
  MaterialEntry() : mat("") {}
};

class ShapesContainer {
public:
  ShapesContainer(int memoryBufferId);
  ~ShapesContainer();

  void loadMaterial(Material mat);
  void overwriteMaterial(Material mat);
  void removeMaterial(std::string name);

  void addShape(Shape * shape);
  void destroyShape(Shape * shape);
  void updateShape(Shape * shape);

  void setMaterial(Shape * shape, std::string name);
  std::vector<std::string> getMaterials();

  void bind(sf::Shader & shader, std::string bufferName);

  void update();

  void render(sf::RenderTexture & renderTarget);
private:
  ShaderMemoryBuffer _memoryBuffer;
  sf::Uint8 * _writeBuffer;

  std::unordered_map<std::string, MaterialEntry> _materials;

  Material _defaultMat;
};
#endif