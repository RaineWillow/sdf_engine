#pragma once
#ifndef State_hpp
#define State_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "../renderer/renderer.hpp"
#include "../utils/file.hpp"
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <utility>

struct FrameCounter {
  int64_t deltaTime;
  float fps;
};

class State {
public:
  State(sf::RenderWindow * _window, int w, int h);
  ~State();

  bool running;

  sf::Time deltaTime;
  sf::Time frameTime;

  sf::Time updateTime;
  sf::Time renderTime;


  int currentSecond;

  sf::Clock deltaClock;

  sf::Clock elapsedClock;

  std::vector<FrameCounter> collectedTimes;

  int windowWidth;
  int windowHeight;

  Renderer rayMarcher;

  File outFile;

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
