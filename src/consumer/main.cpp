#include <arquebus/arquebus.hpp>
#include <fmt/core.h>

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  fmt::println("starting consumer...");

  auto ver = arquebus::version();
  fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

  fmt::println("creating queue...");
  auto queue = arquebus::make_varariable_length_message_spsc_queue_consumer<23>("spsc1");
  fmt::println("attaching queue...");
  queue.attach();

  fmt::println("..done");
  fmt::println("Press Enter to quit");

  getchar();

  fmt::println("Exiting");
  return 0;
}
