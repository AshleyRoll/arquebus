#include <arquebus/arquebus.hpp>
#include <fmt/core.h>

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  fmt::println("starting consumer...");

  auto ver = arquebus::version();
  fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

  return 0;
}
