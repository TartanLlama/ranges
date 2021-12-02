#include <catch2/catch.hpp>
#include "tl/enumerate.hpp"
#include "tl/generate_n.hpp"
#include <ranges>

TEST_CASE("generate n") {
   for (auto [i, x] :
      tl::views::generate_n([x = 0]() mutable { return x++; }, 10)
      | tl::views::enumerate) {
      REQUIRE(i == x);
   }
}