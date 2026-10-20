# `<xstd/bits/bit_functional/bit_greater.hpp>`, `<xstd/bits/bit_functional/bit_less.hpp>`, `<xstd/bits/bit_functional/bit_greater_equal.hpp>`, `<xstd/bits/bit_functional/bit_less_equal.hpp>`

## `bit_greater`, `bit_less`, `bit_greater_equal`, `bit_less_equal`

```cpp
struct bit_less {
  using is_transparent = void;

  template<class T, class U>
      requires requires (const T& x, const U& y) { xstd::ranges::bit_lexicographical_compare_three_way(x, y); }
    constexpr auto operator()(const T& x, const U& y) const noexcept -> bool;
};
```

As `greater<>`, `less<>`, `greater_equal<>` and `less_equal<>` in [[comparisons]](https://eel.is/c++draft/comparisons),
each of the same shape as `bit_less`, with:

- *Constraints:* `xstd::ranges::bit_lexicographical_compare_three_way(x, y)` is a valid expression, which admits the
  pairs `bit_equal_to` takes.
- *Returns:* `is_gt(c)`, `is_lt(c)`, `is_gteq(c)` and `is_lteq(c)` respectively, where `c` is
  `xstd::ranges::bit_lexicographical_compare_three_way(x, y)`.
- `bit_less()(bit_fixed_set<70>{1}, basic_bit_fixed_set<size_t, uint8_t, 70>{1, 65})` is `true`, and a
  `std::set<bit_fixed_set<100>, bit_less>` finds a key through `bit_set_view` and through `uint8_t` blocks.
- The pairs `bit_equal_to` turns away are turned away.
