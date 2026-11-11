# `<xstd/bits/bit_set/hash.hpp>`, `<xstd/bits/bit_set/container_hash.hpp>`

`hash.hpp` includes Boost.Hash2, and no umbrella includes it: a project that includes it links `Boost::hash2`.

## `hash`, `is_range`, `is_tuple_like`

- `hash<T>` for each owner and for `bit_set_view`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`. Equal sets hash equal, whatever their widths.
- `is_range` and `is_tuple_like` are `false_type` for each owner, and `is_range` for the view, so Boost.ContainerHash
  and Hash2 take the hook of `basic_bit_set`.
