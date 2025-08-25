#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc_queue_variable_message_length_header.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus::spsc::var_msg {

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

    void initialise()
    {
      m_queueOwner.create();

      m_queue = m_queueOwner.mapping();
      m_queue->initialise();
    }

  private:
    impl::shared_memory_owner<QueueLayout> m_queueOwner;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
