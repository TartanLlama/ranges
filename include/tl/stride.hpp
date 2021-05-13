#ifndef TL_RANGES_STRIDE_HPP
#define TL_RANGES_STRIDE_HPP

#include <iterator>
#include <ranges>
#include <type_traits>
#include "common.hpp"

namespace tl {
   namespace detail {
      //Stride view can be common if the underlying range is not bidirectional, 
      //but if it is bidirectional then it is required to be sized because
      //working out where to go when you decrement the end iterator requires computing 
      //the offset.
      template <class V>
      concept stride_view_is_common = std::ranges::common_range<V> &&
         (!std::ranges::bidirectional_range<V> || std::ranges::sized_range<V>);
   }
   template <std::ranges::input_range V>
   requires std::ranges::view<V> class stride_view
      : public std::ranges::view_interface<stride_view<V>> {
      V base_;
      std::ranges::range_difference_t<V> stride_;

      template <bool Const>
      class sentinel;

      //Case when underlying range is not bidirectional, i.e. we don't care about calculating offsets
      template <bool Const, class Base = std::conditional_t<Const, const V, V>, bool = std::ranges::bidirectional_range<Base>, bool = std::ranges::sized_range<Base>>
      struct iterator_base {
         std::ranges::iterator_t<Base> current_{};
         std::ranges::range_difference_t<Base> stride_;
         Base* base_;

         iterator_base() = default;
         constexpr explicit iterator_base(std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> stride, Base* base)
            : current_{ std::move(current) }, stride_{ stride }, base_{ base }  {}

         void set_offset(std::ranges::range_difference_t<Base> off) {}

         std::ranges::range_difference_t<Base> get_offset() {
            return 0;
         }
      };

      //Case when underlying range is bidirectional but not sized. We need to keep track of the offset if we hit the end iterator.
      template <bool Const, class Base>
      struct iterator_base<Const, Base, true, false> {
         std::ranges::iterator_t<Base> current_{};
         std::ranges::range_difference_t<Base> stride_;
         Base* base_;

         iterator_base() = default;
         constexpr explicit iterator_base(std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> stride, Base* base)
            : current_{ std::move(current) }, stride_{ stride }, base_{ base }  {}

         using difference_type = std::ranges::range_difference_t<Base>;
         difference_type offset_;

         void set_offset(difference_type off) {
            offset_ = off;
         }

         difference_type get_offset() {
            return offset_;
         }
      };

      //Case where underlying is bidirectional and sized. We can calculate offsets from the end on-demand.
      template <bool Const, class Base>
      struct iterator_base<Const, Base, true, true> {
         std::ranges::iterator_t<Base> current_{};
         std::ranges::range_difference_t<Base> stride_;
         Base* base_;

         iterator_base() = default;
         constexpr explicit iterator_base(std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> stride, Base* base)
            : current_{ std::move(current) }, stride_{ stride }, base_{ base }  {}

         void set_offset(std::ranges::range_difference_t<Base>) {}

         std::ranges::range_difference_t<Base> get_offset() {
            return stride_ - (std::ranges::size(*base_) % stride_);
         }
      };

      template <bool Const>
      class iterator : iterator_base<Const> {
         using Base = std::conditional_t<Const, const V, V>;

      public:
         using iterator_category = typename std::iterator_traits<
            std::ranges::iterator_t<Base>>::iterator_category;
         using reference = std::ranges::range_reference_t<Base>;
         using value_type = std::ranges::range_value_t<Base>;
         using difference_type = std::ranges::range_difference_t<Base>;

         iterator() = default;
         constexpr explicit iterator(std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> stride, Base* base)
            : iterator_base<Const>{ std::move(current), stride, base } {}

         constexpr iterator(iterator<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<Base>>
            : iterator_base<!Const>{ std::move(i.current_), i.stride_, i.base_ } {}

         constexpr std::ranges::iterator_t<Base> base()
            const& requires std::copyable<std::ranges::iterator_t<Base>> {
            return this->current_;
         }
         constexpr std::ranges::iterator_t<Base> base()&& {
            return std::move(this->current_);
         }

         constexpr decltype(auto) operator*() const {
            return *this->current_;
         }

         constexpr iterator& operator++() {
            auto last = std::ranges::end(*this->base_);
            auto delta = std::ranges::advance(this->current_, this->stride_, last);
            this->set_offset(delta);
            return *this;
         }
         constexpr void operator++(int) requires(!std::ranges::forward_range<Base>) {
            (void)operator++();
         }
         constexpr iterator operator++(
            int) requires std::ranges::forward_range<Base> {
            auto temp = *this;
            this->operator++();
            return temp;
         }

         constexpr iterator&
            operator--() requires std::ranges::bidirectional_range<Base> {
            auto delta = -this->stride_;
            if (this->current_ == std::ranges::end(*this->base_)) {
               delta += this->get_offset();
            }
            std::advance(this->current_, delta);
            return *this;
         }
         constexpr iterator operator--(
            int) requires std::ranges::bidirectional_range<Base> {
            auto temp = *this;
            this->operator--();
            return temp;
         }
         constexpr iterator& operator+=(
            difference_type x) requires std::ranges::random_access_range<Base> {
            if (x == 0) return *this;

            auto last = std::ranges::end(*this->base_);
            x *= this->stride_;

            if (x > 0) {
               auto delta = std::advance(this->current_, x, last);
               this->set_offset(delta);
            }
            else if (x < 0) {
               if (this->current_ == last) {
                  x += this->get_offset();
               }
               std::advance(this->current, x);
            }

            return *this;
         }
         constexpr iterator& operator-=(
            difference_type x) requires std::ranges::random_access_range<Base> {
            this->operator+=(-x);
            return *this;
         }
         constexpr decltype(auto) operator[](difference_type n) const
            requires std::ranges::random_access_range<Base> {
            return *((*this) + n);
         }

         friend constexpr bool operator==(const iterator& x,
            const iterator& y) requires std::
            equality_comparable<std::ranges::iterator_t<Base>> {
            return x.current_ == y.current_;
         }

         friend constexpr auto operator<(
            const iterator& x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return x.current_ < y.current_;
         }
         friend constexpr auto operator>(
            const iterator& x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return x.current_ > y.current_;
         }
         friend constexpr auto operator<=(
            const iterator& x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return x.current_ <= y.current_;
         }
         friend constexpr auto operator>=(
            const iterator& x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return x.current_ >= y.current_;
         }
         friend constexpr auto operator<=>(const iterator& x,
            const iterator& y) requires std::ranges::
            random_access_range<Base>&& std::three_way_comparable<
            std::ranges::iterator_t<Base>> {
            return x.current_ <=> y.current_;
         }

         friend constexpr iterator operator+(
            const iterator& x,
            difference_type y) requires std::ranges::random_access_range<Base> {
            return iterator{ x } += y;
         }
         friend constexpr iterator operator+(
            difference_type x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return y + x;
         }
         friend constexpr iterator operator-(
            const iterator& x,
            difference_type y) requires std::ranges::random_access_range<Base> {
            return iterator{ x } - y;
         }

         friend class iterator<!Const>;
         template <bool>
         friend class sentinel;
      };

      template <bool Const>
      class sentinel {
         using parent = std::conditional_t<Const, const V, V>;
         std::ranges::sentinel_t<parent> end_;

      public:
         sentinel() = default;
         sentinel(std::ranges::sentinel_t<parent> end) : end_(std::move(end)) {}

         friend constexpr bool operator==(sentinel const& s, std::ranges::iterator_t<parent> const& it) {
            return it.current_ == s.end_;
         }

         constexpr sentinel(sentinel<!Const> other) requires Const &&
            (std::convertible_to<std::ranges::sentinel_t<parent>, std::ranges::sentinel_t<const parent>>)
            : end_(std::move(other.end_)) {
         }
      };

   public:
      stride_view() = default;
      stride_view(V base, std::ranges::range_difference_t<V> d) : base_(std::move(base)), stride_(d) {}

      constexpr auto begin() requires (!simple_view<V>) {
         return iterator<false>(std::ranges::begin(base_), stride_, std::addressof(base_));
      }
      constexpr auto begin() const requires (std::ranges::range<const V>) {
         return iterator<true>(std::ranges::begin(base_), stride_, std::addressof(base_));
      }

      constexpr auto end() requires (!simple_view<V> && detail::stride_view_is_common<V>) {
         return iterator<false>(std::ranges::end(base_), stride_, std::addressof(base_));
      }

      constexpr auto end() const requires (std::ranges::range<const V>&& detail::stride_view_is_common<const V>) {
         return iterator<true>(std::ranges::end(base_), stride_, std::addressof(base_));
      }

      constexpr auto end() requires (!simple_view<V> && !detail::stride_view_is_common<V>) {
         return sentinel<false>(std::ranges::end(base_), stride_, std::addressof(base_));
      }

      constexpr auto end() const requires (std::ranges::range<const V> && !detail::stride_view_is_common<const V>) {
         return sentinel<true>(std::ranges::end(base_), stride_, std::addressof(base_));
      }

      constexpr auto size() requires (std::ranges::sized_range<V>) {
         return (std::ranges::size(base_) + stride_ - 1) / stride_;
      }

      constexpr auto size() const requires (std::ranges::sized_range<const V>) {
         return (std::ranges::size(base_) + stride_ - 1) / stride_;
      }

      constexpr V base() const& requires std::copy_constructible<V> {
         return base_;
      }
      constexpr V base()&& { return std::move(base_); }
   };

   template <class R>
   stride_view(R&&, std::ranges::range_difference_t<R>)->stride_view<std::views::all_t<R>>;

   namespace views {
      namespace detail {
         template <std::integral D>
         struct stride_view_closure {
            D stride_;

            template <std::ranges::viewable_range V>
            friend constexpr auto operator|(V&& v, stride_view_closure const& clos) {
               return tl::stride_view{ std::forward<V>(v), clos.stride_ };
            }
         };

         class stride_fn {
         public:
            template <std::ranges::viewable_range V>
            constexpr auto operator()(V&& v, std::ranges::range_difference_t<V> d) const {
               return tl::stride_view{ std::forward<V>(v), d };
            }

            template <std::integral D>
            constexpr auto operator()(D d) const {
               return stride_view_closure{ d };
            }
         };
      }  // namespace detail

      inline constexpr detail::stride_fn stride;
   }  // namespace views
}  // namespace tl

#endif