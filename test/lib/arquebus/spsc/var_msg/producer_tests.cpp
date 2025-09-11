#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "arquebus/impl/shared_memory_helper.hpp"
#include "arquebus/spsc/var_msg/host.hpp"
#include "arquebus/spsc/var_msg/producer.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <span>
#include <string_view>

// NOLINTBEGIN(*-magic-numbers, *-identifier-length, *-pointer-arithmetic, *-unused-variable)

namespace {
  template<std::unsigned_integral TMessageSize>
  void test_can_record_messages(std::string_view name)
  {
    using namespace arquebus::spsc::var_msg;
    using namespace arquebus::impl;
    using Catch::Matchers::RangeEquals;

    // 2^6 = 64 bytes of queue
    using SizeType = TMessageSize;
    using HostType = host<6, SizeType>;
    using ProducerType = producer<6, 20, SizeType>;
    using ObserverType = shared_memory_user<typename ProducerType::QueueLayout>;

    HostType host{ name };
    ProducerType prod{ name };
    ObserverType obs{ name };

    host.create(danger_delete_existing_shared_memory_segment_tag{});
    prod.attach();
    obs.attach();

    auto *pQueue = obs.mapping();
    REQUIRE(pQueue != nullptr);

    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    auto w1 = prod.allocate_write(15);
    CHECK(w1.size() == 15);
    std::iota(w1.begin(), w1.end(), 1);
    CHECK(pQueue->read_index.load() == 0);
    prod.flush();
    CHECK(pQueue->read_index.load() == w1.size() + sizeof(SizeType));
    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    auto w2 = prod.allocate_write(5);
    CHECK(w2.size() == 5);
    std::iota(w2.begin(), w2.end(), 10);
    CHECK(pQueue->read_index.load() == w1.size() + sizeof(SizeType));
    prod.flush();
    CHECK(pQueue->read_index.load() == w1.size() + sizeof(SizeType) + w2.size() + sizeof(SizeType));
    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    // validate format
    auto &data = obs.mapping()->data;

    SizeType s{};

    std::memcpy(&s, &data[0], sizeof(SizeType));
    REQUIRE(s == w1.size());
    std::span<std::uint8_t const> m1{ &data[sizeof(SizeType)], w1.size() };
    CHECK_THAT(m1, RangeEquals(w1));

    std::memcpy(&s, &data[sizeof(SizeType) + w1.size()], sizeof(SizeType));
    REQUIRE(s == w2.size());
    std::span<std::uint8_t const> m2{ &data[sizeof(SizeType) + w1.size() + sizeof(SizeType)], w2.size() };
    CHECK_THAT(m2, RangeEquals(w2));
  }

  template<std::unsigned_integral TMessageSize>
  void test_wrap_queue(std::string_view name)
  {
    using namespace arquebus::spsc::var_msg;
    using namespace arquebus::impl;
    using Catch::Matchers::RangeEquals;

    // 2^6 = 64 bytes of queue
    using SizeType = TMessageSize;
    using HostType = host<6, SizeType>;
    using ProducerType = producer<6, 50, SizeType>;
    using ObserverType = shared_memory_user<typename ProducerType::QueueLayout>;

    HostType host{ name };
    ProducerType prod{ name };
    ObserverType obs{ name };

    host.create(danger_delete_existing_shared_memory_segment_tag{});
    prod.attach();
    obs.attach();

    auto *pQueue = obs.mapping();

    // fill with junk so we can detect correct "zero" write
    std::fill(std::begin(pQueue->data), std::end(pQueue->data), 0xA5);

    auto &data = obs.mapping()->data;
    SizeType s{};
    REQUIRE(pQueue != nullptr);

    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    auto w1 = prod.allocate_write(20);
    prod.flush();
    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    // should have a length recorded in location 0
    std::memcpy(&s, &data[0], sizeof(SizeType));
    REQUIRE(s == w1.size());

    [[maybe_unused]] auto w2 = prod.allocate_write(20);
    prod.flush();
    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    std::memcpy(&s, &data[sizeof(SizeType) + w1.size()], sizeof(SizeType));
    REQUIRE(s == 20);

    [[maybe_unused]] auto w3 = prod.allocate_write(35);
    prod.flush();
    CHECK(pQueue->write_index.load() >= pQueue->read_index.load());

    // should have a skip indicator because the next
    // message would not fit

    std::memcpy(&s, &data[sizeof(SizeType) + w1.size() + sizeof(SizeType) + w2.size()], sizeof(SizeType));
    REQUIRE(s == 0);

    // and now offset 0 should have a size of 20 as the second message should have wrapped
    std::memcpy(&s, &data[0], sizeof(SizeType));
    REQUIRE(s == w3.size());
  }

}  // namespace

TEST_CASE("spsc::var_msg::producer can record messages uint8_t size", "[arquebus][spsc][producer]")
{
  test_can_record_messages<std::uint8_t>("spsc-var_msg-can_record_test-8");
}

TEST_CASE("spsc::var_msg::producer can record messages uint16_t size", "[arquebus][spsc][producer]")
{
  test_can_record_messages<std::uint16_t>("spsc-var_msg-can_record_test-16");
}

TEST_CASE("spsc::var_msg::producer can record messages uint32_t size", "[arquebus][spsc][producer]")
{
  test_can_record_messages<std::uint32_t>("spsc-var_msg-can_record_test-32");
}

TEST_CASE("spsc::var_msg::producer can record messages uint64_t size", "[arquebus][spsc][producer]")
{
  test_can_record_messages<std::uint64_t>("spsc-var_msg-can_record_test-64");
}

TEST_CASE("spsc::var_msg::producer wraps queue uint8_t size", "[arquebus][spsc][producer]")
{
  test_wrap_queue<std::uint8_t>("spsc-var_msg-wrap_test-8");
}

TEST_CASE("spsc::var_msg::producer wraps queue uint16_t size", "[arquebus][spsc][producer]")
{
  test_wrap_queue<std::uint16_t>("spsc-var_msg-wrap_test-16");
}

TEST_CASE("spsc::var_msg::producer wraps queue uint32_t size", "[arquebus][spsc][producer]")
{
  test_wrap_queue<std::uint32_t>("spsc-var_msg-wrap_test-32");
}

TEST_CASE("spsc::var_msg::producer wraps queue uint64_t size", "[arquebus][spsc][producer]")
{
  // note that this case hits the wrap exactly at the boundary case
  test_wrap_queue<std::uint64_t>("spsc-var_msg-wrap_test-64");
}

// NOLINTEND(*-magic-numbers, *-identifier-length, *-pointer-arithmetic, *-unused-variable)
