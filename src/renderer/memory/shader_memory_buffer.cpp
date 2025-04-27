#include "shader_memory_buffer.hpp"


ShaderMemoryBuffer::ShaderMemoryBuffer(int itemsPerRow, int numRows, int itemSize, int id) : 
waitTime(8) {
    _itemSize = itemSize;
    _itemsPerRow = itemsPerRow;
    _memoryBufferResolutionX = itemsPerRow*itemSize;
    _memoryBufferResolutionY = numRows;

    last = std::chrono::steady_clock::now();

    const_cast<sf::Texture&>(_memoryBuffer.getTexture()).setSrgb(false);

    _memoryBuffer.create(_memoryBufferResolutionX, _memoryBufferResolutionY);
    _memoryBuffer.setSmooth(false);

    std::cout << const_cast<sf::Texture&>(_memoryBuffer.getTexture()).isSrgb() << std::endl;

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
  
  for (auto const& x : _bufferedWrite) {
    sf::Uint8 * updateBuffer = x.second;
    delete[] updateBuffer;
  }
  _bufferedWrite.clear();
  _bufferedWrite.clear();
}

Pixel ShaderMemoryBuffer::newItem() {
  if (_freeIndices.size() == 0) {
    throw std::overflow_error("Memory Buffer" + _bufferName + " out of memory!");
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
  //std::cout << "Data write: " << (void *)_uniqueWrites[index] << std::endl;
  //_updates.insert(memoryY);

  //int memoryY = index/_itemsPerRow;
  //int memoryX = (index-memoryY*_itemsPerRow)*_itemSize;

  //_memoryBuffer.update(itemArray, _itemSize, 1, memoryX, memoryY);

}

void ShaderMemoryBuffer::bind(sf::Shader & shader, std::string bufferName) {
  shader.setUniform(bufferName, _memoryBuffer.getTexture());
  shader.setUniform(bufferName+"BufferResolution", sf::Glsl::Vec2(_memoryBufferResolutionX, _memoryBufferResolutionY));
  shader.setUniform(bufferName+"ItemSize", _itemSize);
  _bufferName = bufferName;
}

void ShaderMemoryBuffer::update() {
  //TODO::GET RID OF THIS
  /*
  this is a temporary fix -- each time this runs, there is a call to the gpu to
  write a lot of data and also flush the gpu write buffer, which is calling often 
  multiple times per frame. At 60 fps, that's not a lot, but at say, 3000, that's a
  significant number of writes and flushes, which can lead to completely flooding
  the PCI write bus. So, by default, it is locked to actually updating once
  every 8 milliseconds, preventing this issue. However, in the future, more
  performant code that creates less writes, and a rewrite of the library function 
  to add a flag to prevent GPU flushes before it is done writing may see an decrease
  in wait times or even removal of this limit.
  */
  currentTime = std::chrono::steady_clock::now();
  if (!((currentTime-last) > waitTime) && initialUpdate) {
    initialUpdate = true;
    return;
  }
  //std::cout << _bufferName << ": " << _uniqueWrites.size() << std::endl;
  for (const auto& elem: _uniqueWrites) {
    
    int memoryY = elem.first/_itemsPerRow;
    int memoryX = (elem.first-memoryY*_itemsPerRow)*_itemSize;

    //std::cout << "Got here: " << elem.second << std::endl;

    memcpy(&_bufferedWrite[memoryY][memoryX*4], elem.second, _itemSize*4*sizeof(sf::Uint8));

    //std::cout << "Didn't get here" << std::endl;
    
    _updates.insert(memoryY);
    //_uniqueWrites.erase(elem.first);
  }

  _uniqueWrites.clear();

  for (const auto & elem: _updates) {
    const_cast<sf::Texture&>(_memoryBuffer.getTexture()).update(_bufferedWrite[elem], _itemsPerRow*_itemSize, 1, 0, elem);
    //_updates.erase(elem);
  }

  _updates.clear();


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
  drawEnable.setTexture(_memoryBuffer.getTexture());
  drawEnable.setTextureRect(sf::IntRect(0, 0, 300, 100));
  drawEnable.setPosition(sf::Vector2f(20, 20));
  sf::RectangleShape newRect(sf::Vector2f(300, 100));
  newRect.setPosition(sf::Vector2f(20, 20));
  newRect.setFillColor(sf::Color(255, 255, 255));
  renderTarget.draw(newRect);
  renderTarget.draw(drawEnable);
}
