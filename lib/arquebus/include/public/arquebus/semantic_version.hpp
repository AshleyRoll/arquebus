#pragma once

#include <charconv>
#include <cstdint>
#include <ranges>
#include <string_view>

namespace arquebus {

  struct semantic_version
  {
    std::uint16_t major{ 0 };
    std::uint16_t minor{ 0 };
    std::uint16_t patch{ 0 };

    static constexpr semantic_version parse(std::string_view version)
    {
      std::array<std::uint16_t, 3> parts{};
      std::size_t i = 0;

      for (auto part : std::ranges::views::split(version, '.') | std::views::take(3)) {
        if(part.size() == 0) {
          ++i;
          continue;
        }

        if(not std::from_chars(part.begin(), part.end(), parts.at(i++))) {
          throw std::invalid_argument("invalid semantic version string");
        }
      }

      return { parts[0], parts[1], parts[2] };
    }

    friend bool operator==(semantic_version const &lhs, semantic_version const &rhs) = default;
    friend auto operator<=>(semantic_version const &lhs, semantic_version const &rhs) = default;
  };

}  // namespace ardb
