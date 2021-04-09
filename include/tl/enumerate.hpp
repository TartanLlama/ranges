#ifndef TL_RANGES_ENUMERATE_HPP
#define TL_RANGES_ENUMERATE_HPP

#include <iterator>
#include <ranges>
#include <type_traits>

namespace tl {
   namespace detail {
      template <class R>
      concept simple_view = std::ranges::view<R> && std::ranges::range<const R> &&
         std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
         std::same_as<std::ranges::sentinel_t<R>,
         std::ranges::sentinel_t<const R>>;
   }

   template <std::ranges::input_range V>
   requires std::ranges::view<V> class enumerate_view
      : public std::ranges::view_interface<enumerate_view<V>> {
      V base_;

      template <bool Const>
      class sentinel;
      template <bool Const>
      class iterator {
         using Base = std::conditional_t<Const, const V, V>;
         using count_type = decltype([] {
            if constexpr (std::ranges::sized_range<Base>)
               return std::ranges::range_size_t<Base>();
            else {
               return std::make_unsigned_t<std::ranges::range_difference_t<Base>>();
            }
            }());

         std::ranges::iterator_t<Base> current_{};
         count_type pos_ = 0;

      public:
         using iterator_category = typename std::iterator_traits<
            std::ranges::iterator_t<Base>>::iterator_category;
         using reference =
            std::pair<count_type, std::ranges::range_reference_t<Base>>;
         using value_type = std::pair<count_type, std::ranges::range_value_t<Base>>;

         using difference_type = std::ranges::range_difference_t<Base>;

         iterator() = default;
         constexpr explicit iterator(std::ranges::iterator_t<Base> current,
            difference_type pos)
            : current_{ std::move(current) }, pos_{ static_cast<count_type>(pos) } {}

         constexpr iterator(iterator<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<Base>>
            : current_{ std::move(i.current_) }, pos_{ i.pos_ } {}

         constexpr std::ranges::iterator_t<Base> base()
            const& requires std::copyable<std::ranges::iterator_t<Base>> {
            return current_;
         }
         constexpr std::ranges::iterator_t<Base> base()&& {
            return std::move(current_);
         }

         constexpr decltype(auto) operator*() const {
            return reference{ pos_, *current_ };
         }

         constexpr iterator& operator++() {
            ++current_;
            ++pos_;
            return *this;
         }
         constexpr void operator++(int) requires(!std::ranges::forward_range<Base>) {
            (void)operator++();
         }
         constexpr iterator operator++(
            int) requires std::ranges::forward_range<Base> {
            auto temp = *this;
            ++pos_;
            ++current_;
            return temp;
         }

         constexpr iterator&
            operator--() requires std::ranges::bidirectional_range<Base> {
            --pos_;
            --current_;
            return *this;
         }
         constexpr iterator operator--(
            int) requires std::ranges::bidirectional_range<Base> {
            auto temp = *this;
            --pos_;
            --current_;
            return temp;
         }
         constexpr iterator& operator+=(
            difference_type x) requires std::ranges::random_access_range<Base> {
            pos_ += x;
            current_ += x;
            return *this;
         }
         constexpr iterator& operator-=(
            difference_type x) requires std::ranges::random_access_range<Base> {
            pos_ -= x;
            current_ -= x;
            return *this;
         }
         constexpr decltype(auto) operator[](difference_type n) const
            requires std::ranges::random_access_range<Base> {
            return reference{ static_cast<difference_type>(pos_ + n), *(current_ + n) };
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
         friend constexpr difference_type operator-(
            const iterator& x,
            const iterator& y) requires std::ranges::random_access_range<Base> {
            return y - x;
         }

         template <bool>
         friend class sentinel;
         friend class iterator<!Const>;
      };
      template <bool Const>
      class sentinel {
         using Base = std::conditional_t<Const, const V, V>;
         std::ranges::sentinel_t<Base> end_{};

      public:
         sentinel() = default;
         constexpr explicit sentinel(std::ranges::sentinel_t<Base> end)
            : end_{ std::move(end) } {}
         constexpr sentinel(sentinel<!Const> other) requires Const&& std::
            convertible_to<std::ranges::sentinel_t<V>,
            std::ranges::sentinel_t<Base>>
            : end_{ std::move(other.end_) } {}

         constexpr std::ranges::sentinel_t<Base> base() const { return end_; }

         friend constexpr bool operator==(const iterator<Const>& x,
            const sentinel& y) {
            return x.base() == y.end_;
         }
         friend constexpr std::ranges::range_difference_t<Base> operator-(
            const iterator<Const>& x,
            const sentinel&
            y) requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
            std::ranges::iterator_t<Base>> {
            return x.base() - y.end_;
         }
         friend constexpr std::ranges::range_difference_t<Base> operator-(
            const sentinel& x,
            const iterator<Const>&
            y) requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
            std::ranges::iterator_t<Base>> {
            return x.end_ - y.base();
         }
      };

   public:
      enumerate_view() = default;
      enumerate_view(V base) : base_(std::move(base)) {}

      constexpr auto begin() requires(!detail::simple_view<V>) {
         return iterator<false>(std::ranges::begin(base_), 0);
      }
      constexpr auto begin() const requires detail::simple_view<V> {
         return iterator<true>(std::ranges::begin(base_), 0);
      }

      constexpr auto end() { return sentinel<false>{std::ranges::end(base_)}; }

      constexpr auto
         end() requires std::ranges::common_range<V>&& std::ranges::sized_range<V> {
         return iterator<false>{
            std::ranges::end(base_),
               static_cast<std::ranges::range_difference_t<V>>(size())};
      }

      constexpr auto end() const requires std::ranges::range<const V> {
         return sentinel<true>{std::ranges::end(base_)};
      }

      constexpr auto end() const requires std::ranges::common_range<
         const V>&& std::ranges::sized_range<V> {
         return iterator<true>{
            std::ranges::end(base_),
               static_cast<std::ranges::range_difference_t<V>>(size())};
      }

      constexpr auto size() requires std::ranges::sized_range<V> {
         return std::ranges::size(base_);
      }
      constexpr auto size() const requires std::ranges::sized_range<const V> {
         return std::ranges::size(base_);
      }

      constexpr V base() const& requires std::copy_constructible<V> {
         return base_;
      }
      constexpr V base()&& { return std::move(base_); }
   };

   template <class R>
   enumerate_view(R&&)->enumerate_view<std::views::all_t<R>>;

   namespace views {
      namespace detail {
         class enumerate_fn {
         public:
            template <std::ranges::viewable_range V>
            constexpr auto operator()(V&& v) const {
               return tl::enumerate_view{ std::forward<V>(v) };
            }

            template <std::ranges::viewable_range V>
            friend constexpr auto operator|(V&& v, enumerate_fn) {
               return tl::enumerate_view{ std::forward<V>(v) };
            }
         };
      }  // namespace detail

      inline constexpr detail::enumerate_fn enumerate;
   }  // namespace views
}  // namespace tl

#endif