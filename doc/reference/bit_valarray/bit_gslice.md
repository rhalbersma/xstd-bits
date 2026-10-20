# `<xstd/bits/bit_valarray/bit_gslice.hpp>`

## `bit_gslice`

```cpp
class bit_gslice {
public:
  bit_gslice();
  bit_gslice(size_t start, const valarray<size_t>& lengths, const valarray<size_t>& strides);
  explicit bit_gslice(const gslice& g);

  auto start() const noexcept -> size_t;
  auto size() const -> valarray<size_t>;
  auto stride() const -> valarray<size_t>;
};
```

- As [[class.gslice]](https://eel.is/c++draft/class.gslice), with its positions worked out once, in its constructor,
  and kept for every subscript that reuses it: as one bit per position where they ascend, and as the positions
  themselves where they repeat or turn back.
- It selects what the `gslice` of the same numbers selects, over every owner and block.
- Copies share the positions, and a proxy holds a copy, so a `bit_gslice_array` outlives the `bit_gslice` it was
  made from.
