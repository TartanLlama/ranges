#include <catch2/catch.hpp>
#include <ranges>
#include <tl/concat.hpp>

TEST_CASE("concat") {
    std::vector<int> a{ 0, 1, 2 };
    std::vector<int> b{ 3, 4, 5 };
    std::vector<int> c{ 6, 7, 8 };
    auto concat = tl::views::concat(a, b, c);
    auto it = std::ranges::begin(concat);
    for (int i = 0; i < 9; ++i) {
        REQUIRE(i == *it);
        ++it;
    }
}