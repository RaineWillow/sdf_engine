#pragma once
#ifndef Light_hpp
#define Light_hpp

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "memory/memory_pixel.hpp"
#include "memory/writable.hpp"
#include <vector>
#include <string>

class Light : public Writable {
public:
    Light();
    ~Light();

    void setAddress(Pixel address);
    Pixel getAddress();

    int getParamsSize();

    void updateParams(sf::Uint8 * &dataArray);

    void setPosition(sf::Glsl::Vec3 position);
    void setColor(sf::Glsl::Vec3 color);
    void setIntensity(float intensity);
    void setAttenuation(sf::Glsl::Vec3 attenuation);
    void setNext(Pixel address);


    sf::Glsl::Vec3 getPosition();
    sf::Glsl::Vec3 getColor();
    float getIntensity();
    sf::Glsl::Vec3 getAttenuation();
    Pixel getNext();

private:
    sf::Uint8 * _writeData;
    Pixel _address;

    std::vector<Pixel> _params;
    int _paramsSize;

    sf::Glsl::Vec3 _position;
    sf::Glsl::Vec3 _color;
    float _intensity;
    sf::Glsl::Vec3 _attenuation;
    Pixel _next;
};

#endif