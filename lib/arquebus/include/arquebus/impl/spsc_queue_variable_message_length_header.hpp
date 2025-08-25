#pragma once

#include "arquebus/version.hpp"
#include "buffer_size.hpp"
#include "common_header.hpp"
#include "queue_type.hpp"

#include <atomic>
#include <chrono>
#include <concepts>
#include <new>
#include <stdexcept>
#include <thread>

namespace arquebus::impl {

  template<std::uint8_t Size2NBits, std::unsigned_integral TMessageSize>
  struct spsc_queue_variable_message_length_header
  {
    using MessageSize = TMessageSize;
    using BufferSize = buffer_size<Size2NBits>;

    static constexpr auto CacheLineSize = std::hardware_destructive_interference_size;
    static constexpr auto QueueType = queue_type::SingleProducerSingleConsumerVariableMessageLength;

    common_header header{};

    alignas(CacheLineSize) std::atomic_uint64_t write_index{ 0 };
    alignas(CacheLineSize) std::atomic_uint64_t read_index{ 0 };

    // we are using C-style array to avoid initialisation, it will be zero filled when we
    // map it into memory as a shared memory region
    // NOLINTNEXTLINE(*-avoid-c-arrays)
    alignas(CacheLineSize) std::byte data[BufferSize::Bytes];


    // the owner should initialise the queue
    void initialise()
    {
      auto type = header.type.load(std::memory_order_acquire);
      if (type != queue_type::None) {
        throw std::logic_error("queue is already initialised");
      }

      header.arquebus_version = version_details{}.version;
      header.message_size_type_size = sizeof(MessageSize);
      header.max_producers = 1;
      header.max_consumers = 1;
      header.size_of_queue = BufferSize::Bytes;

      write_index.store(0, std::memory_order_release);
      read_index.store(0, std::memory_order_release);
      header.type.store(QueueType, std::memory_order_release);
    }


    // a user (producer or consumer) should wait for the queue to be initialised and
    // validate that it meets expectations.
    void wait_and_validate()
    {
      queue_type type = queue_type::None;
      do {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        type = header.type.load(std::memory_order_seq_cst);
      } while (type == queue_type::None);

      // owner has initialised

      if (type != QueueType) {
        throw std::logic_error("incorrect queue type");
      }
      if(header.message_size_type_size != sizeof(MessageSize)) {
        throw std::logic_error("incorrect message size type");
      }
      if (header.max_producers != 1) {
        throw std::logic_error("incorrect max producers");
      }
      if (header.max_consumers != 1) {
        throw std::logic_error("incorrect max consumers");
      }
      if (header.size_of_queue != BufferSize::Bytes) {
        throw std::logic_error("incorrect size of queue");
      }
    }
  };

}  // namespace arquebus
