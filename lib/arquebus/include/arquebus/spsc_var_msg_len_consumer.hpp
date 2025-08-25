#pragma once

#include "impl/spsc_queue_variable_message_length_header.hpp"
#include "shared_memory.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus {

  template<std::uint8_t Size2NBits, std::unsigned_integral TMessageSize = std::uint32_t>
  class spsc_var_msg_len_consumer
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits, TMessageSize>;

  public:
    explicit spsc_var_msg_len_consumer(std::string_view name)
      : m_queueUser(name)
    {}

    void attach()
    {
      m_queueUser.attach();

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();
    }

  private:
    shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
