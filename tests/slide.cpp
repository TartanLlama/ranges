#include <catch2/catch.hpp>
#include <vector>
#include <tl/slide.hpp>
#include <tl/zip.hpp>
#include <ranges>
#include <iostream>
#include <list>
#include <forward_list>

TEST_CASE("slide random access") {
    std::vector<int> a{ 0,1,2,3,4,5,6 };
    std::vector<std::vector<int>> results{
       {0,1,2},
       {1,2,3},
       {2,3,4},
       {3,4,5},
       {4,5,6}
    };

    REQUIRE(tl::views::slide(a, 3).size() == 5);

    for (auto const& [a, b] : tl::views::zip(tl::views::slide(a,3), results)) {
        auto res = std::ranges::equal(a, b);
        REQUIRE(res);
    }
}

TEST_CASE("slide bidirectional") {
    std::list<int> a{ 0,1,2,3,4,5,6 };
    std::vector<std::vector<int>> results{
       {0,1,2},
       {1,2,3},
       {2,3,4},
       {3,4,5},
       {4,5,6}
    };

    REQUIRE(tl::views::slide(a, 3).size() == 5);

    for (auto const& [a, b] : tl::views::zip(tl::views::slide(a, 3), results)) {
        auto res = std::ranges::equal(a, b);
        REQUIRE(res);
    }
}

TEST_CASE("slide forward") {
    std::forward_list<int> a{ 0,1,2,3,4,5,6 };
    std::vector<std::vector<int>> results{
       {0,1,2},
       {1,2,3},
       {2,3,4},
       {3,4,5},
       {4,5,6}
    };

    for (auto const& [a, b] : tl::views::zip(tl::views::slide(a, 3), results)) {
        auto res = std::ranges::equal(a, b);
        REQUIRE(res);
    }
}