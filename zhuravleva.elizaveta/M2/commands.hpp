#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "shapes.hpp"
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace zhuravleva
{
  struct Storage
  {
    std::map< std::string, Shape > shapes;
    std::map< std::string, std::vector< std::string > > sets;
  };
  bool addCircle(Storage& storage, const std::vector< std::string >& args);
  bool addEllipse(Storage& storage, const std::vector< std::string >& args);
  bool rotateShape(Storage& storage, const std::vector< std::string >& args);
  bool addSet(Storage& storage, const std::vector< std::string >& args);
  bool showShapes(const Storage& storage, const std::vector< std::string >& args, std::ostream& out);
  bool showSets(const Storage& storage, const std::vector< std::string >& args, std::ostream& out);
  bool showFrame(const Storage& storage, const std::vector< std::string >& args, std::ostream& out);
  bool showSetFrame(const Storage& storage, const std::vector< std::string >& args, std::ostream& out);
}

#endif
