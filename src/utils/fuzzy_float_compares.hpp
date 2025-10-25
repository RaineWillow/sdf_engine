#pragma once
#ifndef Fuzzy_Float_Compares_hpp
#define Fuzzy_Float_Compares_hpp
#include <cmath>

constexpr float FUZZY_EPSILON = 0.0001f;

inline bool fuzzyEquals(float rhs, float lhs) {
  return std::abs(rhs-lhs) < FUZZY_EPSILON;
}

inline bool fuzzyLTE(float rhs, float lhs) {
  return (rhs-lhs) < FUZZY_EPSILON;
}

inline bool fuzzyGTE(float rhs, float lhs) {
  return (rhs-lhs) > -FUZZY_EPSILON;
}

inline bool fuzzyLT(float rhs, float lhs) {
  return (lhs-rhs) > FUZZY_EPSILON;
}

inline bool fuzzyGT(float rhs, float lhs) {
  return (rhs-lhs) > FUZZY_EPSILON;
}


#endif