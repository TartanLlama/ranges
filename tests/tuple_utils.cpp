#include <catch2/catch.hpp>
#include "tl/utility/tuple_utils.hpp"
#include <ranges>

TEST_CASE("tuple_zip") {
   std::tuple a{ 0, 1 };
   std::tuple b{ 3, 4 };
   auto res = tl::tuple_zip(a, b);
   REQUIRE(std::get<0>(res) == std::pair(0, 3));
   REQUIRE(std::get<1>(res) == std::pair(1, 4));
}

TEST_CASE("tuple fold") {
   std::tuple a{ 4 ,1, 3 };
   auto res = tl::tuple_fold(a, 255, std::ref(std::ranges::min));
   REQUIRE(res == 1);
}