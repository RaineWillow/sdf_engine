#pragma once
#ifndef Render_tester_hpp
#define Render_tester_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System/Clock.hpp"
#include <cmath>
#include <vector>
#include <cstdlib>

#include "scene.hpp"
#include "../core/state.hpp"
#include "../core/console.hpp"
#include "../renderer/shapes.hpp"

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

  sf::View _renderView;

  float testSphereOffset = 0.0;

  std::vector<Sphere*> spheres;
};

#endif /* end of include guard: Render_tester_hpp */
