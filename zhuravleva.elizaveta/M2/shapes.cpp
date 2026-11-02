#include "shapes.hpp"
#include <cmath>

zhuravleva::Rectangle zhuravleva::getBounds(const Shape& shape)
{
  const double cos_a = std::cos(shape.angle);
  const double sin_a = std::sin(shape.angle);
  const double rx = shape.rx;
  const double ry = shape.ry;
  const double half_width = std::sqrt(rx * rx * cos_a * cos_a + ry * ry * sin_a * sin_a);
  const double half_height = std::sqrt(rx * rx * sin_a * sin_a + ry * ry * cos_a * cos_a);

  return Rectangle{shape.x - half_width, shape.x + half_width, shape.y - half_height, shape.y + half_height};
}

bool zhuravleva::isInside(double x, double y, const Shape& shape)
{
  const double dx = x - shape.x;
  const double dy = y - shape.y;
  const double cos_a = std::cos(shape.angle);
  const double sin_a = std::sin(shape.angle);
  const double u = (dx * cos_a + dy * sin_a) / shape.rx;
  const double v = (dy * cos_a - dx * sin_a) / shape.ry;

  return u * u + v * v <= 1.0;
}
