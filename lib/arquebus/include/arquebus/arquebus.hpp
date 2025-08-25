#pragma once

#include "spsc_var_msg_len_consumer.hpp"
#include "spsc_var_msg_len_host.hpp"
#include "spsc_var_msg_len_producer.hpp"


#include <arquebus/version.hpp>

namespace arquebus {

  // get the current library version
  inline auto version() -> version_details { return {}; }


}  // namespace arquebus
