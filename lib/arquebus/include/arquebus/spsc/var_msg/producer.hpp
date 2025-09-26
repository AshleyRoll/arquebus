#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc/variable_message_length_header.hpp"

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string_view>

namespace arquebus::spsc::var_msg {

  /// Single Producer Single Consumer Queue Producer interface
  ///
  /// @tparam Size2NBits Queue Size in exponent for 2^N
  /// @tparam NBytesBatchMessageReserve Number of bytes to allocate from queue as a chunk to prevent constant
  /// write index updates.
  /// @tparam TMessageSize Type for indicating size of message.
  /// @tparam CacheLineSize The CPU cache line size. Defaults to std::hardware_destructive_interference_size
  ///
  template<
    std::uint8_t Size2NBits,
    std::size_t NBytesBatchMessageReserve,
    std::unsigned_integral TMessageSize = std::uint32_t,
    std::size_t CacheLineSize = std::hardware_destructive_interference_size >
  class producer
  {
  public:
    using QueueLayout = impl::spsc::variable_message_length_header<Size2NBits, TMessageSize, CacheLineSize>;
    using MessageSize = TMessageSize;
    using StorageType = QueueLayout::StorageType;

    static constexpr auto BatchMessageReserve = std::uint64_t{ NBytesBatchMessageReserve };

    static_assert(
      BatchMessageReserve < (QueueLayout::BufferSize::Bytes - sizeof(MessageSize)),
      "Can not reserve more than the queue size"
    );

    /// Create a producer for the given queue name. The name must match that created by the host
    /// and used by the consumer.
    ///
    /// @param name The unique name of the queue to attach to
    explicit producer(std::string_view name)
      : m_queueUser(name)
    {}

    /// Attach the producer to the queue that has been created by a host.
    void attach()
    {
      m_queueUser.attach();

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();
      // establish the initial write offset
      m_queue->write_index.store(m_cachedWriteIndex, std::memory_order_release);
    }

    /// Allocate a write buffer for a message of numberBytes in length.
    ///
    /// A messageSizeBytes of zero or greater or equal to BatchMessageReserve is not supported and
    /// will cause incorrect behaviour.
    ///
    /// @param messageSizeBytes Message Length
    /// @return a span<> for the caller to fill with message data
    [[nodiscard]] auto allocate_write(MessageSize messageSizeBytes) noexcept -> std::span<StorageType>
    {
      // message + the next size / skip block ready for next message
      auto const allocationSize = messageSizeBytes + sizeof(MessageSize);

      // We have to be careful of wrapping around the data index as a std::span<>
      // can not cope with that. Therefore, we have to maintain the following:
      // * If we can't fit a message into the current space, we must mark it as unused
      //   so the consumer can skip it. We will indicate this with a zero message size.
      // * We need to maintain a known safe "MessageSize" stream that doesn't wrap so
      //   we can always write a skip if the next message is too large


      if (m_cachedWriteIndex - m_allocatedIndex < allocationSize) [[unlikely]] {
        // allocate more storage. Skipping over index wrap if required
        reserve(allocationSize);
      }

      // we have ensured that our allocation will not wrap so safe to index in
      auto *pBuffer = &m_queue->data[QueueLayout::BufferSize::to_offset(m_allocatedIndex)];

      // write the message size into the buffer, note that this is actually already reserved
      // and know safe place to write "before" the current allocation index
      std::memcpy(pBuffer - sizeof(MessageSize), &messageSizeBytes, sizeof(MessageSize));
      m_allocatedIndex += allocationSize;
      return { pBuffer, messageSizeBytes };
    }

    /// Flush any allocated writes.
    ///
    /// It is the caller's responsibility to ensure that all
    /// allocated message buffer spans have been filled before calling flush().
    void flush() noexcept
    {
      // update the shared read index to release all pending messages
      // We are pre-allocating the next size/skip indicator, so we have to release to just before that
      // as it is not yet valid
      m_queue->read_index.store(m_allocatedIndex - sizeof(MessageSize), std::memory_order_release);
    }

  private:
    impl::shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
    // a local copy of the write index, we take chunks of data at a time
    // any only update the atomic write index when we need more
    std::uint64_t m_cachedWriteIndex{ sizeof(MessageSize) };

    // a local read index of allocated, but not committed/flushed message data
    // once the caller calls flush(), we release this to the consumer
    //
    // This actually maintains a "reserved" size area so that we know that we will always have a
    // continuous sequence of bytes to write the next size or skip into.
    std::uint64_t m_allocatedIndex{ sizeof(MessageSize) };

    void reserve(std::size_t minimumRequired) noexcept
    {
      // increase the write index by the BatchMessageReserve and this message size to ensure enough space
      // and then determine if we need to skip the allocation forward to the next index wrap.
      m_cachedWriteIndex += BatchMessageReserve + sizeof(MessageSize);

      // The current allocation and minRequired already contains the size bytes
      auto const offsetOfAllocatedIndex = QueueLayout::BufferSize::to_offset(m_allocatedIndex - sizeof(MessageSize));
			auto const offsetOfNextAllocationIndex = QueueLayout::BufferSize::to_offset(m_allocatedIndex + minimumRequired);
      // have we wrapped?
      if (offsetOfNextAllocationIndex< offsetOfAllocatedIndex) [[unlikely]] {
        // We know we have a safe allocation to store MessageSize, so we need to indicate that the
        // remaining block is no longer valid and the consumer should skip back to the beginning of the
        // queue buffer.
        auto *pBuffer = &m_queue->data[offsetOfAllocatedIndex];
        MessageSize zero{ 0u };
        // write the message size into the buffer, note that this is actually already reserved
        // and know safe place to write "before" the current allocation index
        std::memcpy(pBuffer, &zero, sizeof(MessageSize));

        // increment our allocation along to the beginning of the queue buffer, and include the
        // reserved "next" size. this will effectively place the next size at index 0
        auto wrapCount = QueueLayout::BufferSize::distance_to_buffer_start(m_allocatedIndex) + sizeof(MessageSize);
        m_allocatedIndex += wrapCount;
        m_cachedWriteIndex += wrapCount;
      }

      // inform consumer of updated allocation
      m_queue->write_index.store(m_cachedWriteIndex, std::memory_order_release);
    }
  };

}  // namespace arquebus::spsc::var_msg
