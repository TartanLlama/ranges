#include <catch2/catch.hpp>
#include <ranges>
#include <iostream>
#include "tl/transform_join.hpp"

TEST_CASE("transform join view") {
  std::vector<int> a{ 0 ,1 ,2 };
  for (auto&& e : a | tl::views::transform_join([](auto i) { return std::views::iota(0, i); })) {
    std::cout << e;
  }
}

TEST_CASE("transform join container") {
  std::vector<int> a{ 0 ,1 ,2 };
  for (auto&& e : a | tl::views::transform_join([](auto i) { return std::vector<int>{i,i}; })) {
    std::cout << e;
  }
}