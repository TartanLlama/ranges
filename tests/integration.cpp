#include "tl/cartesian_product.hpp"
#include "tl/to.hpp"
#include "tl/functional/curry.hpp"
#include <catch2/catch.hpp>
#include <iostream>
#include <vector>
#include <ranges>

template <class F, std::ranges::forward_range... Vs>
requires(std::regular_invocable<F,
   std::ranges::range_reference_t<Vs>...>)
   auto find_tuples_satisfying(F f, Vs&&... vs) {
   return tl::views::cartesian_product(std::forward<Vs>(vs)...)
      | std::views::filter(tl::uncurry(f))
      | tl::to<std::vector>();
}


TEST_CASE("find tuples satisfying") {
   std::vector<int> a{ 0,1,2 };
   std::vector<int> b{ 2,3,4 };

   for (auto&& [i, j] : find_tuples_satisfying(std::ranges::equal_to{}, a, b)) {
      REQUIRE(i == 2);
      REQUIRE(j == 2);
   }
}