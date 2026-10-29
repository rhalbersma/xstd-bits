# `<xstd/bits/bit_set/bit_set_view.hpp>`

## `bit_set_view`

```cpp
template<bit_blocks Blocks, size_t N>
class bit_set_view {
public:
  // types, lookups, iterators and set algebra as basic_bit_set's, with
  template<class W>
      requires see below
    constexpr explicit bit_set_view(W&& blocks) noexcept;
  template<class Owner>
      requires see below
    constexpr bit_set_view(Owner& c) noexcept;
};

template<class Owner>
    requires see below
  bit_set_view(Owner&) -> bit_set_view<see below>;
template<class W>
    requires see below
  bit_set_view(W&&) -> bit_set_view<see below>;

namespace std::ranges {
  template<class Blocks, size_t N>
    constexpr bool enable_view<xstd::bit_set_view<Blocks, N>> = true;
  template<class Blocks, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_set_view<Blocks, N>> = true;
}
```

The set reading of bits another object owns, as `basic_bit_set` over `size_t` keys, with:

- `Blocks` names the blocks the view refers to: a block, a `span` of blocks, or the block container of the owner it
  views, `const` where they are; `N` is their width in bits, or `dynamic_extent`.
- The constructor from blocks is `explicit`. It takes an lvalue block, or a range of blocks that is an lvalue or a
  borrowed range, and its guide deduces a block as itself and a range as the `span` that lends it, so
  `bit_set_view(board)` over a `uint64_t` is a `bit_set_view<uint64_t>`.
- The constructor from an owner is implicit, and takes an lvalue owner of the set reading; a `const` owner gives a
  view of `const` bits. A view of mutable bits converts implicitly to a view of the same bits as `const`.
- It has no default constructor, owns nothing, and is a `ranges::view` and a borrowed range.
- Its modifiers write the bits it refers to, and exist only where those bits are not `const`. `insert` over a
  run-time width grows the owner's storage. The compound operators take any set over the same storage and keys,
  owner or view; the non-member operators, which copy, are the owner's alone.
- `==` and `<=>` compare it with whatever converts to it, its owner and lent blocks included, as `string_view`
  compares.
- It hashes what it presents: `hash<bit_set_view<Blocks, N>>` and the Hash2 hook answer as for its owner.
