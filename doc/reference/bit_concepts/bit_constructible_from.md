# `bit_constructible_from`

```cpp
template<class To, class Blocks>
concept bit_constructible_from =
  see below and
  constructible_from<To, from_blocks_t, Blocks>;
```

- `see below` is that `remove_cvref_t<Blocks>` is bit storage a container can own: it models `bit_blocks` and
  `regular`, and it is an unsigned integer or a range whose subscript yields `ranges::range_reference_t<Blocks>`
  through a mutable object and `ranges::range_const_reference_t<Blocks>` through a `const` one, the alias of
  [P2278R4](https://wg21.link/P2278R4). A storage whose `const` subscript yields a writable reference is refused.
- A type models it if `To(from_blocks, b)` takes the blocks `b` as `To`'s bits, position `i` being bit `i` of `b`.
- An owner's own block container at a run-time width models it: `bit_vector` and `bit_set` from `vector<size_t>`,
  and `bit_inplace_set<100>` from its `block_container_type`.
- Blocks holding at least a fixed width model it: `bit_fixed_set<64>` from `array<uint64_t, 1>`, and
  `bit_array<20>` from `array<uint8_t, 3>`.
- Another block type does not, `bit_vector` from `vector<uint8_t>`, nor does a field too narrow,
  `bit_fixed_set<256>` from `array<uint64_t, 3>`, nor a container an inplace owner does not hold,
  `bit_inplace_set<100>` from `array<uint64_t, 2>` and `bit_inplace_vector<100>` from `vector<size_t>`.
- What has bit storage without being it does not: `bit_fixed_set<64>` from `bitset<64>`, and `bit_vector` from
  `bit_set`. Each converts through `bit_convert` instead.
- A view owns nothing to take blocks into: `bit_set_view<uint64_t>` from `uint64_t` does not model it.
- A built-in array is read and never owned: `bit_fixed_set<64>` is constructible from `from_blocks` and a
  `const uint32_t(&)[2]`, and `bit_constructible_from<bit_fixed_set<64>, uint64_t[1]>` is `false`.
