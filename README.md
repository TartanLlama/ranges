# ranges

Implementations of ranges that didn't make C++20. Coded live [on Twitch](https://twitch.tv/tartanllama).

[![Documentation Status](https://readthedocs.org/projects/tl-docs/badge/?version=latest)](https://tl.tartanllama.xyz/en/latest/?badge=latest)
[![CMake](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml/badge.svg)](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml)

## Types
### `tl::enumerate_view`/`tl::views::enumerate`

A view which lets you iterate over the items in a range and their indices at the same time.

```cpp
std::vector<int> v;
for (auto&& [index, item] : tl::views::enumerate(v)) {
  //...
}
```

### `tl::cycle_view`/`tl::views::cycle`

Turns a view into an infinitely cycling one.

```cpp
std::vector<int> v { 0, 1, 2 };
for (auto&& item : tl::views::cycle(v)) {
  std::cout << item << ' '; 
  //0 1 2 0 1 2 0 1 2 0 1 2 0 1 2 0...
}
```

### `tl::cartesian_product_view`/`tl::views::cartesian_product`

A view representing the [cartesian product](https://en.wikipedia.org/wiki/Cartesian_product) of any number of other views.

```cpp
std::vector<int> v { 0, 1, 2 };
for (auto&& [a,b,c] : tl::views::cartesian_product(v, v, v)) {
  std::cout << a << ' ' << b << ' ' << c << '\n';
  //0 0 0
  //0 0 1
  //0 0 2
  //0 1 0
  //0 1 1
  //...
}
```

### `tl::to`

Converts a range into a container.

```cpp
std::list<int> l;
std::map<int, int> m;

// copy a list to a vector of the same type
auto a = tl::to<std::vector<int>>(l);

//Specify an allocator
auto b = tl::to<std::vector<int, Alloc>>(l, alloc);

// copy a list to a vector of the same type, deducing value_type
auto c = tl::to<std::vector>(l);

// copy to a container of types ConvertibleTo
auto d = tl::to<std::vector<long>>(l);

//Supports converting associative container to sequence containers
auto f = tl::to<vector>(m); //std::vector<std::pair<int,int>>

//Supports converting sequence containers to associative ones
auto g = tl::to<map>(f); //std::map<int,int>

//Pipe syntax
auto g = l | ranges::view::take(42) | tl::to<std::vector>();

//Pipe syntax with allocator
auto h = l | ranges::view::take(42) | tl::to<std::vector>(alloc);

//The pipe syntax also support specifying the type and conversions
auto i = l | ranges::view::take(42) | tl::to<std::vector<long>>();

// Nested ranges
std::list<std::forward_list<int>> lst = {{0, 1, 2, 3}, {4, 5, 6, 7}};
auto vec1 = tl::to<std::vector<std::vector<int>>>(lst);
auto vec2 = tl::to<std::vector<std::deque<double>>>(lst); 
```

## Compiler Support

Tested on:
- Visual Studio 2019 version 16.9
- GCC 10.2

## Next up

- `chunk`

----------

[![CC0](http://i.creativecommons.org/p/zero/1.0/88x31.png)]("http://creativecommons.org/publicdomain/zero/1.0/")

To the extent possible under law, [Sy Brand](https://twitter.com/TartanLlama) has waived all copyright and related or neighboring rights to the `optional` library. This work is published from: United Kingdom.
