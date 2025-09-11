#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc/variable_message_length_header.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus::spsc::var_msg {

  struct danger_delete_existing_shared_memory_segment_tag
  {
  };

  /// Single Producer Single Consumer Queue Host interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam TMessageSize Type for indicating size of message.
  /// @tparam CacheLineSize The CPU cache line size. Defaults to std::hardware_destructive_interference_size
  template<
    std::uint8_t Size2NBits,
    std::unsigned_integral TMessageSize = std::uint32_t,
    std::size_t CacheLineSize = std::hardware_destructive_interference_size>
  class host
  {
    using QueueLayout = impl::spsc::variable_message_length_header<Size2NBits, TMessageSize, CacheLineSize>;

  public:
    /// Create a host for the given queue name. The name must match that used by the producer and consumer.
    ///
    /// @param name The unique name of the queue to create
    explicit host(std::string_view name)
      : m_queueOwner(name)
    {}

    /// Open and create the shared memory queue.
    ///
    /// This must occur before the producer or consumer can attempt to connect.
    void create()
    {
      m_queueOwner.create();

      m_queue = m_queueOwner.mapping();
      m_queue->initialise();
    }

    /// If the shared memory segment already exists, delete it before creating a new one
    ///
    /// WARNING: existing open mapping will still see old segment, new mappings will see
    ///          new file. Be VERY sure you want to do this!
    void create(danger_delete_existing_shared_memory_segment_tag /*unused*/)
    {
      m_queueOwner.delete_existing();
      create();
    }

  private:
    impl::shared_memory_owner<QueueLayout> m_queueOwner;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus::spsc::var_msg
