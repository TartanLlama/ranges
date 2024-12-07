#ifndef TL_RANGES_K_COMBINATIONS
#define TL_RANGES_K_COMBINATIONS


#include <ranges>
#include <tl/common.hpp>
#include <tl/fold.hpp>
#include <vector>
#include <tl/basic_iterator.hpp>

namespace tl {
    template <std::ranges::forward_range V>
        requires std::ranges::view<V>
    class k_combinations_view : public std::ranges::view_interface<k_combinations_view<V>> {
    public:
        template <bool Const>
        class cursor;

        template <bool Const>
        class sentinel {
        public:
            using base = std::ranges::iterator_t <maybe_const<Const, V>>;
            sentinel() = default;
            sentinel(base end) : end_(std::move(end)) {}
            template <bool>
            friend class cursor;

        private:
            base end_;
        };

        template <bool Const>
        class cursor {
        private:
            template<class T>
            using constify = std::conditional_t<Const, const T, T>;

        public:
            cursor() = default;
            constexpr explicit cursor(constify<V>* base, std::size_t n, std::ranges::iterator_t<constify<V>> it) :
                base_(base), current_(n, it) {
            }

            //const-converting constructor
            constexpr cursor(cursor<!Const> i) requires Const&& std::convertible_to<
                std::ranges::iterator_t<V>,
                std::ranges::iterator_t<constify<V>>>
                : base_(i.base_), current_{ std::move(i.current_) } {
            }

            constexpr decltype(auto) read() const {
                return std::views::transform(current_, [](auto&& i) {
                    return std::ref(*i);
                    });
            }

            constexpr void next() {
                auto it = current_.rbegin();
                auto end = current_.rend();
                while (it != end) {
                    ++(*it);
                    if (*it == std::ranges::end(*base_)) {
                        if (it != end - 1) {
                            *it = std::ranges::begin(*base_);
                        }
                        ++it;
                    }
                    else {
                        break;
                    }
                }
            }

            void prev() requires (std::ranges::bidirectional_range<constify<V>>) {
                auto it = current_.rbegin();
                auto end = current_.rend();
                while (it != end) {
                    if (*it == std::ranges::begin(*base_)) {
                        std::ranges::advance(*it, std::ranges::end(*base_));
                        ++it;
                    }
                    --(*it);
                }
            }

            //TODO advance

            constexpr bool equal(const cursor& rhs) const
                requires (std::equality_comparable<std::ranges::iterator_t<constify<V>>>) {
                return current_ == rhs.current_;
            }

            constexpr bool equal(const sentinel<Const>& s) const {
                return current_.front() == s.end_;
            }

            constexpr auto distance_to(cursor const& other) const
                requires (std::ranges::sized_range<constify<V>>) {
                std::ptrdiff_t distance = 0;
                for (std::size_t i = current_.size() - 1; i >= 0; --i) {
                    distance += std::ranges::distance(current_[i], other.current_[i]) * std::ranges::size(*base_);
                }
                return distance;
            }

        private:
            constify<V>* base_;
            std::vector<std::ranges::iterator_t<V>> current_;

            friend class cursor<!Const>;
            friend class k_combinations_view;
        };

        constexpr k_combinations_view() = default;

        constexpr explicit k_combinations_view(V view, std::size_t n)
            : base_(std::move(view)), n_(n) {
        }

        constexpr auto begin() requires(!tl::simple_view<V>) {
            return basic_iterator{ cursor<false>{ std::addressof(base_), n_, std::ranges::begin(base_)} };
        }

        constexpr auto begin() const
            requires(std::ranges::range<const V>) {
            return basic_iterator{ cursor<true>{ std::addressof(base_), n_, std::ranges::begin(base_) } };
        }

        constexpr auto end() requires(!tl::simple_view<V>) {
            if constexpr (std::ranges::common_range<V> and std::ranges::sized_range<V>) {
                return basic_iterator{ cursor<false>(std::addressof(base_), n_, std::ranges::end(base_)) };
            }
            else {
                return sentinel<false>{std::ranges::end(base_)};
            }
        }

        constexpr auto end() const
            requires(std::ranges::range<const V>) {
            if constexpr (std::ranges::common_range<const V> and std::ranges::sized_range<V>) {
                return basic_iterator{ cursor<true>(std::addressof(base_), n_, std::ranges::end(base_)) };
            }
            else {
                return sentinel<true>{std::ranges::end(base_)};
            }
        }

        constexpr auto size() requires(std::ranges::sized_range<V>) {
            return std::pow(std::ranges::size(base_), n_);
        }

        constexpr auto size() const requires(std::ranges::sized_range<const V>) {
            return std::pow(std::ranges::size(base_), n_);
        }

    private:
        V base_;
        std::size_t n_;
    };

    template <class R>
    k_combinations_view(R&&, std::size_t n) -> k_combinations_view<std::views::all_t<R>>;

    namespace views {
        namespace detail {
            struct k_combinations_fn {
                template <std::ranges::viewable_range R>
                constexpr auto operator()(R&& r, std::size_t n) const
                    requires (std::ranges::forward_range<R>) {
                    return k_combinations_view(std::forward<R>(r), n);
                }
            };

        }

        constexpr inline detail::k_combinations_fn k_combinations;
    }
}

#endif
