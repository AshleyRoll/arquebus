#include <arquebus/arquebus.hpp>
#include <fmt/core.h>

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  static constexpr auto Bits = 23;
  static constexpr auto Size = (1 << Bits);
  static_assert(Size == 0x800000);

  fmt::println("starting host...");
  auto ver = arquebus::version();
  fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

  fmt::println("creating queue...");
  arquebus::spsc_var_msg_len_host<23> queue{"spsc1"};
  fmt::println("initialising queue...");
  queue.initialise();

  fmt::println("..done");
  fmt::println("Press Enter to quit");

  getchar();

  fmt::println("Exiting");

  return 0;
}
