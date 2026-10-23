# `<xstd/bits/bit_set/bit_inplace_set.hpp>`

## `basic_bit_inplace_set`, `bit_inplace_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
         see below Compare = less<Key>>
  requires see below
using basic_bit_inplace_set = basic_bit_set<Key, see below, N, KeyMapping, Compare>;

template<size_t N>
using bit_inplace_set = basic_bit_inplace_set<size_t, size_t, N>;

// what basic_bit_set has over a storage of constant capacity
// capacity
static constexpr integral_constant<size_t, N> max_size = {};
```

A [`basic_bit_set`](bit_set.md) capped at `N`, with:

- `bit_inplace_set<N>` holds keys below a capacity of `N` in `size_t` blocks, inline. The requires-clause admits
  `Block` as `basic_bit_set`'s does. *Mandates:* as `basic_bit_fixed_set`'s.
- The storage is `basic_bit_inplace_vector`'s, for `K = ceil(N / D)`. There is no allocator.
- An alias deduces nothing, so blocks adopted whole are deduced through `basic_bit_set`:
  `basic_bit_set(from_blocks, blocks)` over `K` inline blocks is `basic_bit_inplace_set<size_t, Block, K * D>`.
- `max_size` is a constant of the type, `N`; `size()` and `empty()` count the keys.
- A key past the width grows the width up to `N`. A key whose position is `N` or more throws `bad_alloc`, as
  `inplace_vector`'s growth does, where the mapping leaves the universe open, and `out_of_range` where the mapping
  closes it.
- `<<= n` keeps the keys that land below `N`, asks for no storage past it, and is `noexcept`.
- *`from_blocks` constructor, `replace`, `extract`:* As `basic_bit_set`'s, with the capacity as a precondition.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined, and is not a
  structural type.
