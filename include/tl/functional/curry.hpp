#ifndef TL_RANGES_FUNCTIONAL_CURRY_HPP
#define TL_RANGES_FUNCTIONAL_CURRY_HPP

#include <utility>

//tl::curry adapts a function which takes some number of arguments into
//one which takes its arguments from a tuple.

namespace tl {
   template <class F>
   auto curry(F f) {
      return[f_ = std::move(f)](auto&& tuple) {
         return std::apply(f_, std::forward<decltype(tuple)>(tuple));
      };
   }
}

#endif