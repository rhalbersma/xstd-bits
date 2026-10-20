# `bit_digest`

```cpp
template<class T, hash_algorithm H, class Flavor>
  requires see below
class bit_digest {
public:
  using result_type = typename H::result_type;

  bit_digest() = default;
  constexpr explicit bit_digest(uint64_t seed);
  constexpr bit_digest(const unsigned char* p, size_t n);

  constexpr auto operator()(const T& v) const -> result_type;
};
```

As xstd-misc's `digest`, with:

- *Constraints:* `T` is a type `bit_hash_append` takes, by either form.
- `operator()`, *Returns:* `h.result()`, where `h` is a copy of the prototype after `bit_hash_append(h, Flavor(), v)`.
- There is no specialization for a string key.
- Equal bits digest equal across block types: under `fnv1a_64`, `basic_bit_fixed_set<size_t, Block, 200>{1, 64,
  199}` digests to `0xB72B'9E62'AF48'F732` at every block type, and under `xxhash_64` to `0xF8C0'7F80'B85C'9EA1`.
- `bit_hash<T, H, Flavor>()(x)` is `boost::hash2::get_integral_result<size_t>` of the state whose result
  `bit_digest<T, H, Flavor>()(x)` is.
