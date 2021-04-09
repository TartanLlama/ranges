#include "tl/enumerate.hpp"
#include <catch2/catch.hpp>
#include <iostream>
#include <vector>

TEST_CASE("basic vector") {
  std::vector a{1, 2, 3};
  int i = 0;
  for (auto&& [index, item] : a | tl::views::enumerate) {
    REQUIRE(index == i);
    REQUIRE(item == i + 1);
    ++i;
  }
}

TEST_CASE("basic const vector") {
  const std::vector a{1, 2, 3};
  int i = 0;
  for (auto&& [index, item] : a | tl::views::enumerate) {
    REQUIRE(index == i);
    REQUIRE(item == i + 1);
    ++i;
  }
}

TEST_CASE("transform") {
  const std::vector a{1, 2, 3};
  int i = 0;
  for (auto&& [index, item] :
       (a | std::views::transform([](auto i) { return i - 1; }) |
        tl::views::enumerate)) {
    REQUIRE(index == i);
    REQUIRE(item == i);
    ++i;
  }
}

TEST_CASE("modify vector") {
  std::vector a{1, 2, 3};
  int i = 0;
  for (auto&& [index, item] : a | tl::views::enumerate) {
    REQUIRE(index == i);
    REQUIRE(item == i + 1);
    item--;
    ++i;
  }
  i = 0;
  for (auto&& [index, item] : a | tl::views::enumerate) {
    REQUIRE(index == i);
    REQUIRE(item == i);
    ++i;
  }
  REQUIRE(a == std::vector{0, 1, 2});
}

struct iota_view
   : public std::ranges::view_interface<iota_view> {
   struct sentinel {};
   struct iterator {
      std::size_t pos_ = 0;

      using iterator_category = std::input_iterator_tag;
      using reference = std::size_t&;
      using value_type = std::size_t;
      using difference_type = std::ptrdiff_t;

      iterator() = default;
      constexpr decltype(auto) operator*() const {
         return pos_;
      }

      constexpr iterator& operator++() {
         ++pos_;
         return *this;
      }

      constexpr iterator operator++(int) {
         auto temp = *this;
         ++pos_;
         return temp;
      }

      friend constexpr bool operator==(const iterator& x, const iterator& y) {
         return x.pos_ == y.pos_;
      }
      friend constexpr bool operator==(const iterator& x, const sentinel& y) {
         return false;
      }
   };

   iota_view() = default;

   constexpr auto begin() {
      return iterator{};
   }
   constexpr auto end() { return sentinel{}; }
};

TEST_CASE("unsized view") {
   for (auto&& [i, j] : (iota_view{} | tl::views::enumerate | std::views::take(10))) {
      REQUIRE(i == j);
   }
}