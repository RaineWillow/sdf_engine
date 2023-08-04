#pragma once
#ifndef Shader_Memory_Buffer_hpp
#define Shader_Memory_Buffer_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "memory_pixel.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>

class ShaderMemoryBuffer {
public:
    ShaderMemoryBuffer(int itemsPerRow, int numRows, int itemSize, int id);

    Pixel newItem();
    void freeItem(int index);
    void writeItem(int index, sf::Uint8 * itemArray);

    void bind(sf::Shader & shader, std::string bufferName);
private:
    int _id;
    int _itemSize;
    int _itemsPerRow;
    int _memoryBufferResolutionX;
    int _memoryBufferResolutionY;
    sf::Texture _memoryBuffer;

    std::vector<int> _freeIndices;
};


#endif