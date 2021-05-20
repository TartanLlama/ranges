#ifndef TL_RANGES_CHUNK_BY
#define TL_RANGES_CHUNK_BY

#include <ranges>
#include <iterator>
#include "common.hpp"
#include "basic_iterator.hpp"

namespace tl {
   template <std::ranges::forward_range V, std::predicate<std::ranges::range_reference_t<V>, std::ranges::range_reference_t<V>> F>
   class chunk_by_view 
      : public std::ranges::view_interface<chunk_by_view<V,F>> {
   private:
      V base_;
      F func_;

      template <bool Const>
      struct cursor {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;

         std::ranges::iterator_t<constify<V>> current_;
         std::ranges::iterator_t<constify<V>> end_of_current_range_;
         constify<chunk_by_view>* parent_;

         using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
         using pointer_type = value_type*;
         using iterator_category = std::forward_iterator_tag;

         void find_end_of_current_range() {
            auto first_failed = std::adjacent_find(current_, std::end(parent_->base_), std::not_fn(parent_->func_));
            end_of_current_range_ = std::ranges::next(first_failed, 1, std::end(parent_->base_));
         }

         cursor() = default;
         constexpr cursor(std::ranges::iterator_t<constify<V>> current, constify<chunk_by_view>* parent)
            : current_(std::move(current)), parent_(parent) {
            find_end_of_current_range();
         }

         constexpr cursor(cursor<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) }, end_of_current_range_{ std::move(i.end_of_current_range_) },
            parent_(i.parent_){}

         constexpr auto read() const {
            return std::ranges::subrange{ current_, end_of_current_range_ };
         }

         constexpr void next() {
            current_ = end_of_current_range_;
            find_end_of_current_range();
         }
 
         constexpr bool equal(cursor const& rhs) const{
            return current_ == rhs.current_;
         }
         constexpr bool equal(basic_sentinel<V,Const> const& rhs) const {
            return current_ == rhs.end();
         }

         friend struct cursor<!Const>;
      };

   public:
      chunk_by_view() = default;
      chunk_by_view(V v, F f) : base_(std::move(v)), func_(std::move(f)) {}

      constexpr auto begin() requires (!simple_view<V>) {
         return basic_iterator{ cursor<false>{ std::ranges::begin(base_), this } };
      }

      constexpr auto begin() const requires (std::ranges::range<const V>) {
         return basic_iterator{ cursor<true>{ std::ranges::begin(base_), this } };
      }

      constexpr auto end() requires(!simple_view<V>) {
         return basic_sentinel<V, false>{std::ranges::end(base_)};
      }

      constexpr auto end() const requires std::ranges::range<const V> {
         return basic_sentinel<V, true>{std::ranges::end(base_)};
      }

      auto& base() {
         return base_;
      }

      auto const& base() const {
         return base_;
      }
   };

   template <class R, class F>
   chunk_by_view(R&&, F f)->chunk_by_view<std::views::all_t<R>, F>;

   namespace views {
      namespace detail {
         template <class F>
         struct chunk_by_closure {
            F f;

            template <std::ranges::forward_range R>
            friend constexpr auto operator|(R&& r, chunk_by_closure&& c) {
               return chunk_by_view(std::forward<R>(r), std::move(c.f));
            }
         };

         struct chunk_by_fn {
            template <class F>
            constexpr auto operator()(F f) const {
               return chunk_by_closure<F>{ std::move(f) };
            }
         };
      }

      constexpr inline detail::chunk_by_fn chunk_by;
   }
}

#endif