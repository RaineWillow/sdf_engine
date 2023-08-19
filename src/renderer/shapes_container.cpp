#include "shapes_container.hpp"

ShapesContainer::ShapesContainer(int memoryBufferId) : _memoryBuffer(150, 7200, 48, memoryBufferId) {
  _writeBuffer = new sf::Uint8[48*4];
}

ShapesContainer::~ShapesContainer() {
  delete[] _writeBuffer;
}

void ShapesContainer::addShape(Shape * shape) {
  Pixel newAddress = _memoryBuffer.newItem();
  shape->setAddress(newAddress);
  shape->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::destroyShape(Shape * shape) {
  _memoryBuffer.freeItem(shape->getAddress().pointerIndex());
}

void ShapesContainer::updateShape(Shape * shape) {
  shape->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::bind(sf::Shader & shader, std::string bufferName) {
  _memoryBuffer.bind(shader, bufferName);
}

void ShapesContainer::render(sf::RenderTexture & renderTarget) {
  _memoryBuffer.render(renderTarget);
}
