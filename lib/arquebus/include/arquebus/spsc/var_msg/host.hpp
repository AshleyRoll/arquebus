#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc_queue_variable_message_length_header.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus::spsc::var_msg {

  struct danger_delete_existing_shared_memory_segment_tag {};

  /// Single Producer Single Consumer Queue Host interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam TMessageSize Type for indicating size of message.
  template<std::uint8_t Size2NBits, std::unsigned_integral TMessageSize = std::uint32_t>
  class host
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits, TMessageSize>;

  public:
    explicit host(std::string_view name)
      : m_queueOwner(name)
    {}


    void create()
    {
      m_queueOwner.create();

      m_queue = m_queueOwner.mapping();
      m_queue->initialise();
    }

    // If the shared memory segment already exists, delete it before creating a new one
    //
    // WARNING: existing open mapping will still see old segment, new mappings will see
    //          new file. Be VERY sure you want to do this!
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
