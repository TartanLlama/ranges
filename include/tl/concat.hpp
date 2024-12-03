#ifndef TL_RANGES_CONCAT_HPP
#define TL_RANGES_CONCAT_HPP

#include <ranges>
#include <tuple>
#include "tl/common.hpp"
#include <array>
#include "tl/fold.hpp"
#include <variant>
#include "tl/utility/meta.hpp"
#include "tl/basic_iterator.hpp"

namespace tl {
    namespace detail {
        template <class... Rs>
        using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

        template <class... Rs>
        using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

        template <class... Rs>
        using concat_rvalue_reference_t = std::common_reference_t<std::ranges::range_rvalue_reference_t<Rs>...>;

        template <class Ref, class RRef, class It>
        concept concat_indirectly_readable_impl =
            requires (const It it) {
                { *it } -> std::convertible_to<Ref>;
                { std::ranges::iter_move(it) } -> std::convertible_to<RRef>;
        };

        template <class... Rs>
        concept concat_indirectly_readable =
            std::common_reference_with<concat_reference_t<Rs...>&&,
            concat_value_t<Rs...>&>&&
            std::common_reference_with<concat_reference_t<Rs...>&&,
            concat_rvalue_reference_t<Rs...>&&>&&
            std::common_reference_with<concat_rvalue_reference_t<Rs...>&&,
            concat_value_t<Rs...> const&> &&
            (concat_indirectly_readable_impl<concat_reference_t<Rs...>,
                concat_rvalue_reference_t<Rs...>,
                std::ranges::iterator_t<Rs>> && ...);

        template <class... Rs>
        concept concatable = requires {
            typename concat_reference_t<Rs...>;
            typename concat_value_t<Rs...>;
            typename concat_rvalue_reference_t<Rs...>;
        }&& concat_indirectly_readable<Rs...>;

        template <bool Const, class... Rs>
        concept concat_is_random_access =
            all_random_access<Const, Rs...> &&
            (std::ranges::common_range<maybe_const<Const, Rs>> && ...);

        template <bool Const, class... Rs>
        concept concat_is_bidirectional =
            all_bidirectional<Const, Rs...> &&
            (std::ranges::common_range<maybe_const<Const, Rs>> && ...);
    }

    template <std::ranges::input_range... Vs>
        requires (std::ranges::view<Vs> && ...) and (sizeof...(Vs) > 0) and detail::concatable<Vs...>
    class concat_view : public std::ranges::view_interface<concat_view<Vs...>> {
    private:
        template <std::size_t... Is, class F>
        static auto invoke_with_index(std::index_sequence<Is...>, std::size_t i, F f) {
            using ret = std::invoke_result_t<F, std::integral_constant<std::size_t, 0>>;
            return std::array<ret(*)(F), sizeof...(Vs)>{ [](F f) {
                return f(std::integral_constant<std::size_t, Is>());
                }... } [i] (f);
        }

        template <class F>
        static auto invoke_with_index(std::size_t i, F f) {
            return invoke_with_index(std::index_sequence_for<Vs...>{}, i, f);
        }
    public:
        struct sentinel {};
        template <bool Const>
        class cursor {
        private:
            template<class T>
            using constify = std::conditional_t<Const, const T, T>;

        public:
            using reference =
                detail::concat_reference_t<constify<Vs>...>;
            using value_type =
                detail::concat_value_t<constify<Vs>...>;

            using difference_type = std::common_type_t<std::ranges::range_difference_t<constify<Vs>>...>;


            using base_iter =
                std::variant<std::ranges::iterator_t<constify<Vs>>...>;

        private:
            constify<concat_view>* parent_ = nullptr;
            base_iter current_;

            template <std::size_t N>
            void satisfy() {
                if constexpr (N < (sizeof...(Vs) - 1)) {
                    if (std::get<N>(current_) == std::ranges::end(std::get<N>(parent_->bases_))) {
                        current_.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(parent_->bases_)));
                        satisfy<N + 1>();
                    }
                }
            }

            struct prev_impl {
                template <std::size_t N, class It>
                void operator()(std::integral_constant<std::size_t, N>) {
                    if constexpr (N == 0) {
                        --std::get<N>(current_);
                    }
                    else {
                        if (std::get<N>(current_) == std::ranges::begin(std::get<N>(parent_->bases_))) {
                            current_.template emplace<N - 1>(std::ranges::end(std::get<N - 1>(parent_->bases_)));
                            prev_impl{}(std::integral_constant<std::size_t, N - 1>{}, std::get<N - 1>(current_));
                        }
                        else {
                            --std::get<N>(current_);
                        }
                    }
                }
            };

            template <std::size_t N>
            void advance_fwd(difference_type offset, difference_type steps) {
                using underlying_diff_type = std::iter_difference_t<std::variant_alternative_t<N, base_iter>>;
                if constexpr (N == sizeof...(Vs) - 1) {
                    std::get<N>(current_) += static_cast<underlying_diff_type>(steps);
                }
                else {
                    auto n_size = std::ranges::distance(std::get<N>(parent_->bases_));
                    if (offset + steps < n_size) {
                        std::get<N>(current_) += static_cast<underlying_diff_type>(steps);
                    }
                    else {
                        current_.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(parent_->bases_)));
                        advance_fwd<N + 1>(0, offset + steps - n_size);
                    }
                }
            }

            template <std::size_t N>
            void advance_bwd(difference_type offset, difference_type steps) {
                using underlying_diff_type = std::iter_difference_t<std::variant_alternative_t<N, base_iter>>;
                if constexpr (N == 0) {
                    std::get<N>(current_) -= static_cast<underlying_diff_type>(steps);
                }
                else {
                    if (offset >= steps) {
                        std::get<N>(current_) -= static_cast<underlying_diff_type>(steps);
                    }
                    else {
                        auto prev_size = std::ranges::distance(std::get<N - 1>(parent_->bases_));
                        current_.template emplace<N - 1>(std::ranges::end(std::get<N - 1>(parent_->bases_)));
                        advance_bwd<N - 1>(prev_size, steps - offset);
                    }
                }
            }

        public:
            cursor() = default;
            template <class... Args>
            constexpr explicit cursor(constify<concat_view>* parent, Args&&... args)
                : parent_{ parent }, current_(std::forward<Args>(args)...)
            {
            }

            //const-converting constructor
            constexpr cursor(cursor<!Const> i) requires Const && (std::convertible_to<
                std::ranges::iterator_t<Vs>,
                std::ranges::iterator_t<constify<Vs>>> && ...)
                : parent_{ i.parent_ }, current_{ std::move(i.current_) } {
            }

            constexpr decltype(auto) read() const {
                using reference = detail::concat_reference_t<maybe_const<Const, Vs>...>;
                return std::visit([](auto&& it) -> reference {
                    return *it; }, current_);
            }

            constexpr void next() {
                invoke_with_index(current_.index(), [this](auto i) {
                    ++std::get<i>(current_);
                    satisfy<i>();
                    });
            }

            void prev() requires (detail::concat_is_bidirectional<Const, Vs...>) {
                invoke_with_index(current_.index(), prev_impl{});
            }

            template <std::size_t N = (sizeof...(Vs) - 1)>
            void advance(difference_type n) requires (detail::concat_is_random_access<Const, Vs...>) {
                if (n > 0) {
                    invoke_with_index(current_.index(), [this, n](auto i) {
                        advance_fwd<i>(std::get<i>(current_) - std::ranges::begin(std::get<i>(parent_->bases_)), n);
                        });
                }
                else if (n < 0) {
                    invoke_with_index(current_.index(), [this, n](auto i) {
                        advance_bwd<i>(std::get<i>(current_) - std::ranges::begin(std::get<i>(parent_->bases_)), -n);
                        });
                }
            }

            constexpr bool equal(const cursor& rhs) const
                requires (std::equality_comparable<std::ranges::iterator_t<constify<Vs>>> && ...) {
                return current_ == rhs.current_;
            }

            constexpr bool equal(sentinel) const {
                constexpr auto last_idx = sizeof...(Vs) - 1;
                return current_.index() == last_idx &&
                    std::get<last_idx>(current_) == std::ranges::end(std::get<last_idx>(parent_->bases_));
            }

            constexpr auto distance_to(cursor const& other) const
                requires (detail::concat_is_random_access<Const, Vs...>) {
                if (current_.index() > other.current_.index()) {
                    auto dx = invoke_with_index(current_.index(), [this](auto i) {
                        return std::ranges::distance(std::ranges::begin(std::get<i>(parent_->bases_)), std::get<i>(current_));
                        });
                    auto dy = invoke_with_index(other.current_.index(), [other](auto i) {
                        return std::ranges::distance(std::get<i>(other.current_), std::ranges::end(std::get<i>(other.parent_->bases_)));
                        });

                    auto other_distances = std::views::iota(other.current_.index() + 1, current_.index())
                        | std::views::transform([this](std::size_t i) {
                        return invoke_with_index(i, [this](auto i) {
                            return std::ranges::distance(std::get<i>(parent_->bases_));
                            });
                            });
                    auto s = tl::sum(other_distances);
                    return dx + dy + s;
                }
                else if (current_.index() < other.current_.index()) {
                    return -(other.distance_to(*this));
                }
                else {
                    return invoke_with_index(current_.index(), [this, other](auto i) {
                        return std::get<i>(current_) - std::get<i>(other.current_);
                        });
                }
            }

            friend class cursor<!Const>;
            friend class concat_view;
        };

        constexpr concat_view() = default;

        constexpr explicit concat_view(Vs... views)
            : bases_(std::move(views)...) {
        }

        constexpr auto begin() requires(!(tl::simple_view<Vs> && ...)) {
            basic_iterator it{ cursor<false>{this, std::in_place_index<0>, std::ranges::begin(std::get<0>(bases_))} };
            it.get().template satisfy<0>();
            return it;
        }

        constexpr auto begin() const
            requires((std::ranges::range<const Vs> && ...) && detail::concatable<const Vs...>) {
            basic_iterator it{ cursor<true>{this, std::in_place_index<0>, std::ranges::begin(std::get<0>(bases_))} };
            it.get().template satisfy<0>();
            return it;
        }

        constexpr auto end() requires(!(tl::simple_view<Vs> && ...)) {
            constexpr auto N = sizeof...(Vs);
            if constexpr (std::ranges::common_range<maybe_const<false, tl::meta::back<Vs...>>>) {
                return basic_iterator{ cursor<false>(this, std::in_place_index<N - 1>,
                    std::ranges::end(std::get<N - 1>(bases_))) };
            }
            else {
                return sentinel{};
            }
        }

        constexpr auto end() const
            requires((std::ranges::range<const Vs> && ...) && detail::concatable<const Vs...>) {
            constexpr auto N = sizeof...(Vs);
            if constexpr (std::ranges::common_range<maybe_const<true, tl::meta::back<Vs...>>>) {
                return basic_iterator{ cursor<true>(this, std::in_place_index<N - 1>,
                    std::ranges::end(std::get<N - 1>(bases_))) };
            }
            else {
                return sentinel{};
            }
        }

        constexpr auto size() requires(std::ranges::sized_range<Vs>&&...) {
            return std::apply([](auto&&... bases) {
                using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
                return (static_cast<size_type>(std::ranges::size(bases)) + ...);
                }, bases_);
        }

        constexpr auto size() const requires(std::ranges::sized_range<const Vs>&&...) {
            return std::apply([](auto&&... bases) {
                using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
                return (static_cast<size_type>(std::ranges::size(bases)) + ...);
                }, bases_);
        }

    private:
        std::tuple<Vs...> bases_;

    };

    template <class... R>
    concat_view(R&&...) -> concat_view<std::views::all_t<R>...>;

    namespace views {
        namespace detail {
            struct concat_fn {
                template <std::ranges::viewable_range... Rs>
                constexpr auto operator()(Rs&&... rs) const
                    requires (std::ranges::forward_range<Rs> && ...) {
                    return concat_view(std::forward<Rs>(rs)...);
                }
            };

        }

        constexpr inline detail::concat_fn concat;
    }
}

#endif