#pragma once

#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace arquebus {

  namespace impl {

    class shared_memory
    {
    public:
      // shm namespace prefix for all shared memory objects created by this class
      static constexpr std::string_view ShmPrefix = "/arquebus_";

      shared_memory(std::string_view name, std::size_t mappingSize);
      ~shared_memory();

      // no move or copy for now.
      shared_memory(shared_memory &&) = delete;
      auto operator=(shared_memory &&) -> shared_memory & = delete;
      shared_memory(shared_memory const &) = delete;
      auto operator=(shared_memory const &) -> shared_memory & = delete;

      [[nodiscard]] auto name() const -> std::string const & { return m_name; }
      [[nodiscard]] auto mapping() const -> void * { return m_mapping; }

      // attempt to open and map the shared memory segment
      // return true on success, false if the segment does not yet exist.
      auto attach() -> bool;

      // attempt to create or open the shared memory segment as the owner
      // if the segment was created, true is returned, if the segment already exists,
      // false is returned.
      auto create() -> bool;

      // close the shared memory segment and unmap it
      // safe to call if already closed, or the open failed.
      void close();

    private:
      std::string m_name;
      bool m_isMappingOwner;
      void *m_mapping;
      std::size_t const m_mappingSize;
    };

  }  // namespace impl


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

    auto create() -> bool
    {
      if (!m_sharedMemory.create()) {
        return false;
      }

      // construct the T in place
      // NOLINTNEXTLINE(*-owning-memory)
      m_mapping = new (m_sharedMemory.mapping()) T;

      return true;
    }

    [[nodiscard]] auto name() const -> std::string const & { return m_sharedMemory.name(); }
    [[nodiscard]] auto mapping() const -> T * { return m_mapping; }

    void close() { m_sharedMemory.close(); }

  private:
    impl::shared_memory m_sharedMemory;
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

    auto attach() -> bool
    {
      if (!m_sharedMemory.attach()) {
        return false;
      }

      // construct the T in place
      // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
      m_mapping = reinterpret_cast<T *>(m_sharedMemory.mapping());

      return true;
    }

    [[nodiscard]] auto name() const -> std::string const & { return m_sharedMemory.name(); }
    [[nodiscard]] auto mapping() const -> T * { return m_mapping; }

    void close() { m_sharedMemory.close(); }

  private:
    impl::shared_memory m_sharedMemory;
    T *m_mapping{ nullptr };
  };


}  // namespace arquebus
