#include <catch2/catch.hpp>
#include <vector>
#include "tl/zip_transform.hpp"

TEST_CASE("zip_transform") {
   std::vector<int> a{ 0,1,2 };
   std::vector<int> b{ 3,4,5 };
   std::vector<int> c{ 6,7,8 };

   std::vector<int> results{ 9, 12, 15 };
   auto trans = tl::views::zip_transform([](auto i, auto j, auto k) { return i + j + k; }, a, b, c);
   for (auto res : tl::views::zip_transform(std::equal_to{}, trans, results)) {
      REQUIRE(res);
   }
}