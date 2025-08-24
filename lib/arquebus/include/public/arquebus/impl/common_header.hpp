#pragma once

#include "arquebus/arquebus_export.hpp"
#include "arquebus/semantic_version.hpp"
#include "queue_type.hpp"

#include <atomic>
#include <cstdint>

namespace arquebus {

  // The header is mapped into the first bytes of the shared memory segment and is used to
  // identify and validate the queue type. The host will configure it, and the producers and consumers
  // will validate their settings agree before proceeding.
  //
  // The type is used as a flag to indicate that initialisation is complete. it will be zero until
  // all initialisation is completed. Producers and consumers should spin on that value waiting for
  // the host to complete configuration.
  ARQUEBUS_EXPORT struct common_header
  {

    std::atomic<queue_type> type;
    semantic_version arquebus_version;
    std::size_t max_producers;
    std::size_t max_consumers;
    std::uint64_t size_of_queue;
  };

}  // namespace arquebus
