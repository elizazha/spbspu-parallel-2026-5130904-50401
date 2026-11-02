
#include "process.hpp"
#include "ipc.hpp"
#include "monte_carlo.hpp"
#include <cerrno>
#include <csignal>
#include <iomanip>
#include <limits>
#include <poll.h>
#include <chrono>
#include <ostream>
#include <unistd.h>
#include <sys/wait.h>

namespace
{
  volatile std::sig_atomic_t stop_requested = 0;

  void requestStop(int)
  {
    stop_requested = 1;
  }

  void closeIfOpen(int fd)
  {
    if (fd >= 0)
    {
      close(fd);
    }
  }

  void waitForChild(pid_t pid)
  {
    int status = 0;
    while (waitpid(pid, &status, 0) == -1)
    {
      if (errno != EINTR)
      {
        break;
      }
    }
  }

  void markFailed(zhuravleva::ProcessStorage& storage, const std::string& name)
  {
    for (auto& item : storage.calculations)
    {
      if (item.second.process_name == name && item.second.state == zhuravleva::CalcState::pending)
      {
        item.second.state = zhuravleva::CalcState::failed;
      }
    }
  }

  void childLoop(int read_fd, int write_fd, unsigned int seed)
  {
    constexpr std::uint64_t max_shapes = 1000000;
    while (!stop_requested)
    {
      zhuravleva::AreaRequest request{};
      if (!zhuravleva::readAll(read_fd, &request, sizeof(request)))
      {
        break;
      }
      if (request.shape_count > max_shapes || request.threads == 0 || request.tries == 0
          || request.shape_count > std::numeric_limits< std::size_t >::max())
      {
        break;
      }
      std::vector< zhuravleva::Shape > shapes(static_cast< std::size_t >(request.shape_count));
      if (!shapes.empty() && !zhuravleva::readAll(read_fd, shapes.data(), shapes.size() * sizeof(shapes.front())))
      {
        break;
      }
      zhuravleva::AreaResult result{};
      result.id = request.id;
      const auto start = std::chrono::steady_clock::now();
      try
      {
        result.area = zhuravleva::calculateArea(shapes, static_cast< std::size_t >(request.threads),
            static_cast< std::size_t >(request.tries), static_cast< std::size_t >(seed) + request.id);
        result.success = 1;
      }
      catch (...)
      {
        result.success = 0;
      }
      const auto finish = std::chrono::steady_clock::now();
      result.milliseconds = std::chrono::duration< double, std::milli >(finish - start).count();
      if (!zhuravleva::writeAll(write_fd, &result, sizeof(result)))
      {
        break;
      }
    }
  }

  bool collectResult(zhuravleva::ProcessStorage& storage, const std::string& process_name)
  {
    const auto process_it = storage.processes.find(process_name);
    if (process_it == storage.processes.end())
    {
      return false;
    }
    zhuravleva::AreaResult result{};
    if (!zhuravleva::readAll(process_it->second.read_fd, &result, sizeof(result)))
    {
      markFailed(storage, process_name);
      return false;
    }
    for (auto& item : storage.calculations)
    {
      if (item.second.id == result.id && item.second.process_name == process_name)
      {
        item.second.area = result.area;
        item.second.milliseconds = result.milliseconds;
        item.second.state = result.success != 0 ? zhuravleva::CalcState::done : zhuravleva::CalcState::failed;
        return true;
      }
    }
    markFailed(storage, process_name);
    return false;
  }
}

bool zhuravleva::spawnProcess(ProcessStorage& storage, const std::string& name, unsigned int seed)
{
  if (name.empty() || storage.processes.find(name) != storage.processes.end())
  {
    return false;
  }
  int to_child[2] = {-1, -1};
  int from_child[2] = {-1, -1};
  if (pipe(to_child) == -1)
  {
    return false;
  }
  if (pipe(from_child) == -1)
  {
    closeIfOpen(to_child[0]);
    closeIfOpen(to_child[1]);
    return false;
  }
  const pid_t pid = fork();
  if (pid == -1)
  {
    closeIfOpen(to_child[0]);
    closeIfOpen(to_child[1]);
    closeIfOpen(from_child[0]);
    closeIfOpen(from_child[1]);
    return false;
  }
  if (pid == 0)
  {
    closeIfOpen(to_child[1]);
    closeIfOpen(from_child[0]);
    for (const auto& item : storage.processes)
    {
      closeIfOpen(item.second.write_fd);
      closeIfOpen(item.second.read_fd);
    }
    stop_requested = 0;
    std::signal(SIGUSR1, requestStop);
    const char ready = 1;
    if (!writeAll(from_child[1], &ready, sizeof(ready)))
    {
      _exit(1);
    }
    childLoop(to_child[0], from_child[1], seed);
    closeIfOpen(to_child[0]);
    closeIfOpen(from_child[1]);
    _exit(0);
  }
  char ready = 0;
  if (!readAll(from_child[0], &ready, sizeof(ready)) || ready != 1)
  {
    closeIfOpen(to_child[1]);
    closeIfOpen(from_child[0]);
    waitForChild(pid);
    return false;
  }
  closeIfOpen(to_child[0]);
  closeIfOpen(from_child[1]);
  storage.processes.emplace(name, Process{pid, to_child[1], from_child[0], seed});
  return true;
}

bool zhuravleva::killProcess(ProcessStorage& storage, const std::string& name)
{
  const auto it = storage.processes.find(name);
  if (it == storage.processes.end())
  {
    return false;
  }
  const Process process = it->second;
  if (kill(process.pid, SIGUSR1) == -1 && errno != ESRCH)
  {
    return false;
  }
  closeIfOpen(process.write_fd);
  while (true)
  {
    pollfd fd{};
    fd.fd = process.read_fd;
    fd.events = POLLIN;
    const int result = poll(&fd, 1, -1);
    if (result == -1 && errno == EINTR)
    {
      continue;
    }
    if (result <= 0)
    {
      break;
    }
    if (fd.revents & POLLIN)
    {
      if (!collectResult(storage, name))
      {
        break;
      }
      continue;
    }
    break;
  }
  closeIfOpen(process.read_fd);
  waitForChild(process.pid);
  for (auto& item : storage.calculations)
  {
    if (item.second.process_name == name && item.second.state == CalcState::pending)
    {
      item.second.state = CalcState::canceled;
    }
  }
  storage.processes.erase(it);
  return true;
}

void zhuravleva::shutdownProcesses(ProcessStorage& storage)
{
  for (auto& item : storage.processes)
  {
    kill(item.second.pid, SIGTERM);
    closeIfOpen(item.second.write_fd);
    closeIfOpen(item.second.read_fd);
  }
  for (auto& item : storage.processes)
  {
    waitForChild(item.second.pid);
    markFailed(storage, item.first);
  }
  storage.processes.clear();
}

bool zhuravleva::submitArea(ProcessStorage& storage, const std::string& process_name, const std::string& calc_name,
    const std::vector< Shape >& shapes, std::size_t threads, std::size_t tries)
{
  const auto it = storage.processes.find(process_name);
  if (it == storage.processes.end() || calc_name.empty()
      || storage.calculations.find(calc_name) != storage.calculations.end() || shapes.empty() || threads == 0
      || tries == 0)
  {
    return false;
  }
  AreaRequest request{};
  request.id = storage.next_id;
  request.shape_count = shapes.size();
  request.threads = threads;
  request.tries = tries;
  if (!writeAll(it->second.write_fd, &request, sizeof(request))
      || !writeAll(it->second.write_fd, shapes.data(), shapes.size() * sizeof(Shape)))
  {
    killProcess(storage, process_name);
    return false;
  }
  storage.calculations.emplace(calc_name, Calculation{request.id, process_name, CalcState::pending, 0.0, 0.0});
  ++storage.next_id;
  return true;
}

bool zhuravleva::waitCalculation(ProcessStorage& storage, const std::string& calc_name, double& area)
{
  const auto it = storage.calculations.find(calc_name);
  if (it == storage.calculations.end())
  {
    return false;
  }
  const std::string process_name = it->second.process_name;
  while (it->second.state == CalcState::pending)
  {
    if (!collectResult(storage, process_name))
    {
      return false;
    }
  }
  if (it->second.state != CalcState::done)
  {
    return false;
  }
  area = it->second.area;
  return true;
}

void zhuravleva::refreshCalculations(ProcessStorage& storage)
{
  for (const auto& item : storage.processes)
  {
    while (true)
    {
      pollfd fd{};
      fd.fd = item.second.read_fd;
      fd.events = POLLIN;
      const int result = poll(&fd, 1, 0);
      if (result == -1 && errno == EINTR)
      {
        continue;
      }
      if (result <= 0)
      {
        break;
      }
      if (fd.revents & POLLIN)
      {
        if (!collectResult(storage, item.first))
        {
          break;
        }
        continue;
      }
      if (fd.revents & (POLLHUP | POLLERR | POLLNVAL))
      {
        markFailed(storage, item.first);
      }
      break;
    }
  }
}

bool zhuravleva::showStatus(ProcessStorage& storage, const std::string& name, std::ostream& out)
{
  const auto it = storage.calculations.find(name);
  if (it == storage.calculations.end())
  {
    return false;
  }
  refreshCalculations(storage);
  const Calculation& calc = it->second;
  out << calc.process_name << ' ';
  if (calc.state == CalcState::pending)
  {
    out << "<IN PROGRESS>\n";
  }
  else if (calc.state == CalcState::done)
  {
    const auto flags = out.flags();
    const auto precision = out.precision();
    out << std::fixed << std::setprecision(3) << calc.milliseconds << ' ' << calc.area << '\n';
    out.flags(flags);
    out.precision(precision);
  }
  else if (calc.state == CalcState::canceled)
  {
    out << "<CANCELED>\n";
  }
  else
  {
    out << "<FAILED>\n";
  }
  return true;
}

bool zhuravleva::timeoutCalculation(
    ProcessStorage& storage, const std::string& name, int milliseconds, double& area, bool& finished)
{
  finished = false;
  const auto it = storage.calculations.find(name);
  if (it == storage.calculations.end() || milliseconds < 0)
  {
    return false;
  }
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
  while (true)
  {
    refreshCalculations(storage);
    if (it->second.state == CalcState::done)
    {
      area = it->second.area;
      finished = true;
      return true;
    }
    if (it->second.state == CalcState::canceled)
    {
      return true;
    }
    const auto process_it = storage.processes.find(it->second.process_name);
    if (process_it == storage.processes.end())
    {
      return false;
    }
    const auto now = std::chrono::steady_clock::now();
    if (now >= deadline)
    {
      return true;
    }
    const auto remaining = std::chrono::duration_cast< std::chrono::milliseconds >(deadline - now);
    pollfd fd{};
    fd.fd = process_it->second.read_fd;
    fd.events = POLLIN;
    const int result = poll(&fd, 1, static_cast< int >(remaining.count()));
    if (result == -1 && errno != EINTR)
    {
      return false;
    }
  }
}
