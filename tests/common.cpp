#include <catch2/catch.hpp>
#include <ranges>
#include <iostream>
#include <list>
#include <forward_list>
#include <tl/common.hpp>

TEST_CASE("common_iterator_category") {
   constexpr bool a = std::is_same_v<tl::common_iterator_category<std::ranges::ref_view<std::vector<int>>, std::ranges::ref_view<std::vector<double>>>, std::random_access_iterator_tag>;
   STATIC_REQUIRE(a);
   constexpr bool b = std::is_same_v<tl::common_iterator_category<std::ranges::ref_view<std::vector<int>>, std::ranges::ref_view<std::list<double>>>, std::bidirectional_iterator_tag>;
   STATIC_REQUIRE(b);
   constexpr bool c = std::is_same_v<tl::common_iterator_category<std::ranges::ref_view<std::vector<int>>, std::ranges::ref_view<std::forward_list<double>>>, std::forward_iterator_tag>;
   STATIC_REQUIRE(c);
   constexpr bool d = std::is_same_v<tl::common_iterator_category<std::ranges::ref_view<std::vector<int>>, std::views::all_t<std::ranges::basic_istream_view<double, char, std::char_traits<char>>>>, std::input_iterator_tag>;
   STATIC_REQUIRE(d);
}
