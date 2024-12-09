#ifndef TL_RANGES_PREDICATES_HPP
#define TL_RANGES_PREDICATES_HPP

namespace tl {
    inline constexpr auto proj = [](auto&& f, auto&& p) {
        return [f = std::forward<decltype(f)>(f), p = std::forward<decltype(p)>(p)](auto&&... args) {
            return std::invoke(f, std::invoke(p, std::forward<decltype(args)>(args)...));
        };
    };

    inline constexpr auto eq = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return x == y;
        };
    };
    inline constexpr auto neq = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return x != y;
        };
    };
    inline constexpr auto lt = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return y < x;
        };
    };
    inline constexpr auto gt = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return y > x;
        };
    };
    inline constexpr auto lte = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return y <= x;
        };
    };
    inline constexpr auto gte = [](auto&& x) {
        return [x = std::forward<decltype(x)>(x)](auto&& y) {
            return y >= x;
        };
    };
    inline constexpr auto is_even = [](auto&& x) {
        return x % 2 == 0;
    };
    inline constexpr auto is_odd = [](auto&& x) {
        return x % 2 != 0;
    };
    inline constexpr auto is_positive = [](auto&& x) {
        return x > 0;
    };
    inline constexpr auto is_negative = [](auto&& x) {
        return x < 0;
    };
    inline constexpr auto is_zero = [](auto&& x) {
        return x == 0;
    };
    inline constexpr auto is_nonzero = [](auto&& x) {
        return x != 0;
    };
    inline constexpr auto is_true = [](auto&& x) {
        return x;
    };
    inline constexpr auto is_false = [](auto&& x) {
        return !x;
    };
    inline constexpr auto is_null = [](auto&& x) {
        return x == nullptr;
    };
    inline constexpr auto is_not_null = [](auto&& x) {
        return x != nullptr;
    };
    inline constexpr auto is_empty = [](auto&& x) {
        return x.empty();
    };
    inline constexpr auto is_not_empty = [](auto&& x) {
        return !x.empty();
    };
    inline constexpr auto both = [](auto&&... predicates) {
        return [predicates = std::make_tuple(std::forward<decltype(predicates)>(predicates)...)](auto&& x) {
            return std::apply([&x](auto&&... predicates) {
                return (predicates(x) && ...);
            }, predicates);
        };
    };
    inline constexpr auto either = [](auto&&... predicates) {
        return [predicates = std::make_tuple(std::forward<decltype(predicates)>(predicates)...)](auto&& x) {
            return std::apply([&x](auto&&... predicates) {
                return (predicates(x) || ...);
            }, predicates);
        };
    };
    inline constexpr auto negation = [](auto&& predicate) {
        return [predicate = std::forward<decltype(predicate)>(predicate)](auto&& x) {
            return !predicate(x);
        };
    };
    inline constexpr auto is_between = [](auto&& lower, auto&& upper) {
        return [lower = std::forward<decltype(lower)>(lower), upper = std::forward<decltype(upper)>(upper)](auto&& x) {
            return lower <= x && x <= upper;
        };
    };
}

#endif