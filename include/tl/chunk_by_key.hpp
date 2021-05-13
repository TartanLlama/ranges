#ifndef TL_RANGES_CHUNK_BY_KEY
#define TL_RANGES_CHUNK_BY_KEY

#include <ranges>
#include <iterator>
#include <utility>
#include "common.hpp"

namespace tl {
   template <std::ranges::forward_range V, std::invocable<std::ranges::range_reference_t<V>> F>
   class chunk_by_key_view {
   private:
      V base_;
      F func_;

      friend struct iterator;

      template <bool Const>
      struct iterator {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;

         using key_type = std::invoke_result_t<F, std::ranges::range_reference_t<constify<V>>>;

         std::ranges::iterator_t<constify<V>> current_;
         std::ranges::iterator_t<constify<V>> end_of_current_range_;
         key_type current_key_;
         constify<chunk_by_key_view>* parent_;

         using value_type = std::pair<key_type, std::ranges::subrange<std::ranges::iterator_t<V>>>;
         using pointer_type = value_type*;
         using iterator_category = std::forward_iterator_tag;

         void find_end_of_current_range() {
            if (current_ != std::ranges::end(parent_->base_)) {
               current_key_ = std::invoke(parent_->func_, *current_);
               end_of_current_range_ = std::find_if(current_, std::end(parent_->base_), [this](auto&& v) { return std::invoke(parent_->func_, v) != current_key_; });
            }
         }

         iterator() = default;
         constexpr explicit iterator(std::ranges::iterator_t<constify<V>> current, constify<chunk_by_key_view>* parent)
            : current_(std::move(current)), parent_(parent) {
            find_end_of_current_range();
         }

         constexpr iterator(iterator<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) }, end_of_current_range_{ std::move(i.end_of_current_range_) },
            parent_(i.parent_){}

         constexpr auto operator*() {
            return std::pair(current_key_, std::ranges::subrange{ current_, end_of_current_range_ });
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
      chunk_by_key_view() = default;
      chunk_by_key_view(V v, F f) : base_(std::move(v)), func_(std::move(f)) {}

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
   chunk_by_key_view(R&&, F f)->chunk_by_key_view<std::views::all_t<R>, F>;

   namespace views {
      namespace detail {
         template <class F>
         struct chunk_by_key_closure {
            F f;

            template <std::ranges::forward_range R>
            friend constexpr auto operator|(R&& r, chunk_by_key_closure&& c) {
               return chunk_by_key_view(std::forward<R>(r), std::move(c.f));
            }
         };

         struct chunk_by_key_fn {
            template <class F>
            constexpr auto operator()(F f) const {
               return chunk_by_key_closure<F>{ std::move(f) };
            }
         };
      }

      constexpr inline detail::chunk_by_key_fn chunk_by_key;
   }
}

#endif