# `<xstd/bits/ext/boost/bit_small_vector.hpp>`

## `basic_bit_small_vector`, `bit_small_vector`

```cpp
template<unsigned_integer Block, size_t N, class Alloc>
  requires see below
class basic_bit_small_vector {
public:
  // as basic_bit_vector, with
  using allocator_type       = see below;
  using block_container_type = boost::container::small_vector<Block, see below, Alloc>;

  constexpr basic_bit_small_vector() noexcept(noexcept(allocator_type()));
  constexpr explicit basic_bit_small_vector(const allocator_type& a) noexcept;
  constexpr basic_bit_small_vector(from_blocks_t, block_container_type blocks)
    noexcept(is_nothrow_move_constructible_v<block_container_type>);
  constexpr basic_bit_small_vector(from_blocks_t, block_container_type blocks, const allocator_type& a);
};
```

As `basic_bit_vector`, with:

- `bit_small_vector<N>` holds its bools in `size_t` blocks, inline up to `N` of them. The requires-clause admits
  `Block` as `basic_bit_vector`'s does, and `Alloc` defaults to Boost's own, as `small_vector`'s does.
- `allocator_type` is the small vector's, which wraps `Alloc`, and every allocator argument is of that type.
  Constructing from an allocator does not throw, inline blocks allocating nothing.
- `capacity()`, `reserve` and `shrink_to_fit` are the small vector's. There are no deduction guides.
- The Hash2 hook's model is `vector<bool>`.
