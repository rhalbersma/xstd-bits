# `<xstd/bits/bit_span/bit_subspan.hpp>`

## `bit_subspan`

```cpp
template<bit_blocks Blocks, size_t Extent, size_t N>
class bit_subspan {
public:
  // as bit_span, with
  static constexpr size_t extent = Extent;

  template<class OtherBlocks, size_t OtherExtent>
      requires see below
    constexpr explicit(Extent != dynamic_extent) bit_subspan(const bit_subspan<OtherBlocks, OtherExtent, N>& other)
      noexcept;
};
```

A window: the part of a `bit_span` or of another window that `first`, `last` and `subspan` return. As `bit_span`,
with:

- It stores what `span` stores, the start split over a pointer and a position because bits are not addressable,
  and the count where `Extent` is `dynamic_extent`: three words, or two at a static extent.
- It is made by `first`, `last` and `subspan`, which on a window return a window over the same `Blocks` and `N`.
  Between extents the rule is `span`'s: a static window converts to a dynamic one implicitly, and a dynamic one to a
  static one explicitly. *Preconditions:* the sizes agree. A window of mutable bits converts to one of `const` bits.
- At a static extent, `size`, `empty` and `max_size` are constants of the type, as for `basic_bit_array`; at the
  dynamic extent they are functions, and `max_size()` is `size()`.
- A write to a window masks its end blocks, so the bits around it, which share those blocks, are untouched.
