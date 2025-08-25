#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc_queue_variable_message_length_header.hpp"

#include <stdexcept>
#include <string_view>

namespace arquebus::spsc::var_msg {

  /// Single Producer Single Consumer Queue Consumer interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam TMessageSize Type for indicating size of message.
  template<std::uint8_t Size2NBits, std::unsigned_integral TMessageSize = std::uint32_t>
  class consumer
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits, TMessageSize>;

  public:
    explicit consumer(std::string_view name)
      : m_queueUser(name)
    {}

    void attach()
    {
      m_queueUser.attach();

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();
    }

  private:
    impl::shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
  };

}  // namespace arquebus
