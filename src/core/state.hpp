#pragma once
#ifndef State_hpp
#define State_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "../renderer/renderer.hpp"
#include <string>
#include <map>
#include <vector>
#include <utility>

class State {
public:
  State(sf::RenderWindow * _window, int w, int h);
  ~State();

  bool running;

  sf::Time deltaTime;
  sf::Time frameTime;

  sf::Time updateTime;
  sf::Time renderTime;

  sf::Clock deltaClock;

  int windowWidth;
  int windowHeight;

  Renderer rayMarcher;

  float getTargetFPS();
  void setTargetFPS(float target);
  std::string currentScene;

  sf::Time getTargetTime();

  void addCommand(std::string commandName);
  std::vector<std::string> commandHelps;

private:
  sf::RenderWindow * _window;
  sf::Time _targetTime;
  float _targetFPS;
};

#endif
