#include <arquebus/spsc/var_msg/producer.hpp>
#include <arquebus/version.hpp>

#include <exception>
#include <fmt/core.h>

static constexpr auto QueueSizeBits = 23u;
static constexpr auto QueueMessageReservationSize = 100'000u;

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  try {
    fmt::println("starting producer...");
    auto ver = arquebus::version();
    fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

    fmt::println("creating queue...");
    arquebus::spsc::var_msg::producer<QueueSizeBits, QueueMessageReservationSize> queue{ "spsc1" };
    fmt::println("attaching queue...");
    queue.attach();

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
