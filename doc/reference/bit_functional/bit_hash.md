# `bit_hash`

```cpp
template<class T, hash_algorithm H, class Flavor>
  requires see below
class bit_hash {
public:
  using is_avalanching = bool_constant<hash_algorithm_is_avalanching_v<H>>;
  using is_transparent = void;

  bit_hash() = default;
  constexpr explicit bit_hash(uint64_t seed);
  constexpr bit_hash(const unsigned char* p, size_t n);

  template<class U>
      requires see below
    constexpr auto operator()(const U& v) const -> size_t;
};
```

As xstd-misc's `hash`, with:

- *Constraints:* `T` is a type `bit_hash_append` takes, by either form.
- One algorithm, `H`, defaulted to `boost::hash2::xxhash_64` on every target, where `size_t` takes Hash2's fold of
  its result. There is no `H32` and no `hash_type`; `is_avalanching` is read from `H`, so it is `true` under
  xxHash and SipHash and `false` under FNV-1a.
- `is_transparent` is declared: equal bits hash equal whatever blocks or storage hold them, so a lookup may cross
  containers of one reading.
- There is no specialization for a string key.
- `operator()`:
  - *Constraints:* `U` is `T`, or `xstd::ranges::bit_equal(x, y)` is valid for a `const T& x` and a `const U& y`;
    and `U` is a type `bit_hash_append` takes. So `bit_hash<bit_array<64>>` does not take a `bit_span`.
  - *Effects:* Copies the prototype to `h` and calls `bit_hash_append(h, Flavor(), v)`. The prototype is unchanged.
  - *Returns:* `boost::hash2::get_integral_result<size_t>(h)`.
  - `v` is appended as the `U` it is, not converted to `T`.
- It hashes the bits, not the value as its standard model has it: no key and no count, so `bit_hash<T, H>()(x)` and
  xstd-misc's `hash<T, H>()(x)` differ.
- Equal bits hash equal: a `bit_fixed_set<100>` and its `bit_set_view`, the same set over `uint8_t` blocks, and a
  `bit_vector`, a `basic_bit_small_vector<uint8_t, 9>` and a `basic_bit_inplace_vector<uint64_t, 256>` of one value.
- Where `hash<T>` is enabled for an owner or a set view `T`, it is this hash at its defaults: `bit_hash<T>()(x)` is
  `hash<T>()(x)`.
