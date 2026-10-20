# `basic_bit_vector`, `bit_vector`

```cpp
template<unsigned_integer Block, class Allocator>
  requires see below
class basic_bit_vector {
public:
  // types as basic_bit_array's, with
  using allocator_type       = Allocator;
  using block_container_type = vector<Block, Allocator>;

  // construct: [vector.bool.pspc]'s, with
  constexpr basic_bit_vector(from_blocks_t, vector<Block, Allocator> blocks) noexcept;
  constexpr basic_bit_vector(from_blocks_t, vector<Block, Allocator> blocks, const Allocator& a);

  // block exchange
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  friend constexpr auto swap(basic_bit_vector& x, basic_bit_vector& y) noexcept(noexcept(x.swap(y))) -> void;
};

template<input_iterator InputIterator, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(InputIterator, InputIterator, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<ranges::input_range R, class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(from_range_t, R&&, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<class Allocator = allocator<size_t>>
    requires simple_allocator<Allocator>
  basic_bit_vector(initializer_list<bool>, Allocator = Allocator())
    -> basic_bit_vector<typename Allocator::value_type, Allocator>;
template<unsigned_integer Block, class Allocator>
  basic_bit_vector(from_blocks_t, vector<Block, Allocator>) -> basic_bit_vector<Block, Allocator>;
template<unsigned_integer Block, class Allocator>
  basic_bit_vector(from_blocks_t, vector<Block, Allocator>, Allocator) -> basic_bit_vector<Block, Allocator>;
```

As [[vector.bool]](https://eel.is/c++draft/vector.bool), with:

- `bit_vector` holds its bits in `size_t` blocks, and `basic_bit_vector<Block, Allocator>` in `Block`s, in a
  `vector<Block, Allocator>`. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two
  and at least `unsigned char`'s.
- The reference and the iterator are `basic_bit_array`'s. [[vector.bool]](https://eel.is/c++draft/vector.bool)'s
  extras are left out: `flip()` on the container and on the proxy, and the static `swap(reference, reference)`.
  `const_reference` is a read-only proxy rather than `bool`.
- `push_back` returns `void`, as [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) has it. Growth goes
  through the blocks' own `resize`, so `vector`'s growth policy picks the capacity: `n` appends at the end make
  O(log n) allocations.
- `max_size()` answers the widest whole number of blocks' bits the blocks allow and that `difference_type` can
  count. A size past it throws `length_error` before anything is allocated.
- An insertion whose count is known shifts the tail in place; one from a single-pass range with no size, or from a
  window of the sequence itself, rebuilds aside and moves in, with the strong guarantee. A range that throws while it
  is read leaves the sequence as it was.
- The guides deduce the block from the allocator where one is given, and `size_t` where none is; the element is
  `bool`. The `from_blocks` guides deduce the block and the allocator both.
- *`from_blocks` constructors:* *Effects:* Move `blocks` in, the second under `a`. Every bit is a position, so
  `size()` is `blocks.size() * D`.
- *`replace`, `extract`:* As for `basic_bit_inplace_vector`, without a capacity to respect.
- `get_allocator()` returns the blocks' allocator, a construction argument and never part of the value.
