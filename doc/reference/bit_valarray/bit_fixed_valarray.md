# `<xstd/bits/bit_valarray/bit_fixed_valarray.hpp>`

## `basic_bit_fixed_valarray`, `bit_fixed_valarray`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_fixed_valarray {
public:
  // as basic_bit_valarray, with
  using block_container_type = array<Block, see below>;

  constexpr basic_bit_fixed_valarray() noexcept;
  template<size_t M>
      requires (M == N)
    constexpr explicit basic_bit_fixed_valarray(const bool (&a)[M]) noexcept;
  template<class Bits>
      requires see below
    constexpr basic_bit_fixed_valarray(from_blocks_t, const Bits& b) noexcept;
};
```

As `basic_bit_valarray`, at a size fixed by its type, with:

- `bit_fixed_valarray<N>` holds `N` bools in `size_t` blocks: `std::bitset<N>` read elementwise. The requires-clause
  admits `Block` as `basic_bit_array`'s does.
- There is no constructor from a count, no `resize`, no allocator and no adopting `from_blocks` constructor. The
  `initializer_list` constructor sets the leading positions and leaves the rest `false`; the default, built-in array
  and `from_blocks` constructors are `basic_bit_array`'s, so a default-initialized one is every position `false`.
  `size()` is a function.
- *Preconditions:* The pointer constructor's `n`, and the size a proxy selects for its constructor, is `N`.
- A `const` subscript gathers into `basic_bit_inplace_valarray<Block, N>`, where `Block` admits one, and throws
  `bad_alloc` where a gather selects more than `N` positions.
- Its `constexpr` members are usable in a constant expression, the subscripts by a `bit_slice` and by a mask
  included, and it is a structural type exactly where `N` fills its blocks.
