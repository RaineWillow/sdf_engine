#pragma once
#ifndef Render_tester_hpp
#define Render_tester_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System/Clock.hpp"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>

#include "scene.hpp"
#include "../core/state.hpp"
#include "../core/console.hpp"
#include "../renderer/shapes.hpp"
#include "../renderer/light.hpp"
#include "../renderer/camera_3d.hpp"
#include "../renderer/transform.hpp"
#include "../utils/vector3.hpp"
#include "../utils/quaternion.hpp"

class RenderTester : public Scene {
public:
  RenderTester(Console * inConsole, State * inState);
  ~RenderTester();
  void handleScreensizeChange();
  void handleEvent(sf::RenderWindow * window, sf::Event & event);
  void handleController();
  void update(sf::RenderWindow * window);
  void render(sf::RenderWindow * window);
private:
  Console * console;
  State * state;

  Sphere * testSphere;
  Sphere * testSphere2;
  Box * testBox;

  Transform testParent;

  Light * testLight1;
  Light * testLight2;

  sf::View _renderView;

  float testSphereOffset = 0.0;
  bool onOff = false;

  sf::Vector2i _center;
  sf::Vector2i _globalCenter;

  bool lockedMouse = false;

  std::vector<Sphere*> spheres;
  std::vector<Vector3> basePositions;
};

#endif /* end of include guard: Render_tester_hpp */
