#include "shader_memory_buffer.hpp"

ShaderMemoryBuffer::ShaderMemoryBuffer(int itemsPerRow, int numRows, int itemSize) {
    _itemSize = itemSize;
    _itemsPerRow = itemsPerRow;
    _memoryBufferResolutionX = itemsPerRow*itemSize;
    _memoryBufferResolutionY = numRows;

    _memoryBuffer.create(_memoryBufferResolutionX, _memoryBufferResolutionY);

    for (int i = _memoryBufferResolutionX*_memoryBufferResolutionY-1; i>=0; i--) {
      _freeIndices.push_back(i);
    }
}

int ShaderMemoryBuffer::newItem(sf::Uint8 * itemArray) {
  int newIndex = _freeIndices.back();
  _freeIndices.pop_back();
  int memoryY = newIndex/_itemsPerRow;
  int memoryX = (newIndex-memoryY*_itemsPerRow)*_itemSize;

  _memoryBuffer.update(itemArray, _itemSize, 1, memoryX, memoryY);
  return newIndex;
}

void ShaderMemoryBuffer::freeItem(int index) {
  if (std::find(_freeIndices.begin(), _freeIndices.end(), index) != _freeIndices.end()) {
    throw std::invalid_argument("The pointer " + std::to_string(index) + " was already free!");
    return;
  }
  
  _freeIndices.push_back(index);
}

void ShaderMemoryBuffer::writeItem(int index, sf::Uint8 * itemArray) {
  if (std::find(_freeIndices.begin(), _freeIndices.end(), index) != _freeIndices.end()) {
    throw std::invalid_argument("The pointer " + std::to_string(index) + " is free and you attempted to write to it!");
    return;
  }

  int memoryY = index/_itemsPerRow;
  int memoryX = (index-memoryY*_itemsPerRow)*_itemSize;

  _memoryBuffer.update(itemArray, _itemSize, 1, memoryX, memoryY);
}

void ShaderMemoryBuffer::bind(sf::Shader & shader, std::string bufferName) {
  shader.setUniform(bufferName, _memoryBuffer);
  shader.setUniform(bufferName+"BufferResolution", sf::Glsl::Vec2(_memoryBufferResolutionX, _memoryBufferResolutionY));
  shader.setUniform(bufferName+"ItemSize", _itemSize);
}