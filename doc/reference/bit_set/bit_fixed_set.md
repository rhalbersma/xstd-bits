# `<xstd/bits/bit_set/bit_fixed_set.hpp>`

## `basic_bit_fixed_set`, `bit_fixed_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_fixed_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = array<Block, see below>;

  // construct: [set.cons]'s without the allocator forms, with
  constexpr explicit basic_bit_fixed_set(indeterminate_t) noexcept;
  template<class Bits>
      requires see below
    constexpr basic_bit_fixed_set(from_blocks_t, const Bits& b) noexcept;
  constexpr basic_bit_fixed_set(const Key& mask) noexcept;                     // see below
  constexpr operator Key() const noexcept;                                      // see below

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
};

constexpr auto operator~(const basic_bit_fixed_set& x) noexcept -> basic_bit_fixed_set;

template<unsigned_integer Block>
  basic_bit_fixed_set(from_blocks_t, Block) -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<Block>>;
template<unsigned_integer Block, size_t K>
  basic_bit_fixed_set(from_blocks_t, array<Block, K>)
    -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<array<Block, K>>>;
template<unsigned_integer Block, size_t K>
  basic_bit_fixed_set(from_blocks_t, const Block (&)[K])
    -> basic_bit_fixed_set<size_t, Block, bit_blocks_extent_v<array<Block, K>>>;
template<class Enum>
    requires is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
  basic_bit_fixed_set(initializer_list<Enum>)
    -> basic_bit_fixed_set<Enum, least_block_t<bit_key_mapping<Enum>::size>, bit_key_mapping<Enum>::size,
                           bit_key_mapping<Enum>>;
```

As `basic_bit_set`, with:

- `bit_fixed_set<N>` holds the keys `0` to `N - 1` in `size_t` blocks. The requires-clause admits `Block` as
  `basic_bit_array`'s does. *Mandates:* where `KeyMapping` models `sized_bit_index_mapping<KeyMapping, Key>`,
  `KeyMapping::size == N`.
- There is no allocator, and neither `allocator_type` nor `get_allocator`. The default constructor is `noexcept`.
- *`indeterminate_t` constructor:* *Effects:* Leaves the blocks default-initialized, writing nothing, at any width.
  *Remarks:* Until it is assigned, the only valid operations on the set are assignment and destruction. See
  [`indeterminate`](indeterminate.md).
- `max_size` is a constant of the type, `N`; `size()` and `empty()` are functions that count the keys.
- `insert`, `emplace` and the ranged forms throw `out_of_range` for a key whose position is `N` or more. An
  enumerator's `|=`, `^=`, `-=` and `&=`, the non-members over them, and `operator[]` take a position below `N` as
  their precondition, and are `noexcept`.
- `~x` complements every key below `N`. `<<= n` keeps the keys that land below `N`, and `>>= n` the keys at or
  above `n`; both are the native shifts, as an integer's are, with `n < N` as their precondition. `xstd::shl` and
  `xstd::shr` are the total forms, which empty the set where the count is `N` or more and shift the other way for a
  negative count. Every operator over two sets is `noexcept`.
- *`from_blocks` constructor:* As `basic_bit_array`'s: position `i` is a key where bit `i` of `b` is set. The guides
  deduce the width the blocks name, `Key` being `size_t`. `xstd::bit_convert` goes the other way, from the set to a
  block, an array of blocks or a `std::bitset`, position `i` staying position `i`.
- The `initializer_list` guide deduces, from a list of enumerators whose `bit_key_mapping` names a size, the type
  `bit_enum_set<Enum>` names.
- It is a structural type exactly where `N` fills its blocks, as `bit_align<bit_fixed_set<N>>` does at any `N`.
- **The mask members**, present only where `bit_mask_mapping<KeyMapping, Key>` holds, which reads every value of
  `Key` as the block of its one-bit keys:
    - The constructor from a mask is implicit and sets the keys whose bits the mask has. *Preconditions:* the mask has
      no bit at or above `N`.
    - The conversion to `Key` is implicit and returns the mask with this set's keys.
    - The `initializer_list` constructor is the union of its values, and `noexcept`.
    - `&`, `|`, `^` and `-` take a mask on either side, and `&=`, `|=`, `^=` and `-=` one on the right, each over
      the mask's exact type. `&` and `-` with the mask on the right are total, the mask's bits at or above `N` meeting
      nothing; the others have the constructor's precondition. `set == mask` is total, and false for a mask with a bit
      at or above `N`.
