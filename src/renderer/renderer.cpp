#include "renderer.hpp"

Renderer::Renderer(int width, int height) : _marchDrawable(sf::Quads, 4) {
  if (!_marcher.loadFromFile("src/glsl/ray_marcher.frag", sf::Shader::Fragment)) {

  }
  sf::Vector2f uResolution(width, height);

  _marcher.setUniform("u_resolution", uResolution);
  _marcher.setUniform("numObjects", 0);
  _width = width;
  _height = height;

  _context.create(width, height);
  _context.setSmooth(false);

  _marchDrawable[0].position = sf::Vector2f(0, 0);
  _marchDrawable[1].position = sf::Vector2f(uResolution.x, 0);
  _marchDrawable[2].position = sf::Vector2f(uResolution.x, uResolution.y);
  _marchDrawable[3].position = sf::Vector2f(0, uResolution.y);
  _marchDrawable[0].color = sf::Color(0, 0, 0, 0);
  _marchDrawable[1].color = sf::Color(0, 0, 0, 0);
  _marchDrawable[2].color = sf::Color(0, 0, 0, 0);
  _marchDrawable[3].color = sf::Color(0, 0, 0, 0);

  _memoryBufferResolutionX = 4800;
  _memoryBufferResolutionY = 1000;

  _memoryBuffer.create(_memoryBufferResolutionX, _memoryBufferResolutionY);

  _marcher.setUniform("memoryBuffer", _memoryBuffer);
  _marcher.setUniform("memoryBufferResolution", sf::Glsl::Vec2(_memoryBufferResolutionX, _memoryBufferResolutionY));

  _memoryBufferUpdateArray = new sf::Uint8[_memoryBufferResolutionX*_memoryBufferResolutionY*4];
}

Renderer::~Renderer() {
  for (auto shape : _shapes) {
    delete shape;
  }
  _shapes.clear();

  delete[] _memoryBufferUpdateArray;
}

void Renderer::addShape(Shape * shape) {
}

void Renderer::destroyShape(Shape * shape) {
}

void Renderer::update() {
}

const sf::Texture & Renderer::render() {
  _context.draw(_marchDrawable, &_marcher);
  _context.display();
  return _context.getTexture();
}
