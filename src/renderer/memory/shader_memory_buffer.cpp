#include "shader_memory_buffer.hpp"

ShaderMemoryBuffer::ShaderMemoryBuffer(int itemsPerRow, int numRows, int itemSize, int id) {
    _itemSize = itemSize;
    _itemsPerRow = itemsPerRow;
    _memoryBufferResolutionX = itemsPerRow*itemSize;
    _memoryBufferResolutionY = numRows;

    _memoryBuffer.create(_memoryBufferResolutionX, _memoryBufferResolutionY);
    _memoryBuffer.setSmooth(false);

    for (int i = _itemsPerRow*_memoryBufferResolutionY-1; i>=0; i--) {
      _freeIndices.push_back(i);
      _allIndices.push_back(false);
    }

    for (int i = 0; i < _memoryBufferResolutionY; i++) {
      _bufferedWrite[i] = new sf::Uint8[_memoryBufferResolutionX*4];
    }

    _id = id;
}

ShaderMemoryBuffer::~ShaderMemoryBuffer() {
  /*
  for (auto const& x : _bufferedWrite) {
    sf::Uint8 * updateBuffer = x.second;
    delete[] updateBuffer;
    _bufferedWrite.erase(x.first);
  }
  */
}

Pixel ShaderMemoryBuffer::newItem() {
  if (_freeIndices.size() == 0) {
    throw std::overflow_error("Memory Buffer out of memory!");
  }
  int newIndex = _freeIndices.back();
  _allIndices[newIndex] = true;
  _freeIndices.pop_back();
  Pixel retPixel;
  retPixel.toPointer(newIndex, _id);
  return retPixel;
}

void ShaderMemoryBuffer::freeItem(int index) {
  if (!_allIndices[index]) {
    throw std::invalid_argument("The pointer " + std::to_string(index) + " was already free when delete was attempted.");
    return;
  }
  
  _freeIndices.push_back(index);
  _allIndices[index] = false;
}

void ShaderMemoryBuffer::writeItem(int index, sf::Uint8 * itemArray) {
  if (!_allIndices[index]) {
    throw std::invalid_argument("The pointer " + std::to_string(index) + " was free when write was attempted.");
    return;
  }

  numWrites += 1;
  
  int memoryY = index/_itemsPerRow;
  int memoryX = (index-memoryY*_itemsPerRow)*_itemSize;

  
  _uniqueWrites[index] = itemArray;
  //_updates.insert(memoryY);

  //int memoryY = index/_itemsPerRow;
  //int memoryX = (index-memoryY*_itemsPerRow)*_itemSize;

  //_memoryBuffer.update(itemArray, _itemSize, 1, memoryX, memoryY);

}

void ShaderMemoryBuffer::bind(sf::Shader & shader, std::string bufferName) {
  shader.setUniform(bufferName, _memoryBuffer);
  shader.setUniform(bufferName+"BufferResolution", sf::Glsl::Vec2(_memoryBufferResolutionX, _memoryBufferResolutionY));
  shader.setUniform(bufferName+"ItemSize", _itemSize);
  _bufferName = bufferName;
}

void ShaderMemoryBuffer::update() {
  
  //std::cout << _bufferName << ": " << _uniqueWrites.size() << std::endl;
  for (const auto& elem: _uniqueWrites) {
    
    int memoryY = elem.first/_itemsPerRow;
    int memoryX = (elem.first-memoryY*_itemsPerRow)*_itemSize;

    memcpy(&_bufferedWrite[memoryY][memoryX*4], elem.second, _itemSize*4*sizeof(sf::Uint8));
    _updates.insert(memoryY);
    _uniqueWrites.erase(elem.first);
  }

  for (const auto & elem: _updates) {
    
    _memoryBuffer.update(_bufferedWrite[elem], _itemsPerRow*_itemSize, 1, 0, elem);
    _updates.erase(elem);
  }


  numWrites = 0;

  
  
/*
  int numPrinted = 0;
  for (auto const& x : _bufferedWrite) {
    sf::Uint8 * updateBuffer = x.second;

    int memoryY = x.first/_itemsPerRow;
    int memoryX = (x.first-memoryY*_itemsPerRow)*_itemSize;
    _memoryBuffer.update(updateBuffer, _itemSize, 1, memoryX, memoryY);
    
    delete[] updateBuffer;
    _bufferedWrite.erase(x.first);
    numPrinted+=1;
  }
*/
  //std::cout << _bufferName << ": Number of updates: " << numPrinted << std::endl;
}

void ShaderMemoryBuffer::render(sf::RenderTexture & renderTarget) {
  sf::Sprite drawEnable;
  drawEnable.setTexture(_memoryBuffer);
  drawEnable.setTextureRect(sf::IntRect(0, 0, 300, 100));
  drawEnable.setPosition(sf::Vector2f(20, 20));
  sf::RectangleShape newRect(sf::Vector2f(300, 100));
  newRect.setPosition(sf::Vector2f(20, 20));
  newRect.setFillColor(sf::Color(255, 255, 255));
  renderTarget.draw(newRect);
  renderTarget.draw(drawEnable);
}
