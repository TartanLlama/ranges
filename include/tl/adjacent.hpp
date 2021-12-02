#ifndef TL_RANGES_ADJACENT_HPP
#define TL_RANGES_ADJACENT_HPP

#include <ranges>
#include "common.hpp"
#include "basic_iterator.hpp"
#include "functional/pipeable.hpp"
#include "utility/meta.hpp"
#include "utility/tuple_utils.hpp"

namespace tl {
    template<std::ranges::forward_range V, std::size_t N>
        requires std::ranges::view<V> && (N > 0)
    class adjacent_view : public std::ranges::view_interface<adjacent_view<V, N>> {
        V base_ = V();

        template<bool Const>
        class cursor {
            using Base = maybe_const<Const, V>;

        public:
            std::array<std::ranges::iterator_t<Base>, N> current_ = std::array<std::ranges::iterator_t<Base>, N>();
            
            using value_type = tl::meta::repeat_into<std::ranges::range_value_t<Base>, N, detail::tuple_or_pair_impl>::type;

            using difference_type = std::ranges::range_difference_t<Base>;

            cursor() = default;
            constexpr cursor(cursor<!Const> i)
                requires Const&& std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>> : current_(std::move(i.current_)) {}

            constexpr cursor(std::ranges::iterator_t<Base> first, std::ranges::sentinel_t<Base> last) {
                current_[0] = first;
                for (auto i : std::views::iota(std::size_t(1), N)) {
                    current_[i] = std::ranges::next(current_[i - 1], 1, last);
                }
            }
            constexpr cursor(as_sentinel_t, std::ranges::iterator_t<Base> first, std::ranges::iterator_t<Base> last) {
                if constexpr (!std::ranges::bidirectional_range<Base>) {
                    for (auto& it : current_) {
                        it = last;
                    }
                }
                else {
                    current_[N - 1] = last;
                    for (auto i : std::views::iota(std::size_t(1), N)) {
                        current_[N - 1 - i] = std::ranges::prev(current_[N - i], 1, first);
                    }
                }
            }

            constexpr auto read() const {
                return tuple_transform([](auto& i) -> decltype(auto) { return *i; }, current_);
            }

            constexpr void next() {
                std::ranges::copy(current_ | std::views::drop(1), current_.begin());
                ++current_.back();
            }

            constexpr void prev() requires std::ranges::bidirectional_range<Base> {
                std::ranges::copy_backward(current_ | std::views::take(N - 1), current_.end());
                --current_.front();
            }

            constexpr void advance(difference_type x) requires std::ranges::random_access_range<Base> {
                for (auto& i : current_) { i += x; }
            }

            constexpr bool equal(cursor const& rhs) const
                requires std::equality_comparable<std::ranges::iterator_t<Base>>{
                return current_.back() == rhs.current_.back();
            }

            constexpr bool equal(basic_sentinel<V, Const> const& rhs) const
                requires std::sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>> {
                return current_.back() == rhs.end_;
            }

            constexpr auto distance_to(const cursor& rhs) const
                requires std::sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>> {
                return rhs.current_.back() - current_.back();
            }

            constexpr auto distance_to(const basic_sentinel<V, Const>& rhs) const
                requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>> {
                return rhs.end_ - current_.back();
            }
        };

    public:
        constexpr adjacent_view() = default;
        constexpr explicit adjacent_view(V base) : base_(std::move(base)) {}

        constexpr auto begin() requires (!tl::simple_view<V>) {
            return basic_iterator{ cursor<false>(std::ranges::begin(base_), std::ranges::end(base_)) };
        }

        constexpr auto begin() const requires std::ranges::range<const V> {
            return basic_iterator{ cursor<true>(std::ranges::begin(base_), std::ranges::end(base_)) };
        }

        constexpr auto end() requires (!tl::simple_view<V> && !std::ranges::common_range<V>) {
            return basic_sentinel<V, false>(std::ranges::end(base_));
        }

        constexpr auto end() requires (!tl::simple_view<V>&& std::ranges::common_range<V>) {
            return basic_iterator{ cursor<false>{as_sentinel, std::ranges::begin(base_), std::ranges::end(base_)} };
        }

        constexpr auto end() const requires std::ranges::range<const V> {
            return basic_sentinel<V, true>(std::ranges::end(base_));
        }

        constexpr auto end() const requires std::ranges::common_range<const V> {
            return basic_iterator{ cursor<true>(as_sentinel, std::ranges::begin(base_), std::ranges::end(base_)) };
        }

        constexpr auto size() requires std::ranges::sized_range<V> {
            auto sz = std::ranges::size(base_);
            sz -= std::min<decltype(sz)>(sz, N - 1);
            return sz;
        }
        constexpr auto size() const requires std::ranges::sized_range<const V> {
            auto sz = std::ranges::size(base_);
            sz -= std::min<decltype(sz)>(sz, N - 1);
            return sz;
        }
    };

    namespace views {
        namespace detail {
            template <std::size_t N>
            class adjacent_fn {
            public:
                template <std::ranges::viewable_range V>
                constexpr auto operator()(V&& v) const
                    requires std::ranges::forward_range<V> {
                    return tl::adjacent_view<std::views::all_t<V>, N>{ std::forward<V>(v) };
                }
            };
        }  // namespace detail

        template <std::size_t N>
        inline constexpr auto adjacent = pipeable(detail::adjacent_fn<N>{});
        inline constexpr auto pairwise = pipeable(detail::adjacent_fn<2>{});
    }  // namespace views
}  // namespace tl

namespace std::ranges {
    template <class R, std::size_t N>
    inline constexpr bool enable_borrowed_range<tl::adjacent_view<R, N>> = enable_borrowed_range<R>;
}

#endif