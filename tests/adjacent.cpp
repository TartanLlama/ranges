#include <catch2/catch.hpp>
#include "tl/adjacent.hpp"
#include "tl/zip.hpp"
#include <iostream>
#include <vector>
template<class>struct TC;
TEST_CASE("adjacent") {
   std::vector<int> a{ 0,1,2,3,4,5,6 };
   std::vector<std::tuple<int, int, int>> results{
      {0,1,2},
      {1,2,3},
      {2,3,4},
      {3,4,5},
      {4,5,6}
   };

   REQUIRE(tl::views::adjacent<3>(a).size() == 5);

   for (auto const& [a,b] : tl::views::zip(tl::views::adjacent<3>(a), results)) {
      REQUIRE(a == b);
   }
}
