#include "commands.hpp"
#include "parse.hpp"
#include "process.hpp"
#include <csignal>
#include <iostream>
#include <string>
#include <vector>

int main()
{
  constexpr std::size_t area_arg_count = 6;
  constexpr std::size_t tries_arg_index = 5;
  std::signal(SIGPIPE, SIG_IGN);
  zhuravleva::Storage storage;
  zhuravleva::ProcessStorage process_storage;
  std::string line;

  while (std::getline(std::cin, line))
  {
    const std::vector< std::string > args = zhuravleva::splitLine(line);
    if (args.empty())
    {
      continue;
    }
    bool valid = false;
    if (args[0] == "circle")
    {
      valid = zhuravleva::addCircle(storage, args);
    }
    else if (args[0] == "ellipse")
    {
      valid = zhuravleva::addEllipse(storage, args);
    }
    else if (args[0] == "rotate")
    {
      valid = zhuravleva::rotateShape(storage, args);
    }
    else if (args[0] == "set")
    {
      valid = zhuravleva::addSet(storage, args);
    }
    else if (args[0] == "show")
    {
      valid = zhuravleva::showShapes(storage, args, std::cout);
    }
    else if (args[0] == "showset")
    {
      valid = zhuravleva::showSets(storage, args, std::cout);
    }
    else if (args[0] == "frame")
    {
      valid = zhuravleva::showFrame(storage, args, std::cout);
    }
    else if (args[0] == "frameset")
    {
      valid = zhuravleva::showSetFrame(storage, args, std::cout);
    }
    else if (args[0] == "spawn")
    {
      int seed = 0;
      valid = args.size() == 3 && zhuravleva::parseInt(args[2], seed) && seed >= 0;
      if (valid)
      {
        valid = zhuravleva::spawnProcess(process_storage, args[1], static_cast< unsigned int >(seed));
      }
    }
    else if (args[0] == "kill")
    {
      valid = args.size() == 2 && zhuravleva::killProcess(process_storage, args[1]);
    }
    else if (args[0] == "area-on")
    {
      int threads = 0;
      int tries = 0;
      valid = args.size() == area_arg_count && zhuravleva::parseInt(args[4], threads)
          && zhuravleva::parseInt(args[tries_arg_index], tries) && threads > 0 && tries > 0;
      if (valid)
      {
        const auto set_it = storage.sets.find(args[3]);
        valid = set_it != storage.sets.end();
        if (valid)
        {
          std::vector< zhuravleva::Shape > shapes;
          for (const std::string& name : set_it->second)
          {
            const auto shape_it = storage.shapes.find(name);
            if (shape_it == storage.shapes.end())
            {
              valid = false;
              break;
            }
            shapes.push_back(shape_it->second);
          }
          if (valid)
          {
            valid = zhuravleva::submitArea(process_storage, args[1], args[2], shapes,
                static_cast< std::size_t >(threads), static_cast< std::size_t >(tries));
          }
        }
      }
    }
    else if (args[0] == "wait")
    {
      double area = 0.0;
      valid = args.size() == 2 && zhuravleva::waitCalculation(process_storage, args[1], area);
      if (valid)
      {
        valid = zhuravleva::showStatus(process_storage, args[1], std::cout);
      }
    }
    else if (args[0] == "status")
    {
      valid = args.size() == 2;
      if (valid)
      {
        valid = zhuravleva::showStatus(process_storage, args[1], std::cout);
      }
    }
    else if (args[0] == "timeout")
    {
      int milliseconds = 0;
      valid = args.size() == 3 && zhuravleva::parseInt(args[2], milliseconds) && milliseconds >= 0;
      if (valid)
      {
        double area = 0.0;
        bool finished = false;
        valid = zhuravleva::timeoutCalculation(process_storage, args[1], milliseconds, area, finished);
        if (valid)
        {
          valid = zhuravleva::showStatus(process_storage, args[1], std::cout);
        }
      }
    }
    else
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }
    if (!valid)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }
  zhuravleva::shutdownProcesses(process_storage);
  return 0;
}
