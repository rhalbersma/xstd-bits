# `<xstd/bits/bit_concepts/bit_convertible_to.hpp>`

## `bit_convertible_to`

```cpp
template<class From, class To>
concept bit_convertible_to = requires (From&& from) {
  { bit_convert<To>(std::forward<From>(from)) } -> same_as<remove_cv_t<To>>;
};
```

- It holds exactly where `bit_convert<To>(from)` is valid: the positions map, though a source of run-time width may
  still throw. The source comes first, as in `convertible_to`, so `bit_convertible_to<To> auto` constrains a source.
- A fixed width converts into one at least as wide: `bit_fixed_set<64>` to `uint64_t`, `bitset<20>` to
  `bit_array<20>`, `array<uint8_t, 3>` to `bit_fixed_set<24>`, `bit_array<64>` to `bitset<64>`,
  `bit_set_view<uint64_t>` to `uint64_t`, `bit_array<5>` to `uint8_t`, `bit_fixed_set<64>` to `bit_fixed_set<65>`,
  and `uint64_t[4]` to `bit_array<256>` and `bitset<257>`.
- A run-time width converts into a fixed one: `bit_set` to `uint64_t`, `bit_vector` to `bitset<70>` and to
  `bit_array<64>`, `bit_inplace_set<64>` to `bit_fixed_set<10>`, and a `bit_subspan` window to `bit_array<100>`.
- Anything converts into a run-time width: `bit_span<array<uint8_t, 3>>`, `bit_array<64>` and
  `bit_span<vector<uint64_t>>` to `bit_vector`, `bitset<70>` to `bit_small_vector<64>`, `bit_vector` to
  `bit_inplace_set<64>`, `uint64_t` to `bit_set`, and a window to `bit_vector`.
- A reference or a `const` source is the conversion of the type it names: `const bit_set&` and `bit_set&&` to
  `bit_vector`. A cv-qualified target converts as the bare one: `bit_array<64>` to `const uint64_t` and to
  `volatile bitset<64>`, and `bit_vector` to `const bit_set`.
- A volatile scalar converts by one read: `volatile uint32_t&` to `bit_array<32>`, and `const volatile uint64_t&` to
  `bit_set`.
- Narrowing one fixed width into another would truncate, and is no conversion: `bit_array<9>` to `uint8_t` or
  `const uint8_t`, `bit_fixed_set<65>` to `uint64_t`, `uint32_t` to `bit_array<20>`, `uint64_t` to `bitset<63>`,
  `bitset<65>` to `bit_array<64>`, and `uint64_t[4]` to `bit_array<255>` are turned away.
- A view is read from and never written into: `uint64_t` to `bit_set_view<uint64_t>`, and `bit_vector` to
  `bit_span<array<uint8_t, 3>>` or to a window, are turned away. So is a built-in array as a target, which no
  function returns: `bit_array<256>`, `array<uint64_t, 4>` and `bit_vector` to `uint64_t[4]`.
- What has no bit storage converts to nothing: `vector<bool>` to `bit_vector`, and `bit_vector` to
  `vector<uint64_t>`.
- Volatile storage read by more than one access is no source: `volatile bit_array<64>&`, `volatile bitset<64>&`,
  `volatile uint32_t(&)[2]` and `volatile array<uint32_t, 2>&` are turned away.
- With `<xstd/bits/ext/boost/dynamic_bitset.hpp>` included, `boost::dynamic_bitset<>` converts to `bit_vector` and
  `uint64_t`, `bit_set` and `bitset<70>` to `boost::dynamic_bitset<>`, and one `boost::dynamic_bitset` into
  another does not.
