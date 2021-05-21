#ifndef TL_UTILITY_TUPLE_TRANSFORM_HPP
#define TL_UTILITY_TUPLE_TRANSFORM_HPP

#include <type_traits>
#include <tuple>
#include <utility>

namespace tl {
   //If the size of Ts is 2, returns pair<Ts...>, otherwise returns tuple<Ts...>
   namespace detail {
      template<class... Ts>
      struct tuple_or_pair_impl : std::type_identity<std::tuple<Ts...>> {};
      template<class Fst, class Snd>
      struct tuple_or_pair_impl<Fst, Snd> : std::type_identity<std::pair<Fst, Snd>> {};
   }
   template<class... Ts>
   using tuple_or_pair = detail::tuple_or_pair_impl<Ts...>::type;

   //Call f on every element of the tuple, returning a new one
   template<class F, class Tuple>
   constexpr auto tuple_transform(F&& f, Tuple&& tuple)
   {
      return std::apply([&]<class... Ts>(Ts&&... elements) {
         return tuple_or_pair<std::invoke_result_t<F&, Ts>...>(
            std::invoke(f, std::forward<Ts>(elements))...
            );
      }, std::forward<Tuple>(tuple));
   }
}
#endif