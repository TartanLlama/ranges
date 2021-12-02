#include <catch2/catch.hpp>
#include "tl/adjacent_transform.hpp"
#include "tl/zip.hpp"
#include <iostream>
#include <vector>

TEST_CASE("adjacent_transform") {
   std::vector<int> a{ 0,1,2,3,4,5,6 };
   std::vector<int> results{
      3,
      6,
      9,
      12,
      15
   };

   std::ranges::input_range auto r = tl::views::adjacent_transform<3>(a, [](int i, int j, int k) { return i + j + k; });

   for (auto [a, b] : tl::views::zip(tl::views::adjacent_transform<3>(a, [](int i, int j, int k) { return i + j + k; }), results)) {
      REQUIRE(a == b);
   }
}

TEST_CASE("pairwise") {
   std::vector<int> a{ 0,1,2,3 };
   std::vector<int> results{
      0,
      2,
      6
   };

   for (auto [a, b] : tl::views::zip(tl::views::pairwise_transform(a, std::multiplies{}), results)) {
      REQUIRE(a == b);
   }
}