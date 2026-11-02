#include "ipc.hpp"

#include <cerrno>
#include <unistd.h>

bool zhuravleva::writeAll(int fd, const void* data, std::size_t size)
{
  const char* const bytes = static_cast< const char* >(data);
  std::size_t written = 0;
  while (written < size)
  {
    const ssize_t count = write(fd, bytes + written, size - written);
    if (count == -1 && errno == EINTR)
    {
      continue;
    }
    if (count <= 0)
    {
      return false;
    }
    written += static_cast< std::size_t >(count);
  }
  return true;
}

bool zhuravleva::readAll(int fd, void* data, std::size_t size)
{
  char* const bytes = static_cast< char* >(data);
  std::size_t received = 0;
  while (received < size)
  {
    const ssize_t count = read(fd, bytes + received, size - received);
    if (count == -1 && errno == EINTR)
    {
      continue;
    }
    if (count <= 0)
    {
      return false;
    }
    received += static_cast< std::size_t >(count);
  }
  return true;
}
