#include <catch2/catch.hpp>
#include <vector>
#include <iostream>
#include "tl/chunk.hpp"
#include "tl/to.hpp"
#include "tl/enumerate.hpp"

TEST_CASE("chunk") {
   std::vector<int> a{ 0,1,2,3,4,5,6,7,8,9,10 };
   std::vector<std::vector<int>> groups{ {0,1,2,3}, {4,5,6,7}, {8,9,10} };

   for (auto&& [idx, group] : a | tl::views::chunk(4) | tl::views::enumerate) {
      REQUIRE(std::ranges::equal(group, groups[idx]));
   }
}