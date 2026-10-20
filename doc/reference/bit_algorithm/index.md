# `<xstd/bits/bit_algorithm.hpp>`

```cpp
namespace xstd::ranges {
  // [alg.nonmodifying], non-modifying sequence operations
  // [alg.all.of], all of
  template<see below R>
    constexpr auto bit_all_of(const R& r) noexcept -> bool;

  // [alg.any.of], any of
  template<see below R>
    constexpr auto bit_any_of(const R& r) noexcept -> bool;

  // [alg.none.of], none of
  template<see below R>
    constexpr auto bit_none_of(const R& r) noexcept -> bool;

  // [alg.foreach], for each
  template<class R, class F>
      requires see below
    constexpr auto bit_for_each(R&& r, F f) -> ranges::for_each_result<ranges::borrowed_iterator_t<R>, F>;
  template<class R, class F>
      requires see below
    constexpr auto bit_for_each(R&& r, F f) -> ranges::for_each_result<ranges::borrowed_iterator_t<R>, F>;

  // [alg.find], find
  template<see below R>
    constexpr auto bit_find(R&& r, bool value) noexcept -> ranges::borrowed_iterator_t<R>;

  // [alg.find.last], find last
  template<see below R>
    constexpr auto bit_find_last(R&& r, bool value) noexcept -> ranges::borrowed_subrange_t<R>;

  // [alg.count], count
  template<see below R>
    constexpr auto bit_count(const R& r, bool value) noexcept -> ranges::range_difference_t<const R>;

  // [mismatch], mismatch
  template<class R1, class R2>
      requires see below
    constexpr auto bit_mismatch(R1&& r1, R2&& r2) noexcept
      -> ranges::mismatch_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>>;

  // [alg.equal], equal
  template<class R1, class R2>
      requires see below
    constexpr auto bit_equal(const R1& r1, const R2& r2) noexcept -> bool;

  // [alg.modifying.operations], mutating sequence operations
  // [alg.copy], copy
  template<see below R, see below O>
    constexpr auto bit_copy(R&& r, O out) noexcept -> ranges::copy_result<ranges::borrowed_iterator_t<R>, O>;
  template<see below R, see below O>
    constexpr auto bit_copy_backward(R&& r, O out_last) noexcept
      -> ranges::copy_backward_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.move], move
  template<see below R, see below O>
    constexpr auto bit_move(R&& r, O out) noexcept -> ranges::move_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.swap], swap
  template<see below R1, see below R2>
    constexpr auto bit_swap_ranges(R1&& r1, R2&& r2) noexcept
      -> ranges::swap_ranges_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>>;

  // [alg.transform], transform
  template<see below R, see below O, see below F>
    constexpr auto bit_transform(R&& r, O out, F f)
      -> ranges::unary_transform_result<ranges::borrowed_iterator_t<R>, O>;
  template<see below R1, see below R2, see below O, see below F>
    constexpr auto bit_transform(R1&& r1, R2&& r2, O out, F f)
      -> ranges::binary_transform_result<ranges::borrowed_iterator_t<R1>, ranges::borrowed_iterator_t<R2>, O>;

  // [alg.fill], fill
  template<see below R>
    constexpr auto bit_fill(R&& r, bool value) noexcept -> ranges::borrowed_iterator_t<R>;

  // [alg.reverse], reverse
  template<see below R>
      requires see below
    constexpr auto bit_reverse(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;
  template<see below R, see below O>
    constexpr auto bit_reverse_copy(R&& r, O out) noexcept
      -> ranges::reverse_copy_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.rotate], rotate
  template<see below R>
      requires see below
    constexpr auto bit_rotate(R&& r, ranges::iterator_t<R> middle) noexcept -> ranges::borrowed_subrange_t<R>;
  template<see below R, see below O>
    constexpr auto bit_rotate_copy(R&& r, ranges::iterator_t<R> middle, O out) noexcept
      -> ranges::rotate_copy_result<ranges::borrowed_iterator_t<R>, O>;

  // [alg.shift], shift
  template<see below R>
      requires see below
    constexpr auto bit_shift_left(R&& r, ranges::range_difference_t<R> n) noexcept
      -> ranges::borrowed_subrange_t<R>;
  template<see below R>
      requires see below
    constexpr auto bit_shift_right(R&& r, ranges::range_difference_t<R> n) noexcept
      -> ranges::borrowed_subrange_t<R>;

  // [alg.sorting], sorting and related operations
  template<see below R>
    constexpr auto bit_sort(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;
  template<see below R>
    constexpr auto bit_is_sorted(const R& r) noexcept -> bool;

  // [alg.partitions], partitions
  template<see below R>
    constexpr auto bit_is_partitioned(const R& r) noexcept -> bool;
  template<see below R>
    constexpr auto bit_partition(R&& r) noexcept -> ranges::borrowed_subrange_t<R>;
  template<see below R>
    constexpr auto bit_partition_point(R&& r) noexcept -> ranges::borrowed_iterator_t<R>;

  // [includes], includes, with intersects and disjoint beside it
  template<see below S>
    constexpr auto bit_includes(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;
  template<see below S>
    constexpr auto bit_intersects(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;
  template<see below S>
    constexpr auto bit_disjoint(const S& s1, const type_identity_t<S>& s2) noexcept -> bool;

  // [alg.set.operations], set operations, each returning the set
  template<see below S>
      requires requires (const S& s) { { s | s } -> same_as<S>; }
    constexpr auto bit_set_union(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s & s } -> same_as<S>; }
    constexpr auto bit_set_intersection(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s - s } -> same_as<S>; }
    constexpr auto bit_set_difference(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;
  template<see below S>
      requires requires (const S& s) { { s ^ s } -> same_as<S>; }
    constexpr auto bit_set_symmetric_difference(const S& s1, const type_identity_t<S>& s2) noexcept(see below) -> S;

  // [alg.three.way], three-way comparison algorithms
  template<class R1, class R2>
      requires see below
    constexpr auto bit_lexicographical_compare_three_way(const R1& r1, const R2& r2) noexcept -> strong_ordering;
}
```

| Header | Declares |
| :----- | :------- |
| [`<xstd/bits/bit_algorithm/bit_all_of.hpp>`](bit_all_of.md) | `bit_all_of` |
| [`<xstd/bits/bit_algorithm/bit_any_of.hpp>`](bit_all_of.md) | `bit_any_of` |
| [`<xstd/bits/bit_algorithm/bit_none_of.hpp>`](bit_all_of.md) | `bit_none_of` |
| [`<xstd/bits/bit_algorithm/bit_for_each.hpp>`](bit_for_each.md) | `bit_for_each` |
| [`<xstd/bits/bit_algorithm/bit_find.hpp>`](bit_find.md) | `bit_find` |
| [`<xstd/bits/bit_algorithm/bit_find_last.hpp>`](bit_find.md) | `bit_find_last` |
| [`<xstd/bits/bit_algorithm/bit_count.hpp>`](bit_count.md) | `bit_count` |
| [`<xstd/bits/bit_algorithm/bit_mismatch.hpp>`](bit_mismatch.md) | `bit_mismatch` |
| [`<xstd/bits/bit_algorithm/bit_equal.hpp>`](bit_equal.md) | `bit_equal` |
| [`<xstd/bits/bit_algorithm/bit_copy.hpp>`](bit_copy.md) | `bit_copy` |
| [`<xstd/bits/bit_algorithm/bit_copy_backward.hpp>`](bit_copy.md) | `bit_copy_backward` |
| [`<xstd/bits/bit_algorithm/bit_move.hpp>`](bit_copy.md) | `bit_move` |
| [`<xstd/bits/bit_algorithm/bit_swap_ranges.hpp>`](bit_swap_ranges.md) | `bit_swap_ranges` |
| [`<xstd/bits/bit_algorithm/bit_transform.hpp>`](bit_transform.md) | `bit_transform` |
| [`<xstd/bits/bit_algorithm/bit_fill.hpp>`](bit_fill.md) | `bit_fill` |
| [`<xstd/bits/bit_algorithm/bit_reverse.hpp>`](bit_reverse.md) | `bit_reverse` |
| [`<xstd/bits/bit_algorithm/bit_reverse_copy.hpp>`](bit_reverse_copy.md) | `bit_reverse_copy` |
| [`<xstd/bits/bit_algorithm/bit_rotate.hpp>`](bit_rotate.md) | `bit_rotate` |
| [`<xstd/bits/bit_algorithm/bit_rotate_copy.hpp>`](bit_rotate_copy.md) | `bit_rotate_copy` |
| [`<xstd/bits/bit_algorithm/bit_shift_left.hpp>`](bit_shift_left.md) | `bit_shift_left` |
| [`<xstd/bits/bit_algorithm/bit_shift_right.hpp>`](bit_shift_left.md) | `bit_shift_right` |
| [`<xstd/bits/bit_algorithm/bit_sort.hpp>`](bit_sort.md) | `bit_sort` |
| [`<xstd/bits/bit_algorithm/bit_is_sorted.hpp>`](bit_is_sorted.md) | `bit_is_sorted` |
| [`<xstd/bits/bit_algorithm/bit_is_partitioned.hpp>`](bit_is_sorted.md) | `bit_is_partitioned` |
| [`<xstd/bits/bit_algorithm/bit_partition.hpp>`](bit_partition.md) | `bit_partition` |
| [`<xstd/bits/bit_algorithm/bit_partition_point.hpp>`](bit_partition_point.md) | `bit_partition_point` |
| [`<xstd/bits/bit_algorithm/bit_includes.hpp>`](bit_includes.md) | `bit_includes` |
| [`<xstd/bits/bit_algorithm/bit_intersects.hpp>`](bit_includes.md) | `bit_intersects` |
| [`<xstd/bits/bit_algorithm/bit_disjoint.hpp>`](bit_includes.md) | `bit_disjoint` |
| [`<xstd/bits/bit_algorithm/bit_set_union.hpp>`](bit_set_union.md) | `bit_set_union` |
| [`<xstd/bits/bit_algorithm/bit_set_intersection.hpp>`](bit_set_union.md) | `bit_set_intersection` |
| [`<xstd/bits/bit_algorithm/bit_set_difference.hpp>`](bit_set_union.md) | `bit_set_difference` |
| [`<xstd/bits/bit_algorithm/bit_set_symmetric_difference.hpp>`](bit_set_union.md) | `bit_set_symmetric_difference` |
| [`<xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp>`](bit_lexicographical_compare_three_way.md) | `bit_lexicographical_compare_three_way` |

Each algorithm has a header of its own under `<xstd/bits/bit_algorithm/>`, named after it, and the umbrella exports
them in the order `<algorithm>` gives their counterparts, `bit_intersects` and `bit_disjoint` beside
`bit_includes`. Each is specified as the `std::ranges` algorithm it is named after, *with* what the packing changes.
What every one of them changes is stated here once:

- **No function object but the caller's.** None takes a projection. Where the counterpart takes a predicate it is
  `identity()`, so `bit_all_of(r)` is `ranges::all_of(r, identity())`; where it takes a comparison it is
  `ranges::less`, so `bit_sort(r)` is `ranges::sort(r)`; a value searched, counted or filled is a `bool`. Only
  `bit_for_each` and `bit_transform` take a function, and the set operations return a set where
  [[alg.set.operations]](https://eel.is/c++draft/alg.set.operations) writes through an output iterator.
- **Ranges of ours alone.** The constraints admit the library's own owners, views and iterators, whose blocks the
  algorithm reads, and nothing else: `vector<bool>`, `set<size_t>` and their iterators are turned away, as is a
  sequence where a set is asked for and a set where a sequence is. The `std::ranges` algorithms run over those and
  over ours alike.
- **A block at a time.** Complexity is counted in block operations, each a read, write or popcount of one block. `N`
  is the size of the range, `D` the digits of its block, and O(`N`/`D`) means at most a constant times
  ⌈`N`/`D`⌉ + 1.
- **The counterpart's results.** Each returns the type its counterpart returns, so the iterator into an rvalue owner
  is `ranges::dangling`. Every algorithm that calls no function of the caller's is `noexcept`; `bit_for_each` and
  `bit_transform` throw what their function throws, and a set operation is `noexcept` where its operator is.

The constraints are exposition-only concepts in the code, written below as these phrases:

- A *sequence of ours* is an owner or a view of the sequence reading, the valarray reading included:
  `basic_bit_array`, `basic_bit_inplace_vector`, `basic_bit_vector`, `basic_bit_small_vector`, the valarrays,
  `bit_span` and `bit_subspan`, `const` or not. A *whole sequence* is one that is not a window.
- A sequence is *writable* where its storage is reached as writable: an owner that is not `const`, or a view of bits
  that are not `const`, whatever the view's own `const`.
- A *set of ours* is an owner or a view of the set reading: `basic_bit_set`, `basic_bit_fixed_set`,
  `basic_bit_inplace_set`, `basic_bit_small_set`, `bit_set_view` and the aliases over them.
- An *output iterator of ours* is an iterator of a sequence of ours that is `indirectly_writable<O, bool>`, a
  view's or a window's included: `iterator`, never `const_iterator`.
