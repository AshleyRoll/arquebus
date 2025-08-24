#pragma once

#include "spsc_var_msg_len_host.hpp"
#include "spsc_var_msg_len_producer.hpp"
#include "spsc_var_msg_len_consumer.hpp"


#include <arquebus/arquebus_export.hpp>
#include <arquebus/version.hpp>

#include <string_view>

namespace arquebus {

  // get the current library version
  ARQUEBUS_EXPORT auto version() -> version_details const&;


  template<std::uint8_t Size2NBits>
  ARQUEBUS_EXPORT auto make_varariable_length_message_spsc_queue_host(std::string_view name)
  {
    return spsc_var_msg_len_host<Size2NBits>(name);
  }

  template<std::uint8_t Size2NBits>
  ARQUEBUS_EXPORT auto make_varariable_length_message_spsc_queue_producer(std::string_view name)
  {
    return spsc_var_msg_len_producer<Size2NBits>(name);
  }

  template<std::uint8_t Size2NBits>
  ARQUEBUS_EXPORT auto make_varariable_length_message_spsc_queue_consumer(std::string_view name)
  {
    return spsc_var_msg_len_consumer<Size2NBits>(name);
  }


}
