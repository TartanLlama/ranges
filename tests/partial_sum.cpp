#include "tl/partial_sum.hpp"
#include "tl/zip.hpp"
#include <catch2/catch.hpp>
#include <vector>

TEST_CASE("partial sum") {
   std::vector<int> v{ 0,1,2,3,4 };
   std::vector<int> res{
      0, 1, 3, 6, 10
   };

   for (auto&& [a, b] : tl::views::zip(tl::views::partial_sum(v, std::plus{}), res)) {
      REQUIRE(a == b);
   }
}

TEST_CASE("partial sum pipe") {
   std::vector<int> v{ 0,1,2,3,4 };
   std::vector<int> res{
      0, 1, 3, 6, 10
   };

   for (auto&& [a, b] : tl::views::zip(v | tl::views::partial_sum(), res)) {
      REQUIRE(a == b);
   }
}

TEST_CASE("partial multiply") {
   std::vector<int> v{ 1,2,3,4 };
   std::vector<int> res{
      1, 2, 6, 24
   };

   for (auto&& [a, b] : tl::views::zip(tl::views::partial_sum(v, std::multiplies{}), res)) {
      REQUIRE(a == b);
   }
}