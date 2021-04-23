#ifndef TL_RANGES_COMMON_HPP
#define TL_RANGES_COMMON_HPP

#include <iterator>
#include <ranges>

namespace tl {
   namespace detail {
      template <typename... V>
      constexpr auto common_iterator_category() {
         if constexpr ((std::ranges::random_access_range<V> && ...))
            return std::random_access_iterator_tag{};
         else if constexpr ((std::ranges::bidirectional_range<V> && ...))
            return std::bidirectional_iterator_tag{};
         else if constexpr ((std::ranges::forward_range<V> && ...))
            return std::forward_iterator_tag{};
         else if constexpr ((std::ranges::input_range<V> && ...))
            return std::input_iterator_tag{};
         else
            return std::output_iterator_tag{};
      }
   }

   template <class... V>
   using common_iterator_category = decltype(detail::common_iterator_category<V...>());

   template <class R>
   concept simple_view = std::ranges::view<R> && std::ranges::range<const R> &&
      std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
      std::same_as<std::ranges::sentinel_t<R>,
      std::ranges::sentinel_t<const R>>;

   struct begin_tag_t {};
   constexpr inline begin_tag_t begin_tag;
   struct end_tag_t {};
   constexpr inline end_tag_t end_tag;
}

#endif