#include "tl/utility/meta.hpp"
#include <catch2/catch.hpp>
#include <tuple>
#include <concepts>

using namespace tl::meta;

TEST_CASE("repeat_into") {
   STATIC_REQUIRE(std::same_as<repeat_into<int, 5, std::tuple>, std::tuple<int, int, int, int, int>>);
}

TEST_CASE("adjacent") {
   STATIC_REQUIRE(std::same_as<repeat_into<int, 2, partial<std::tuple, const int&>::template apply>::type, std::tuple<const int&, int, int>>);
}