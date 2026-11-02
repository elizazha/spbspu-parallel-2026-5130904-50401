#include "monte_carlo.hpp"
#include <algorithm>
#include <stdexcept>
#include <random>
#include <future>
#include <functional>

zhuravleva::Rectangle zhuravleva::getSetBounds(const std::vector< Shape >& shapes)
{
  if (shapes.empty())
  {
    throw std::invalid_argument("Empty shape set");
  }

  Rectangle bounds = getBounds(shapes.front());

  for (std::size_t i = 1; i < shapes.size(); ++i)
  {
    const Rectangle current = getBounds(shapes[i]);

    bounds.min_x = std::min(bounds.min_x, current.min_x);
    bounds.max_x = std::max(bounds.max_x, current.max_x);
    bounds.min_y = std::min(bounds.min_y, current.min_y);
    bounds.max_y = std::max(bounds.max_y, current.max_y);
  }
  return bounds;
}

bool zhuravleva::checkPoint(const std::vector< Shape >& shapes, double x, double y)
{
  for (const Shape& shape : shapes)
  {
    if (isInside(x, y, shape))
    {
      return true;
    }
  }
  return false;
}

std::size_t zhuravleva::calcMonte(
    const std::vector< Shape >& shapes, const Rectangle& bounds, std::size_t tries, std::size_t seed)
{
  std::default_random_engine generator(seed);
  const double range = static_cast< double >(generator.max()) - generator.min() + 1.0;
  const double scale_x = (bounds.max_x - bounds.min_x) / range;
  const double scale_y = (bounds.max_y - bounds.min_y) / range;
  std::size_t hits = 0;
  const bool single_circle = shapes.size() == 1 && shapes.front().type == ShapeType::circle;
  const double center_x = single_circle ? shapes.front().x : 0.0;
  const double center_y = single_circle ? shapes.front().y : 0.0;
  const double radius = single_circle ? shapes.front().rx : 0.0;
  const double radius_squared = radius * radius;
  for (std::size_t i = 0; i < tries; ++i)
  {
    const double x = bounds.min_x + (generator() - generator.min()) * scale_x;
    const double y = bounds.min_y + (generator() - generator.min()) * scale_y;
    if (single_circle)
    {
      const double dx = x - center_x;
      const double dy = y - center_y;
      if (dx * dx + dy * dy <= radius_squared)
      {
        ++hits;
      }
    }
    else if (checkPoint(shapes, x, y))
    {
      ++hits;
    }
  }
  return hits;
}

std::size_t zhuravleva::calcParallMonte(const std::vector< Shape >& shapes, const Rectangle& bounds,
    std::size_t threads, std::size_t tries, std::size_t seed)
{
  const std::size_t workers = std::min(threads, tries);
  const std::size_t base = tries / workers;
  const std::size_t leftover = tries % workers;
  std::vector< std::future< std::size_t > > futures;

  for (std::size_t i = 0; i < workers; ++i)
  {
    const std::size_t count = base + (i < leftover ? 1 : 0);

    futures.push_back(std::async(std::launch::async, calcMonte, std::cref(shapes), std::cref(bounds), count, seed + i));
  }
  std::size_t total_hits = 0;

  for (std::future< std::size_t >& future : futures)
  {
    total_hits += future.get();
  }
  return total_hits;
}

double zhuravleva::calculateArea(
    const std::vector< Shape >& shapes, std::size_t threads, std::size_t tries, std::size_t seed)
{
  if (threads == 0 || tries == 0)
  {
    throw std::invalid_argument("Invalid Monte Carlo parameters");
  }

  if (shapes.empty())
  {
    return 0.0;
  }

  const Rectangle bounds = getSetBounds(shapes);
  const double width = bounds.max_x - bounds.min_x;
  const double height = bounds.max_y - bounds.min_y;
  const double area = width * height;
  const std::size_t hits = calcParallMonte(shapes, bounds, threads, tries, seed);

  return area * static_cast< double >(hits) / tries;
}
