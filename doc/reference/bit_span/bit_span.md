# `bit_span`

```cpp
template<bit_blocks Blocks, size_t N>
class bit_span {
public:
  // types as basic_bit_array's, with
  using element_type = see below;
  static constexpr size_t extent = N;
  using subspan_type = bit_subspan<Blocks, dynamic_extent, N>;

  template<class W>
      requires see below
    constexpr explicit bit_span(W&& blocks) noexcept;
  template<class Owner>
      requires see below
    constexpr bit_span(Owner& c) noexcept;

  // subviews
  constexpr auto first(size_type count) const noexcept -> subspan_type;
  constexpr auto last(size_type count) const noexcept -> subspan_type;
  constexpr auto subspan(size_type offset, size_type count = dynamic_extent) const noexcept -> subspan_type;
  template<size_t Count>
      requires see below
    constexpr auto first() const noexcept -> bit_subspan<Blocks, Count, N>;
  template<size_t Count>
      requires see below
    constexpr auto last() const noexcept -> bit_subspan<Blocks, Count, N>;
  template<size_t Offset, size_t Count = dynamic_extent>
      requires see below
    constexpr auto subspan() const noexcept -> bit_subspan<Blocks, see below, N>;

  // observers
  constexpr auto size() const noexcept -> size_type;
  constexpr auto empty() const noexcept -> bool;
  constexpr auto max_size() const noexcept -> size_type;
};

template<class Owner>
    requires see below
  bit_span(Owner&) -> bit_span<see below>;
template<class W>
    requires see below
  bit_span(W&&) -> bit_span<see below>;
```

The sequence reading of bits another object owns. As [[views.span]](https://eel.is/c++draft/views.span) and
[[span.overview]](https://eel.is/c++draft/span.overview), with:

- `Blocks` names the blocks the view refers to, as for `bit_set_view`, and `N` their width in bits, or
  `dynamic_extent`. Over an owner they are the owner's block container and width: a `bit_span` over a
  `basic_bit_array<Block, N>` is a `bit_span<array<Block, K>, N>`.
- `element_type` is `bool`, or `const bool` where the bits are `const`; `value_type` is `bool`; `extent` is `N`.
- The reference and the iterator are `basic_bit_array`'s. So `pointer` names `void`, and what `span` builds on an
  address is absent: `data()`, `size_bytes()`, `as_bytes` and `as_writable_bytes`, and the constructors from a
  contiguous iterator, an array or a contiguous range of `bool`, for which a view over blocks or over an owner stands
  in. There is no default constructor.
- The constructor from blocks is `explicit`, and takes what `bit_set_view`'s does, its guide deducing as that one's.
- The constructor from an owner is implicit, and takes an lvalue owner of the sequence or the valarray reading; a
  `const` owner gives a view of `const` bits. A view of mutable bits converts implicitly to a view of the same bits
  as `const`.
- `size()`, `empty()` and `max_size()` are functions, whatever the width; `max_size()` is `size()`. `at(n)` throws
  `out_of_range` where `n >= size()`.
- *Subviews:* As [[span.sub]](https://eel.is/c++draft/span.sub), returning `bit_subspan`; `dynamic_extent` as a
  count reaches the end. *Preconditions:* as [[span.sub]](https://eel.is/c++draft/span.sub)'s, checked by an
  `assert`. A compile-time count the static width says cannot fit fails the constraints.
- Like `span`, it does not compare, order or hash: `hash<bit_span<Blocks, N>>` is disabled and there is no Hash2
  hook. It has no bitwise operators and no shifts.
- It is trivially copyable, a `ranges::view` and a borrowed range. A write through a `const` view writes the bits,
  as through a `const span`. Its iterators rebuild the storage from the blocks they hold, so they outlive the view.
- It cannot grow what it views: it has no `resize`, `push_back`, `insert` or `erase`.
