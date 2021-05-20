#ifndef TL_RANGES_CHUNK
#define TL_RANGES_CHUNK

#include <ranges>
#include <iterator>
#include "common.hpp"
#include "basic_iterator.hpp"

namespace tl {
   template <std::ranges::forward_range V>
   class chunk_view
      : public std::ranges::view_interface<chunk_view<V>> {
   private:
      V base_;
      std::ranges::range_size_t<V> size_;

      //Case when underlying range is not bidirectional, i.e. we don't care about calculating offsets
      template <bool Const, class Base = std::conditional_t<Const, const V, V>, bool = std::ranges::bidirectional_range<Base>, bool = std::ranges::sized_range<Base>>
      struct cursor_base {
         std::ranges::iterator_t<Base> current_{};
         Base* base_;

         cursor_base() = default;
         constexpr explicit cursor_base(std::ranges::iterator_t<Base> current, Base* base)
            : current_{ std::move(current) }, base_{ base }  {}

         void set_offset(std::ranges::range_difference_t<Base> off) {}

         std::ranges::range_difference_t<Base> get_offset() {
            return 0;
         }
      };

      //Case when underlying range is bidirectional but not sized. We need to keep track of the offset if we hit the end iterator.
      template <bool Const, class Base>
      struct cursor_base<Const, Base, true, false> {
         std::ranges::iterator_t<Base> current_{};
         Base* base_;

         cursor_base() = default;
         constexpr explicit cursor_base(std::ranges::iterator_t<Base> current, Base* base)
            : current_{ std::move(current) }, base_{ base }  {}

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
      struct cursor_base<Const, Base, true, true> {
         std::ranges::iterator_t<Base> current_{};
         Base* base_;

         cursor_base() = default;
         constexpr explicit cursor_base(std::ranges::iterator_t<Base> current, Base* base)
            : current_{ std::move(current) }, base_{ base }  {}

         void set_offset(std::ranges::range_difference_t<Base>) {}

         std::ranges::range_difference_t<Base> get_offset() {
            return base_->size_ - (std::ranges::size(*base_) % base_->size_);
         }
      };

      template <bool Const>
      struct cursor : cursor_base<Const> {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;
         using Base = constify<V>;

         std::ranges::iterator_t<constify<V>> current_;
         constify<chunk_view>* parent_;

         using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
         using pointer_type = value_type*;
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ranges::range_difference_t<Base>;

         cursor() = default;
         constexpr cursor(begin_tag_t, constify<chunk_view>* parent)
            : current_(std::ranges::begin(parent->base_)), parent_(parent) {
         }
         constexpr cursor(end_tag_t, constify<chunk_view>* parent)
            : current_(std::ranges::end(parent->base_)), parent_(parent) {
         }

         constexpr cursor(cursor<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) },
            parent_(i.parent_){}

         constexpr auto read() const {
            auto last = std::ranges::next(current_, parent_->size_, std::ranges::end(parent_->base_));
            return std::ranges::subrange{ current_, last };
         }

         constexpr void next() {
            auto delta = std::ranges::advance(current_, parent_->size_, std::ranges::end(parent_->base_));
            this->set_offset(delta);
         }

         constexpr void prev() requires std::ranges::bidirectional_range<Base> {
            auto delta = -this->parent_->size;
            if (this->current_ == std::ranges::end(*this->base_)) {
               delta += this->get_offset();
            }
            std::advance(this->current_, delta);
         }

         constexpr void advance(difference_type x) 
            requires std::ranges::random_access_range<Base> {
            if (x == 0) return;

            auto last = std::ranges::end(*this->base_);
            x *= this->parent_->size_;

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
         }

         constexpr bool equal(cursor const& rhs) const {
            return current_ == rhs.current_;
         }
         constexpr bool equal(std::default_sentinel_t) const {
            return current_ == std::ranges::end(parent_->base());
         }

         friend struct cursor<!Const>;
      };

   public:
      template <class T>
      static constexpr bool am_common = std::ranges::common_range<T> &&
         (!std::ranges::bidirectional_range<T> || std::ranges::sized_range<T>);

      template <class T> static constexpr bool am_sized = std::ranges::sized_range<T>;

      chunk_view() = default;
      chunk_view(V v, std::ranges::range_size_t<V> n) : base_(std::move(v)), size_(n) {}

      constexpr auto begin() requires (!simple_view<V>) {
         return basic_iterator{ cursor<false>{ begin_tag, this } };
      }

      constexpr auto begin() const requires (std::ranges::range<const V>) {
         return basic_iterator{ cursor<true>{ begin_tag, this } };
      }

      constexpr auto end() requires (!simple_view<V> && am_common<V>) {
         return basic_iterator{ cursor<false>{ end_tag, this } };
      }

      constexpr auto end() const requires (std::ranges::range<const V> && am_common<const V>) {
         return basic_iterator{ cursor<true>{ end_tag, this } };
      }

      constexpr std::default_sentinel_t end() const {
         return std::default_sentinel;
      }

      constexpr auto size() requires (am_sized<V>) {
         return (std::ranges::size(base_) + size_ - 1) / size_;
      }

      constexpr auto size() const requires (am_sized<const V>) {
         return (std::ranges::size(base_) + size_ - 1) / size_;
      }

      auto& base() {
         return base_;
      }

      auto const& base() const {
         return base_;
      }
   };

   template <class R, class N>
   chunk_view(R&&, N n)->chunk_view<std::views::all_t<R>>;

   namespace views {
      namespace detail {
         template <class N>
         struct chunk_closure {
            N n;

            template <std::ranges::forward_range R>
            friend constexpr auto operator|(R&& r, chunk_closure&& c) {
               return chunk_view(std::forward<R>(r), c.n);
            }
         };

         struct chunk_fn {
            template <std::integral N>
            constexpr auto operator()(N n) const {
               return chunk_closure<N>{ n };
            }

            template <std::ranges::forward_range R, std::integral N>
            constexpr auto operator()(R&& r, N n) const {
               return chunk_view{ std::forward<R>(r),  n };
            }
         };
      }

      constexpr inline detail::chunk_fn chunk;
   }
}

#endif