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

  _memoryBuffer.create(4800, 200);

  _marcher.setUniform("memoryBuffer", _memoryBuffer);
  _marcher.setUniform("memoryBufferResolution", sf::Glsl::Vec2(4800, 200));

  _memoryBufferUpdateArray = new sf::Uint8[4800*200*4];
}

Renderer::~Renderer() {
  for (auto shape : _shapes) {
    delete shape;
  }
  _shapes.clear();

  delete _memoryBufferUpdateArray;
}

void Renderer::addShape(Shape * shape) {
  if (_shapes.size() >= maxShapes) {
    return;
  }
  _shapes.push_back(shape);
  _marcher.setUniform("numObjects", (int)_shapes.size());
  shape->apply((int)_shapes.size()-1, _memoryBufferUpdateArray, 4800*200*4, -1, -1);
}

void Renderer::destroyShape(Shape * shape) {
  shape->destroy();
  shapesDestroyed=true;
}

void Renderer::update() {
  if (shapesDestroyed) {
    int position = 0;
    for (int i = 0; i < _shapes.size(); i++) {
      if (_shapes[i]->destroyed()) {
        delete _shapes[i];
        _shapes.erase(_shapes.begin() + i);
      } else {
        _shapes[i]->apply(position, _memoryBufferUpdateArray, 4800*200*4, -1, -1);
        position += 1;
      }
    }
    _marcher.setUniform("numObjects", (int)_shapes.size());
    return;
  }

  for (int i = 0; i < _shapes.size(); i++) {
    if (_shapes[i]->requiresUpdate()) {
      _shapes[i]->apply(i, _memoryBufferUpdateArray, 4800*200*4, -1, -1);
    }
  }

  _memoryBuffer.update(_memoryBufferUpdateArray);
}

const sf::Texture & Renderer::render() {
  _context.draw(_marchDrawable, &_marcher);
  _context.display();
  return _context.getTexture();
}
