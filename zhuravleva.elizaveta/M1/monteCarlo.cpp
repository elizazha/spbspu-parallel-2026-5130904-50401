#include "monteCarlo.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <random>
#include <limits>
#include <string>
#include <istream>
#include <vector>
#include <future>
#include <functional>

bool zhuravleva::isInside(const Circle& circle, double x, double y)
{
  const double dx = x - circle.x;
  const double dy = y - circle.y;
  const double rada = circle.radius;
  const double radb = circle.second == 0 ? rada : circle.second;
  return dx * dx / (rada * rada) + dy * dy / (radb * radb) <= 1.0;
}

zhuravleva::Bounds zhuravleva::getBounds(const std::vector< Circle >& circles)
{
  const Circle& first = circles.front();
  const double first_b = first.second == 0 ? first.radius : first.second;
  Bounds bounds{static_cast< double >(first.x) - first.radius, static_cast< double >(first.x) + first.radius,
      static_cast< double >(first.y) - first_b, static_cast< double >(first.y) + first_b};
  for (std::size_t i = 1; i < circles.size(); i++)
  {
    const Circle& circle = circles[i];
    const double b = circle.second == 0 ? circle.radius : circle.second;
    bounds.min_x = std::min(bounds.min_x, static_cast< double >(circle.x) - circle.radius);
    bounds.max_x = std::max(bounds.max_x, static_cast< double >(circle.x) + circle.radius);
    bounds.min_y = std::min(bounds.min_y, static_cast< double >(circle.y) - b);
    bounds.max_y = std::max(bounds.max_y, static_cast< double >(circle.y) + b);
  }
  return bounds;
}

std::vector< zhuravleva::Circle > zhuravleva::readCircles(std::istream& in)
{
  std::vector< Circle > circles;
  while (true)
  {
    int radius = 0, second = 0;
    int x = 0, y = 0;

    if (!(in >> radius))
    {
      if (in.eof())
      {
        break;
      }
      throw std::invalid_argument("Invalid circle input");
    }
    if (!(in >> second >> x >> y))
    {
      throw std::invalid_argument("Incomplete circle input");
    }
    if (radius <= 0 || second < 0)
    {
      throw std::invalid_argument("Invalid figure radius");
    }
    circles.push_back(Circle{radius, second, x, y});
  }
  return circles;
}

zhuravleva::Hits zhuravleva::checkPoint(const std::vector< Circle >& circles, double x, double y)
{
  bool in_any = false;
  bool in_all = true;

  for (const Circle& circle : circles)
  {
    const bool inside = isInside(circle, x, y);
    in_any = in_any || inside;
    in_all = in_all && inside;
  }

  return Hits{in_any, in_all};
}

zhuravleva::Hits zhuravleva::calcMonte(
    const std::vector< Circle >& circles, const Bounds& bounds, std::size_t tries, std::size_t seed)
{
  std::default_random_engine generator(seed);
  std::uniform_real_distribution< double > dist_x(bounds.min_x, bounds.max_x);
  std::uniform_real_distribution< double > dist_y(bounds.min_y, bounds.max_y);

  Hits total{0, 0};

  for (std::size_t i = 0; i < tries; ++i)
  {
    const double x = dist_x(generator);
    const double y = dist_y(generator);

    const Hits result = checkPoint(circles, x, y);
    total.any += result.any;
    total.all += result.all;
  }

  return total;
}

zhuravleva::Args zhuravleva::parseArgs(int argc, char* argv[])
{
  if (argc != 3 && argc != 4)
  {
    throw std::invalid_argument("Usage: lab threads tries [seed]");
  }

  const std::string threads_arg = argv[1];
  const std::string tries_arg = argv[2];
  const std::string seed_arg = argc == 4 ? argv[3] : "0";

  const auto parse_number = [](const std::string& str) -> std::size_t
  {
    if (str.empty() || str.find_first_not_of("0123456789") != std::string::npos)
    {
      throw std::invalid_argument("Invalid numeric argument");
    }

    const unsigned long long value = std::stoull(str);

    if (value > std::numeric_limits< std::size_t >::max())
    {
      throw std::out_of_range("Numeric argument is too large");
    }

    return static_cast< std::size_t >(value);
  };

  std::size_t threads = parse_number(threads_arg);
  const std::size_t tries = parse_number(tries_arg);
  const std::size_t seed = parse_number(seed_arg);

  if (tries == 0)
  {
    throw std::invalid_argument("Tries must be positive");
  }

  if (threads == 0)
  {
    threads = 1;
  }

  return Args{threads, tries, seed};
}

zhuravleva::Hits zhuravleva::calcParallMonte(const std::vector< Circle >& circles, const Bounds& bounds,
    std::size_t threads, std::size_t tries, std::size_t seed)
{
  std::vector< std::future< zhuravleva::Hits > > futures;
  const std::size_t workers = std::min(threads, tries);
  const std::size_t base = tries / workers;
  const std::size_t leftover = tries % workers;
  for (std::size_t i = 0; i < workers; i++)
  {
    const std::size_t count = base + (i < leftover ? 1 : 0);
    futures.push_back(
        std::async(std::launch::async, calcMonte, std::cref(circles), std::cref(bounds), count, seed + i));
  }
  zhuravleva::Hits total{0, 0};
  for (std::future< zhuravleva::Hits >& future : futures)
  {
    const zhuravleva::Hits result = future.get();
    total.any += result.any;
    total.all += result.all;
  }
  return total;
}
