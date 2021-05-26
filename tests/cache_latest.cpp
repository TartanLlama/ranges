#include <catch2/catch.hpp>
#include <vector>
#include <ranges>
#include "tl/cache_latest.hpp"

TEST_CASE("cache latest") {
   std::vector<int> a{ 0,1,2 };

   auto r = a | std::views::transform([i = 0] (auto) mutable  { ++i; return i; }) | tl::views::cache_latest;
   auto it = r.begin();
   REQUIRE(*it == 1);
   REQUIRE(*it == 1);
   REQUIRE(*it == 1);
   REQUIRE(*it == 1);
   REQUIRE(*it == 1);
}