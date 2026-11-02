#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include "shapes.hpp"
#include <vector>
#include <cstddef>

namespace zhuravleva
{
  Rectangle getSetBounds(const std::vector< Shape >& shapes);
  bool checkPoint(const std::vector< Shape >& shapes, double x, double y);
  std::size_t calcMonte(
      const std::vector< Shape >& shapes, const Rectangle& bounds, std::size_t tries, std::size_t seed);
  std::size_t calcParallMonte(const std::vector< Shape >& shapes, const Rectangle& bounds, std::size_t threads,
      std::size_t tries, std::size_t seed);
  double calculateArea(const std::vector< Shape >& shapes, std::size_t threads, std::size_t tries, std::size_t seed);
}

#endif
