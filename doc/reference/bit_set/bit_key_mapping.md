# `bit_key_mapping`, `bit_range_mapping`, `bit_find_mapping`, `enum_traits`

```cpp
template<class Key, Key First, size_t N>
  requires see below
struct bit_range_mapping {
  static constexpr size_t size = N;
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};

template<class Key, const auto& Keys>
  requires see below
struct bit_find_mapping {
  static constexpr size_t size = ranges::size(Keys);
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};
```

A key's mapping is an order-preserving bijection onto the positions `0` to `size - 1`: `a < b` exactly where
`to_index(a) < to_index(b)`, and `from_index` inverts `to_index`.

- `bit_range_mapping<Key, First, N>` maps the `N` consecutive keys from `First`, of an integer or an enumeration,
  onto `0` to `N - 1`. `is_key(k)` holds for `First <= k < First + N`, compared as an unsigned distance from
  `First`, so no bound overflows `Key`. A key outside the range ranks at `N` or above.
- `bit_find_mapping<Key, Keys>` maps each key of the list `Keys` to its rank there, found by binary search, so a gap
  costs no position. *Mandates:* `Keys` ascends strictly. `is_key(k)` holds for a listed key, and any other ranks at
  `size`.
- `bit_key_mapping<Key>` is the mapping a set takes by default. Its primary template is declared and not defined,
  and it is specialized as `char_traits` is:
  - for every unsigned integer `Key`, the identity, with no `size`, leaving the universe open; `is_key(k)` holds
    where `k` survives the round trip through `size_t`;
  - for an enumeration whose `enum_traits<Key>::values` is declared, `bit_range_mapping` where the values run without
    a gap and `bit_find_mapping` over the list otherwise;
  - for an enumeration that declares no list and has an enumerator `count`, `bit_range_mapping` from `Key{}` to
    `count`, the sentinel itself no key. *Mandates:* `count` is not negative.
  - A program may specialize it for a key type of its own, such as a strong index.
- `enum_traits<Enum>` is empty. A program specializes it beside an enumeration with a `static constexpr`
  `std::array` named `values`, listing the enumerators ascending, each once.
