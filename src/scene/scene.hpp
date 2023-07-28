#pragma once
#ifndef Scene_hpp
#define Scene_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "../core/state.hpp"
#include "../core/console.hpp"

class Scene {
public:
  virtual void handleScreensizeChange()=0;
  virtual void handleEvent(sf::RenderWindow * window, sf::Event & event)=0;
  virtual void handleController()=0;
  virtual void update(sf::RenderWindow * window)=0;
  virtual void render(sf::RenderWindow * window)=0;
private:
};
#endif /* end of include guard:  */
