#ifndef TL_RANGES_CARTESIAN_PRODUCT_HPP
#define TL_RANGES_CARTESIAN_PRODUCT_HPP

#include <iterator>
#include <ranges>
#include <type_traits>
#include <tuple>
#include "common.hpp"

namespace tl {
   namespace detail {
      template <class... Vs>
      concept cartesian_product_is_common = (std::ranges::common_range<Vs> && ...)
         || ((std::ranges::random_access_range<Vs> && ...) && (std::ranges::sized_range<Vs> && ...));
   }
   template <std::ranges::forward_range... Vs>
   requires (std::ranges::view<Vs> && ...) class cartesian_product_view
      : public std::ranges::view_interface<cartesian_product_view<Vs...>> {
      std::tuple<Vs...> bases_;

      template <bool Const>
      class iterator {
         template<class T>
         using constify = std::conditional_t<Const, const T, T>;

         constify<std::tuple<Vs...>>* bases_;
         std::tuple<std::ranges::iterator_t<constify<Vs>>...> currents_{};

      public:
         using iterator_category = tl::common_iterator_category<constify<Vs>...>;
         using reference =
            std::tuple<std::ranges::range_reference_t<constify<Vs>>...>;
         using value_type =
            std::tuple<std::ranges::range_value_t<constify<Vs>>...>;

         using difference_type = std::ptrdiff_t;

         iterator() = default;
         constexpr explicit iterator(begin_tag_t, constify<std::tuple<Vs...>>* bases)
            : bases_{ bases }, currents_{ std::apply([](auto&&... bs) { return std::make_tuple(std::ranges::begin(bs)...); }, *bases) }
         {}
         constexpr explicit iterator(end_tag_t, constify<std::tuple<Vs...>>* bases)
            requires(std::ranges::common_range<Vs> && ...)
            : iterator{ begin_tag, bases }
         {
            std::get<0>(currents_) = std::ranges::end(std::get<0>(*bases_));
         }
         constexpr explicit iterator(end_tag_t, constify<std::tuple<Vs...>>* bases)
            requires(!(std::ranges::common_range<Vs> && ...) && (std::ranges::random_access_range<Vs> && ...) && (std::ranges::sized_range<Vs> && ...))
            : iterator{ begin_tag, bases }
         {
            std::get<0>(currents_) += std::ranges::size(std::get<0>(*bases_));
         }

         constexpr iterator(iterator<!Const> i) requires Const && (std::convertible_to<
            std::ranges::iterator_t<Vs>,
            std::ranges::iterator_t<constify<Vs>>> && ...)
            : bases_{ std::move(i.bases_) }, currents_{ std::move(i.currents_) } {}


         constexpr decltype(auto) operator*() const {
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
         void prev() {
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
         void advance(difference_type n) {
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

         constexpr iterator& operator++() {
            next();
            return *this;
         }
         constexpr void operator++(int) requires(!(std::ranges::forward_range<constify<Vs>> && ...)) {
            (void)operator++();
         }
         constexpr iterator operator++(
            int) requires (std::ranges::forward_range<constify<Vs>> && ...) {
            auto temp = *this;
            next();
            return temp;
         }

         constexpr iterator&
            operator--() requires (std::ranges::bidirectional_range<constify<Vs>> && ...) {
            prev();
            return *this;
         }
         constexpr iterator operator--(
            int) requires (std::ranges::bidirectional_range<constify<Vs>> && ...) {
            auto temp = *this;
            prev();
            return temp;
         }

         constexpr iterator& operator+=(
            difference_type x) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            advance(x);
            return *this;
         }
         constexpr iterator& operator-=(
            difference_type x) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            this->operator+=(-x);
            return *this;
         }
         constexpr decltype(auto) operator[](difference_type n) const
            requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return *((*this) + n);
         }

         friend constexpr iterator operator+(
            const iterator& x,
            difference_type y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return iterator{ x } += y;
         }
         friend constexpr iterator operator+(
            difference_type x,
            const iterator& y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return y + x;
         }
         friend constexpr iterator operator-(
            const iterator& x,
            difference_type y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return iterator{ x } -= y;
         }

         friend constexpr bool operator==(const iterator& x,
            const iterator& y) requires (std::
               equality_comparable < std::ranges::iterator_t<constify<Vs>>> && ...) {
            return x.currents_ == y.currents_;
         }

         friend constexpr bool operator==(const iterator& x,
            const std::default_sentinel_t&) {
            return std::get<0>(x.currents_) == std::ranges::end(std::get<0>(*x.bases_));
         }

         friend constexpr auto operator<(
            const iterator& x,
            const iterator& y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return x.currents_ < y.currents_;
         }
         friend constexpr auto operator>(
            const iterator& x,
            const iterator& y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return x.currents_ > y.currents_;
         }
         friend constexpr auto operator<=(
            const iterator& x,
            const iterator& y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return x.currents_ <= y.currents_;
         }
         friend constexpr auto operator>=(
            const iterator& x,
            const iterator& y) requires (std::ranges::random_access_range<constify<Vs>> && ...) {
            return x.currents_ >= y.currents_;
         }
         friend constexpr auto operator<=>(const iterator& x,
            const iterator& y) requires ((std::ranges::random_access_range<constify<Vs>>&& std::three_way_comparable<constify<Vs>>) && ...) {
            return x.currents_ <=> y.currents_;
         }

         friend class sentinel;
         friend class iterator<!Const>;
      };

   public:
      cartesian_product_view() = default;
      explicit cartesian_product_view(Vs... bases) : bases_(std::move(bases)...) {}

      constexpr auto begin() requires (!(simple_view<Vs> && ...)) {
         return iterator<false>(begin_tag, std::addressof(bases_));
      }
      constexpr auto begin() const requires (std::ranges::range<const Vs> && ...) {
         return iterator<true>(begin_tag, std::addressof(bases_));
      }

      constexpr auto end() requires (!(simple_view<Vs> && ...) && detail::cartesian_product_is_common<Vs...>) {
         return iterator<false>(end_tag, std::addressof(bases_));
      }

      constexpr auto end() const requires (detail::cartesian_product_is_common<Vs...>) {
         return iterator<true>(end_tag, std::addressof(bases_));
      }

      constexpr auto end() const {
         return std::default_sentinel;
      }

      constexpr auto size() requires (std::ranges::sized_range<Vs> && ...) {
         return std::apply([](auto&&... bases) {
            using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
            return (static_cast<size_type>(std::ranges::size(bases)) * ...);
            });
      }

      constexpr auto size() const requires (std::ranges::sized_range<const Vs> && ...) {
         //Multiply all the sizes together, returning the common type of all of them
         return std::apply([](auto&&... bases) {
            using size_type = std::common_type_t<std::ranges::range_size_t<decltype(bases)>...>;
            return (static_cast<size_type>(std::ranges::size(bases)) * ...);
            });
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