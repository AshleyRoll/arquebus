#pragma once

#include "impl/spsc_queue_variable_message_length_header.hpp"
#include "shared_memory.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus {

  template<std::uint8_t Size2NBits, std::unsigned_integral TMessageSize = std::uint32_t>
  class spsc_var_msg_len_host
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits, TMessageSize>;

  public:
    explicit spsc_var_msg_len_host(std::string_view name)
      : m_queueOwner(name)
    {}

    void initialise()
    {
      m_queueOwner.create();

      m_queue = m_queueOwner.mapping();
      m_queue->initialise();
    }

  private:
    shared_memory_owner<QueueLayout> m_queueOwner;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
