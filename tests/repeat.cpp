#include <catch2/catch.hpp>
#include <vector>
#include <tl/repeat.hpp>
#include <ranges>
#include <iostream>

TEST_CASE("repeat") {
  for (auto i : tl::views::repeat(42) | std::views::take(10)) {
    REQUIRE(i == 42);
  }
}
