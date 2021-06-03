#ifndef TL_UTILITY_META_HPP
#define TL_UTILITY_META_HPP

#include <type_traits>

namespace tl {
   template <class Tuple>
   constexpr inline std::size_t tuple_size = std::tuple_size_v<std::remove_cvref_t<Tuple>>;

   template <std::size_t N>
   using index_constant = std::integral_constant<std::size_t, N>;
}

#endif