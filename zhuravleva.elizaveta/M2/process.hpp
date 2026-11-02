#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "shapes.hpp"
#include <cstdint>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include <sys/types.h>

namespace zhuravleva
{
  struct Process
  {
    pid_t pid;
    int write_fd;
    int read_fd;
    unsigned int seed;
  };

  enum class CalcState
  {
    pending,
    done,
    failed,
    canceled
  };

  struct Calculation
  {
    std::uint64_t id;
    std::string process_name;
    CalcState state;
    double area;
    double milliseconds;
  };

  struct ProcessStorage
  {
    std::map< std::string, Process > processes;
    std::map< std::string, Calculation > calculations;
    std::uint64_t next_id = 1;
  };

  bool spawnProcess(ProcessStorage& storage, const std::string& name, unsigned int seed);
  bool killProcess(ProcessStorage& storage, const std::string& name);
  void shutdownProcesses(ProcessStorage& storage);
  bool submitArea(ProcessStorage& storage, const std::string& process_name, const std::string& calc_name,
      const std::vector< Shape >& shapes, std::size_t threads, std::size_t tries);
  bool waitCalculation(ProcessStorage& storage, const std::string& calc_name, double& area);
  void refreshCalculations(ProcessStorage& storage);
  bool showStatus(ProcessStorage& storage, const std::string& name, std::ostream& out);
  bool timeoutCalculation(
      ProcessStorage& storage, const std::string& name, int milliseconds, double& area, bool& finished);
}
#endif
