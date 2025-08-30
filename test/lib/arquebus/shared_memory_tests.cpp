#include <catch2/catch_test_macros.hpp>

#include "arquebus/impl/shared_memory_helper.hpp"

#include <array>
#include <filesystem>

struct test_memory
{
  std::array<int, 100> data;  // NOLINT
};


TEST_CASE("shared_memory_owner can create and clean up", "[arquebus]")
{
  using namespace arquebus::impl;

  shared_memory_owner<test_memory> owner{"test1"};

  CHECK(not std::filesystem::exists("/dev/shm" + owner.name()));

  REQUIRE_NOTHROW(owner.create());

  CHECK(std::filesystem::exists("/dev/shm" + owner.name()));

  auto const *ptr = owner.mapping();

  REQUIRE(ptr != nullptr);

  owner.close();

  CHECK(not std::filesystem::exists("/dev/shm" + owner.name()));
}

TEST_CASE("shared_memory_user fails open when owner not created", "[arquebus]")
{
  using namespace arquebus::impl;

  shared_memory_user<test_memory> user{"test2"};

  REQUIRE_THROWS( user.attach() );
}

TEST_CASE("can map same data between owner and user", "[arquebus]")
{
  using namespace arquebus::impl;

  shared_memory_owner<test_memory> owner{"test3"};
  shared_memory_user<test_memory> user{"test3"};

  REQUIRE_NOTHROW(owner.create());
  REQUIRE_NOTHROW(user.attach());

  auto *oMap = owner.mapping();
  auto *uMap = user.mapping();

  CHECK(oMap != uMap);

  CHECK(oMap->data[0] == 0);
  CHECK(uMap->data[0] == 0);

  oMap->data[0] = 1;
  CHECK(uMap->data[0] == 1);
}

TEST_CASE("can not create shared memory if it exists", "[arquebus]")
{
  using namespace arquebus::impl;

  shared_memory_owner<test_memory> owner1{"test4"};
  shared_memory_owner<test_memory> owner2{"test4"};

  REQUIRE_NOTHROW(owner1.create());
  REQUIRE_THROWS(owner2.create());

}
