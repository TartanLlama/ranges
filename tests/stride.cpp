#include <catch2/catch.hpp>
#include <vector>
#include <tl/stride.hpp>
#include <tl/cartesian_product.hpp>
#include <ranges>
#include <iostream>
#include <list>

TEST_CASE("stride") {
   std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6 };

   int i = 0;
   for (auto&& e : tl::stride_view(v, 3)) {
      REQUIRE(e == i);
      i += 3;
   }
}
template<class>struct TC;
TEST_CASE("stride pipe") {
   std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6 };

   int i = 0;
   for (auto&& e : v | tl::views::stride(3)) {
      REQUIRE(e == i);
      i += 3;
   }
}

TEST_CASE("longer pipeline") {
   std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6 };

   int i = 0;
   for (auto&& e : v | tl::views::stride(3) | std::views::transform([](auto i) { return i * 2; })) {
      REQUIRE(e == i*2);
      i += 3;
   }

   std::vector<int> small_v{ 0,1,2 };
 
   i = 0;
   for (auto&& e : tl::views::cartesian_product(small_v, small_v, small_v) | tl::views::stride(9)) {
      REQUIRE(std::get<0>(e) == i);
      ++i;
   }
}

TEST_CASE("size") {
   std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6 };
   auto s1 = v | tl::views::stride(1);
   REQUIRE(std::ranges::size(s1) == 7);
   auto s2 = v | tl::views::stride(2);
   REQUIRE(std::ranges::size(s2) == 4);
   auto s3 = v | tl::views::stride(3);
   REQUIRE(std::ranges::size(s3) == 3);
   auto s4 = v | tl::views::stride(4);
   REQUIRE(std::ranges::size(s4) == 2);
   auto s5 = v | tl::views::stride(5);
   REQUIRE(std::ranges::size(s5) == 2);
}

TEST_CASE("offset") {
   std::list<int> l {0, 1, 2, 3};

   {
      auto v = tl::stride_view(l, 3);
      auto it = v.begin();
      REQUIRE(*it == 0);
      ++it;
      REQUIRE(*it == 3);
      ++it;
      --it;
      REQUIRE(*it == 3);
   }

   {
      auto v = tl::stride_view(l | std::views::filter([](auto&&) { return true; }), 3);
      auto it = v.begin();
      REQUIRE(*it == 0);
      ++it;
      REQUIRE(*it == 3);
      ++it;
      --it;
      REQUIRE(*it == 3);
   }
}