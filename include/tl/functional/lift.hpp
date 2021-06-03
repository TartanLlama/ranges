#ifndef TL_RANGES_FUNCTIONAL_LIFT_HPP
#define TL_RANGES_FUNCTIONAL_LIFT_HPP

#include <functional>
#define TL_LIFT(func) \
   [](auto&&... args) \
      noexcept(noexcept(std::invoke(func, std::forward<decltype(args)>(args)...)))\
      ->decltype (std::invoke(func, std::forward<decltype(args)>(args)...)) \
   {\
      return std::invoke(func, std::forward<decltype(args)>(args)...);\
   }

#endif