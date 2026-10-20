# `<xstd/bits/bit_iterator.hpp>`

```cpp
namespace xstd::ranges {
  // [range.iter.ops], range iterator operations
  template<see below I>
    constexpr auto bit_advance(I& it, iter_difference_t<I> n) noexcept -> void;
  template<see below I>
    constexpr auto bit_advance(I& it, iter_difference_t<I> n, I bound) noexcept -> iter_difference_t<I>;

  template<see below I>
    constexpr auto bit_distance(I first, I last) noexcept -> iter_difference_t<I>;
  template<ranges::common_range R>
      requires see below
    constexpr auto bit_distance(R&& r) noexcept -> ranges::range_difference_t<R>;

  template<see below I>
    constexpr auto bit_next(I it, iter_difference_t<I> n) noexcept -> I;
  template<see below I>
    constexpr auto bit_next(I it, iter_difference_t<I> n, I bound) noexcept -> I;

  template<see below I>
      requires bidirectional_iterator<I>
    constexpr auto bit_prev(I it, iter_difference_t<I> n) noexcept -> I;
  template<see below I>
      requires bidirectional_iterator<I>
    constexpr auto bit_prev(I it, iter_difference_t<I> n, I bound) noexcept -> I;
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/bit_iterator/bit_advance.hpp>` | `bit_advance` |
| `<xstd/bits/bit_iterator/bit_distance.hpp>` | `bit_distance` |
| `<xstd/bits/bit_iterator/bit_next.hpp>` | `bit_next` |
| `<xstd/bits/bit_iterator/bit_prev.hpp>` | `bit_prev` |

The iterator operations over any iterator of ours, each in a header of its own under `<xstd/bits/bit_iterator/>`. A
sequence's iterator is random access and steps by an offset, so over it each is its `std::ranges` counterpart. A
set's iterator is bidirectional and steps a key at a time, so over it each skips whole blocks by their popcount and
selects within the block that holds its target: select is `*bit_next(s.begin(), n)`, and the rank of `k` is
`bit_distance(s.begin(), s.lower_bound(k))`.

An *iterator of ours* is an iterator of a sequence or a set of ours, `const` or not, a view's or a window's
included; `vector<bool>::iterator` and `set<size_t>::iterator` are turned away. Only the forms that take a count are
provided: there is no `bit_advance(it, bound)`, `bit_next(it)`, `bit_next(it, bound)` or `bit_prev(it)`.
