# `basic_bit_inplace_set`, `bit_inplace_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_inplace_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = see below;

  // construct: [set.cons]'s without the allocator forms, with
  constexpr basic_bit_inplace_set(from_blocks_t, block_container_type blocks) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
};

template<unsigned_integer Block, size_t K>
  basic_bit_inplace_set(from_blocks_t, see below) -> basic_bit_inplace_set<size_t, Block, bit_blocks_extent_v<Block> * K>;
```

As `basic_bit_set`, with:

- `bit_inplace_set<N>` holds keys below a capacity of `N` in `size_t` blocks, inline. The requires-clause admits
  `Block` as `basic_bit_set`'s does. *Mandates:* as `basic_bit_fixed_set`'s.
- `block_container_type` is `basic_bit_inplace_vector`'s, for `K = ceil(N / D)`. There is no allocator.
- `max_size` is a constant of the type, `N`; `size()` and `empty()` count the keys.
- A key past the width grows the width up to `N`. A key whose position is `N` or more throws `bad_alloc`, as
  `inplace_vector`'s growth does, where the mapping leaves the universe open, and `out_of_range` where the mapping
  closes it.
- `<<= n` keeps the keys that land below `N`, asks for no storage past it, and is `noexcept`.
- *`from_blocks` constructor, `replace`, `extract`:* As `basic_bit_set`'s, with the capacity as a precondition. The
  guide deduces the capacity the blocks hold.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined, and is not a
  structural type.
