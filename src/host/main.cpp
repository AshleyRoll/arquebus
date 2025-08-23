#include <arquebus/arquebus.hpp>
#include <fmt/core.h>

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  fmt::println("starting host...");

  return arquebus::foo();
}
