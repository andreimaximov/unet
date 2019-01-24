#pragma once

#include <limits>
#include <random>

namespace unet {

// Return a random integer T in the range [min, max].
template <typename T>
T randInt(T min = std::numeric_limits<T>::min(),
          T max = std::numeric_limits<T>::max()) {
  static thread_local std::random_device rand;
  static thread_local std::mt19937 eng{rand()};
  std::uniform_int_distribution<T> dist{min, max};
  return dist(eng);
}

}  // namespace unet
