#pragma once

#include "impl/spsc_queue_variable_message_length_header.hpp"
#include "shared_memory.hpp"

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>

namespace arquebus {

  /// Single Producer Single Consumer Queue Producer interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam NBytesBatchMessageReserve Number of bytes to allocate from queue as a chunk to prevent constant
  /// write index updates.
  /// @tparam TMessageSize Type for indicating size of message.
  ///
  template<
    std::uint8_t Size2NBits,
    std::size_t NBytesBatchMessageReserve,
    std::unsigned_integral TMessageSize = std::uint32_t>
  class spsc_var_msg_len_producer
  {
    using QueueLayout = impl::spsc_queue_variable_message_length_header<Size2NBits, TMessageSize>;
    using MessageSize = TMessageSize;
    static constexpr auto BatchMessageReserve = std::uint64_t{ NBytesBatchMessageReserve };

    static_assert(NBytesBatchMessageReserve < QueueLayout::QueueSizeBytes);

  public:
    explicit spsc_var_msg_len_producer(std::string_view name)
      : m_queueUser(name)
    {}

    void attach()
    {
      m_queueUser.attach();

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();
    }

    /// Allocate a write buffer for a message of numberBytes in length.
    ///
    /// A size of zero is not supported.
    ///
    /// @param messageSizeBytes Message Length
    /// @return a span<> for the caller to fill with message data
    auto allocate_write(MessageSize messageSizeBytes) -> std::span<std::byte>
    {
      auto const allocationSize = sizeof(MessageSize) + messageSizeBytes;


      // We have to be careful of wrapping around the data index as a std::span<>
      // can not cope with that. Therefore, we have to maintain the following:
      // * The cached write index (end of our "preallocated" space) must not wrap.
      //   We will ensure this when we allocate more space from the shared write index
      // * If we can't fit a message into the current space, we must mark it as unused
      //   so the consumer can skip it. We will indicate this with a zero message size.


      if(m_cachedWriteIndex - m_allocatedIndex < allocationSize) [[unlikely]] {
        // allocate more storage. Skipping over index wrap if required
      }



        return {nullptr, messageSizeBytes};
    }

    /// Flush any allocated writes.
    ///
    /// It is the caller's responsibility to ensure that all
    /// allocated message buffer spans have been filled before calling flush().
    void flush()
    {
      // update the shared read index to release all pending messages
    }


  private:
    shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
    // a local copy of the write index, we take chunks of data at a time
    // any only update the atomic write index when we need more
    std::uint64_t m_cachedWriteIndex{ 0 };
    // a local read index of allocated, but not committed/flushed message data
    // once the caller calls flush(), we release this to the consumer
    std::uint64_t m_allocatedIndex{ 0 };

    void skip_and_reserve() { }
    void reserve() {}
  };

}  // namespace arquebus
