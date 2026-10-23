# `<xstd/bits/bit_concepts/bit_storage.hpp>`

## `bit_storage`

```cpp
template<class Blocks>
concept bit_storage = see below;
```

- `Blocks` models it where:
    - it models `bit_blocks` as a range of blocks, with a cv-unqualified block whose digits are a power of two and at
      least `unsigned char`'s;
    - it models `regular`, and its `const` subscript yields a reference through which no block is written, as
      [P2278R4](https://wg21.link/P2278R4)'s `range_const_reference_t` names it;
    - `resize(n, block)`, `push_back(block)`, `insert(end(), first, last)`, `clear()` and `max_size()` are valid on it.
- *Semantic requirements:* `resize` either succeeds or throws with the blocks as they were. The owners' no-effects
  guarantees rest on it.
- It is the constraint on the storage of `basic_bit_vector`, `basic_bit_valarray` and `basic_bit_set`, as
  [[flat.set]](https://eel.is/c++draft/flat.set) constrains its `KeyContainer`. `capacity`, `reserve` and
  `shrink_to_fit` are asked where the storage has them, and a `capacity()` usable as a constant caps the owner.
- `vector<uint64_t>`, `pmr::vector<uint32_t>`, `inplace_vector<uint64_t, 2>` where the library provides it,
  `boost::container::static_vector<uint16_t, 4>` and `boost::container::small_vector<uint64_t, 2>` model it.
- `vector<bool>` and `vector<int>` do not: bools are not blocks, and a signed block is not one either.
- `array<uint64_t, 2>` does not: its count is fixed, and `bit_array` and `bit_fixed_set` own such blocks.
- `deque<uint64_t>` does not, its blocks not contiguous, and `span<uint64_t>` does not, owning none: `bit_span` and
  `bit_set_view` view such blocks.
