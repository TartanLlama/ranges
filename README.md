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

## Compiler Support

Tested on:
- Visual Studio 2019 version 16.9
- GCC 10.2

## Next up

- `cycle`
- `cartesian_product`

----------

[![CC0](http://i.creativecommons.org/p/zero/1.0/88x31.png)]("http://creativecommons.org/publicdomain/zero/1.0/")

To the extent possible under law, [Sy Brand](https://twitter.com/TartanLlama) has waived all copyright and related or neighboring rights to the `optional` library. This work is published from: United Kingdom.
