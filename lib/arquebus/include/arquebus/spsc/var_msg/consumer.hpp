#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc/variable_message_length_header.hpp"

#include <string_view>

namespace arquebus::spsc::var_msg {

  /// Single Producer Single Consumer Queue Consumer interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam TMessageSize Type for indicating size of message.
  /// @tparam CacheLineSize The CPU cache line size. Defaults to std::hardware_destructive_interference_size
  template<
    std::uint8_t Size2NBits,
    std::unsigned_integral TMessageSize = std::uint32_t,
    std::size_t CacheLineSize = std::hardware_destructive_interference_size>
  class consumer
  {
    using QueueLayout = impl::spsc::variable_message_length_header<Size2NBits, TMessageSize, CacheLineSize>;

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

}  // namespace arquebus::spsc::var_msg
