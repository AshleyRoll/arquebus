#include <arquebus/spsc/var_msg/producer.hpp>
#include <arquebus/version.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
//#include <thread>

#include <fmt/core.h>

static constexpr auto QueueSizeBits = 23u;
static constexpr auto QueueMessageReservationSize = 1'024u;

static constexpr auto NumMessages = 10'000;  //'000;
//static constexpr auto Delay = std::chrono::milliseconds{ 10 };

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

    std::uint64_t totalSize{ 0 };

    auto start = std::chrono::high_resolution_clock::now();

    for (std::uint64_t i = 0; i < NumMessages; ++i) {
      std::uint32_t const size = (i & 0x0Fu) + 2;
      auto message = queue.allocate_write(size);
      // emulate writing message data
      std::ranges::fill(message, std::byte{ 1 });
      totalSize += size;
      queue.flush();
      fmt::print("."); if(i % 100 == 0) { fmt::print("\n"); }  // NOLINT
      //std::this_thread::sleep_for(Delay);
    }

    // send the "end" 1 byte message
    auto message = queue.allocate_write(1);
    std::ranges::fill(message, std::byte{ 1 });
    totalSize += 1;
    queue.flush();

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    fmt::println("..done");
    fmt::println("duration: {}ms", elapsed.count());
    fmt::println("total size: {}", totalSize);

    fmt::println("Press Enter to quit");

    getchar();  // NOLINT

    fmt::println("Exiting");

    return 0;
  } catch (std::exception const &e) {
    fmt::println("error: {}", e.what());
    return 1;
  }
}
