#ifndef TL_RANGES_CHUNK_BY_KEY
#define TL_RANGES_CHUNK_BY_KEY

#include <ranges>
#include <iterator>
#include <utility>
#include "common.hpp"
#include "basic_iterator.hpp"
#include "utility/semiregular_box.hpp"

namespace tl {
   template <std::ranges::forward_range V, std::invocable<std::ranges::range_reference_t<V>> F>
   requires std::ranges::view<V>
   class chunk_by_key_view 
      : public std::ranges::view_interface<chunk_by_key_view<V,F>> {
   private:
      V base_;
      //Need to wrap F in a semiregular_box to ensure the view is moveable and default-initializable
      [[no_unique_address]] semiregular_box<F> func_;

      template <bool Const>
      struct cursor {
         template <class T>
         using constify = std::conditional_t<Const, const T, T>;

         using key_type = std::invoke_result_t<F, std::ranges::range_reference_t<constify<V>>>;

         std::ranges::iterator_t<constify<V>> current_;
         std::ranges::iterator_t<constify<V>> end_of_current_range_;
         std::optional<key_type> current_key_;
         constify<chunk_by_key_view>* parent_;

         //When the cursor is constructed or advanced then we'll calculate the end of the current range
         //by walking over the range until we find an adjacent pair that have different keys.
         void find_end_of_current_range() {
            if (current_ != std::ranges::end(parent_->base_)) {
               current_key_ = std::invoke(*parent_->func_, *current_);
               end_of_current_range_ = std::find_if(current_, std::end(parent_->base_), [this](auto&& v) { return std::invoke(*parent_->func_, v) != *current_key_; });
            }
         }

         cursor() = default;
         constexpr explicit cursor(std::ranges::iterator_t<constify<V>> current, constify<chunk_by_key_view>* parent)
            : current_(std::move(current)), parent_(parent) {
            find_end_of_current_range();
         }

         //const-converting constructor
         constexpr cursor(cursor<!Const> i) requires Const&& std::convertible_to<
            std::ranges::iterator_t<V>,
            std::ranges::iterator_t<const V>>
            : current_{ std::move(i.current_) }, end_of_current_range_{ std::move(i.end_of_current_range_) },
            parent_(i.parent_){}

         constexpr auto read() const {
            return std::pair(*current_key_, std::ranges::subrange{ current_, end_of_current_range_ });
         }

         constexpr void next() {
            current_ = end_of_current_range_;
            find_end_of_current_range();
         }
   
         constexpr bool equal(cursor const& rhs) const {
            return current_ == rhs.current_;
         }
         constexpr bool equal(basic_sentinel<V, Const> const& rhs) const {
            return current_ == rhs.end();
         }

         friend struct cursor<!Const>;
      };

   public:
      chunk_by_key_view() = default;
      chunk_by_key_view(V v, F f) : base_(std::move(v)), func_(std::move(f)) {}

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