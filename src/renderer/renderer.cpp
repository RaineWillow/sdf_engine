#include "renderer.hpp"

Renderer::Renderer(int width, int height, float cameraDist) : 
_marchDrawable(sf::Quads, 4), 
_shapesContainer(1),
_BVHUnion(2),
_defaultCamera(Vector3(0.0, 0.0, cameraDist)) {
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

  _currentCamera = &_defaultCamera;
}

Renderer::~Renderer() {
}

void Renderer::loadMaterial(Material mat) {
  _shapesContainer.loadMaterial(mat);
}

void Renderer::overwriteMaterial(Material mat) {
  _shapesContainer.overwriteMaterial(mat);
}

void Renderer::removeMaterial(std::string name) {
  _shapesContainer.removeMaterial(name);
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

void Renderer::setMaterial(Shape * shape, std::string name) {
  _shapesContainer.setMaterial(shape, name);
}

std::vector<std::string> Renderer::getMaterials() {
  return _shapesContainer.getMaterials();
}

void Renderer::addLight(Light * light) {
  _shapesContainer.addLight(light, _marcher);
}

void Renderer::destroyLight(Light * light) {
  _shapesContainer.destroyLight(light, _marcher);
}

void Renderer::updateLight(Light * light) {
  _shapesContainer.updateLight(light);
}

Camera3d* Renderer::getCurrentCamera() {
  return _currentCamera;
}

void Renderer::setCurrentCamera(Camera3d * currentCamera) {
  _currentCamera = currentCamera;
}

void Renderer::resetToDefaultCamera() {
  _currentCamera = &_defaultCamera;
}

void Renderer::update() {
  _marcher.setUniform("cameraPosition", _currentCamera->loadCameraPosition());
  _marcher.setUniform("cameraForward", _currentCamera->loadCameraForward());
  _marcher.setUniform("cameraUp", _currentCamera->loadCameraUp());
  _marcher.setUniform("cameraRight", _currentCamera->loadCameraRight());
  _marcher.setUniform("cameraUp", _currentCamera->loadCameraUp());
  _shapesContainer.update();
  _BVHUnion.update();

  FinishHelperContext barrier;
}

const sf::Texture & Renderer::render() {
  _context.clear();
  _context.draw(_marchDrawable, &_marcher);
  _context.display();
  return _context.getTexture();
}
