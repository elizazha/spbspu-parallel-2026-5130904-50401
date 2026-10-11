#include "monteCarlo.hpp"
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

int main(int argc, char* argv[])
{
  try
  {
    const zhuravleva::Args args = zhuravleva::parseArgs(argc, argv);

    const std::vector< zhuravleva::Circle > circles = zhuravleva::readCircles(std::cin);
    if (circles.empty())
    {
      std::cout << "0 0\n";
      return 0;
    }

    const zhuravleva::Bounds bounds = zhuravleva::getBounds(circles);
    const double width = bounds.max_x - bounds.min_x;
    const double height = bounds.max_y - bounds.min_y;
    const double area = width * height;

    const zhuravleva::Hits hits = zhuravleva::calcParallMonte(circles, bounds, args.threads, args.tries, args.seed);

    const double area_any = area * static_cast< double >(hits.any) / args.tries;
    const double area_all = area * static_cast< double >(hits.all) / args.tries;

    std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
    std::cout << area_any << ' ' << area_all << '\n';
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
