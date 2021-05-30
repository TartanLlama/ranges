# ranges

Implementations of ranges that didn't make C++20. Coded live [on Twitch](https://twitch.tv/tartanllama).

[![Documentation Status](https://readthedocs.org/projects/tl-docs/badge/?version=latest)](https://tl.tartanllama.xyz/en/latest/?badge=latest)
[![CMake](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml/badge.svg)](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml)

Find the documentation [here](https://tl.tartanllama.xyz/en/latest/api/ranges/index.html)

## Features

- Many range adaptors that are not available in C++20, such as `enumerate`, `cartesian_product`, and `chunk`.
- `tl::basic_iterator` which simplifies the creation of your own range adaptors.
- `tl::to` for converting a range into a container.
- Several functional programming utilities to simplify ranges implementation, like `compose` and `bind_back`.

## Examples

### Range Adaptors

```cpp
for (auto&& [index, element] : my_vector | tl::views::enumerate) {
  //use both index and element
}

template <class F, std::ranges::forward_range... Vs>
requires(std::regular_invocable<F, 
          std::ranges::range_reference_t<Vs>...>)
auto find_tuples_satisfying(F f, Vs&&... vs) {
  return tl::views::cartesian_product(std::forward<Vs>(vs)...) 
    | std::views::filter(tl::uncurry(f))
    | tl::to<std::vector>();
}
```

### `tl::basic_iterator`

```cpp
struct cursor {
  //set this to indicate your cursor is single-pass-only (default false)
  static constexpr bool single_pass; 
  
  //set this to indicate your cursor is contiguous (default false)
  static constexpr bool contiguous; 

  //implement this to get operator++() and operator++(int)
  void next(); 
  
  //implement this to get operator--() and operator--(int)
  void prev();
  
  //implement this to get operator+=, operator-=, and operator[]
  void advance(difference_type n);
  
  //implement this to get operator== and operator!=
  bool equal(cursor const&);
  
  //implement this to get operator-, operator<, operator>, operator<=, operator>=, and operator<=>
  difference_type distance_to(cursor const&);
  
  //make sure your cursor is default constructible
  cursor()=default;
};

auto my_range_view::begin() {
  return tl::basic_iterator{cursor{}};
}
```

### `tl::to`

```cpp
std::list<int> l;
std::map<int, int> m;

// copy a list to a vector of the same type
auto a = tl::to<std::vector<int>>(l);

// copy a list to a vector of the same type, deducing value_type
auto c = tl::to<std::vector>(l);

//Supports converting associative container to sequence containers
auto f = tl::to<vector>(m); //std::vector<std::pair<int,int>>

//Supports converting sequence containers to associative ones
auto g = tl::to<map>(f); //std::map<int,int>

//Pipe syntax
auto g = l | ranges::view::take(42) | tl::to<std::vector>();

// Nested ranges
std::list<std::forward_list<int>> lst = {{0, 1, 2, 3}, {4, 5, 6, 7}};
auto vec1 = tl::to<std::vector<std::vector<int>>>(lst);
auto vec2 = tl::to<std::vector<std::deque<double>>>(lst);
```

### Functional Utilities

```cpp
auto h = tl::compose(f,g); //h(args...) calls f(g(args...))
auto fp = tl::bind_back(f, n, m); //fp(args...) calls f(args..., n, m)

//Use tl::uncurry to adapt a function into one which takes its arguments from a pair/tuple
std::vector<int> a { 0, 42, 69 };
std::vector<int> b { 18, 64, 69 };
auto v = tl::views::zip(a,b) 
       | std::views::filter(tl::uncurry(std::ranges::equal_to{}))
       | tl::to<std::vector>();
//v == {(69,69)}       
       
//Use tl::pipeable to make a partially-applied range adaptor pipeable to one from the standard library
auto enumerate_reverse = tl::views::enumerate | tl::pipeable(std::views::reverse);
for (auto e : my_vec | enumerate_reverse) {
    //...
}
```

## Compiler Support

Tested on:
- Visual Studio 2019 version 16.9
- GCC 10.2

----------

[![CC0](http://i.creativecommons.org/p/zero/1.0/88x31.png)]("http://creativecommons.org/publicdomain/zero/1.0/")

To the extent possible under law, [Sy Brand](https://twitter.com/TartanLlama) has waived all copyright and related or neighboring rights to the `ranges` library. This work is published from: United Kingdom.
