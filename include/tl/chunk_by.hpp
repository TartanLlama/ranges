#ifndef TL_RANGES_CHUNK_BY
#define TL_RANGES_CHUNK_BY

#include <ranges>
#include <iterator>
#include "common.hpp"

namespace tl {
   template <std::ranges::forward_range V, std::predicate<std::ranges::range_reference_t<V>, std::ranges::range_reference_t<V>> F>
   class chunk_by_view {
   private:
      V base_;
      F func_;

      friend struct iterator;

      template <bool Const>
      struct iterator {
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

         iterator() = default;
         constexpr iterator(std::ranges::iterator_t<constify<V>> current, constify<chunk_by_view>* parent)
            : current_(std::move(current)), parent_(parent) {
            find_end_of_current_range();
         }

         constexpr iterator(iterator<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) }, end_of_current_range_{ std::move(i.end_of_current_range_) },
            parent_(i.parent_){}

         constexpr auto operator*() {
            return std::ranges::subrange{ current_, end_of_current_range_ };
         }

         constexpr iterator& operator++() {
            current_ = end_of_current_range_;
            find_end_of_current_range();
            return *this;
         }
         constexpr iterator operator++(int) {
            auto tmp = *this;
            ++* this;
            return tmp;
         }
         friend constexpr bool operator==(iterator const& lhs, iterator const& rhs) {
            return lhs.current_ == rhs.current_;
         }
         friend constexpr bool operator==(iterator const& lhs, std::default_sentinel_t) {
            return lhs.current_ == std::ranges::end(lhs.parent_->base());
         }

         friend struct iterator<!Const>;
      };

   public:
      chunk_by_view() = default;
      chunk_by_view(V v, F f) : base_(std::move(v)), func_(std::move(f)) {}

      constexpr auto begin() requires (!simple_view<V>) {
         return iterator<false>{ std::ranges::begin(base_), this };
      }

      constexpr auto begin() const requires (std::ranges::range<const V>) {
         return iterator<true>{ std::ranges::begin(base_), this };
      }

      constexpr std::default_sentinel_t end() const {
         return std::default_sentinel;
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