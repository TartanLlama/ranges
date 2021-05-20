#ifndef TL_RANGES_TRANSFORM_MAYBE
#define TL_RANGES_TRANSFORM_MAYBE

#include <ranges>
#include <concepts>
#include "common.hpp"
#include "basic_iterator.hpp"

namespace tl {
   template <std::ranges::input_range V, std::invocable<std::ranges::range_reference_t<V>> F>
   requires std::ranges::view<V>
   class transform_maybe_view : std::ranges::view_interface<transform_maybe_view<V,F>> {
   private:
      V base_;
      F func_;

      template <bool Const>
      struct sentinel {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;

         std::ranges::iterator_t<constify<V>> end_;
      };

      template <bool Const>
      struct cursor {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;

         static constexpr inline bool single_pass = detail::single_pass_iterator<std::ranges::iterator_t<constify<V>>>;

         std::ranges::iterator_t<constify<V>> current_;
         constify<transform_maybe_view>* parent_;
         mutable std::remove_cvref_t<std::invoke_result_t<F, std::ranges::range_reference_t<V>>> cache_;

         cursor() = default;
         constexpr cursor(begin_tag_t, constify<transform_maybe_view>* parent)
            : current_(std::ranges::begin(parent->base_)), parent_(parent) {
            auto result = std::invoke(parent_->func_, *current_);
            while (!result && current_ != std::ranges::end(parent_->base_)) {
               ++current_;
               result = std::invoke(parent_->func_, *current_);
            };
            cache_ = std::move(result);
         }
         constexpr cursor(end_tag_t, constify<transform_maybe_view>* parent)
            : current_(std::ranges::end(parent->base_)), parent_(parent) {}

         constexpr cursor(cursor<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) },
            parent_(i.parent_){}

         auto const& read() const {
            return *cache_;
         }

         void next() {
            decltype(cache_) result = std::nullopt;
            do {
               ++current_;
               if (current_ == std::ranges::end(parent_->base_)) return;
               result = std::invoke(parent_->func_, *current_);
            } while (!result);
            cache_ = std::move(result);
         }

         bool equal(cursor const& s) const {
            return current_ == s.current_;
         }

         bool equal(basic_sentinel<V,Const> const& s) const {
            return current_ == s.end_;
         }
      };
   public:
      template <class T>
      constexpr inline static bool am_common = std::ranges::common_range<T>;

      transform_maybe_view() = default;
      transform_maybe_view(V v, F f) : base_(std::move(v)), func_(std::move(f)) {}

      constexpr auto begin() requires(!simple_view<V>) {
         return basic_iterator{ cursor<false>(begin_tag, this) };
      }
      constexpr auto begin() const requires std::ranges::range<const V> {
         return basic_iterator{ cursor<true>(begin_tag, this) };
      }

      constexpr auto end() requires(!simple_view<V> && am_common<V>) {
         return basic_iterator{ cursor<false>(end_tag, this) };
      }
      constexpr auto end() const requires (std::ranges::range<const V> && am_common<const V>) {
         return basic_iterator{ cursor<true>(end_tag, this) };
      }
      
      constexpr auto end() requires (!simple_view<V> && !am_common<V>) {
         return basic_sentinel<V,false>{std::ranges::end(base_)};
      }
      constexpr auto end() const requires (std::ranges::range<const V> && !am_common<const V>) {
         return basic_sentinel<V,true>{std::ranges::end(base_)};
      }
   };

   template <class R, class F>
   transform_maybe_view(R&&, F f)->transform_maybe_view<std::views::all_t<R>, F>;

   namespace views {
      namespace detail {
         template <class F>
         struct transform_maybe_closure {
            F f;

            template <std::ranges::forward_range R>
            friend constexpr auto operator|(R&& r, transform_maybe_closure&& c) {
               return transform_maybe_view(std::forward<R>(r), std::move(c.f));
            }
         };

         struct transform_maybe_fn {
            template <class F>
            constexpr auto operator()(F f) const {
               return transform_maybe_closure<F>{ std::move(f) };
            }
         };
      }

      constexpr inline detail::transform_maybe_fn transform_maybe;
   }
}

namespace std::ranges {
   template <class V, class F>
   inline constexpr bool enable_borrowed_range<tl::transform_maybe_view<V, F>> = enable_borrowed_range<V>;
}
#endif