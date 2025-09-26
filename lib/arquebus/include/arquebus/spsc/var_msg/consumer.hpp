#pragma once

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/impl/spsc/variable_message_length_header.hpp"

#include <cstdint>
#include <cstring>
#include <optional>
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
    using StorageType = QueueLayout::StorageType;
    using MessageSize = TMessageSize;

  public:
    /// Create a consumer for the given queue name. The name must match that created by the host
    /// and used by the producer.
    ///
    /// @param name The unique name of the queue to attach to
    explicit consumer(std::string_view name)
      : m_queueUser(name)
    {}

    /// Attach the consumer to the queue that has been created by a host.
    void attach()
    {
      m_queueUser.attach();

      m_queue = m_queueUser.mapping();
      m_queue->wait_and_validate();

      // find the current read-released location and remember them - this allows a consumer to access
      // and already started queue
      m_cachedReadIndex = m_queue->read_index.load(std::memory_order_acquire);
      m_cachedWriteIndex = m_queue->write_index.load(std::memory_order_acquire);
      m_readIndex = m_cachedReadIndex;
    }

    /// Read the next message from the queue.
    ///
    /// This will not block and return immediately. If there is no available message, the optional will be empty.
    /// The caller is responsible for managing the spinning and retrying for new messages.
    ///
    /// If the consumer is overrun by the producer, a std::runtime_error will be thrown
    ///
    /// @return An optional span containing the next message data
    auto read() -> std::optional<std::span<StorageType const>>
    {
      if (m_readIndex < m_cachedReadIndex) [[likely]] {
        return decode_message();
      }

      update_cached_indices();

      if (m_readIndex < m_cachedReadIndex) [[likely]] {
        return decode_message();
      }

      // no waiting message
      return std::nullopt;
    }

  private:
    impl::shared_memory_user<QueueLayout> m_queueUser;
    QueueLayout *m_queue{ nullptr };
    std::uint64_t m_cachedWriteIndex{ 0 };
    std::uint64_t m_cachedReadIndex{ 0 };
    std::uint64_t m_readIndex{ 0 };

    // Decode a message waiting in the queue.
    // We can assume that the message will never wrap around the queue buffer as the writer
    // takes care of that. We need to decode the zero length messages and wrap ourselves correctly
    // to match.
    auto decode_message() noexcept -> std::span<StorageType const>
    {
      // read the length
      MessageSize messageSize{ 0 };
      auto *pBuffer = &m_queue->data[QueueLayout::BufferSize::to_offset(m_readIndex)];
      std::memcpy(&messageSize, pBuffer, sizeof(MessageSize));

      if (messageSize == 0) [[unlikely]] {
        // The next message would wrap in the queue buffer, so the producer has moved it to the
        // beginning of the buffer. Advance our read index to the beginning of the buffer again.
        //
        // if we have read the zero, we know the producer has already moved the read and write indices
        // beyond the wrap and past the next message. This means that we have also, already received those
        // updated indices so at least the next message is within our read range.
        m_readIndex += QueueLayout::BufferSize::distance_to_buffer_start(m_readIndex);

        // we have wrapped to the beginning
        pBuffer = &m_queue->data[0];
        std::memcpy(&messageSize, pBuffer, sizeof(MessageSize));
      }

      // update our cached read index (including the size data)
      m_readIndex += messageSize + sizeof(MessageSize);

      // return the span
      return { pBuffer + sizeof(MessageSize), messageSize };
    }

    void update_cached_indices()
    {
      // currently we have no new data in our cached counters, so we update them
      // from the shared atomics in the queue and try again.
      m_cachedWriteIndex = m_queue->write_index.load(std::memory_order_acquire);

      // check for overrun
      // This will occur if our current read index (as a physical buffer offset) has been
      // overrun by the new write index.
      //
      // The non-offset bits of the index should be considered a "generation" and the offset (Mask) bits
      // are the actual location in the buffer.
      //
      // An overlap occurs when the read offset < write offset AND read generation < write generation
      //
      // Note that we check the generation first before computing and checking the offset because
      // it only when the buffer initially wraps that they will be different as the write index advances
      // by a batch size that is less than the overall length.
      auto readGeneration = QueueLayout::BufferSize::to_generation(m_readIndex) ;
      auto writeGeneration = QueueLayout::BufferSize::to_generation(m_cachedWriteIndex);

      if(readGeneration < writeGeneration) [[ unlikely]] {
        auto readOffset = QueueLayout::BufferSize::to_offset(m_readIndex) ;
        auto writeOffset = QueueLayout::BufferSize::to_offset(m_cachedWriteIndex);
        if(readOffset < writeOffset) [[unlikely]] {
          throw std::runtime_error("Queue Overrun detected");
        }
      }

      m_cachedReadIndex = m_queue->read_index.load(std::memory_order_acquire);
    }
  };

}  // namespace arquebus::spsc::var_msg
