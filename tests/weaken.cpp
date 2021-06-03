#include "tl/weaken.hpp"
#include <catch2/catch.hpp>
#include <ranges>
#include <vector>

TEST_CASE("weaken") {
   std::vector a{ 0,1,2 };
  
   {
      auto v = a | tl::views::weaken<tl::weakening::non_common, tl::weakening::non_sized>;
      using V = decltype(v);
      STATIC_REQUIRE(std::ranges::random_access_range<V>);
      STATIC_REQUIRE(std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(std::ranges::range<const V>);
      STATIC_REQUIRE(!std::ranges::sized_range<V>);
      STATIC_REQUIRE(!std::ranges::common_range<V>);
   }

   {
      auto v = a | tl::views::weaken<tl::weakening::non_sized>;
      using V = decltype(v);
      STATIC_REQUIRE(std::ranges::bidirectional_range<V>); //common non-sized ranges are not random-access
      STATIC_REQUIRE(std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(std::ranges::range<const V>);
      STATIC_REQUIRE(!std::ranges::sized_range<V>);
      STATIC_REQUIRE(std::ranges::common_range<V>);
   }


   {
      auto v = a | tl::views::weaken<tl::weakening::non_borrowable, tl::weakening::non_const_iterable>;
      using V = decltype(v);
      STATIC_REQUIRE(std::ranges::random_access_range<V>);
      STATIC_REQUIRE(!std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(!std::ranges::range<const V>);
      STATIC_REQUIRE(std::ranges::sized_range<V>);
      STATIC_REQUIRE(std::ranges::common_range<V>);
   }

   {
      auto v = a | tl::views::weaken<tl::weakening::input>;
      using V = decltype(v);
      STATIC_REQUIRE(!std::ranges::random_access_range<V>);
      STATIC_REQUIRE(!std::ranges::bidirectional_range<V>);
      STATIC_REQUIRE(!std::ranges::forward_range<V>);
      STATIC_REQUIRE(std::ranges::input_range<V>);
      STATIC_REQUIRE(std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(std::ranges::range<const V>);
      STATIC_REQUIRE(std::ranges::sized_range<V>);
      STATIC_REQUIRE(std::ranges::common_range<V>);
   }

   {
      auto v = a | tl::views::weaken<tl::weakening::forward>;
      using V = decltype(v);
      STATIC_REQUIRE(!std::ranges::random_access_range<V>);
      STATIC_REQUIRE(!std::ranges::bidirectional_range<V>);
      STATIC_REQUIRE(std::ranges::forward_range<V>);
      STATIC_REQUIRE(std::ranges::input_range<V>);
      STATIC_REQUIRE(std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(std::ranges::range<const V>);
      STATIC_REQUIRE(std::ranges::sized_range<V>);
      STATIC_REQUIRE(std::ranges::common_range<V>);
   }

   {
      auto v = a | tl::views::weaken<tl::weakening::bidirectional>;
      using V = decltype(v);
      STATIC_REQUIRE(!std::ranges::random_access_range<V>);
      STATIC_REQUIRE(std::ranges::bidirectional_range<V>);
      STATIC_REQUIRE(std::ranges::forward_range<V>);
      STATIC_REQUIRE(std::ranges::input_range<V>);
      STATIC_REQUIRE(std::ranges::borrowed_range<V>);
      STATIC_REQUIRE(std::ranges::range<const V>);
      STATIC_REQUIRE(std::ranges::sized_range<V>);
      STATIC_REQUIRE(std::ranges::common_range<V>);
   }

}