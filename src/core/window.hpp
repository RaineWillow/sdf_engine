#pragma once
#ifndef Window_hpp
#define Window_hpp
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include "state.hpp"
#include "console.hpp"
#include "../renderer/shapes.hpp"
#include "../renderer/renderer.hpp"
#include "../scene/scene.hpp"
#include "../scene/render_tester.hpp"
#include <math.h>
#include "SFML/System.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"

class Window {
public:
  Window(int w, int h, std::string title);
  ~Window();
  void run();

private:
  State * _state;
  sf::RenderWindow * _window;
  Console * console;
  std::map<std::string, Scene*> scenes;

  float movement = 0;
};

#endif
