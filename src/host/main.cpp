#include <arquebus/spsc/var_msg/host.hpp>
#include <arquebus/version.hpp>

#include <exception>
#include <fmt/core.h>

static constexpr auto QueueSizeBits = 23u;

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  try {
    static constexpr auto Bits = 23u;
    static constexpr auto Size = (1u << Bits);

    static_assert(Size == 0x800000u); // NOLINT

    fmt::println("starting host...");
    auto ver = arquebus::version();
    fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

    fmt::println("creating queue...");
    arquebus::spsc::var_msg::host<QueueSizeBits> queue{ "spsc1" };
    fmt::println("initialising queue...");
    queue.create();

    fmt::println("..done");
    fmt::println("Press Enter to quit");

    getchar();  // NOLINT

    fmt::println("Exiting");

    return 0;
  } catch (std::exception const &e) {
    fmt::println("error: {}", e.what());
    return 1;
  }
}
