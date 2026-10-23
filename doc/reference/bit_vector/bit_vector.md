# `<xstd/bits/bit_vector/bit_vector.hpp>`

## `basic_bit_vector`, `bit_vector`

```cpp
template<bit_storage Blocks = vector<size_t>, size_t N = dynamic_extent>
  requires see below
class basic_bit_vector {
public:
  // types as basic_bit_array's, with
  using allocator_type       = typename Blocks::allocator_type;  // where Blocks has one
  using block_container_type = Blocks;

  // construct: [vector.bool.pspc]'s, each allocator form where Blocks has an allocator, with
  constexpr basic_bit_vector(from_blocks_t, block_container_type blocks)
    noexcept(is_nothrow_move_constructible_v<block_container_type>);
  constexpr basic_bit_vector(from_blocks_t, block_container_type blocks, const allocator_type& a);

  // block exchange
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y))) -> void;
};

using bit_vector = basic_bit_vector<>;

template<input_iterator InputIterator, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(InputIterator, InputIterator, Allocator = Allocator())
    -> basic_bit_vector<vector<typename Allocator::value_type, Allocator>>;
template<ranges::input_range R, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(from_range_t, R&&, Allocator = Allocator())
    -> basic_bit_vector<vector<typename Allocator::value_type, Allocator>>;
template<class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(initializer_list<bool>, Allocator = Allocator())
    -> basic_bit_vector<vector<typename Allocator::value_type, Allocator>>;
template<bit_storage Blocks>
  basic_bit_vector(from_blocks_t, Blocks) -> basic_bit_vector<Blocks, see below>;
template<bit_storage Blocks, class Allocator>
  basic_bit_vector(from_blocks_t, Blocks, Allocator) -> basic_bit_vector<Blocks, see below>;
```

As [[vector.bool]](https://eel.is/c++draft/vector.bool), over the blocks of `Blocks` as
[[flat.set]](https://eel.is/c++draft/flat.set) is over its `KeyContainer`, with:

- `bit_vector` holds its bits in a `vector<size_t>`. `Blocks` is any [`bit_storage`](../bit_concepts/bit_storage.md):
  `vector` over another block or allocator, `pmr::vector` over an arena, `inplace_vector`, Boost's `static_vector` and
  `small_vector`, or a container of the user's own. `bit_inplace_vector` names the inline one, and the aliases under
  [`ext/`](../ext_boost/index.md) name other libraries' containers.
- `N` caps the width in bits. It defaults to `dynamic_extent`: no cap over a storage of no constant capacity, and its
  whole blocks over one with a constant capacity, such as `inplace_vector<Block, K>`. A cap is admitted only over a
  constant capacity, and only in its last block, so `N` is at most that capacity and more than the blocks before it.
- `capacity`, `reserve`, `shrink_to_fit` and `max_size` are the storage's, in bits. Under a constant capacity they are
  `basic_bit_inplace_vector`'s, and so are the modifiers that refuse growth past it.
- The reference and the iterator are `basic_bit_array`'s. [[vector.bool]](https://eel.is/c++draft/vector.bool)'s
  extras are left out: `flip()` on the container and on the proxy, and the static `swap(reference, reference)`.
  `const_reference` is a read-only proxy rather than `bool`.
- `push_back` returns `void`, as [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) has it, where the
  storage has no constant capacity. Growth goes through the blocks' own `resize`, so the storage's growth policy picks
  the capacity: `n` appends at the end make O(log n) allocations over `vector`.
- `max_size()` answers the widest whole number of blocks' bits the blocks allow and that `difference_type` can
  count. A size past it throws `length_error` before anything is allocated.
- An insertion whose count is known shifts the tail in place; one from a single-pass range with no size, or from a
  window of the sequence itself, rebuilds aside and moves in, with the strong guarantee. A range that throws while it
  is read leaves the sequence as it was.
- *Allocators:* Where `Blocks` has an `allocator_type` that allocates, each constructor of
  [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) has its allocator-extended form, and
  `get_allocator()` returns the blocks' allocator, a construction argument and never part of the value. Where it has
  none, there is no allocator argument.
- The guides deduce a `vector` over the block of the allocator where one is given, and `vector<size_t>` where none
  is; the element is `bool`. The `from_blocks` guides deduce the storage, capped at its constant capacity where it has
  one.
- *`from_blocks` constructors:* *Effects:* Move `blocks` in, the second under `a`. Every bit is a position, so
  `size()` is `blocks.size() * D`.
- *`replace`, `extract`:* As for `basic_bit_inplace_vector`, without a capacity to respect where the storage has none.
- `bit_rebind` and `bit_resize` go through the storage's [`bit_storage_rebind`](../bit_type_traits/bit_storage_rebind.md):
  another block, and under a count of blocks in the type, the count that holds the width.
