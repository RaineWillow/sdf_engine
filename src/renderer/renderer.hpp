#pragma once
#ifndef Renderer_hpp
#define Renderer_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "shapes.hpp"
#include <vector>
#include <string>
#include "SFML/OpenGL.hpp"
#include <iostream>
#include "shapes_container.hpp"
#include "BVH_tree.hpp"
#include "material.hpp"
#include "light.hpp"
#include "camera_3d.hpp"
#include <chrono>

class Renderer {
public:
  Renderer(int width, int height, float cameraDist = 20.0);
  ~Renderer();

  void loadMaterial(Material mat);
  void overwriteMaterial(Material mat);
  void removeMaterial(std::string name);

  void addShape(Shape * shape);
  void destroyShape(Shape * shape);
  void updateShape(Shape * shape);

  void setMaterial(Shape * shape, std::string name);
  std::vector<std::string> getMaterials();

  void addLight(Light * light);
  void destroyLight(Light * light);
  void updateLight(Light * light);

  Camera3d* getCurrentCamera();
  void setCurrentCamera(Camera3d * currentCamera);
  void resetToDefaultCamera();

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

  Camera3d _defaultCamera;

  Camera3d * _currentCamera;
};

#endif
