#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include <vector>
#include <istream>
#include <cstddef>

namespace zhuravleva
{
  struct Circle
  {
    int radius, second, x, y;
  };

  struct Bounds
  {
    double min_x, max_x, min_y, max_y;
  };

  struct Hits
  {
    std::size_t any;
    std::size_t all;
  };

  struct Args
  {
    std::size_t threads;
    std::size_t tries;
    std::size_t seed;
  };

  bool isInside(const Circle& circle, double x, double y);
  Bounds getBounds(const std::vector< Circle >& circles);
  std::vector< Circle > readCircles(std::istream& in);
  Hits checkPoint(const std::vector< Circle >& circles, double x, double y);
  Hits calcMonte(const std::vector< Circle >& circles, const Bounds& bounds, std::size_t tries, std::size_t seed);
  Args parseArgs(int argc, char* argv[]);
  Hits calcParallMonte(const std::vector< Circle >& circles, const Bounds& bounds, std::size_t threads,
      std::size_t tries, std::size_t seed);
}

#endif
