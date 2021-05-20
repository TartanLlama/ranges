# ranges

Implementations of ranges that didn't make C++20. Coded live [on Twitch](https://twitch.tv/tartanllama).

[![Documentation Status](https://readthedocs.org/projects/tl-docs/badge/?version=latest)](https://tl.tartanllama.xyz/en/latest/?badge=latest)
[![CMake](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml/badge.svg)](https://github.com/TartanLlama/ranges/actions/workflows/cmake.yaml)

## Types


### `tl::cartesian_product_view`

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

### `tl::chunk_view`

A view which chunks a range into subranges of the given size.

```cpp
std::vector<int> vec { 0,1,2,3,4,5,6,7,8,9,10 };

for (auto&& group : vec | tl::views::chunk(4)) {
    std::cout << "Group: ";
    for (auto&& e : group) {
      std::cout << e << ' ';
    }
    std::cout << '\n';
}
//Group: 0 1 2 3
//Group: 4 5 6 7
//Group: 8 9 10
```

### `tl::chunk_by_view`

A view which chunks a range into subranges where the consecutive elements satisfy a binary predicate.

```cpp
struct cat {
   std::string name;
   int age;
};

std::vector<cat> cats {
  {"potato", 12},
  {"bard", 12},
  {"soft boy", 9},
  {"vincent van catto", 12},
  {"oatmeal", 12},
};

for (auto&& group : cats | tl::views::chunk_by([](auto&& left, auto&& right) { return left.age == right.age; })) {
  //group 1 == { potato, bard }
  //group 2 == { soft boy }
  //group 3 == { vincent van catto, oatmeal }
}
```

### `tl::chunk_by_key_view`

A view which chunks a range into subranges where the consecutive elements share the same key given by a projection function.

```cpp
struct cat {
   std::string name;
   int age;
};

std::vector<cat> cats {
  {"potato", 12},
  {"bard", 12},
  {"soft boy", 9},
  {"vincent van catto", 12},
  {"oatmeal", 12},
};

for (auto&& group : cats | tl::views::chunk_by_key([](auto&& c) { return c.age; })) {
  //group 1 == { potato, bard }
  //group 2 == { soft boy }
  //group 3 == { vincent van catto, oatmeal }
}
```

### `tl::cycle_view`

Turns a view into an infinitely cycling one.

```cpp
std::vector<int> v { 0, 1, 2 };
for (auto&& item : tl::views::cycle(v)) {
  std::cout << item << ' '; 
  //0 1 2 0 1 2 0 1 2 0 1 2 0 1 2 0...
}
```

### `tl::enumerate_view`

A view which lets you iterate over the items in a range and their indices at the same time.

```cpp
std::vector<int> v;
for (auto&& [index, item] : tl::views::enumerate(v)) {
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

### `tl::transform_maybe_view`

Takes a function from `range_reference_t<Range>` to `optional<U>`. Transforms the given range by calling the function on each element, but only returning the value of engaged optionals.

```cpp
std::vector<int> vec{ 0,1,2,3,4 };
auto f = [](auto i) { return (i % 2 == 0) ? std::optional(i / 2) : std::nullopt; };

for (auto&& e : vec | tl::views::transform_maybe(f)) {
   std::cout << e << ' ';
   //0 1 2
}
```

### `tl::stride_view`

A view which walks over the given range with the given stride size.

```cpp
std::vector<int> v{ 0, 1, 2, 3, 4, 5, 6 };

for (auto&& e : v | tl::views::stride(3)) {
  std::cout << e << ' ';
  //0 3 6
}
```

## Compiler Support

Tested on:
- Visual Studio 2019 version 16.9
- GCC 10.2

----------

[![CC0](http://i.creativecommons.org/p/zero/1.0/88x31.png)]("http://creativecommons.org/publicdomain/zero/1.0/")

To the extent possible under law, [Sy Brand](https://twitter.com/TartanLlama) has waived all copyright and related or neighboring rights to the `optional` library. This work is published from: United Kingdom.
