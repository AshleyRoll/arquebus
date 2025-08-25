#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "arquebus/impl/buffer_size.hpp"


TEST_CASE("buffer_size computes pow2 size and mask", "[arquebus]")
{
  using namespace arquebus::impl;

  CHECK(buffer_size<1>::Bytes == (1u << 1));
  CHECK(buffer_size<2>::Bytes == (1u << 2));
  CHECK(buffer_size<3>::Bytes == (1u << 3));
  CHECK(buffer_size<4>::Bytes == (1u << 4));
  CHECK(buffer_size<5>::Bytes == (1u << 5));
  CHECK(buffer_size<6>::Bytes == (1u << 6));
  CHECK(buffer_size<7>::Bytes == (1u << 7));
  CHECK(buffer_size<8>::Bytes == (1u << 8));
  CHECK(buffer_size<20>::Bytes == (1u << 20));
  CHECK(buffer_size<30>::Bytes == (1u << 30));
  CHECK(buffer_size<31>::Bytes == (1u << 31));

  CHECK(buffer_size<1>::Mask == 0b0000'0000'0000'0000'0000'0000'0000'0001ul);
  CHECK(buffer_size<2>::Mask == 0b0000'0000'0000'0000'0000'0000'0000'0011ul);
  CHECK(buffer_size<3>::Mask == 0b0000'0000'0000'0000'0000'0000'0000'0111ul);
  CHECK(buffer_size<4>::Mask == 0b0000'0000'0000'0000'0000'0000'0000'1111ul);
  CHECK(buffer_size<5>::Mask == 0b0000'0000'0000'0000'0000'0000'0001'1111ul);
  CHECK(buffer_size<6>::Mask == 0b0000'0000'0000'0000'0000'0000'0011'1111ul);
  CHECK(buffer_size<7>::Mask == 0b0000'0000'0000'0000'0000'0000'0111'1111ul);
  CHECK(buffer_size<8>::Mask == 0b0000'0000'0000'0000'0000'0000'1111'1111ul);
  CHECK(buffer_size<20>::Mask == 0b0000'0000'0000'1111'1111'1111'1111'1111ul);
  CHECK(buffer_size<30>::Mask == 0b0011'1111'1111'1111'1111'1111'1111'1111ul);
  CHECK(buffer_size<31>::Mask == 0b0111'1111'1111'1111'1111'1111'1111'1111ul);
}

TEST_CASE("buffer_size to_offset", "[arquebus]")
{
  using namespace arquebus::impl;

  CHECK(buffer_size<8>::to_offset(0u) == 0u);
  CHECK(buffer_size<8>::to_offset(200) == 200u);
  CHECK(buffer_size<8>::to_offset(255) == 255u);
  CHECK(buffer_size<8>::to_offset(256) == 0u);
  CHECK(buffer_size<8>::to_offset(1023) == 255u);
  CHECK(buffer_size<8>::to_offset(1024) == 0u);
}

TEST_CASE("buffer_size distance_to_buffer_start", "[arquebus]")
{
  using namespace arquebus::impl;
  using B = buffer_size<8>;

  auto i = GENERATE(0, 1, 2, 3, 100, 200, 254, 255, 256, 257, 400, 500, 510, 511, 512, 513, 514);

  CHECK(B::to_offset(static_cast<std::uint64_t>(i) + B::distance_to_buffer_start(static_cast<std::uint64_t>(i))) == 0);
}
