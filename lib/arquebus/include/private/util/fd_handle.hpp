#pragma once

#include <unistd.h>

namespace arquebus::util {

  class fd_handle
  {
  public:
    explicit fd_handle(int fd)
      : m_fd(fd)
    {}

    ~fd_handle()
    {
      if (m_fd != -1) {
        ::close(m_fd);
      }
    }

    operator int() const { return m_fd; }

  private:
    int m_fd;
  };


}  // namespace arquebus::util
