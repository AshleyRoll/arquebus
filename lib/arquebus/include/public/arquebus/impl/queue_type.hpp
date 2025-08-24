#pragma once

#include "arquebus/arquebus_export.hpp"

#include <cstdint>

namespace arquebus {

  ARQUEBUS_EXPORT enum struct queue_type : std::uint32_t
  {
    None = 0, // default value, will be set to this before the shared memory segment is initialised

    SingleProducerSingleConsumerVariableMessageLength,
    SingleProducerMultiConsumerVariableMessageLength,
    MultiProducerSingleConsumerVariableMessageLength,
  };

}
