#include <catch2/catch_test_macros.hpp>

#include "private/shared_memory.hpp"

#include <filesystem>

struct TestMemory
{
  int data[100];

};


TEST_CASE("shared_memory_owner can create and clean up", "[arquebus]")
{
  using namespace arquebus;

  shared_memory_owner<TestMemory> owner{"test1"};

  CHECK(not std::filesystem::exists("/dev/shm" + owner.name()));

  REQUIRE_NOTHROW(owner.create());

  CHECK(std::filesystem::exists("/dev/shm" + owner.name()));

  auto *ptr = owner.mapping();

  REQUIRE(ptr != nullptr);

  owner.close();

  CHECK(not std::filesystem::exists("/dev/shm" + owner.name()));

}

TEST_CASE("shared_memory_user fails open when owner not created", "[arquebus]")
{
  using namespace arquebus;

  shared_memory_user<TestMemory> user{"test2"};

  REQUIRE_NOTHROW( user.attach() == false );
}

TEST_CASE("can map same data between owner and user", "[arquebus]")
{
  using namespace arquebus;

  shared_memory_owner<TestMemory> owner{"test2"};
  shared_memory_user<TestMemory> user{"test2"};

  REQUIRE_NOTHROW(owner.create());
  REQUIRE_NOTHROW(user.attach());

  auto *o = owner.mapping();
  auto *u = user.mapping();

  CHECK(o != u);

  CHECK(o->data[0] == 0);
  CHECK(u->data[0] == 0);

  o->data[0] = 1;
  CHECK(u->data[0] == 1);

}
