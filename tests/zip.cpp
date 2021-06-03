#include <vector>
#include <catch2/catch.hpp>
#include "tl/zip.hpp"
#include <iostream>
#include <list>
#include "tl/functional/curry.hpp"
#include "tl/weaken.hpp"

auto tuple_eq(auto&& lhs, auto&& rhs) {
   return tl::tuple_fold(
      tl::tuple_transform(std::equal_to{}, lhs, rhs), 
      true ,std::logical_and{});
}

TEST_CASE("sized random-access") {
   std::vector v1 = { 1, 2 };
   std::vector v2 = { 'a', 'b', 'c' };

   auto r = tl::views::zip(v1, v2);
   auto it = r.begin();
   REQUIRE(tuple_eq(*it++, std::pair(1, 'a')));
   REQUIRE(tuple_eq(*it++, std::pair(2, 'b')));
   REQUIRE(it == r.end());

   it = r.end();
   REQUIRE(tuple_eq(*--it, std::pair(2, 'b')));
   REQUIRE(tuple_eq(*--it, std::pair(1, 'a')));
   REQUIRE(it == r.begin());
}

TEST_CASE("bidirectional common") {
   std::list v1 = { 1, 2 };
   std::list v2 = { 'a', 'b', 'c' };

   auto r = tl::views::zip(v1, v2);
   auto it = r.begin();
   REQUIRE(tuple_eq(*it++, std::pair(1, 'a')));
   REQUIRE(tuple_eq(*it++, std::pair(2, 'b')));
   REQUIRE(it == r.end());

   std::ranges::advance(it, r.end());
   REQUIRE(tuple_eq(*--it, std::pair(2, 'b')));
   REQUIRE(tuple_eq(*--it, std::pair(1, 'a')));
   REQUIRE(it == r.begin());
}

TEST_CASE("bidirectional non-common") {
   std::vector v1_common = { 1, 2 };
   std::vector v2_common = { 'a', 'b', 'c' };
   auto v1 = v1_common | tl::views::weaken<tl::weakening::bidirectional, tl::weakening::non_common>;
   auto v2 = v2_common | tl::views::weaken<tl::weakening::bidirectional, tl::weakening::non_common>;

   auto r = tl::views::zip(v1, v2);
   auto it = r.begin();
   REQUIRE(tuple_eq(*it++, std::pair(1, 'a')));
   REQUIRE(tuple_eq(*it++, std::pair(2, 'b')));
   REQUIRE(it == r.end());

   std::ranges::advance(it, r.end());
   REQUIRE(tuple_eq(*--it, std::pair(2, 'b')));
   REQUIRE(tuple_eq(*--it, std::pair(1, 'a')));
   REQUIRE(it == r.begin());
}