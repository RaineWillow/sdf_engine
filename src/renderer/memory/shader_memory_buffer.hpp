#pragma once
#ifndef Shader_Memory_Buffer_hpp
#define Shader_Memory_Buffer_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "memory_pixel.hpp"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

class ShaderMemoryBuffer {
public:
    ShaderMemoryBuffer(int itemsPerRow, int numRows, int itemSize, int id);
    ~ShaderMemoryBuffer();

    Pixel newItem();
    void freeItem(int index);
    void writeItem(int index, sf::Uint8 * itemArray);

    void reset();

    void bind(sf::Shader & shader, std::string bufferName);

    void update();

    void render(sf::RenderTexture & renderTarget);
private:
    int _id;
    int _itemSize;
    int _itemsPerRow;
    int _memoryBufferResolutionX;
    int _memoryBufferResolutionY;

    int numWrites = 0;

    sf::Texture _memoryBuffer;

    std::string _bufferName;

    std::vector<int> _freeIndices;

    std::vector<bool> _allIndices;

    Pixel _debugData;
    sf::Uint8 * _debugDataArray;
    std::vector<Pixel> _debugParams;

    std::unordered_map<int, sf::Uint8*> _uniqueWrites;
    std::unordered_map<int, sf::Uint8*> _bufferedWrite;
    std::unordered_set<int> _updates;

    std::chrono::milliseconds waitTime;
    std::chrono::time_point<std::chrono::steady_clock> last;
    std::chrono::time_point<std::chrono::steady_clock> currentTime;
    bool initialUpdate = false;
};

#endif
