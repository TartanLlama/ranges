#include <catch2/catch.hpp>
#include "tl/functional/predicates.hpp"

TEST_CASE("eq") {
    auto eq = tl::eq(5);
    REQUIRE(eq(5));
    REQUIRE_FALSE(eq(6));
}

TEST_CASE("neq") {
    auto neq = tl::neq(5);
    REQUIRE_FALSE(neq(5));
    REQUIRE(neq(6));
}

TEST_CASE("lt") {
    auto lt = tl::lt(5);
    REQUIRE(lt(4));
    REQUIRE_FALSE(lt(5));
    REQUIRE_FALSE(lt(6));
}

TEST_CASE("gt") {
    auto gt = tl::gt(5);
    REQUIRE_FALSE(gt(4));
    REQUIRE_FALSE(gt(5));
    REQUIRE(gt(6));
}

TEST_CASE("lte") {
    auto lte = tl::lte(5);
    REQUIRE(lte(4));
    REQUIRE(lte(5));
    REQUIRE_FALSE(lte(6));
}

TEST_CASE("gte") {
    auto gte = tl::gte(5);
    REQUIRE_FALSE(gte(4));
    REQUIRE(gte(5));
    REQUIRE(gte(6));
}

TEST_CASE("is_even") {
    auto ie = tl::is_even;
    REQUIRE(ie(0));
    REQUIRE(ie(2));
    REQUIRE(ie(4));
    REQUIRE_FALSE(ie(1));
    REQUIRE_FALSE(ie(3));
    REQUIRE_FALSE(ie(5));
}

TEST_CASE("is_odd") {
    auto io = tl::is_odd;
    REQUIRE(io(1));
    REQUIRE(io(3));
    REQUIRE(io(5));
    REQUIRE_FALSE(io(0));
    REQUIRE_FALSE(io(2));
    REQUIRE_FALSE(io(4));
}

TEST_CASE("is_positive") {
    auto ip = tl::is_positive;
    REQUIRE(ip(1));
    REQUIRE(ip(2));
    REQUIRE(ip(3));
    REQUIRE_FALSE(ip(0));
    REQUIRE_FALSE(ip(-1));
    REQUIRE_FALSE(ip(-2));
    REQUIRE_FALSE(ip(-3));
}

TEST_CASE("is_negative") {
    auto in = tl::is_negative;
    REQUIRE_FALSE(in(1));
    REQUIRE_FALSE(in(2));
    REQUIRE_FALSE(in(3));
    REQUIRE_FALSE(in(0));
    REQUIRE(in(-1));
    REQUIRE(in(-2));
    REQUIRE(in(-3));
}

TEST_CASE("is_zero") {
    auto iz = tl::is_zero;
    REQUIRE(iz(0));
    REQUIRE_FALSE(iz(1));
    REQUIRE_FALSE(iz(-1));
}

TEST_CASE("is_nonzero") {
    auto inz = tl::is_nonzero;
    REQUIRE_FALSE(inz(0));
    REQUIRE(inz(1));
    REQUIRE(inz(-1));
}

TEST_CASE("is_true") {
    auto it = tl::is_true;
    REQUIRE(it(true));
    REQUIRE_FALSE(it(false));
}

TEST_CASE("is_false") {
    auto iff = tl::is_false;
    REQUIRE_FALSE(iff(true));
    REQUIRE(iff(false));
}

TEST_CASE("is_null") {
    auto in = tl::is_null;
    REQUIRE(in(nullptr));
    REQUIRE_FALSE(in(&in));
}

TEST_CASE("is_not_null") {
    auto inn = tl::is_not_null;
    REQUIRE_FALSE(inn(nullptr));
    REQUIRE(inn(&inn));
}

TEST_CASE("both") {
    auto c = tl::both(tl::is_even, tl::is_positive);
    REQUIRE_FALSE(c(0));
    REQUIRE_FALSE(c(1));
    REQUIRE(c(2));
    REQUIRE(c(4));
}

TEST_CASE("either") {
    auto d = tl::either(tl::is_even, tl::is_positive);
    REQUIRE(d(0));
    REQUIRE(d(1));
    REQUIRE(d(2));
    REQUIRE(d(4));
    REQUIRE_FALSE(d(-1));
    REQUIRE(d(-2));
}

TEST_CASE("negation") {
    auto n = tl::negation(tl::is_even);
    REQUIRE(n(1));
    REQUIRE(n(3));
    REQUIRE(n(5));
    REQUIRE_FALSE(n(0));
    REQUIRE_FALSE(n(2));
    REQUIRE_FALSE(n(4));
}

TEST_CASE("compose") {
    auto p = tl::compose([](int x) { return x + 1; }, [](int x) { return x + 2; });
    REQUIRE(p(1) == 4);
    REQUIRE(p(2) == 5);
    REQUIRE(p(3) == 6);
}