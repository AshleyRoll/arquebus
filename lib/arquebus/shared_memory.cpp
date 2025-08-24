#include "shared_memory.hpp"

#include "util/fd_handle.hpp"

#include <fmt/core.h>

#include <climits>
#include <cstdint>
#include <stdexcept>

// POSIX
#include <fcntl.h>
#include <sys/mman.h>


namespace {
  using namespace arquebus::impl;

  auto make_shm_name(std::string_view name) -> std::string
  {
    if (name.empty() or name.size() >= (NAME_MAX - shared_memory::ShmPrefix.size())) {
      throw std::invalid_argument("name is empty or too long");
    }

    std::string fullName{ shared_memory::ShmPrefix };
    fullName.append(name);

    return fullName;
  }
}  // namespace


namespace arquebus::impl {

  shared_memory::shared_memory(std::string_view name, std::size_t mappingSize)
    : m_name{ make_shm_name(name) }
    , m_isMappingOwner{ false }
    , m_mapping{ nullptr }
    , m_mappingSize{ mappingSize }
  {}

  shared_memory::~shared_memory() { close(); }

  auto shared_memory::attach() -> bool
  {
    if(m_mapping != nullptr) {
      throw std::logic_error("Shared memory segment already exists");
    }

    // attempt to open the shm object, but do not create it
    util::fd_handle const shmFd{ ::shm_open(m_name.c_str(), O_RDWR, 0) };

    if (shmFd < 0) {
      auto err = errno;
      fmt::println("Failed to open '{}', errno = {}", m_name, err);
      return false;
    }

    // create mapping
    auto *mapping = ::mmap(nullptr, m_mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (mapping == MAP_FAILED) {
      auto err = errno;
      fmt::println("Failed to mmap shared memory segment '{}', errno = {}", m_name, err);
      return false;
    }

    // mapped
    m_isMappingOwner = false;
    m_mapping = mapping;

    return true;
  }

  auto shared_memory::create() -> bool
  {
    if(m_mapping != nullptr) {
      throw std::logic_error("Shared memory segment already exists");
    }

    // attempt to open or create the sgm object
    util::fd_handle const shmFd{ ::shm_open(m_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600) };

    if (shmFd < 0) {
      auto err = errno;
      if (err == EEXIST) {
        fmt::println("Failed to create '{}'. It already exists. Please delete it and try again", m_name);
      } else {
        fmt::println("Failed to create '{}', errno = {}", m_name, err);
      }
      return false;
    }

    // size the shared memory object
    if (::ftruncate(shmFd, static_cast<std::int64_t>(m_mappingSize)) < 0) {
      auto err = errno;
      fmt::println("Failed to resize shared memory segment '{}', errno = {}", m_name, err);
      return false;
    }

    // create mapping
    auto *mapping = ::mmap(nullptr, m_mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (mapping == MAP_FAILED) {
      auto err = errno;
      fmt::println("Failed to mmap shared memory segment '{}', errno = {}", m_name, err);
      return false;
    }

    // mapped
    m_isMappingOwner = true;
    m_mapping = mapping;

    return true;
  }

  void shared_memory::close()
  {
    if (m_mapping != nullptr) {
      ::munmap(m_mapping, m_mappingSize);
      m_mapping = nullptr;
    }

    if (m_isMappingOwner) {
      ::shm_unlink(m_name.c_str());
    }
  }
}  // namespace arquebus::impl
