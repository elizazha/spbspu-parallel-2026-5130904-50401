#include "parse.hpp"
#include <cctype>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

std::vector< std::string > zhuravleva::splitLine(const std::string& line)
{
  std::vector< std::string > words;
  std::size_t i = 0;
  while (i < line.size())
  {
    while (i < line.size() && std::isspace(static_cast< unsigned char >(line[i])))
    {
      ++i;
    }

    const std::size_t start = i;
    while (i < line.size() && !std::isspace(static_cast< unsigned char >(line[i])))
    {
      ++i;
    }
    if (start < i)
    {
      words.push_back(line.substr(start, i - start));
    }
  }
  return words;
}

bool zhuravleva::parseInt(const std::string& text, int& value)
{
  try
  {
    std::size_t pos = 0;
    const long long number = std::stoll(text, &pos);
    if (pos != text.size())
    {
      return false;
    }
    if (number < std::numeric_limits< int >::min() || number > std::numeric_limits< int >::max())
    {
      return false;
    }
    value = static_cast< int >(number);
    return true;
  }
  catch (const std::invalid_argument&)
  {
    return false;
  }
  catch (const std::out_of_range&)
  {
    return false;
  }
}

bool zhuravleva::parseDouble(const std::string& text, double& value)
{
  try
  {
    std::size_t pos = 0;
    const double number = std::stod(text, &pos);
    if (pos != text.size() || !std::isfinite(number))
    {
      return false;
    }
    value = number;
    return true;
  }
  catch (const std::invalid_argument&)
  {
    return false;
  }
  catch (const std::out_of_range&)
  {
    return false;
  }
}
