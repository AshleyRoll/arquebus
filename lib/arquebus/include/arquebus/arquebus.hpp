#pragma once

#include "arquebus/spsc/var_msg/consumer.hpp"
#include "arquebus/spsc/var_msg/host.hpp"
#include "arquebus/spsc/var_msg/producer.hpp"


#include "arquebus/version.hpp"

namespace arquebus {

  // get the current library version
  inline auto version() -> version_details { return {}; }


}  // namespace arquebus
