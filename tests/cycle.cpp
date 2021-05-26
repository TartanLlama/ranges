#include <catch2/catch.hpp>
#include <vector>
#include <iostream>
#include <list>
#include "tl/cycle.hpp"

TEST_CASE("cycle") {
   std::vector<int> a{ 0, 1, 2 };

   int i = 0;
   for (auto&& item : a | tl::views::cycle | std::views::take(20)) {
      REQUIRE(item == (i % 3));
      ++i;
   }
}

TEST_CASE("bidirectional") {
   std::list<int> a{ 0, 2, 1};
   auto cycle = a | tl::views::cycle;
   auto it = std::ranges::begin(cycle);
   for (auto i = 0; i < 20; ++i) {
      REQUIRE((i % 3) == *it);
      --it;
   }
}

TEST_CASE("random access") {
   std::vector<int> a{ 0, 1, 2, 3 };
   auto cycle = a | tl::views::cycle;
   auto it = std::ranges::begin(cycle);
   REQUIRE(*(it + 4) == 0);
   it += 3;
   REQUIRE(*it == 3);
   it += 2;
   REQUIRE(*it == 1);
   it -= 4;
   REQUIRE(*it == 1);
}