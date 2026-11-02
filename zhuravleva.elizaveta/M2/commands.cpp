#include "commands.hpp"
#include "parse.hpp"
#include "monte_carlo.hpp"
#include <cctype>
#include <ostream>

bool zhuravleva::addCircle(Storage& storage, const std::vector< std::string >& args)
{
  constexpr std::size_t circle_arg_count = 5;
  if (args.size() != circle_arg_count)
  {
    return false;
  }
  int radius = 0;
  int x = 0;
  int y = 0;
  if (!parseInt(args[2], radius) || !parseInt(args[3], x) || !parseInt(args[4], y) || radius <= 0
      || (args[1].empty() || !std::isalpha(static_cast< unsigned char >(args[1][0]))))
  {
    return false;
  }
  const Shape shape{ShapeType::circle, radius, radius, x, y, 0.0};
  return storage.shapes.emplace(args[1], shape).second;
}

bool zhuravleva::addEllipse(Storage& storage, const std::vector< std::string >& args)
{
  constexpr std::size_t ellipse_arg_count = 6;
  constexpr std::size_t ellipse_y_index = 5;
  if (args.size() != ellipse_arg_count)
  {
    return false;
  }
  int rx = 0;
  int ry = 0;
  int x = 0;
  int y = 0;

  if (!parseInt(args[2], rx) || !parseInt(args[3], ry) || !parseInt(args[4], x) || !parseInt(args[ellipse_y_index], y)
      || rx <= 0 || ry <= 0 || (args[1].empty() || !std::isalpha(static_cast< unsigned char >(args[1][0]))))
  {
    return false;
  }
  const Shape shape{ShapeType::ellipse, rx, ry, x, y, 0.0};
  return storage.shapes.emplace(args[1], shape).second;
}

bool zhuravleva::rotateShape(Storage& storage, const std::vector< std::string >& args)
{
  if (args.size() != 3)
  {
    return false;
  }

  double angle = 0.0;
  if (!parseDouble(args[2], angle))
  {
    return false;
  }

  auto it = storage.shapes.find(args[1]);
  if (it == storage.shapes.end())
  {
    return false;
  }
  it->second.angle += angle;
  return true;
}

bool zhuravleva::addSet(Storage& storage, const std::vector< std::string >& args)
{
  if (args.size() < 3)
  {
    return false;
  }
  int size = 0;
  if (!parseInt(args[2], size) || size <= 0)
  {
    return false;
  }
  if (static_cast< std::size_t >(size) != args.size() - 3)
  {
    return false;
  }
  if (storage.sets.find(args[1]) != storage.sets.end())
  {
    return false;
  }
  std::vector< std::string > names;
  for (std::size_t i = 3; i < args.size(); ++i)
  {
    if (storage.shapes.find(args[i]) == storage.shapes.end())
    {
      return false;
    }
    names.push_back(args[i]);
  }
  return storage.sets.emplace(args[1], names).second;
}

bool zhuravleva::showShapes(const Storage& storage, const std::vector< std::string >& args, std::ostream& out)
{
  if (args.size() != 2)
  {
    return false;
  }
  const auto it = storage.shapes.find(args[1]);
  if (it == storage.shapes.end())
  {
    return false;
  }
  const Shape& shape = it->second;
  if (shape.type == ShapeType::circle)
  {
    out << shape.rx << " (" << shape.x << ' ' << shape.y << ")\n";
  }
  else
  {
    out << shape.rx << ' ' << shape.ry << " (" << shape.x << ' ' << shape.y << ") " << shape.angle << '\n';
  }
  return true;
}

bool zhuravleva::showSets(const Storage& storage, const std::vector< std::string >& args, std::ostream& out)
{
  if (args.size() != 2)
  {
    return false;
  }
  const auto it = storage.sets.find(args[1]);
  if (it == storage.sets.end())
  {
    return false;
  }
  for (const std::string& name : it->second)
  {
    const auto shape_it = storage.shapes.find(name);
    if (shape_it == storage.shapes.end())
    {
      return false;
    }
    const Shape& shape = shape_it->second;
    if (shape.type == ShapeType::circle)
    {
      out << shape.rx << " (" << shape.x << ' ' << shape.y << ")\n";
    }
    else
    {
      out << shape.rx << ' ' << shape.ry << " (" << shape.x << ' ' << shape.y << ") " << shape.angle << '\n';
    }
  }
  return true;
}

bool zhuravleva::showFrame(const Storage& storage, const std::vector< std::string >& args, std::ostream& out)
{
  if (args.size() != 2)
  {
    return false;
  }
  const auto it = storage.shapes.find(args[1]);
  if (it == storage.shapes.end())
  {
    return false;
  }
  const Rectangle bounds = getBounds(it->second);
  out << '(' << bounds.min_x << ' ' << bounds.min_y << ") (";
  out << bounds.max_x << ' ' << bounds.max_y << ")\n";
  return true;
}

bool zhuravleva::showSetFrame(const Storage& storage, const std::vector< std::string >& args, std::ostream& out)
{
  if (args.size() != 2)
  {
    return false;
  }
  const auto it = storage.sets.find(args[1]);
  if (it == storage.sets.end() || it->second.empty())
  {
    return false;
  }

  std::vector< Shape > shapes;
  for (const std::string& name : it->second)
  {
    const auto shape_it = storage.shapes.find(name);
    if (shape_it == storage.shapes.end())
    {
      return false;
    }
    shapes.push_back(shape_it->second);
  }
  const Rectangle bounds = getSetBounds(shapes);
  out << '(' << bounds.min_x << ' ' << bounds.min_y << ") (";
  out << bounds.max_x << ' ' << bounds.max_y << ")\n";
  return true;
}
