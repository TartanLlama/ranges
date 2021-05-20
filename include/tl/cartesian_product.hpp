#ifndef TL_RANGES_CARTESIAN_PRODUCT_HPP
#define TL_RANGES_CARTESIAN_PRODUCT_HPP

#include <iterator>
#include <ranges>
#include <type_traits>
#include <tuple>
#include "common.hpp"
#include "basic_iterator.hpp"

namespace tl {
   template <std::ranges::forward_range... Vs>
   requires (std::ranges::view<Vs> && ...) class cartesian_product_view
      : public std::ranges::view_interface<cartesian_product_view<Vs...>> {
      template <class... Ts>
      static constexpr bool am_common = (std::ranges::common_range<Ts> && ...)
         || ((std::ranges::random_access_range<Ts> && ...) && (std::ranges::sized_range<Ts> && ...));

      template <class... Ts>
      static constexpr bool am_sized = (std::ranges::sized_range<Ts> && ...);

      template <class... Ts>
      static constexpr bool am_bidirectional =
         ((std::ranges::bidirectional_range<Ts> && ...) && (std::ranges::common_range<Ts> && ...));

      template <class... Ts>
      static constexpr bool am_random_access =
         ((std::ranges::random_access_range<Ts> && ...) &&
            (std::ranges::sized_range<Ts> && ...));

      template <class... Ts>
      static constexpr bool am_distanceable =
         ((std::sized_sentinel_for<std::ranges::iterator_t<Ts>, std::ranges::iterator_t<Ts>>) && ...)
         && am_sized<Ts...>;

      std::tuple<Vs...> bases_;

      template <bool Const>
      class cursor;

      template <bool Const>
      class sentinel {
         using parent = std::conditional_t<Const, const cartesian_product_view, cartesian_product_view>;
         using first_base = decltype(std::get<0>(std::declval<parent>().bases_));
         std::ranges::sentinel_t<first_base> end_;

      public:
         sentinel() = default;
         sentinel(std::ranges::sentinel_t<first_base> end) : end_(std::move(end)) {}

         constexpr sentinel(sentinel<!Const> other) requires Const &&
            (std::convertible_to<std::ranges::sentinel_t<first_base>, std::ranges::sentinel_t<const first_base>>)
            : end_(std::move(other.end_)) {
         }

         template <bool>
         friend class cursor;
      };

      template <bool Const>
      class cursor {
         template<class T>
         using constify = std::conditional_t<Const, const T, T>;

         constify<std::tuple<Vs...>>* bases_;
         std::tuple<std::ranges::iterator_t<constify<Vs>>...> currents_{};

      public:
         using iterator_category = std::forward_iterator_tag;
         using iterator_concept = tl::common_iterator_category<constify<Vs>...>;
         using reference =
            std::tuple<std::ranges::range_reference_t<constify<Vs>>...>;
         using value_type =
            std::tuple<std::ranges::range_value_t<constify<Vs>>...>;

         using difference_type = std::ptrdiff_t;

         cursor() = default;
         constexpr explicit cursor(begin_tag_t, constify<std::tuple<Vs...>>* bases)
            : bases_{ bases }, currents_{ std::apply([](auto&&... bs) { return std::make_tuple(std::ranges::begin(bs)...); }, *bases) }
         {}
         constexpr explicit cursor(end_tag_t, constify<std::tuple<Vs...>>* bases)
            requires(std::ranges::common_range<Vs> && ...)
            : cursor{ begin_tag, bases }
         {
            std::get<0>(currents_) = std::ranges::end(std::get<0>(*bases_));
         }
         constexpr explicit cursor(end_tag_t, constify<std::tuple<Vs...>>* bases)
            requires(!(std::ranges::common_range<Vs> && ...) && (std::ranges::random_access_range<Vs> && ...) && (std::ranges::sized_range<Vs> && ...))
            : cursor{ begin_tag, bases }
         {
            std::get<0>(currents_) += std::ranges::size(std::get<0>(*bases_));
         }

         constexpr cursor(cursor<!Const> i) requires Const && (std::convertible_to<
            std::ranges::iterator_t<Vs>,
            std::ranges::iterator_t<constify<Vs>>> && ...)
            : bases_{ std::move(i.bases_) }, currents_{ std::move(i.currents_) } {}


         constexpr decltype(auto) read() const {
            return std::apply([this](auto&&... currents) {
               return reference{ *currents... };
               }, currents_);
         }

         //Increment the iterator at std::get<N>(currents_)
         //If that iterator hits its end, recurse to std::get<N-1>
         template <std::size_t N = (sizeof...(Vs) - 1)>
         void next() {
            auto& it = std::get<N>(currents_);
            ++it;
            if (it == std::ranges::end(std::get<N>(*bases_))) {
               if constexpr (N > 0) {
                  it = std::ranges::begin(std::get<N>(*bases_));
                  next<N - 1>();
               }
            }
         }

         //Decrement the iterator at std::get<N>(currents_)
         //If that iterator was at its begin, cycle it to end and recurse to std::get<N-1>
         template <std::size_t N = (sizeof...(Vs) - 1)>
         void prev() requires (am_bidirectional<constify<Vs>...>) {
            auto& it = std::get<N>(currents_);
            if (it == std::ranges::begin(std::get<N>(*bases_))) {
               std::ranges::advance(it, std::ranges::end(std::get<N>(*bases_)));
               if constexpr (N > 0) {
                  prev<N - 1>();
               }
            }
            --it;
         }

         template <std::size_t N = (sizeof...(Vs) - 1)>
         void advance(difference_type n) requires (am_random_access<constify<Vs>...>) {
            auto& it = std::get<N>(currents_);
            auto& base = std::get<N>(*bases_);
            auto begin = std::ranges::begin(base);
            auto end = std::ranges::end(base);
            auto size = end - begin;

            auto distance_from_begin = it - begin;

            //Calculate where in the iterator cycle we should end up
            auto offset = (distance_from_begin + n) % size;

            //Calculate how many times incrementing this iterator would cause it to cycle round
            //This will be negative if we cycled by decrementing
            auto times_cycled = (distance_from_begin + n) / size - (offset < 0 ? 1 : 0);

            //Set the iterator to the correct new position
            it = begin + static_cast<difference_type>(offset < 0 ? offset + size : offset);

            if constexpr (N > 0) {
               //If this iterator cycled, then we need to advance the N-1th iterator
               //by the number of times it cycled
               if (times_cycled != 0) {
                  advance<N - 1>(times_cycled);
               }
            }
            else {
               //If we're the 0th iterator, then cycling should set the iterator to the end
               if (times_cycled > 0) {
                  std::ranges::advance(it, end);
               }
            }
         }

         constexpr bool equal(const cursor& rhs) const
            requires (std::equality_comparable<std::ranges::iterator_t<constify<Vs>>> && ...) {
            return currents_ == rhs.currents_;
         }

         constexpr bool equal(const sentinel<Const>& s) const {
            return std::get<0>(currents_) == s.end_;
         }

         template <std::size_t N = (sizeof...(Vs) - 1)>
         constexpr auto distance_to(cursor const& other) const 
            requires (am_distanceable<constify<Vs>...>) {
            if constexpr (N == 0) {
               return std::ranges::distance(std::get<0>(currents_), std::get<0>(other.currents_));
            } 
            else {
               auto distance = distance_to<N - 1>(other);
               auto scale = std::ranges::distance(std::get<N>(*bases_));
               auto diff = std::ranges::distance(std::get<N>(currents_), std::get<N>(other.currents_));
               return difference_type{ distance * scale + diff };
            }
         }

         friend class cursor<!Const>;
      };

   public:
      cartesian_product_view() = default;
      explicit cartesian_product_view(Vs... bases) : bases_(std::move(bases)...) {}

      constexpr auto begin() requires (!(simple_view<Vs> && ...)) {
         return basic_iterator{ cursor<false>(begin_tag, std::addressof(bases_)) };
      }
      constexpr auto begin() const requires (std::ranges::range<const Vs> && ...) {
         return basic_iterator{ cursor<true>(begin_tag, std::addressof(bases_)) };
      }

      constexpr auto end() requires (!(simple_view<Vs> && ...) && am_common<Vs...>) {
         return basic_iterator{ cursor<false>(end_tag, std::addressof(bases_)) };
      }

      constexpr auto end() const requires (am_common<const Vs...>) {
         return basic_iterator{ cursor<true>(end_tag, std::addressof(bases_)) };
      }

      constexpr auto end() requires(!(simple_view<Vs> && ...) && !am_common<Vs...>) {
         return sentinel<false>(std::ranges::end(std::get<0>(bases_)));
      }

      constexpr auto end() const requires ((std::ranges::range<const Vs> && ...) && !am_common<const Vs...>){
         return sentinel<true>(std::ranges::end(std::get<0>(bases_)));
      }

      constexpr auto size() requires (am_sized<Vs...>) {
         return std::apply([](auto&&... bases) {
            using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
            return (static_cast<size_type>(std::ranges::size(bases)) * ...);
            }, bases_);
      }

      constexpr auto size() const requires (am_sized<const Vs...>) {
         //Multiply all the sizes together, returning the common type of all of them
         return std::apply([](auto&&... bases) {
            using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
            return (static_cast<size_type>(std::ranges::size(bases)) * ...);
            }, bases_);
      }
   };

   template <class... Rs>
   cartesian_product_view(Rs&&...)->cartesian_product_view<std::views::all_t<Rs>...>;

   namespace views {
      namespace detail {
         class cartesian_product_fn {
         public:
            constexpr std::ranges::empty_view<std::tuple<>> operator()() const noexcept {
               return {};
            }
            template <std::ranges::viewable_range... V>
            requires (sizeof...(V) != 0)
               constexpr auto operator()(V&&... vs) const {
               return tl::cartesian_product_view{ std::views::all(std::forward<V>(vs))... };
            }
         };
      }  // namespace detail

      inline constexpr detail::cartesian_product_fn cartesian_product;
   }  // namespace views

}  // namespace tl

#endif