#ifndef IPC_HPP
#define IPC_HPP

#include <cstddef>
#include <cstdint>

namespace zhuravleva
{
  struct AreaRequest
  {
    std::uint64_t id;
    std::uint64_t shape_count;
    std::uint64_t threads;
    std::uint64_t tries;
  };

  struct AreaResult
  {
    std::uint64_t id;
    double area;
    double milliseconds;
    std::uint32_t success;
  };

  bool writeAll(int fd, const void* data, std::size_t size);
  bool readAll(int fd, void* data, std::size_t size);
}

#endif
