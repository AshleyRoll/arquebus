#include <arquebus/spsc/var_msg/consumer.hpp>
#include <arquebus/version.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <exception>

#include <fmt/core.h>

static constexpr auto QueueSizeBits = 23u;

auto main(int /*argc*/, char const * /*argv*/[]) -> int
{
  try {
    fmt::println("starting consumer...");

    auto ver = arquebus::version();
    fmt::println("arquebus version: {} #{}", ver.version_string, ver.commit_short_hash);

    fmt::println("creating queue...");
    arquebus::spsc::var_msg::consumer<QueueSizeBits> queue{ "spsc1" };
    fmt::println("attaching queue...");
    queue.attach();

    std::uint64_t numMessages{ 0 };
    std::uint64_t totalSize{ 0 };
    std::uint64_t sum{ 0 };

    auto start = std::chrono::high_resolution_clock::now();

    bool done{ false };
    while (not done) {
      auto message = queue.read();
      if (message.has_value()) {
        numMessages += 1;
        totalSize += message.value().size();
        // emulate reading message data
        sum = std::ranges::fold_left(message.value(), sum, [](std::uint64_t acc, std::byte val) {
          return acc + static_cast<std::uint64_t>(val);
        });

        done = message.value().size() == 1;
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    fmt::println("..done");
    fmt::println("duration: {}ms", elapsed.count());
    fmt::println("total size: {}", totalSize);
    fmt::println("total sum: {}", sum);
    fmt::println("messages: {}", numMessages);

    fmt::println("Press Enter to quit");

    getchar();  // NOLINT

    fmt::println("Exiting");
    return 0;
  } catch (std::exception const &e) {
    fmt::println("error: {}", e.what());
    return 1;
  }
}
