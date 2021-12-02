#include <catch2/catch.hpp>
#include "tl/enumerate.hpp"
#include "tl/generate.hpp"
#include <ranges>

TEST_CASE("generate") {
   for (auto [i, x] : 
      tl::views::generate([x = 0]() mutable { return x++; }) 
      | std::views::take(10) 
      | tl::views::enumerate) {
      REQUIRE(i == x);
   }
}