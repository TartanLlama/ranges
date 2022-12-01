#include "tl/fold.hpp"
#include <catch2/catch.hpp>
#include <vector>
#include <functional>

TEST_CASE("fold") {
    std::vector<int> a{ 1, 2, 3, 4 };

    auto r1 = tl::fold(a, 0, std::plus());
    auto r2 = tl::fold(a, 1, std::multiplies());
    auto r3 = tl::fold_left(a, 0, std::plus());
    auto r4 = tl::fold_left(a, 1, std::multiplies());
    auto r5 = tl::fold_right(a, 0, std::plus());
    auto r6 = tl::fold_right(a, 1, std::multiplies());

    REQUIRE(r1 == 10);
    REQUIRE(r2 == 24);
    REQUIRE(r3 == 10);
    REQUIRE(r4 == 24);
    REQUIRE(r5 == 10);
    REQUIRE(r6 == 24);

    auto r7 = tl::fold(a, 10, std::minus());
    auto r8 = tl::fold_left(a, 10, std::minus());
    auto r9 = tl::fold_right(a, 10, std::minus());

    REQUIRE(r7 == 0);
    REQUIRE(r8 == 0);
    REQUIRE(r9 == 8);
}