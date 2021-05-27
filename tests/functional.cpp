#include <catch2/catch.hpp>
#include <tuple>
#include "tl/functional/curry.hpp"

TEST_CASE("curry") {
   //clang-format off
   tl::uncurry([](auto a, auto b) {
      REQUIRE(a == 42);
      REQUIRE(b == 69);
   })(std::pair(42, 69));

   tl::curry([](std::tuple<int, int> t) {
      REQUIRE(std::get<0>(t) == 42);
      REQUIRE(std::get<1>(t) == 69);
   })(42, 69);

   tl::curry([](std::tuple<int, int, double> t) {
      REQUIRE(std::get<0>(t) == 42);
      REQUIRE(std::get<1>(t) == 69);
      REQUIRE(std::get<2>(t) == 42.69);
   })(42, 69, 42.69);

   tl::curry(tl::uncurry([](int a, int b) {
      REQUIRE(a == 42);
      REQUIRE(b == 69);
   }))(std::pair(42, 69));
   //clang-format on
}