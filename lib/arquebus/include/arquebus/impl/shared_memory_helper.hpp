#pragma once

#include "fd_handle.hpp"

// POSIX
#include <fcntl.h>
#include <sys/mman.h>

#include <climits>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace arquebus::impl {

  class shared_memory_helper
  {
  public:
    // shm namespace prefix for all shared memory objects created by this class
    static constexpr std::string_view ShmPrefix = "/arquebus_";

    shared_memory_helper(std::string_view name, std::size_t mappingSize)
      : m_name{ make_shm_name(name) }
      , m_mappingSize{ mappingSize }
    {}
    ~shared_memory_helper() { close(); }

    // no move or copy for now.
    shared_memory_helper(shared_memory_helper &&) = delete;
    auto operator=(shared_memory_helper &&) -> shared_memory_helper & = delete;
    shared_memory_helper(shared_memory_helper const &) = delete;
    auto operator=(shared_memory_helper const &) -> shared_memory_helper & = delete;

    [[nodiscard]] auto name() const -> std::string const & { return m_name; }
    [[nodiscard]] auto mapping() const -> void * { return m_mapping; }

    void delete_existing()
    {
        ::shm_unlink(m_name.c_str());
    }

    // attempt to open and map the shared memory segment
    void attach()
    {
      if (m_mapping != nullptr) {
        throw std::logic_error("Shared memory segment already exists");
      }

      // attempt to open the shm object, but do not create it
      fd_handle const shmFd{ ::shm_open(m_name.c_str(), O_RDWR, 0) };

      if (shmFd < 0) {
        throw std::runtime_error("Failed to open shared memory segment");
      }

      // create mapping
      auto *memMapping = ::mmap(nullptr, m_mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
      if (memMapping == MAP_FAILED) {
        throw std::runtime_error("Failed to map shared memory segment");
      }

      // mapped
      m_isMappingOwner = false;
      m_mapping = memMapping;
    }

    // attempt to create or open the shared memory segment as the owner
    void create()
    {
      if (m_mapping != nullptr) {
        throw std::logic_error("Shared memory segment already exists");
      }

      // attempt to open or create the sgm object
      fd_handle const shmFd{ ::shm_open(m_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600) };

      if (shmFd < 0) {
        auto err = errno;
        if (err == EEXIST) {
          throw std::runtime_error("Shared memory segment already exists");
        }
        throw std::runtime_error("Failed to create shared memory segment");
      }

      // size the shared memory object
      if (::ftruncate(shmFd, static_cast<std::int64_t>(m_mappingSize)) < 0) {
        throw std::runtime_error("Failed to resize shared memory segment");
      }

      // create mapping
      auto *memMapping = ::mmap(nullptr, m_mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
      if (memMapping == MAP_FAILED) {
        throw std::runtime_error("Failed to map shared memory segment");
      }

      // mapped
      m_isMappingOwner = true;
      m_mapping = memMapping;
    }

    // close the shared memory segment and unmap it
    // safe to call if already closed, or the open failed.
    void close()
    {
      if (m_mapping != nullptr) {
        ::munmap(m_mapping, m_mappingSize);
        m_mapping = nullptr;
      }

      if (m_isMappingOwner) {
        ::shm_unlink(m_name.c_str());
      }
    }

  private:
    std::string m_name;
    bool m_isMappingOwner{ false };
    void *m_mapping{ nullptr };
    std::size_t const m_mappingSize;

    static auto make_shm_name(std::string_view name) -> std::string
    {
      if (name.empty() or name.size() >= (NAME_MAX - ShmPrefix.size())) {
        throw std::invalid_argument("name is empty or too long");
      }

      std::string fullName{ ShmPrefix };
      fullName.append(name);

      return fullName;
    }
  };


  template<typename T>
    requires std::is_trivially_copyable_v<T>
  class shared_memory_owner
  {
  public:
    explicit shared_memory_owner(std::string_view name)
      : m_sharedMemory{ name, sizeof(T) }
    {}
    ~shared_memory_owner() = default;

    // no move or copy for now.
    shared_memory_owner(shared_memory_owner &&) = delete;
    auto operator=(shared_memory_owner &&) -> shared_memory_owner & = delete;
    shared_memory_owner(shared_memory_owner const &) = delete;
    auto operator=(shared_memory_owner const &) -> shared_memory_owner & = delete;

    void delete_existing() { m_sharedMemory.delete_existing(); }

    void create()
    {
      m_sharedMemory.create();

      // construct the T in place
      // NOLINTNEXTLINE(*-owning-memory)
      m_mapping = new (m_sharedMemory.mapping()) T;
    }

    [[nodiscard]] auto name() const -> std::string const & { return m_sharedMemory.name(); }
    [[nodiscard]] auto mapping() const -> T * { return m_mapping; }

    void close() { m_sharedMemory.close(); }

  private:
    shared_memory_helper m_sharedMemory;
    T *m_mapping{ nullptr };
  };


  template<typename T>
    requires std::is_trivially_copyable_v<T>
  class shared_memory_user
  {

  public:
    explicit shared_memory_user(std::string_view name)
      : m_sharedMemory{ name, sizeof(T) }
    {}
    ~shared_memory_user() = default;

    // no move or copy for now.
    shared_memory_user(shared_memory_user &&) = delete;
    auto operator=(shared_memory_user &&) -> shared_memory_user & = delete;
    shared_memory_user(shared_memory_user const &) = delete;
    auto operator=(shared_memory_user const &) -> shared_memory_user & = delete;

    void attach()
    {
      m_sharedMemory.attach();

      // construct the T in place
      // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
      m_mapping = reinterpret_cast<T *>(m_sharedMemory.mapping());
    }

    [[nodiscard]] auto name() const -> std::string const & { return m_sharedMemory.name(); }
    [[nodiscard]] auto mapping() const -> T * { return m_mapping; }

    void close() { m_sharedMemory.close(); }

  private:
    shared_memory_helper m_sharedMemory;
    T *m_mapping{ nullptr };
  };


}  // namespace arquebus::impl
