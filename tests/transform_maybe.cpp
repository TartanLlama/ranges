#include <catch2/catch.hpp>
#include <optional>
#include <iostream>
#include "tl/enumerate.hpp"
#include "tl/transform_maybe.hpp"

TEST_CASE("transform_maybe") {
   std::vector<int> a{ 0,1,2,3,4 };
   auto f = [](auto i) { return (i % 2 == 0) ? std::optional(i / 2) : std::nullopt; };
   for (auto&& [i,e] : a | tl::views::transform_maybe(f) | tl::views::enumerate) {
      REQUIRE(i == e);
   }
}