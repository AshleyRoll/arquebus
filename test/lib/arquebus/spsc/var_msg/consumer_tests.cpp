#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "arquebus/spsc/var_msg/consumer.hpp"
#include "arquebus/spsc/var_msg/host.hpp"
#include "arquebus/spsc/var_msg/producer.hpp"

#include <concepts>
#include <cstdint>
#include <numeric>
#include <string_view>

// NOLINTBEGIN(*-magic-numbers, *-identifier-length, *-pointer-arithmetic, *-unused-variable)

namespace {

  template<std::unsigned_integral TMessageSize>
  void test_can_receive_messages(std::string_view name)
  {
    using namespace arquebus::spsc::var_msg;
    using namespace arquebus::impl;
    using Catch::Matchers::RangeEquals;

    // 2^6 = 64 bytes of queue
    using SizeType = TMessageSize;
    using HostType = host<6, SizeType>;
    using ProducerType = producer<6, 20, SizeType>;
    using ConsumerType = consumer<6, SizeType>;

    HostType host{ name };
    ProducerType prod{ name };
    ConsumerType cons{ name };

    host.create(danger_delete_existing_shared_memory_segment_tag{});
    prod.attach();
    cons.attach();


    // this will wrap eventually
    for (int i = 0; i < 10; i++) {
      auto w1 = prod.allocate_write(10);
      std::iota(w1.begin(), w1.end(), i);
      auto r1 = cons.read();
      CHECK(not r1.has_value());
      prod.flush();
      r1 = cons.read();
      REQUIRE(r1.has_value());
      if(r1.has_value()) {  // avoid unchecked optional warning
        CHECK(r1.value().size() == w1.size());
        CHECK_THAT(r1.value(), RangeEquals(w1));
      }
    }
  }

}  // namespace


TEST_CASE("spsc::var_msg::consumer can receive messages uint8_t size", "[arquebus][spsc][consumer]")
{
  test_can_receive_messages<std::uint8_t>("spsc-var_msg-can_receive_test-8");
}

TEST_CASE("spsc::var_msg::consumer can receive messages uint16_t size", "[arquebus][spsc][consumer]")
{
  test_can_receive_messages<std::uint16_t>("spsc-var_msg-can_receive_test-16");
}

TEST_CASE("spsc::var_msg::consumer can receive messages uint32_t size", "[arquebus][spsc][consumer]")
{
  test_can_receive_messages<std::uint32_t>("spsc-var_msg-can_receive_test-32");
}

TEST_CASE("spsc::var_msg::consumer can receive messages uint64_t size", "[arquebus][spsc][consumer]")
{
  test_can_receive_messages<std::uint64_t>("spsc-var_msg-can_receive_test-64");
}


TEST_CASE("spsc::var_msg::consumer throws on overrun", "[arquebus][spsc][consumer]")
{
  using namespace arquebus::spsc::var_msg;
  using namespace arquebus::impl;
  using Catch::Matchers::RangeEquals;

  // 2^6 = 64 bytes of queue
  using HostType = host<6>;
  using ProducerType = producer<6, 20>;
  using ConsumerType = consumer<6>;

  std::string_view const name{ "spsc-var_msg-throw_on_overrun" };

  HostType host{ name };
  ProducerType prod{ name };
  ConsumerType cons{ name };

  host.create(danger_delete_existing_shared_memory_segment_tag{});
  prod.attach();
  cons.attach();


  // this will eventually overrun as we are writing much more messages as we are reading
  REQUIRE_THROWS([&] {
    for (int i = 0; i < 30; i++) {
      auto w1 = prod.allocate_write(10);
      auto w2 = prod.allocate_write(10);
      CHECK(w1.size() == 10);
      CHECK(w2.size() == 10);
      prod.flush();
      auto r1 = cons.read();
      CHECK((r1.has_value() and r1.value().size() == 10));
    }
  }());
}

// NOLINTEND(*-magic-numbers, *-identifier-length, *-pointer-arithmetic, *-unused-variable)
