#include "state.hpp"
State::State(sf::RenderWindow * window, int w, int h) : rayMarcher(480, 270) {
  running = true;
  windowWidth = w;
  windowHeight = h;
  _window = window;
}

State::~State() {

}

float State::getTargetFPS() {
  return _targetFPS;
}

void State::setTargetFPS(float target) {
  _targetFPS = target;
  _targetTime = sf::seconds(1.f/_targetFPS-0.0000021);
  _window->setFramerateLimit(_targetFPS*2);
}

sf::Time State::getTargetTime() {
  return _targetTime;
}

void State::addCommand(std::string commandName) {
  commandHelps.push_back(commandName);
}
