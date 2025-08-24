
#include <arquebus/arquebus.hpp>

namespace {
  using namespace arquebus;

  constexpr version_details CurrentVersion{};

}


namespace arquebus {

  ARQUEBUS_EXPORT auto version() -> version_details const& { return CurrentVersion; }

}
