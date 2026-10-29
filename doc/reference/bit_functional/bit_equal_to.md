# `<xstd/bits/bit_functional/bit_equal_to.hpp>`, `<xstd/bits/bit_functional/bit_not_equal_to.hpp>`

## `bit_equal_to`, `bit_not_equal_to`

```cpp
struct bit_equal_to {
  using is_transparent = void;

  template<class T, class U>
      requires requires (const T& x, const U& y) { xstd::ranges::bit_equal(x, y); }
    constexpr auto operator()(const T& x, const U& y) const noexcept -> bool;
};
```

As `equal_to<>` and `not_equal_to<>` in [[comparisons]](https://eel.is/c++draft/comparisons), with:

- *Constraints:* `xstd::ranges::bit_equal(x, y)` is a valid expression: `T` and `U` are of one reading and one width
  kind, as `bit_equal` admits them. The constraint is the call itself, so the two cannot drift.
- *Returns:* `xstd::ranges::bit_equal(x, y)`, and for `bit_not_equal_to`, `not xstd::ranges::bit_equal(x, y)`.
- `bit_fixed_set<70>{1, 65}` equals `basic_bit_fixed_set<size_t, uint8_t, 70>{1, 65}`, and `bit_vector` compares with
  `basic_bit_vector<vector<uint8_t>>`.
- `bit_array<8>` does not compare with `bit_vector`, nor `bit_fixed_set<8>` with `bit_set` or with `bit_array<8>`,
  and `vector<bool>` is turned away: a static width never compares with a run-time one, which `bit_hash` keeps apart.
