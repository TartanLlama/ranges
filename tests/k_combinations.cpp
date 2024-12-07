#include <catch2/catch.hpp>
#include <ranges>
#include <tl/k_combinations.hpp>

TEST_CASE("k_combinations") {
    std::vector<int> a{ 0, 1, 2 };
    {
        auto k_combinations = tl::views::k_combinations(a, 2);
        auto it = std::ranges::begin(k_combinations);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                REQUIRE(std::ranges::equal(std::vector{ i, j }, *it));
                ++it;
            }
        }
    }

    {
        auto k_combinations = tl::views::k_combinations(a, 3);
        auto it = std::ranges::begin(k_combinations);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    REQUIRE(std::ranges::equal(std::vector{ i, j, k }, *it));
                    ++it;
                }
            }
        }
    }
}