#pragma once

#include <cstdint>

namespace arquebus::impl {

  template<std::uint8_t Size2NBits>
  struct buffer_size
  {
    static constexpr auto NBits = Size2NBits;
    static constexpr auto Bytes = std::uint64_t{ 1ul << Size2NBits };
    static constexpr auto Mask = std::uint64_t{ Bytes - 1 };

    static auto to_offset(std::uint64_t index) -> std::uint64_t { return index bitand Mask; }
    static auto to_generation(std::uint64_t index) -> std::uint64_t { return index >> NBits; }

    static auto distance_to_buffer_start(std::uint64_t index) -> std::uint64_t { return Bytes - to_offset(index); }
  };


}  // namespace arquebus::impl
