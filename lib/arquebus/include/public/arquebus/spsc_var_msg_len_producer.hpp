#pragma once

#include "arquebus/arquebus_export.hpp"
#include "impl/spsc_queue_variable_message_length_header.hpp"
#include "shared_memory.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus {

  template<std::uint8_t Size2NBits>
  ARQUEBUS_EXPORT class spsc_var_msg_len_producer
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits>;

  public:
    explicit spsc_var_msg_len_producer(std::string_view name)
      : m_queueUser(name)
    {}

    void attach()
    {
      if (!m_queueUser.attach()) {
        throw std::runtime_error("Failed to attach to queue");
      }

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();
    }

  private:
    shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
