# `hash`, `is_range`, `is_tuple_like`

- `hash<basic_bit_array<Block, N>>`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`, the hash of the bits. `array<bool, N>` has no hash; this one
  does, and equal values hash equal.
- A Boost.Hash2 `hash_append` hook appends exactly the message Hash2 writes for the `array<bool, N>` of the same
  value, so `xstd::hash<T, H, Flavor>()(x) == xstd::hash<array<bool, N>, H, Flavor>()(m)` for that `m`.
- `is_range` and `is_tuple_like` are `false_type`, so Boost.ContainerHash and Hash2 take the hook rather than
  hashing the array as a range or as a tuple.
