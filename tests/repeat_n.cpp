#include <catch2/catch.hpp>
#include <vector>
#include <tl/repeat_n.hpp>
#include <ranges>
#include <iostream>

TEST_CASE("repeat_n") {
   for (auto i : tl::views::repeat_n(42, 10)) {
      REQUIRE(i == 42);
   }
}
