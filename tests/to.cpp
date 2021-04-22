#include <catch2/catch.hpp>
#include "tl/to.hpp"
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include "tl/cycle.hpp"

TEST_CASE("copy ctor") {
   std::vector<int> a{ 0,1,2 };
   auto b = tl::to<std::vector<int>>(a);
   REQUIRE(a == b);

   auto c = tl::to<std::vector>(a);
   REQUIRE(a == c);
}

TEST_CASE("list to vector") {
   std::list<int> a{ 0,1,2 };
   auto b = tl::to<std::vector<int>>(a);
   REQUIRE(std::ranges::equal(a, b));

   auto c = tl::to<std::vector>(a);
   REQUIRE(std::ranges::equal(a,c));
}

TEST_CASE("nested") {
   std::list<std::forward_list<int>> lst = { {0, 1, 2, 3}, {4, 5, 6, 7} };
   auto vec1 = tl::to<std::vector<std::vector<int>>>(lst);
   auto vec2 = tl::to<std::vector<std::deque<double>>>(lst);

   auto list_it = lst.begin();
   for (auto& inner : vec1) {
      REQUIRE(std::ranges::equal(inner, *list_it));
      ++list_it;
   }

   list_it = lst.begin();
   for (auto& inner : vec2) {
      REQUIRE(std::ranges::equal(inner, *list_it));
      ++list_it;
   }
}

TEST_CASE("pipe") {
   std::vector<int> a{ 0,1,2 };
   auto b = a | tl::to<std::vector<int>>();
   REQUIRE(a == b);

   auto c = a | tl::to<std::vector>();
   REQUIRE(a == c);
}

TEST_CASE("transform") {
   std::list<int> a{ 0, 1, 2 };
   auto b = a | tl::views::cycle | std::views::take(10) | std::views::transform([](auto x) { return x * 2; }) | tl::to<std::vector>();
   std::vector<int> res{ 0, 2, 4, 0, 2, 4, 0, 2, 4, 0 };
   REQUIRE(b == res);
}

#include <memory_resource>
TEST_CASE("allocator") {
   std::list<int> a{ 0, 1, 2 };
   
   char buffer[256];
   std::pmr::monotonic_buffer_resource res(std::begin(buffer), std::size(buffer));
   auto vec1 = tl::to<std::pmr::vector<int>>(a, &res);
   REQUIRE(std::ranges::equal(a, vec1));
   auto vec2 = a | tl::to<std::pmr::vector<int>>(&res);
   REQUIRE(std::ranges::equal(a, vec2));
}

#include <map>
TEST_CASE("associative") {
   std::map<int, int> a { {0, 1}, { 2,3 }, { 4,5 }};
   auto vec = a | tl::to<std::vector>();
   REQUIRE(std::ranges::equal(vec, a));
   auto map = vec | tl::to<std::map>();
   REQUIRE(std::ranges::equal(map, a));
}