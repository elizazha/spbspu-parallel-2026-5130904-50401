#ifndef SHAPES_HPP
#define SHAPES_HPP

namespace zhuravleva
{
  enum class ShapeType
  {
    circle,
    ellipse
  };

  struct Shape
  {
    ShapeType type;
    int rx;
    int ry;
    int x;
    int y;
    double angle;
  };

  struct Rectangle
  {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
  };

  Rectangle getBounds(const Shape& shape);
  bool isInside(double x, double y, const Shape& shape);
}

#endif
