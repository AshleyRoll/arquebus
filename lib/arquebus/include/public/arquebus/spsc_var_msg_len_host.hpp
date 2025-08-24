#pragma once

#include "arquebus/arquebus_export.hpp"
#include "impl/spsc_queue_variable_message_length_header.hpp"
#include "shared_memory.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus {

  template<std::uint8_t Size2NBits>
  ARQUEBUS_EXPORT class spsc_var_msg_len_host
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits>;

  public:
    explicit spsc_var_msg_len_host(std::string_view name)
      : m_queueOwner(name)
    {}

    void initialise()
    {
      if (!m_queueOwner.create()) {
        throw std::runtime_error("Failed to create queue");
      }

      m_queue = m_queueOwner.mapping();
      m_queue->initialise();
    }

  private:
    shared_memory_owner<QueueLayout> m_queueOwner;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
