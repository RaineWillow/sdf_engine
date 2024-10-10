#include "renderer.hpp"

Renderer::Renderer(int width, int height) : 
_marchDrawable(sf::Quads, 4), 
_shapesContainer(1),
_BVHUnion(2) {
  if (!_marcher.loadFromFile("src/glsl/ray_marcher.frag", sf::Shader::Fragment)) {

  }
  sf::Vector2f uResolution(width, height);

  _marcher.setUniform("u_resolution", uResolution);
  //_marcher.setUniform("numObjects", 0);
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
  _shapesContainer.bind(_marcher, "shapes");
  _BVHUnion.bind(_marcher, "BVHUnion");
}

Renderer::~Renderer() {
}

void Renderer::addShape(Shape * shape) {
  //shape->updateBoundingBox();
  _shapesContainer.addShape(shape);
  shape->setBVHTreeNode(_BVHUnion.addLeaf(shape->getAddress(), shape->getPos(), shape->getBound()));
  //std::cout << numShapes << std::endl;
  //std::cout << _BVHUnion.drawTree() << std::endl;
}

void Renderer::destroyShape(Shape * shape) {
  _shapesContainer.destroyShape(shape);
  _BVHUnion.destroyNode(shape->getBVHTreeNode());
  //std::cout << _BVHUnion.drawTree() << std::endl;
}

void Renderer::updateShape(Shape * shape) {
  //shape->updateBoundingBox();
  _shapesContainer.updateShape(shape);
  _BVHUnion.updateLeaf(shape->getBVHTreeNode(), shape->getPos(), shape->getBound());
  //std::cout << _BVHUnion.drawTree() << std::endl;
}

void Renderer::update() {
  
  _shapesContainer.update();
  _BVHUnion.update();

}

const sf::Texture & Renderer::render() {
  _context.clear();
  _context.draw(_marchDrawable, &_marcher);
  _context.display();
  return _context.getTexture();
}
