# Reference

Each public entity of xstd-bits is specified here the way the standard specifies the library: a synopsis per
header, then for each entity the elements of
[[structure.specifications]](https://eel.is/c++draft/structure.specifications) that apply. The library compiles as
[C++23](https://wg21.link/N4950), but its interfaces are measured against the current
[working draft](https://eel.is/c++draft/), and every clause cited here is the draft's. A name with no namespace is in
`std`.

The synopses leave out `[[nodiscard]]`, as the standard's own have since [P2422R1](https://wg21.link/P2422R1), which
made it a recommended practice rather than part of a library's specification. The code carries it on every function
that returns a value, but for a reference returned for chaining, as `operator&=` returns one, and for a by-product a
caller may ignore: postfix `++` and `--`, `insert`, `insert_range`, `emplace`, `emplace_hint`, `erase`, the free
`erase` and `erase_if`, the reference `push_back` and `emplace_back` return, and the `try_` and `unchecked_` doors.
It carries it on every constructor but `basic_bit_array`'s from an `initializer_list`.

The library's own terms are used throughout:

- A *block* is an unsigned integer holding bits. Position `i` is bit `i % D` of block `i / D`, where `D` is the
  block's `numeric_limits<Block>::digits`.
- The *storage* holds the blocks: *fixed*, the width the type's; *inplace*, a run-time width within a capacity the
  type carries, the blocks held inline; *small*, inline up to a capacity and on the heap past it; or *dynamic*, on
  the heap.
- An *owner* holds its storage, and a *view* refers to storage another object holds.
- A *reading* is what the bits mean: a *sequence* of `bool`, a *set* of keys, or a *valarray*, the sequence read
  elementwise.

Each container answers a standard counterpart's synopsis, and is specified as that counterpart *with* what the
packing changes. Members named `adaptor_type`, `adapted_type`, `derived_type`, `reads_as`, `owns_storage`,
`is_windowed` and `static_extent` answer the library's own traits and are not specified here.

## Relation to the standard library

Each entity is one of five kinds, as across the xstd libraries:
- **Copied:** the standard's own exposition-only wording, word for word, under a name users can spell.
- **Extends:** a standard facility under its own name, giving the standard's answer wherever the standard gives one,
  with overloads or specializations for more types.
- **Generalizes:** the job of a standard facility, done for more types through an interface of its own.
- **Spells:** a portable spelling of a standard attribute or macro.
- **Proposed** or **new:** no counterpart in the standard. A proposed entity follows a WG21 paper; a new one follows
  none.

Nothing here is copied or spelled: the containers, views and algorithms generalize, the specializations of standard
templates extend, and what the standard has no word for is new.

| Entity | Kind | Counterpart |
| :-- | :-- | :-- |
| `basic_bit_array`, `bit_array` | Generalizes | `array<bool, N>`, [[array]](https://eel.is/c++draft/array), doing the job of `bitset`, [[template.bitset]](https://eel.is/c++draft/template.bitset), as a sequence of `bool` |
| `basic_bit_inplace_vector`, `bit_inplace_vector` | Generalizes | `inplace_vector<bool, N>`, [[inplace.vector]](https://eel.is/c++draft/inplace.vector) |
| `basic_bit_vector`, `bit_vector` | Generalizes | `vector<bool>`, [[vector.bool]](https://eel.is/c++draft/vector.bool) |
| `basic_bit_set`, `bit_set` | Generalizes | `set<size_t>`, [[set]](https://eel.is/c++draft/set), doing the job of `bitset` read as a set of positions |
| `basic_bit_fixed_set`, `bit_fixed_set`, `basic_bit_inplace_set`, `bit_inplace_set` | Generalizes | `set`, [[set]](https://eel.is/c++draft/set), at a fixed width or within an inline capacity, which no standard set has |
| `bit_flag_set` | Generalizes | a bitmask type, [[bitmask.types]](https://eel.is/c++draft/bitmask.types), as the set of its one-bit values |
| `bit_enum_set` | New | none: the standard has no set of an enumeration's values |
| `bit_set_view` | New | none: no standard view reads a range as a set |
| `bit_key_mapping`, `bit_range_mapping`, `bit_find_mapping`, `enum_traits`, `bit_flag_mapping` | New | none; `bit_key_mapping` is specialized per key as `char_traits` is, [[char.traits]](https://eel.is/c++draft/char.traits) |
| `basic_bit_valarray`, `bit_valarray`, `basic_bit_fixed_valarray`, `bit_fixed_valarray`, `basic_bit_inplace_valarray`, `bit_inplace_valarray` | Generalizes | `valarray<bool>`, [[template.valarray]](https://eel.is/c++draft/template.valarray), at a run-time, a fixed or an inplace width |
| `bit_slice`, `bit_gslice`, `bit_slice_array`, `bit_gslice_array`, `bit_mask_array`, `bit_indirect_array` | Generalizes | `slice`, [[class.slice]](https://eel.is/c++draft/class.slice), `gslice`, [[class.gslice]](https://eel.is/c++draft/class.gslice), and the four helper arrays, [[template.slice.array]](https://eel.is/c++draft/template.slice.array), [[template.gslice.array]](https://eel.is/c++draft/template.gslice.array), [[template.mask.array]](https://eel.is/c++draft/template.mask.array) and [[template.indirect.array]](https://eel.is/c++draft/template.indirect.array) |
| `bit_span` | Generalizes | `span`, [[views.span]](https://eel.is/c++draft/views.span), over blocks read as a sequence of `bool` |
| `bit_subspan` | Generalizes | `span`'s `subspan`, [[span.sub]](https://eel.is/c++draft/span.sub), at any bit offset |
| `basic_bit_small_vector`, `bit_small_vector`, `basic_bit_small_set`, `bit_small_set`, `basic_bit_small_valarray`, `bit_small_valarray` | Generalizes | `vector<bool>`, `set` and `valarray<bool>` in Boost.Container's `small_vector`, a storage the standard has no counterpart for |
| `to_bit_array` | Generalizes | `to_array`, [[array.creation]](https://eel.is/c++draft/array.creation) |
| `tuple_size`, `tuple_element`, `get` | Extends | `tuple_size`, `tuple_element` and `get`, [[array.tuple]](https://eel.is/c++draft/array.tuple): specializations and an overload for `basic_bit_array` |
| `hash` | Extends | `hash`, [[unord.hash]](https://eel.is/c++draft/unord.hash): specializations for every owner but the valarrays, and for the set view |
| `erase`, `erase_if` | Generalizes | `erase` and `erase_if`, [[vector.erasure]](https://eel.is/c++draft/vector.erasure), [[inplace.vector.erasure]](https://eel.is/c++draft/inplace.vector.erasure) and [[set.erasure]](https://eel.is/c++draft/set.erasure) |
| `is_range`, `is_tuple_like` | New | none in the standard: specializations of Boost.ContainerHash's traits |
| `bit_all_of`, `bit_any_of`, `bit_none_of`, `bit_for_each`, `bit_find`, `bit_find_last`, `bit_count`, `bit_mismatch`, `bit_equal`, `bit_copy`, `bit_copy_backward`, `bit_move`, `bit_swap_ranges`, `bit_transform`, `bit_fill`, `bit_reverse`, `bit_reverse_copy`, `bit_rotate`, `bit_rotate_copy`, `bit_shift_left`, `bit_shift_right`, `bit_sort`, `bit_is_sorted`, `bit_is_partitioned`, `bit_partition`, `bit_partition_point` | Generalizes | the `ranges` algorithm of each name without the prefix, [[algorithms]](https://eel.is/c++draft/algorithms), less the projections, predicates and comparators a range of bits has no use for |
| `bit_includes` | Generalizes | `ranges::includes`, [[includes]](https://eel.is/c++draft/includes) |
| `bit_intersects`, `bit_disjoint` | New | none: the standard has no test of whether two sets share an element |
| `bit_set_union`, `bit_set_intersection`, `bit_set_difference`, `bit_set_symmetric_difference` | Generalizes | `ranges::set_union` and its kin, [[set.union]](https://eel.is/c++draft/set.union), [[set.intersection]](https://eel.is/c++draft/set.intersection), [[set.difference]](https://eel.is/c++draft/set.difference) and [[set.symmetric.difference]](https://eel.is/c++draft/set.symmetric.difference), returning a set rather than writing through an output iterator |
| `bit_lexicographical_compare_three_way` | Proposed | `ranges::lexicographical_compare_three_way`, proposed by [P2022R3](https://wg21.link/p2022r3); the standard has the iterator-pair `lexicographical_compare_three_way`, [[alg.three.way]](https://eel.is/c++draft/alg.three.way) |
| `bit_advance`, `bit_distance`, `bit_next`, `bit_prev` | Generalizes | `ranges::advance`, `ranges::distance`, `ranges::next` and `ranges::prev`, [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), a block at a time over a set |
| `bit_blocks`, `bit_index_mapping`, `sized_bit_index_mapping`, `bit_mask_mapping`, `bit_constructible_from`, `bit_convertible_to` | New | none |
| `bit_rebind`, `bit_resize` | Generalizes | `rebind_t` and `resize_t`, [[simd.traits]](https://eel.is/c++draft/simd.traits), from [P1928R15](https://wg21.link/p1928r15), for owners rather than `simd` types |
| `least_block_t`, `fast_block_t` | Generalizes | `uint_leastN_t` and `uint_fastN_t`, [[cstdint.syn]](https://eel.is/c++draft/cstdint.syn), chosen by a width rather than named by one |
| `bit_least`, `bit_fast`, `bit_align`, `underlying_block_t`, `bit_underlying`, `bit_blocks_extent_v` | New | none |
| `bit_equal_to`, `bit_not_equal_to`, `bit_less`, `bit_greater`, `bit_less_equal`, `bit_greater_equal`, `bit_compare_three_way` | Generalizes | `ranges::equal_to` and the other comparison objects, [[range.cmp]](https://eel.is/c++draft/range.cmp), and `compare_three_way`, [[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way), across block types and storages |
| `bit_hash` | Generalizes | `hash`, [[unord.hash]](https://eel.is/c++draft/unord.hash), under a Boost.Hash2 algorithm the caller names |
| `bit_hash_append` | New | none in the standard: a Boost.Hash2 `hash_append`, after [N3980](https://wg21.link/n3980) |
| `bit_digest` | Proposed | [N3980](https://wg21.link/n3980)'s `uhash`, which returns the algorithm's whole result |
| `from_blocks_t`, `from_blocks` | Generalizes | `from_range_t` and `from_range`, [[ranges.syn]](https://eel.is/c++draft/ranges.syn), from [P1206R7](https://wg21.link/p1206r7) |
| `bit_convert` | Generalizes | `bit_cast`, [[bit.cast]](https://eel.is/c++draft/bit.cast), from [P0476R2](https://wg21.link/p0476r2): position for position across widths and readings, rather than byte for byte at one size |
| `bit_convert` with `boost::dynamic_bitset` | New | none in the standard: conversion to and from Boost.DynamicBitset |

## Prior work

What each entity builds on in C++, and, for an entity that introduces a name or a concept of its own, its counterpart
in another language where that counterpart shares its purpose. A container that generalizes `bitset` or
`vector<bool>` cites no other language. Papers are linked through wg21.link.

- **The containers, and `bitset`:**
  - Chuck Allison's
    [N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf)
    (1991) and
    [N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf)
    (1992), the proposals that became `bitset`, which named both of its readings, an array of bits and a set of
    integers.
  - [P0125R0](https://wg21.link/p0125r0), *std::bitset inclusion test methods* (Vittorio Romeo), the subset test
    `bit_includes` answers.
  - [P0161R0](https://wg21.link/p0161r0), *Bitset Iterators, Masks, and Container Operations* (Nathan Myers), which
    iterates a `bitset` as the sets iterate their keys.
  - [P2417R2](https://wg21.link/p2417r2), *A more constexpr bitset* (Daniil Goncharov), and
    [P3103R2](https://wg21.link/p3103r2), *More bitset operations* (Jan Schultke).
- **`bit_vector`, and `vector<bool>`:** the SGI STL's [`bit_vector`](https://stl.boost.org/bit_vector.html), a
  class of its own for want of partial specialization, and [N1847](https://wg21.link/n1847), *vector<bool>: More
  Problems, Better Solutions* (Herb Sutter).
- **The dynamic and small storages:**
  - [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/), and
    [N2050](https://wg21.link/n2050), *Proposal to Add a Dynamically Sizeable Bitset to the Standard Library
    Technical Report Revision 1* (J. Allsop, A. Meredith, G. Prota), which proposed it for the standard.
  - LLVM's [`BitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/BitVector.h) and
    [`SmallBitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/SmallBitVector.h),
    EASTL's [`bitvector`](https://github.com/electronicarts/EASTL/blob/master/include/EASTL/bitvector.h) and Qt's
    [`QBitArray`](https://doc.qt.io/qt-6/qbitarray.html).
  - Boost.Container's
    [`small_vector`](https://www.boost.org/doc/libs/release/doc/html/boost/container/small_vector.html), which holds
    the small column, and `static_vector`, which holds the inplace one below C++26.
- **`bit_inplace_vector`, `bit_inplace_set`, `bit_inplace_valarray`:** [P0843R14](https://wg21.link/p0843r14),
  *inplace_vector* (Gonzalo Brito Gadeschi, Timur Doumler, Nevin Liber, David Sankel).
- **`bit_span`, `bit_subspan` and the proxies:**
  - [P0237R10](https://wg21.link/p0237r10), *Wording for fundamental bit manipulation utilities* (Vincent Reverdy,
    Robert J. Brunner), whose `bit_reference` and `bit_iterator` address a bit at any offset.
  - itsy_bitsy's [`bit_view`](https://github.com/ThePhD/itsy_bitsy/blob/d5b6bf9509bb2dff6235452d427f0b1c349d5f8b/include/itsy/bit_view.hpp)
    and [`bit_sequence`](https://github.com/ThePhD/itsy_bitsy/blob/d5b6bf9509bb2dff6235452d427f0b1c349d5f8b/include/itsy/bit_sequence.hpp),
    a view and a container of bits over a range of words.
  - [P0122R7](https://wg21.link/p0122r7), *span: bounds-safe views for sequences of objects* (Neil MacIntosh,
    Stephan T. Lavavej).
- **`bit_flag_set`, `bit_enum_set` and the key mappings:**
  - **C++:** [N3110](https://wg21.link/n3110), *Problems with bitmask types in the library* (Jonathan Wakely);
    [N4202](https://wg21.link/n4202), *Strongly Typed Bitset* (Matthew Fioravante); and
    [P4313R1](https://wg21.link/p4313r1), *Bitmask operations for enums* (Iliya Guterman, Anthony Williams), whose
    operators `bit_flag_set` takes as they are. Mozilla's
    [`EnumSet`](https://github.com/mozilla-firefox/firefox/blob/3c71a541b0e7ac6e086907b38670dabca9174c45/mfbt/EnumSet.h),
    whose default word is `bit_underlying`'s, Chromium's
    [`EnumSet`](https://github.com/chromium/chromium/blob/b0840fa3d3ce4939cecdb19aa2125bbffbe069ca/base/containers/enum_set.h),
    type_safe's [`flag_set`](https://github.com/foonathan/type_safe/blob/292e8c127037e33d92f8f52dab0f1184993942d0/include/type_safe/flag_set.hpp)
    and Qt's [`QFlags`](https://doc.qt.io/qt-6/qflags.html).
  - **Java's** [`EnumSet`](https://docs.oracle.com/en/java/javase/21/docs/api/java.base/java/util/EnumSet.html) is
    `bit_enum_set`'s counterpart: a set of one enumeration's values, represented internally as a bit vector.
- **The algorithms and iterator operations:**
  - [P2022R3](https://wg21.link/p2022r3), *Rangified version of lexicographical_compare_three_way* (Ran Regev, Alex
    Dathskovsky), whose signature `bit_lexicographical_compare_three_way` takes.
  - [range-v3](https://github.com/ericniebler/range-v3)'s set views, which return their result as the four set
    operations here do rather than writing it through an output iterator.
  - SDSL's [`rank_support_v`](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/rank_support_v.hpp)
    and [`select_support_mcl`](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/select_support_mcl.hpp),
    the rank and select that `bit_distance` and `bit_advance` compute over a set a block at a time.
- **`bit_rebind`, `bit_resize`:** [P1928R15](https://wg21.link/p1928r15), *std::simd - Merge data-parallel types
  from the Parallelism TS 2* (Matthias Kretz), whose `rebind_t` and `resize_t` they mirror.
- **`bit_hash`, `bit_hash_append`, `bit_digest`:** [N3980](https://wg21.link/n3980), *Types Don't Know #* (Howard
  Hinnant, Vinnie Falco, John Bytheway), [Boost.Hash2](https://github.com/boostorg/hash2) and
  [Boost.ContainerHash](https://www.boost.org/doc/libs/release/libs/container_hash/).
- **`from_blocks`:** [P1206R7](https://wg21.link/p1206r7), *Conversions from ranges to containers* (Corentin
  Jabot, Eric Niebler, Casey Carter), whose `from_range` tag it mirrors.
- **`bit_convert`:** [P0476R2](https://wg21.link/p0476r2), *Bit-casting object representations* (JF Bastien).

## `<xstd/bits/bit_array.hpp>`

```cpp
namespace xstd {
  // [array], class template array
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_array;
  template<size_t N>
    using bit_array = basic_bit_array<size_t, N>;

  // [array.creation], array creation functions
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&a)[N]) -> bit_array<N>;
  template<class T, size_t N>
    constexpr auto to_bit_array(T (&&a)[N]) -> bit_array<N>;
}

namespace std {
  // [array.tuple], tuple interface
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, xstd::basic_bit_array<Block, N>>;
  template<size_t I, class Block, size_t N>
    struct tuple_element<I, const xstd::basic_bit_array<Block, N>>;
  template<class Block, size_t N>
    struct tuple_size<xstd::basic_bit_array<Block, N>>;

  // hash support
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_array<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_array<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_array<Block, N>> : false_type {};
}
```

### `basic_bit_array`, `bit_array`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_array {
public:
  // types
  using value_type             = bool;
  using pointer                = void;
  using const_pointer          = void;
  using reference              = see below;
  using const_reference        = see below;
  using size_type              = size_t;
  using difference_type        = ptrdiff_t;
  using iterator               = see below;
  using const_iterator         = see below;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using block_container_type   = array<Block, see below>;

  // construct
  basic_bit_array() = default;
  constexpr basic_bit_array(initializer_list<bool> il);
  template<size_t M>
      requires (M == N)
    constexpr explicit basic_bit_array(const bool (&a)[M]) noexcept;
  template<class Bits>
      requires see below
    constexpr basic_bit_array(from_blocks_t, const Bits& b) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> size     = {};
  static constexpr bool_constant<N == 0>        empty    = {};
  static constexpr integral_constant<size_t, N> max_size = {};

  friend constexpr auto swap(basic_bit_array& x, basic_bit_array& y) noexcept(noexcept(x.swap(y))) -> void;
};

template<unsigned_integer Block>
  basic_bit_array(from_blocks_t, Block) -> basic_bit_array<Block, bit_blocks_extent_v<Block>>;
template<unsigned_integer Block, size_t K>
  basic_bit_array(from_blocks_t, array<Block, K>) -> basic_bit_array<Block, bit_blocks_extent_v<array<Block, K>>>;
template<unsigned_integer Block, size_t K>
  basic_bit_array(from_blocks_t, const Block (&)[K]) -> basic_bit_array<Block, bit_blocks_extent_v<array<Block, K>>>;
```

As [[array]](https://eel.is/c++draft/array) for `array<bool, N>`, with:

- `bit_array<N>` holds `N` bits in `size_t` blocks, and `basic_bit_array<Block, N>` in `Block`s, `ceil(N / D)` of
  them, none at `N == 0`. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two and at
  least `unsigned char`'s, at any `N`, and any other cv-unqualified `Block` where `N` fits in one of it.
- **A proxy reference.** A bit has no address, so `reference` is a proxy class, `pointer` and `const_pointer` are
  `void`, `data()` is absent, and `iterator` is random access rather than contiguous. `const_reference` is the same
  proxy over const storage, and reads without assigning.
- The class is no aggregate. Its constructors are what `array`'s initialization does:
  - The default constructor makes every position `false`.
  - The `initializer_list` constructor sets the leading positions to the listed values and leaves the rest `false`.
    *Preconditions:* `il.size() <= N`.
  - The built-in array constructor copies `a` position by position, and only an array of `N` elements converts.
- *`from_blocks` constructor:* *Constraints:* `Bits` models `bit_blocks`, and its bits have a layout this width
  reads: a block, a `std::array` or a built-in array of blocks holding at least `N` bits, or, on a little-endian
  target, a trivially copyable object with room for `N` bits, read as `bit_cast` reads it. *Effects:* Position `i`
  takes bit `i` of `b`, for every `i < N`. The guides deduce the width the blocks name.
- *Capacity:* `size`, `empty` and `max_size` are constants of the type. `a.size()` and `A::size()` call
  `integral_constant`'s `operator()`, so each answers a `size_type`, `noexcept`, in a constant expression;
  `A::size` and `a.size` name the constant itself, and `auto n = A::size;` deduces the `integral_constant`.
- `fill`, `swap`, `==` and `<=>` are as [[array]](https://eel.is/c++draft/array); `<=>` answers `strong_ordering`,
  the order `lexicographical_compare_three_way` gives the two sequences of bools, and is `noexcept`.
- It is a structural type exactly where `N` fills its blocks, at `N == 0` included, as `bit_align<bit_array<N>>`
  does at any `N`. At another width the last block's unused bits must stay clear, so the blocks are not public.
- Every member is usable in a constant expression, as [P3372R3](https://wg21.link/P3372R3) makes the standard
  containers' members.
- *Out of range:* `at(n)` throws `out_of_range` where `n >= size()`; `operator[]`, `front` and `back` have
  [[sequence.reqmts]](https://eel.is/c++draft/sequence.reqmts)'s preconditions, checked by an `assert`.

**`reference`, shared by every owner and view of the sequence reading.**

```cpp
class reference {
public:
  using value_type = bool;
  using iterator   = see below;

  reference(const reference&) = default;

  constexpr auto operator&() const noexcept -> iterator;
  constexpr operator bool() const noexcept;
  constexpr auto operator=(bool x) const noexcept -> const reference&;
  constexpr auto operator=(const reference& x) const noexcept -> const reference&;

  friend constexpr auto swap(reference x, reference y) noexcept -> void;
  friend constexpr auto swap(reference x, bool& y) noexcept -> void;
  friend constexpr auto swap(bool& x, reference y) noexcept -> void;
};
```

As `vector<bool>::reference` in [[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc), with:

- The assignments are `const`-qualified and return `const reference&`, the proxy shape of
  [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html). Each writes the bit, never
  rebinds the proxy, and exists only where the storage is not const.
- The three `swap`s are hidden friends, after
  [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html). There is no static
  `swap(reference, reference)` on any container, the form
  [[depr.vector.bool.swap]](https://eel.is/c++draft/depr.vector.bool.swap) deprecates.
- There is no `flip()`: `v[i] = not v[i]` negates one element, and the valarray reading's `!` negates them all.
- `&ref` answers an iterator, so `&*it == it` holds in iterator arithmetic. `bool` is the one conversion, so
  comparisons are the built-in ones through it.
- `formatter<reference, charT>` formats it as `bool`, as [[vector.bool.fmt]](https://eel.is/c++draft/vector.bool.fmt)
  formats `vector<bool>::reference`, and a sequence formats as a range, in brackets.

**`iterator`, shared by every owner and view of the sequence reading.**

- `iterator_category` is `random_access_iterator_tag`, `value_type` is `bool`, `difference_type` is `ptrdiff_t`,
  `pointer` is `void`, and `reference` is the proxy. A mutable iterator converts implicitly to `const_iterator`.
- `iter_move(it)` answers the `bool`, and `iter_swap` exchanges the two bits, so the `ranges` algorithms run over
  the proxies unchanged.
- *Preconditions:* Two iterators compared or subtracted refer to the same storage.
- *Remarks:* An iterator is the storage object and a position, not a pointer into the blocks, so growth that
  reallocates the blocks invalidates no iterator.

### `to_bit_array`

- *Mandates:* `remove_cv_t<T>` is `bool`.
- *Returns:* `bit_array<N>(a)`, the second form from the moved array. A built-in array names no block type, so the
  result takes the default one.

### `tuple_size`, `tuple_element`, `get`

- As [[array.tuple]](https://eel.is/c++draft/array.tuple), with:
- `tuple_size<basic_bit_array<Block, N>>` is `integral_constant<size_t, N>`.
- `tuple_element<I, basic_bit_array<Block, N>>::type` is the proxy `reference`, and for a `const` array the proxy
  `const_reference`, so a structured binding over the array writes through to it. *Mandates:* `I < N`.
- `get<I>(a)`, for an lvalue, a `const` lvalue, an rvalue and a `const` rvalue, returns `a[I]`. *Constraints:*
  `I < N`. It is found by argument-dependent lookup, not as a member of namespace `xstd`.

### `hash`, `is_range`, `is_tuple_like`

- `hash<basic_bit_array<Block, N>>`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`, the hash of the bits. `array<bool, N>` has no hash; this one
  does, and equal values hash equal.
- A Boost.Hash2 `hash_append` hook appends exactly the message Hash2 writes for the `array<bool, N>` of the same
  value, so `xstd::hash<T, H, Flavor>()(x) == xstd::hash<array<bool, N>, H, Flavor>()(m)` for that `m`.
- `is_range` and `is_tuple_like` are `false_type`, so Boost.ContainerHash and Hash2 take the hook rather than
  hashing the array as a range or as a tuple.

## `<xstd/bits/bit_inplace_vector.hpp>`

```cpp
namespace xstd {
  // [inplace.vector], class template inplace_vector
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_inplace_vector;
  template<size_t N>
    using bit_inplace_vector = basic_bit_inplace_vector<size_t, N>;

  // [inplace.vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

#define XSTD_BITS_HAS_CONSTEXPR_INPLACE 1                                     // see below

namespace std {
  template<class Block, size_t N>
    struct hash<xstd::basic_bit_inplace_vector<Block, N>>;
}

namespace boost::container_hash {
  template<class Block, size_t N>
    struct is_range<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
  template<class Block, size_t N>
    struct is_tuple_like<xstd::basic_bit_inplace_vector<Block, N>> : false_type {};
}
```

### `basic_bit_inplace_vector`, `bit_inplace_vector`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_inplace_vector {
public:
  // types as basic_bit_array's, with
  using block_container_type = see below;

  // construct: [inplace.vector.cons]'s, with
  constexpr basic_bit_inplace_vector(from_blocks_t, block_container_type blocks) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
  static constexpr integral_constant<size_t, N> capacity = {};
  static constexpr auto reserve(size_type n) -> void;
  static constexpr auto shrink_to_fit() noexcept -> void;

  // block exchange
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  friend constexpr auto swap(basic_bit_inplace_vector& x, basic_bit_inplace_vector& y)
    noexcept(noexcept(x.swap(y))) -> void;
};

template<unsigned_integer Block, size_t K>
  basic_bit_inplace_vector(from_blocks_t, see below) -> basic_bit_inplace_vector<Block, bit_blocks_extent_v<Block> * K>;
```

As [[inplace.vector]](https://eel.is/c++draft/inplace.vector) for `inplace_vector<bool, N>`, with:

- `bit_inplace_vector<N>` holds up to `N` bits in `size_t` blocks, and `basic_bit_inplace_vector<Block, N>` in
  `Block`s. The requires-clause admits a cv-unqualified `Block` whose digits are a power of two and at least
  `unsigned char`'s. `N` is the capacity exactly, even where the last block has room for more.
- `block_container_type` is `inplace_vector<Block, K>` where the standard library has `<inplace_vector>`, and
  `boost::container::static_vector<Block, K>` elsewhere, for `K = ceil(N / D)`. At `K == 0`, where that type is not
  empty, it is an empty type of the library's own, so `bit_inplace_vector<0>` is an empty type, as
  [[inplace.vector.overview]](https://eel.is/c++draft/inplace.vector.overview) makes `inplace_vector<T, 0>`.
- The reference, the iterator and the departures they force are `basic_bit_array`'s: no `data()`, `pointer` names
  `void`.
- *Capacity:* `capacity` and `max_size` are constants of the type, called as
  [[inplace.vector.capacity]](https://eel.is/c++draft/inplace.vector.capacity) calls them; `size()` and `empty()`
  are functions. `reserve(n)` throws `bad_alloc` where `n > N` and does nothing otherwise; `shrink_to_fit()` does
  nothing.
- *Modifiers:* As [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers): `push_back` and
  `emplace_back` return the proxy for the new element, `try_push_back` and `try_emplace_back` return
  `optional<reference>`, `nullopt` when full, and `unchecked_push_back` and `unchecked_emplace_back` have
  `size() < N` as their precondition. There is no `try_append_range`, which
  [P4022R0](https://wg21.link/P4022R0) takes out of C++26.
- Growth past `N` throws `bad_alloc` before anything is written. `append_range` from a range with no size undoes
  what it appended before the exception leaves, so it has no effects, as
  [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers) asks.
- *`from_blocks` constructor:* *Effects:* Moves `blocks` in. Every bit is a position, so `size()` is
  `blocks.size() * D`. *Preconditions:* `blocks.size() * D <= N`. The guide deduces the capacity the blocks hold.
- *`replace`:* *Effects:* Moves `blocks` in, `size()` becoming `blocks.size() * D`. *Preconditions:* as the
  constructor's.
- *`extract`:* *Returns:* the blocks, moved out, the unused bits of the last one clear. *Postconditions:*
  `size() == 0`. A width short of whole blocks comes back padded with `false`, which `resize` trims.
- `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined exactly where the blocks are an `inplace_vector`, and there alone
  the owner is usable in a constant expression. Over `static_vector` its copies are not `noexcept`, as
  `static_vector`'s are not; its `swap` is `noexcept` over either.
- It is not a structural type at any width.

### `erase`, `erase_if`

- As [[inplace.vector.erasure]](https://eel.is/c++draft/inplace.vector.erasure), with:
- *Constraints:* `c` is an owner of the sequence reading whose width grows, so `c.erase(c.cbegin(), c.cend())` is
  well-formed: `basic_bit_inplace_vector`, `basic_bit_vector` and `basic_bit_small_vector`.
- *Returns:* the number of elements removed.
- They are declared in a namespace of the library's adaptor and found by argument-dependent lookup, not as
  `xstd::erase` and `xstd::erase_if`.
- A `bit_vector` erases as a `vector<bool>` of the same bools does, counting what it removes alike.

### `hash`, `is_range`, `is_tuple_like`

As for `basic_bit_array`, the model being `inplace_vector<bool, N>`.

## `<xstd/bits/bit_vector.hpp>`

```cpp
namespace xstd {
  // [vector.bool.pspc], partial class template specialization vector<bool, Allocator>
  template<unsigned_integer Block, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_vector;
  using bit_vector = basic_bit_vector<size_t>;

  // [vector.erasure], erasure
  template<class U = bool>
    constexpr auto erase(see below& c, const U& value) -> see below;
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Block, class Allocator>
    struct hash<xstd::basic_bit_vector<Block, Allocator>>;
}

namespace boost::container_hash {
  template<class Block, class Allocator>
    struct is_range<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
  template<class Block, class Allocator>
    struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : false_type {};
}
```

### `basic_bit_vector`, `bit_vector`

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

### `erase`, `erase_if`

As for `<xstd/bits/bit_inplace_vector.hpp>`, after [[vector.erasure]](https://eel.is/c++draft/vector.erasure). The
two headers export the same function templates.

### `hash`, `is_range`, `is_tuple_like`

As for `basic_bit_array`, the model being `vector<bool>`, as
[[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) hashes it: equal values hash equal, whatever capacity
or allocation is behind them.

## `<xstd/bits/bit_set.hpp>`

```cpp
namespace xstd {
  // [set], class template set
  template<class Key, unsigned_integer Block, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_set;
  using bit_set = basic_bit_set<size_t, size_t>;

  // the fixed-width and inplace-capacity sets
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>>
      requires see below
    class basic_bit_fixed_set;
  template<size_t N>
    using bit_fixed_set = basic_bit_fixed_set<size_t, size_t, N>;
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>>
      requires see below
    class basic_bit_inplace_set;
  template<size_t N>
    using bit_inplace_set = basic_bit_inplace_set<size_t, size_t, N>;

  // the set over borrowed blocks
  template<bit_blocks Blocks, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_set_view;

  // the key mappings
  template<class Enum>
    struct enum_traits {};
  template<class Key, Key First, size_t N>
      requires see below
    struct bit_range_mapping;
  template<class Key, const auto& Keys>
      requires see below
    struct bit_find_mapping;
  template<class Key>
    struct bit_key_mapping;
  template<class Key, size_t N = see below>
      requires see below
    struct bit_flag_mapping;

  // the fixed sets an enumeration or a bitmask type names
  template<class Enum>
      requires is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
    using bit_enum_set =
      bit_least<basic_bit_fixed_set<Enum, size_t, bit_key_mapping<Enum>::size, bit_key_mapping<Enum>>>;
  template<class Mask, size_t N = see below>
      requires see below
    using bit_flag_set =
      bit_least<basic_bit_fixed_set<Mask, size_t, N, bit_flag_mapping<Mask, N>, greater<Mask>>>;

  // [set.erasure], erasure
  template<class Predicate>
    constexpr auto erase_if(see below& c, Predicate pred) -> see below;
}

namespace std {
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct hash<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct hash<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct hash<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>>;
  template<class Blocks, size_t N>
    struct hash<xstd::bit_set_view<Blocks, N>>;
}

namespace boost::container_hash {
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct is_range<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>> : false_type {};
  template<class Key, class Block, class KeyMapping, class Compare, class Allocator>
    struct is_tuple_like<xstd::basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_range<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_tuple_like<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_range<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare>
    struct is_tuple_like<xstd::basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>> : false_type {};
  template<class Blocks, size_t N>
    struct is_range<xstd::bit_set_view<Blocks, N>> : false_type {};
}
```

### `basic_bit_set`, `bit_set`

```cpp
template<class Key, unsigned_integer Block, bit_index_mapping<Key> KeyMapping, see below Compare, class Allocator>
  requires see below
class basic_bit_set {
public:
  // types
  using key_type               = Key;
  using key_compare            = Compare;
  using value_type             = Key;
  using value_compare          = Compare;
  using allocator_type         = Allocator;
  using pointer                = void;
  using const_pointer          = void;
  using reference              = see below;
  using const_reference        = reference;
  using size_type              = size_t;
  using difference_type        = ptrdiff_t;
  using iterator               = see below;
  using const_iterator         = iterator;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using key_mapping_type       = KeyMapping;
  using block_container_type   = vector<Block, Allocator>;

  // construct: [set.cons]'s, with
  constexpr basic_bit_set(from_blocks_t, vector<Block, Allocator> blocks) noexcept;
  constexpr basic_bit_set(from_blocks_t, vector<Block, Allocator> blocks, const Allocator& a);

  // modifiers: [set.modifiers]'s, with
  constexpr auto insert(value_type x) -> pair<iterator, bool>;
  constexpr auto insert(const_iterator position, value_type x) -> iterator;
  constexpr auto replace(block_container_type&& blocks) noexcept(see below) -> void;
  constexpr auto extract() && noexcept(see below) -> block_container_type;

  // set algebra
  constexpr auto operator&=(const basic_bit_set& other) noexcept -> basic_bit_set&;
  constexpr auto operator|=(const basic_bit_set& other) -> basic_bit_set&;
  constexpr auto operator^=(const basic_bit_set& other) -> basic_bit_set&;
  constexpr auto operator-=(const basic_bit_set& other) noexcept -> basic_bit_set&;
  constexpr auto operator<<=(size_t n) -> basic_bit_set&;
  constexpr auto operator>>=(size_t n) noexcept -> basic_bit_set&;

  friend constexpr auto operator==(const basic_bit_set& x, const basic_bit_set& y) noexcept -> bool;
  friend constexpr auto operator<=>(const basic_bit_set& x, const basic_bit_set& y) noexcept -> strong_ordering;
  friend constexpr auto swap(basic_bit_set& x, basic_bit_set& y) noexcept(noexcept(x.swap(y))) -> void;
};

constexpr auto operator&(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator|(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator^(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator-(const basic_bit_set& x, const basic_bit_set& y) -> basic_bit_set;
constexpr auto operator<<(const basic_bit_set& x, size_t n) -> basic_bit_set;
constexpr auto operator>>(const basic_bit_set& x, size_t n) -> basic_bit_set;
```

As [[set]](https://eel.is/c++draft/set) for `set<Key, Compare, Allocator>`, with:

- `bit_set` is a set of `size_t` in `size_t` blocks, and `basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>`
  a set of `Key` whose positions `KeyMapping` names, in a `vector<Block, Allocator>`. The requires-clause admits a
  cv-unqualified `Block` whose digits are a power of two and at least `unsigned char`'s.
- `KeyMapping` takes a key to its position by `to_index` and back by `from_index`, preserving order. A mapping that
  models `sized_bit_index_mapping` closes the universe at `KeyMapping::size` keys.
- `Compare` is `less<Key>`, `less<>`, `greater<Key>` or `greater<>`: the order of the positions is fixed, so the
  comparator picks only a direction. Each holds no state, so a `comp` argument is taken and dropped, and `key_comp()`
  and `value_comp()` return `Compare()`. With `less<>` or `greater<>` the heterogeneous `contains`, `count`, `find`,
  `lower_bound`, `upper_bound`, `equal_range` and `erase` take a key of another type, over the positions whose keys
  are equivalent to it.
- **No nodes.** A key is a bit in a block, so `node_type`, `insert_return_type`, `extract` of an element,
  `insert(node_type&&)` and `merge` are absent, as `flat_set` drops them. The name `extract` is the block exchange
  below.
- **A proxy reference.** `reference` is a read-only proxy: copyable, not assignable, converting to `Key`, and `&ref`
  answers the iterator. `pointer` names `void`. It formats as `Key` does, and the set formats in braces.
- `iterator` is bidirectional, its `value_type` `Key`, its `reference` the proxy; `iterator` and `const_iterator`
  are one type. Iteration visits the keys in `Compare`'s order. An iterator is the storage object and a position, so
  growth that reallocates the blocks invalidates none.
- `insert` takes its key by value, one overload where [[set]](https://eel.is/c++draft/set) has two. `emplace` and
  `emplace_hint` take at most one argument, from which a `Key` is made.
- `size()` counts the keys, and runs in constant time, the count being kept beside the blocks. A key past the width
  grows the width; the width is capacity and never value.
- `max_size()` answers the keys the storage could hold, and no more than `KeyMapping::size` where the mapping names
  one.
- **Out of range.** `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range` and `erase(const
  key_type&)` are total, as on `set`: a key outside the universe, or a value that is no key at all, is absent, and
  where `Key` is totally ordered the bounds place it among the keys by that order. `insert`, `emplace`, the ranged
  forms and an enumerator's `|=` and `^=` throw `out_of_range` for a value the mapping ranks at or past
  `KeyMapping::size`; a key of an open universe throws `length_error` past what the storage can count.
- *Set algebra:* The compound operators combine with a set of the same type at any two widths, a block at a time:
  `&=` keeps the common keys, `|=` adds the other's, `^=` keeps those in one alone, `-=` removes the other's. `|=`
  and `^=` grow the width to the other's highest key. `<<= n` adds `n` to every key, growing the width towards
  `max_size()` and dropping the keys that land at or past it; `>>= n` subtracts `n` and drops the keys below `n`. The
  non-members return the copy the compound form makes of `x`, and are the owner's alone. They are the shorthand of
  `xstd::ranges::bit_set_intersection`, `bit_set_union`, `bit_set_difference` and `bit_set_symmetric_difference`.
- `==` holds where the two sets hold the same keys, whatever their widths. `<=>` is
  [[associative.reqmts]](https://eel.is/c++draft/associative.reqmts)'s lexicographic order of the keys; over
  descending keys that is the order of the two masks as unsigned numbers.
- *`from_blocks` constructors:* *Effects:* Move `blocks` in, the second under `a`. Every bit is a position, so the
  set's keys are those whose positions are set. Under an allocator equal to theirs the second keeps their storage,
  and under an unequal one it copies them into `a`'s. The guides deduce the block and the allocator.
- *`replace`:* *Effects:* Moves `blocks` in, the keys becoming those whose positions are set. *`extract`:*
  *Returns:* the blocks, moved out. *Postconditions:* `empty()`. A set loses nothing in the round trip, its width
  being capacity.
- The guides of [[set.overview]](https://eel.is/c++draft/set.overview) deduce `Key` as `size_t` and the block from
  the allocator, `size_t` where none is given.
- **Enumerator operands.** Where `Key` is an enumeration and `KeyMapping` does not read it as a mask, an enumerator
  is the one-element set holding it: `&=`, `|=`, `^=` and `-=` take one on the right, and on an owner `&`, `|`, `^`
  and `-` take one on either side. The enumeration itself gains no operator. `|=` and `^=` throw as `insert` does.
- The Boost.Hash2 hook appends the keys in iteration order, then their count, exactly as Hash2 writes the
  `set<Key, Compare>` of the same keys.

### `basic_bit_fixed_set`, `bit_fixed_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_fixed_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = array<Block, see below>;

  // construct: [set.cons]'s without the allocator forms, with
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
- `max_size` is a constant of the type, `N`; `size()` and `empty()` are functions that count the keys.
- `insert`, `emplace`, the ranged forms and an enumerator's `|=` and `^=` throw `out_of_range` for a key whose
  position is `N` or more.
- `~x` complements every key below `N`. `<<= n` keeps the keys that land below `N`, and `>>= n` the keys at or
  above `n`, both emptying the set where `n >= N`. Every operator over two sets is `noexcept`.
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

### `basic_bit_inplace_set`, `bit_inplace_set`

```cpp
template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping, see below Compare>
  requires see below
class basic_bit_inplace_set {
public:
  // types as basic_bit_set's, without allocator_type, with
  using block_container_type = see below;

  // construct: [set.cons]'s without the allocator forms, with
  constexpr basic_bit_inplace_set(from_blocks_t, block_container_type blocks) noexcept;

  // capacity
  static constexpr integral_constant<size_t, N> max_size = {};
};

template<unsigned_integer Block, size_t K>
  basic_bit_inplace_set(from_blocks_t, see below) -> basic_bit_inplace_set<size_t, Block, bit_blocks_extent_v<Block> * K>;
```

As `basic_bit_set`, with:

- `bit_inplace_set<N>` holds keys below a capacity of `N` in `size_t` blocks, inline. The requires-clause admits
  `Block` as `basic_bit_set`'s does. *Mandates:* as `basic_bit_fixed_set`'s.
- `block_container_type` is `basic_bit_inplace_vector`'s, for `K = ceil(N / D)`. There is no allocator.
- `max_size` is a constant of the type, `N`; `size()` and `empty()` count the keys.
- A key past the width grows the width up to `N`. A key whose position is `N` or more throws `bad_alloc`, as
  `inplace_vector`'s growth does, where the mapping leaves the universe open, and `out_of_range` where the mapping
  closes it.
- `<<= n` keeps the keys that land below `N`, asks for no storage past it, and is `noexcept`.
- *`from_blocks` constructor, `replace`, `extract`:* As `basic_bit_set`'s, with the capacity as a precondition. The
  guide deduces the capacity the blocks hold.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined, and is not a
  structural type.

### `bit_set_view`

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

### `bit_key_mapping`, `bit_range_mapping`, `bit_find_mapping`, `enum_traits`

```cpp
template<class Key, Key First, size_t N>
  requires see below
struct bit_range_mapping {
  static constexpr size_t size = N;
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};

template<class Key, const auto& Keys>
  requires see below
struct bit_find_mapping {
  static constexpr size_t size = ranges::size(Keys);
  static constexpr auto is_key(Key key) noexcept -> bool;
  static constexpr auto to_index(Key key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
};
```

A key's mapping is an order-preserving bijection onto the positions `0` to `size - 1`: `a < b` exactly where
`to_index(a) < to_index(b)`, and `from_index` inverts `to_index`.

- `bit_range_mapping<Key, First, N>` maps the `N` consecutive keys from `First`, of an integer or an enumeration,
  onto `0` to `N - 1`. `is_key(k)` holds for `First <= k < First + N`, compared as an unsigned distance from
  `First`, so no bound overflows `Key`. A key outside the range ranks at `N` or above.
- `bit_find_mapping<Key, Keys>` maps each key of the list `Keys` to its rank there, found by binary search, so a gap
  costs no position. *Mandates:* `Keys` ascends strictly. `is_key(k)` holds for a listed key, and any other ranks at
  `size`.
- `bit_key_mapping<Key>` is the mapping a set takes by default. Its primary template is declared and not defined,
  and it is specialized as `char_traits` is:
  - for every unsigned integer `Key`, the identity, with no `size`, leaving the universe open; `is_key(k)` holds
    where `k` survives the round trip through `size_t`;
  - for an enumeration whose `enum_traits<Key>::values` is declared, `bit_range_mapping` where the values run without
    a gap and `bit_find_mapping` over the list otherwise;
  - for an enumeration that declares no list and has an enumerator `count`, `bit_range_mapping` from `Key{}` to
    `count`, the sentinel itself no key. *Mandates:* `count` is not negative.
  - A program may specialize it for a key type of its own, such as a strong index.
- `enum_traits<Enum>` is empty. A program specializes it beside an enumeration with a `static constexpr`
  `std::array` named `values`, listing the enumerators ascending, each once.

### `bit_flag_mapping`

```cpp
template<class Key, size_t N = see below>
  requires see below
struct bit_flag_mapping {
  using block_type = see below;
  static constexpr size_t size = N;
  static constexpr auto is_key(const Key& key) noexcept -> bool;
  static constexpr auto to_index(const Key& key) noexcept -> size_t;
  static constexpr auto from_index(size_t index) noexcept -> Key;
  static constexpr auto to_block(const Key& mask) noexcept -> block_type;
  static constexpr auto from_block(const block_type& block) noexcept -> Key;
};
```

A bitmask type keyed on its own one-bit values.

- `Key` is an enumeration, an integer type other than `bool` and the character types, or a `std::bitset<M>` with
  `M > 0`. `block_type` is the unsigned word a mask is read and written as: an enumeration's or an integer's unsigned
  counterpart, or for a bitset the least block holding `M`, or past the widest block an array of them.
- `N` defaults to every bit of that word but a sign bit, and for a bitset to `M`. An enumeration with no fixed
  underlying type has no default, its values stopping at the bit-field its enumerators need. *Mandates:* `N` is no
  more than that default width.
- `is_key(k)` holds where `k` has exactly one bit set, below `N`; zero and a value of several bits are no key.
- `to_index(k)` is the position of `k`'s one bit. *Preconditions:* `k` has exactly one bit set.
- `from_index(i)` is the value with bit `i` alone. *Preconditions:* `i < N`.
- `to_block` and `from_block` read any value of `Key`, one-bit or not, as its block and back, which makes the
  mapping model `bit_mask_mapping<bit_flag_mapping<Key, N>, Key>`.

### `bit_enum_set`

- `bit_enum_set<Enum>` is the fixed set of an enumeration's keys, one bit each in the smallest block that holds them:
  `basic_bit_fixed_set<Enum, least_block_t<S>, S, bit_key_mapping<Enum>>` for `S = bit_key_mapping<Enum>::size`.
- *Constraints:* `Enum` is an enumeration whose default mapping names a size: its values listed in `enum_traits`,
  ended by a `count` sentinel, or a `bit_key_mapping` specialized directly.
- `basic_bit_fixed_set{E::a, E::b}` deduces the same type. Another block is
  `basic_bit_fixed_set<Enum, Block, S>`'s to spell.
- An enumerator is the one-element set holding it, on either side of `|`, `&`, `^` and `-`, as for `basic_bit_set`.

### `bit_flag_set`

- `bit_flag_set<Mask, N>` is the set of a bitmask type's one-bit values below `N`:
  `basic_bit_fixed_set<Mask, least_block_t<N>, N, bit_flag_mapping<Mask, N>, greater<Mask>>`.
- *Constraints:* `Mask` models `xstd::bit_mask`, is a signed integer, or is an enumeration whose `|` compiles,
  whatever it returns; and it is an enumeration, an integer or a `std::bitset`, as `bit_flag_mapping` takes.
  `N` defaults as `bit_flag_mapping`'s does: a signed type keeps its sign bit out, and an enumeration with no fixed
  underlying type spells its width.
- The mapping reads every value of `Mask` as a mask, so the set converts implicitly from and to it, and takes it in
  every mixed operator, as `basic_bit_fixed_set`'s mask members specify. Code written against
  `std::filesystem::perms` keeps its constants and changes only the variable's type.
- A braced list is the union of its values, and `contains(k)` is membership of one flag, false for a value of
  several bits.
- Iteration yields the one-bit values from the highest down, and `<=>` is the masks' numeric order.
- `xstd::ranges::bit_includes(p, m)`, `bit_intersects(p, m)` and `bit_disjoint(p, m)` take a mask as their second
  argument, and answer all-of, any-of and none-of. `~p` stays within `N`.

### `erase_if`

- As [[set.erasure]](https://eel.is/c++draft/set.erasure), over every set of this header.
- *Returns:* the number of keys removed.
- It is declared in a namespace of the library's adaptor and found by argument-dependent lookup, not as
  `xstd::erase_if`.

### `hash`, `is_range`, `is_tuple_like`

- `hash<T>` for each owner and for `bit_set_view`: `operator()` is `noexcept` and returns
  `xstd::bit_hash<T, boost::hash2::xxhash_64>()(x)`. Equal sets hash equal, whatever their widths.
- `is_range` and `is_tuple_like` are `false_type` for each owner, and `is_range` for the view, so Boost.ContainerHash
  and Hash2 take the hook of `basic_bit_set`.

## `<xstd/bits/bit_valarray.hpp>`

```cpp
namespace xstd {
  // [template.valarray], class template valarray, beside it the fixed and inplace columns
  template<unsigned_integer Block, class Allocator = allocator<Block>>
      requires see below
    class basic_bit_valarray;
  using bit_valarray = basic_bit_valarray<size_t>;
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_fixed_valarray;
  template<size_t N>
    using bit_fixed_valarray = basic_bit_fixed_valarray<size_t, N>;
  template<unsigned_integer Block, size_t N>
      requires see below
    class basic_bit_inplace_valarray;
  template<size_t N>
    using bit_inplace_valarray = basic_bit_inplace_valarray<size_t, N>;

  // [class.slice], class slice
  class bit_slice;
  // [template.slice.array], class template slice_array
  template<unsigned_integer Block>
    class bit_slice_array;
  // [class.gslice], class gslice
  class bit_gslice;
  // [template.gslice.array], class template gslice_array
  template<unsigned_integer Block>
    class bit_gslice_array;
  // [template.mask.array], class template mask_array
  template<unsigned_integer Block>
    class bit_mask_array;
  // [template.indirect.array], class template indirect_array
  template<unsigned_integer Block>
    class bit_indirect_array;
}
```

### `basic_bit_valarray`, `bit_valarray`

```cpp
template<unsigned_integer Block, class Allocator>
  requires see below
class basic_bit_valarray {
public:
  using value_type           = bool;
  using reference            = see below;
  using size_type            = size_t;
  using block_container_type = vector<Block, Allocator>;

  // [valarray.cons]
  basic_bit_valarray() = default;
  constexpr explicit basic_bit_valarray(size_t n);
  constexpr basic_bit_valarray(const bool& value, size_t n);
  constexpr basic_bit_valarray(const bool* p, size_type n);
  constexpr basic_bit_valarray(initializer_list<bool> il);
  constexpr basic_bit_valarray(const bit_slice_array<Block>& a);
  constexpr basic_bit_valarray(const bit_gslice_array<Block>& a);
  constexpr basic_bit_valarray(const bit_mask_array<Block>& a);
  constexpr basic_bit_valarray(const bit_indirect_array<Block>& a);
  constexpr basic_bit_valarray(from_blocks_t, block_container_type blocks) noexcept(see below);

  // [valarray.assign]
  constexpr auto operator=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator=(initializer_list<bool> il) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_slice_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_gslice_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_mask_array<Block>& a) -> basic_bit_valarray&;
  constexpr auto operator=(const bit_indirect_array<Block>& a) -> basic_bit_valarray&;

  // [valarray.access]
  constexpr auto operator[](size_type n) noexcept -> reference;
  constexpr auto operator[](size_type n) const noexcept -> bool;

  // [valarray.sub]
  auto operator[](slice s) const;
  auto operator[](slice s) -> bit_slice_array<Block>;
  constexpr auto operator[](bit_slice s) const;
  constexpr auto operator[](bit_slice s) -> bit_slice_array<Block>;
  auto operator[](const gslice& s) const;
  auto operator[](const gslice& s) -> bit_gslice_array<Block>;
  auto operator[](const bit_gslice& s) const;
  auto operator[](const bit_gslice& s) -> bit_gslice_array<Block>;
  template<class Mask>
      requires see below
    constexpr auto operator[](const Mask& mask) const;
  template<class Mask>
      requires see below
    constexpr auto operator[](const Mask& mask) -> bit_mask_array<Block>;
  auto operator[](const valarray<size_t>& indices) const;
  auto operator[](const valarray<size_t>& indices) -> bit_indirect_array<Block>;

  // [valarray.unary]
  constexpr auto operator!() const -> basic_bit_valarray;
  constexpr auto operator~() const -> basic_bit_valarray;

  // [valarray.cassign]
  constexpr auto operator&=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator|=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator^=(const basic_bit_valarray& other) noexcept -> basic_bit_valarray&;
  constexpr auto operator&=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator|=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator^=(const bool& value) noexcept -> basic_bit_valarray&;
  constexpr auto operator<<=(size_type n) noexcept -> basic_bit_valarray&;
  constexpr auto operator>>=(size_type n) noexcept -> basic_bit_valarray&;

  // [valarray.members]
  constexpr auto swap(basic_bit_valarray& other) noexcept(see below) -> void;
  constexpr auto size() const noexcept -> size_type;
  constexpr auto sum() const noexcept -> bool;
  constexpr auto min() const noexcept -> bool;
  constexpr auto max() const noexcept -> bool;
  constexpr auto shift(int n) const -> basic_bit_valarray;
  constexpr auto cshift(int n) const -> basic_bit_valarray;
  constexpr auto apply(bool func(bool)) const -> basic_bit_valarray;
  constexpr auto apply(bool func(const bool&)) const -> basic_bit_valarray;
  constexpr auto resize(size_type n, bool value = false) -> void;

  // [valarray.binary], [valarray.comparison]: for each @ of & | ^ && || == != < > <= >=
  friend constexpr auto operator@(const basic_bit_valarray& x, const basic_bit_valarray& y) -> basic_bit_valarray;
  friend constexpr auto operator@(const basic_bit_valarray& x, const bool& y) -> basic_bit_valarray;
  friend constexpr auto operator@(const bool& x, const basic_bit_valarray& y) -> basic_bit_valarray;
  friend constexpr auto operator<<(const basic_bit_valarray& x, size_type n) -> basic_bit_valarray;
  friend constexpr auto operator>>(const basic_bit_valarray& x, size_type n) -> basic_bit_valarray;

  // [valarray.special], [valarray.range]
  friend constexpr auto swap(basic_bit_valarray& x, basic_bit_valarray& y) noexcept(see below) -> void;
  friend constexpr auto begin(basic_bit_valarray& v) noexcept;
  friend constexpr auto begin(const basic_bit_valarray& v) noexcept;
  friend constexpr auto end(basic_bit_valarray& v) noexcept;
  friend constexpr auto end(const basic_bit_valarray& v) noexcept;
};
```

As [[template.valarray]](https://eel.is/c++draft/template.valarray) for `valarray<bool>`, with:

- `bit_valarray` holds its bools in `size_t` blocks, and `basic_bit_valarray<Block, Allocator>` in `Block`s, in a
  `vector<Block, Allocator>`. The requires-clause admits `Block` as `basic_bit_vector`'s does. There is no
  allocator argument or `get_allocator`.
- `reference` is the sequence reading's proxy, standing in for `T&`; the `const` subscript returns a `bool`. The
  iterators `begin` and `end` return are the sequence reading's, so it is a random-access range with no member
  `begin` or `end`, and `bit_span` views it as `span<bool>` views a `valarray<bool>`.
- The constructor from a value and a count takes the value first, as
  [[valarray.cons]](https://eel.is/c++draft/valarray.cons) orders them. The constructors from a proxy are
  implicit. `resize(n, value)` assigns `value` to every element, as
  [[valarray.members]](https://eel.is/c++draft/valarray.members) says. A size past what `difference_type` can count
  throws `length_error` before anything is allocated.
- **`~` is logical not**, the same as `!`, where `valarray<bool>::operator~` promotes and answers all `true`.
- **The shifts are `std::bitset`'s.** `v <<= n` moves position `i` to `i + n` and `v >>= n` position `i + n` to `i`,
  zeros filling in, each emptying the array where `n >= size()`; `<<` and `>>` return the copy. `shift(n)` keeps
  [[valarray.members]](https://eel.is/c++draft/valarray.members)'s sense, element `i` taking element `i + n`, so
  `v << n` is `v.shift(-n)`. `cshift(n)` takes element `(i + n) mod size()`, and turns an empty array not at all.
- **Empty reductions are defined.** `sum()` and `max()` are the disjunction of the elements, `false` for an empty
  array, and `min()` their conjunction, `true` for an empty array.
- `apply(func)` calls `func(false)` and `func(true)` once each, and applies the answer a block at a time: a constant,
  the identity or the negation.
- **No arithmetic.** `+`, `-`, `*`, `/`, `%`, unary `+` and `-`, their compound forms and the `<cmath>` functions are
  absent.
- **Comparisons answer arrays.** `==`, `!=`, `<`, `>`, `<=` and `>=` return the array of elementwise answers, so the
  type is not `equality_comparable`, and has no `<=>` and no `hash`, as `valarray` has none. `&&` and `||`
  evaluate both operands, and are `&` and `|`.
- *Preconditions:* The two arrays of a binary or compound operator have one size; each subscript's positions lie
  inside the array, and a mask has the array's size; an index array names no position twice for a write. Each is
  checked by an `assert`.
- **The subscripts.** On a `const` array each of the four gathers into a valarray of its own; on a non-const one it
  returns the proxy over the same block type, which refers to the array. A mask is any owner of the valarray reading
  over `Block` of the array's size, held by reference. `v[slice]` forwards to `v[bit_slice]` and `v[gslice]` to
  `v[bit_gslice]`. A slice or a mask gathers and scatters a block at a time, through
  [P3104](https://wg21.link/P3104)'s `bit_compress` and `bit_expand`.
- *`from_blocks` constructor:* As `basic_bit_vector`'s. A valarray has no `replace` or `extract`.
- Each member that returns a valarray is `noexcept` where copying the blocks is, and the `from_blocks` constructor
  where moving them is.
- Every member but the subscripts by `slice`, `gslice`, `bit_gslice` and `valarray<size_t>` is `constexpr`; the
  heap array is usable in a constant expression as `vector` is.

### `basic_bit_fixed_valarray`, `bit_fixed_valarray`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
class basic_bit_fixed_valarray {
public:
  // as basic_bit_valarray, with
  using block_container_type = array<Block, see below>;

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
  `initializer_list` constructor sets the leading positions and leaves the rest `false`; the built-in array and
  `from_blocks` constructors are `basic_bit_array`'s. `size()` is a function.
- *Preconditions:* The pointer constructor's `n`, and the size a proxy selects for its constructor, is `N`.
- A `const` subscript gathers into `basic_bit_inplace_valarray<Block, N>`, where `Block` admits one, and throws
  `bad_alloc` where a gather selects more than `N` positions.
- Its `constexpr` members are usable in a constant expression, the subscripts by a `bit_slice` and by a mask
  included, and it is a structural type exactly where `N` fills its blocks.

### `basic_bit_inplace_valarray`, `bit_inplace_valarray`

As `basic_bit_valarray`, at a size within a capacity of `N`, with:

- `bit_inplace_valarray<N>` holds up to `N` bools in `size_t` blocks, inline. The requires-clause admits `Block` as
  `basic_bit_inplace_vector`'s does, and `block_container_type` is that class's.
- A size past `N` throws `bad_alloc`, and a refused `resize` leaves the array as it was.
- It is usable in a constant expression exactly where `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined.

### `bit_slice`

```cpp
class bit_slice {
public:
  bit_slice() = default;
  constexpr bit_slice(size_t start, size_t size, size_t stride) noexcept;
  explicit bit_slice(const slice& s) noexcept;

  constexpr auto start() const noexcept -> size_t;
  constexpr auto size() const noexcept -> size_t;
  constexpr auto stride() const noexcept -> size_t;

  friend auto operator==(const bit_slice&, const bit_slice&) -> bool = default;
};
```

- As [[class.slice]](https://eel.is/c++draft/class.slice), usable in a constant expression, where `slice`'s
  constructors are not, so `v[bit_slice(start, size, stride)]` runs in one.
- It selects what the `slice` of the same three numbers selects.

### `bit_gslice`

```cpp
class bit_gslice {
public:
  bit_gslice();
  bit_gslice(size_t start, const valarray<size_t>& lengths, const valarray<size_t>& strides);
  explicit bit_gslice(const gslice& g);

  auto start() const noexcept -> size_t;
  auto size() const -> valarray<size_t>;
  auto stride() const -> valarray<size_t>;
};
```

- As [[class.gslice]](https://eel.is/c++draft/class.gslice), with its positions worked out once, in its constructor,
  and kept for every subscript that reuses it: as one bit per position where they ascend, and as the positions
  themselves where they repeat or turn back.
- It selects what the `gslice` of the same numbers selects, over every owner and block.
- Copies share the positions, and a proxy holds a copy, so a `bit_gslice_array` outlives the `bit_gslice` it was
  made from.

### `bit_slice_array`, `bit_gslice_array`, `bit_mask_array`, `bit_indirect_array`

```cpp
template<unsigned_integer Block>
class bit_slice_array {
public:
  using value_type = bool;

  bit_slice_array() = delete;
  bit_slice_array(const bit_slice_array&) = default;
  ~bit_slice_array() = default;

  constexpr auto operator=(const bit_slice_array& other) const -> const bit_slice_array&;
  template<class A>
      requires see below
    constexpr auto operator=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator&=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator|=(const A& a) const -> void;
  template<class A>
      requires see below
    constexpr auto operator^=(const A& a) const -> void;
  constexpr auto operator=(const bool& value) const -> void;
  constexpr auto operator&=(const bool& value) const -> void;
  constexpr auto operator|=(const bool& value) const -> void;
  constexpr auto operator^=(const bool& value) const -> void;
};
```

As [[template.slice.array]](https://eel.is/c++draft/template.slice.array),
[[template.gslice.array]](https://eel.is/c++draft/template.gslice.array),
[[template.mask.array]](https://eel.is/c++draft/template.mask.array) and
[[template.indirect.array]](https://eel.is/c++draft/template.indirect.array) at `T = bool`. The four have the members
shown for `bit_slice_array`, with:

- Only a non-const subscript of a valarray over `Block` makes one. It refers to the array's blocks, and a copy refers
  to the same elements; it must not outlive the array, or a mask or index array it was made from.
- Assigning one proxy to another assigns the elements, never the reference.
- *Constraints:* `A` is an owner of the valarray reading over `Block`, of any storage, or one of the four proxies
  over `Block`. *Preconditions:* `a`'s size is the number of positions selected.
- *Effects:* The elements of `a`, in order, are written to the selected positions, or combined with them by `&`,
  `|` or `^`. A proxy `a` gathers what it selects before any position is written, so `v[s] = v[t]` reads `v` first.
  A `bool` is written to, or combined with, every selected position.
- The compound assignments from a `bool` are additions; the arithmetic compound assignments and the shifts are
  absent.
- A slice of stride zero and a gslice that repeats a position write one position at a time, in order, so the last
  write to a position stands.

## `<xstd/bits/bit_span.hpp>`

```cpp
namespace xstd {
  // [views.span], class template span
  template<bit_blocks Blocks, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_span;
  // beside it: the window its first, last and subspan return
  template<bit_blocks Blocks, size_t Extent = dynamic_extent, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_subspan;
}

namespace std::ranges {
  template<class Blocks, size_t N>
    constexpr bool enable_view<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_view<xstd::bit_subspan<Blocks, Extent, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_subspan<Blocks, Extent, N>> = true;
}

namespace boost::container_hash {
  template<class Blocks, size_t N>
    struct is_range<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t N>
    struct is_tuple_like<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_range<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_tuple_like<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
}
```

### `bit_span`

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

### `bit_subspan`

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

## `<xstd/bits/ext/boost.hpp>`

```cpp
namespace xstd {
  // the sequence reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_vector;
  template<size_t N>
    using bit_small_vector = basic_bit_small_vector<size_t, N>;

  // the valarray reading
  template<unsigned_integer Block, size_t N, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_valarray;
  template<size_t N>
    using bit_small_valarray = basic_bit_small_valarray<size_t, N>;

  // the set reading
  template<class Key, unsigned_integer Block, size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>,
           see below Compare = less<Key>, class Alloc = boost::container::new_allocator<Block>>
      requires see below
    class basic_bit_small_set;
  template<size_t N>
    using bit_small_set = basic_bit_small_set<size_t, size_t, N>;
}

namespace std {
  template<class Block, size_t N, class Alloc>
    struct hash<xstd::basic_bit_small_vector<Block, N, Alloc>>;
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct hash<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>>;
}

namespace boost::container_hash {
  template<class Block, size_t N, class Alloc>
    struct is_range<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Block, size_t N, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_vector<Block, N, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_range<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
  template<class Key, class Block, size_t N, class KeyMapping, class Compare, class Alloc>
    struct is_tuple_like<xstd::basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>> : false_type {};
}
```

The three readings over the small storage, `boost::container::small_vector<Block, K, Alloc>` for `K = ceil(N / D)`:
inline up to `N` bits, and on the heap past them. `<xstd/bits.hpp>` does not include this header, so the dependency
on Boost.Container's `small_vector` is opted into by name. It also exports the sequence reading's `erase` and
`erase_if` and the set reading's `erase_if`. `<xstd/bits/ext/boost/dynamic_bitset.hpp>`, which converts to and from
`boost::dynamic_bitset`, is outside this part.

### `basic_bit_small_vector`, `bit_small_vector`

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

### `basic_bit_small_set`, `bit_small_set`

As `basic_bit_set`, over the small storage, with:

- `bit_small_set<N>` holds its keys in `size_t` blocks, inline below `N`. *Mandates:* as `basic_bit_fixed_set`'s.
- `allocator_type` and the allocator arguments are as for `basic_bit_small_vector`; the constructor from an
  allocator is `noexcept`, and the `from_blocks` constructor `noexcept` where the block container's move is.
- A key past the inline capacity moves the blocks to the heap, keeping the keys and their count. There are no
  deduction guides.

### `basic_bit_small_valarray`, `bit_small_valarray`

As `basic_bit_valarray`, over the small storage, with:

- `bit_small_valarray<N>` holds its bools in `size_t` blocks, inline up to `N` of them, and `Alloc` defaults to
  Boost's own. Past the inline capacity the blocks move to the heap, and the elements stay what they were.
- A `const` subscript gathers into a `basic_bit_small_valarray` of the same parameters.

## `<xstd/bits/bit_algorithm.hpp>`

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

### `bit_all_of`, `bit_any_of`, `bit_none_of`

As `ranges::all_of`, `ranges::any_of` and `ranges::none_of` in
[[alg.all.of]](https://eel.is/c++draft/alg.all.of), [[alg.any.of]](https://eel.is/c++draft/alg.any.of) and
[[alg.none.of]](https://eel.is/c++draft/alg.none.of), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* Whether every, some or no position of `r` is `true`: `true`, `false` and `true` on an empty sequence.
- *Complexity:* O(`N`/`D`). A whole sequence asks its storage; a window asks the blocks it covers, its end blocks
  masked to its own positions.

### `bit_for_each`

As `ranges::for_each` in [[alg.foreach]](https://eel.is/c++draft/alg.foreach), with:

- *Constraints:* `invocable<F&, ranges::range_value_t<R>>` is modeled, and:
  - for the first form, `R` is a sequence or a set of ours, a window included;
  - for the second, `remove_cvref_t<R>` is `ranges::reverse_view<V>`, where `V` is a view of ours or a
    `ranges::ref_view` of an owner of ours: what `views::reverse` makes of a view or of an lvalue owner.
- *Effects:* Calls `f` with each element by value, a `bool` or a key, in the order `r` iterates. The second form
  walks the range under the `reverse_view` against its order, so `bit_for_each(views::reverse(r), f)` is
  `ranges::for_each(views::reverse(r), f)`.
- *Returns:* `{ranges::end(r), std::move(f)}`.
- *Complexity:* Exactly `ranges::distance(r)` applications of `f`, and one load per block. A set's keys come from
  each block lowest first or highest first, as the set and the direction order them.
- A functor that takes the element as a non-`const` lvalue reference is turned away, and of a functor overloaded on
  value category the overload taking an rvalue is the one called.
- A functor returning `bool` is called on every element, its answer ignored, as `ranges::for_each` calls it.

### `bit_find`, `bit_find_last`

As `ranges::find` and `ranges::find_last` in [[alg.find]](https://eel.is/c++draft/alg.find) and
[[alg.find.last]](https://eel.is/c++draft/alg.find.last), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_find`, `ranges::begin(r) + i`, where `i` is the first position holding `value`, or
  `ranges::size(r)` where none does. For `bit_find_last`, `{ranges::begin(r) + i, ranges::end(r)}`, where `i` is
  the last such position, or `{ranges::end(r), ranges::end(r)}` where none is.
- *Complexity:* O(`N`/`D`). `bit_find` reads up from the first block and `bit_find_last` down from the last, each
  stopping at the block that holds the answer. A search for `false` reads the blocks complemented, at the same cost.

### `bit_count`

As `ranges::count` in [[alg.count]](https://eel.is/c++draft/alg.count), with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* The number of positions of `r` holding `value`, as `ranges::range_difference_t<const R>`, signed as
  `ranges::count`'s result is: the ones popcounted, and the zeros as `ranges::size(r)` less the ones.
- *Complexity:* O(`N`/`D`) popcounts. The padding of the last block is never counted.

### `bit_mismatch`

As `ranges::mismatch` in [[mismatch]](https://eel.is/c++draft/mismatch), with:

- *Constraints:* `R1` is a sequence of ours, and `remove_cvref_t<R1>` and `remove_cvref_t<R2>` are the same type:
  `bit_array<8>` and `bit_array<9>` are turned away.
- *Effects:* Two whole sequences whose storage compares its blocks are compared a block at a time, at two run-time
  sizes as well, stopping at the shorter's end, so set bits in the longer where the shorter has padding do not
  count. Two windows are compared by `ranges::mismatch(r1, r2)` itself.
- *Complexity:* O(min(`N1`, `N2`)/`D`) for two whole sequences; for two windows, `ranges::mismatch`'s.

### `bit_equal`

As `ranges::equal` in [[alg.equal]](https://eel.is/c++draft/alg.equal), with:

- *Constraints:* `R1` and `R2` are of one reading and one width kind:
  - both sequences of ours, or both sets of ours with the same `key_type`, `key_mapping_type` and `key_compare`;
  - the width their types fix, a window's extent where it has one and otherwise its storage's, is the same, or both
    widths are run-time values.

  So `bit_array<8>` compares with `basic_bit_array<uint8_t, 8>` and `bit_vector` with `basic_bit_vector<uint8_t>`,
  and `bit_array<8>` does not compare with `bit_vector`, with `bit_array<9>` or with `bit_fixed_set<8>`, nor
  `bit_fixed_set<8>` with `bit_set` or with a set of the same keys under `greater<>`.
- *Returns:* `ranges::equal(r1, r2)` over the elements in iteration order: for two sequences, whether the sizes and
  every position agree; for two sets, whether they hold the same keys, at any two widths.
- *Complexity:* Two whole operands over one storage type compare their own blocks, in O(`N`/`D`). Any other pair
  compares 64-bit words assembled from each side's blocks at each side's offset, in O(`N1`/`D1` + `N2`/`D2`).
- *Remarks:* The constraints admit exactly the pairs `bit_hash` hashes alike where their values are equal, so what
  compares equal hashes equal. `bit_equal_to` is the transparent comparison over it.
- A capacity of zero against a run-time width that holds bools compares their sizes.

### `bit_copy`, `bit_copy_backward`, `bit_move`

As `ranges::copy` and `ranges::copy_backward` in [[alg.copy]](https://eel.is/c++draft/alg.copy), and `ranges::move`
in [[alg.move]](https://eel.is/c++draft/alg.move), with:

- *Constraints:* `R` is a sequence of ours, a window included, and `O` is an output iterator of ours.
- *Preconditions:* Where the destination lies in the blocks `r` reads, `out`, or for `bit_copy_backward` `out_last`,
  is not in `(ranges::begin(r), ranges::end(r))`. These are [[alg.copy]](https://eel.is/c++draft/alg.copy)'s,
  except that a destination exactly on its source is admitted.
- *Effects:* The `N` positions of `r` are written a block of the destination at a time: lowest first for `bit_copy`
  and `bit_move`, highest first for `bit_copy_backward`. Source and destination may sit at any offsets in storages
  of different block types; the source's bits are assembled from its own blocks where the two differ, and each
  destination block is written under the mask of the positions that remain, so no position outside the destination
  range changes. Moving a `bool` is copying it.
- *Complexity:* O(`N`/`D`), where `D` is the digits of the narrower of the two blocks.

### `bit_swap_ranges`

As `ranges::swap_ranges` in [[alg.swap]](https://eel.is/c++draft/alg.swap), with:

- *Constraints:* `R1` and `R2` are writable sequences of ours, windows included.
- *Effects:* Exchanges the first `M` = min(`ranges::size(r1)`, `ranges::size(r2)`) positions of the two, a block of
  the narrower block type at a time, at any two offsets.
- *Complexity:* O(`M`/`D`), where `D` is the digits of the narrower block.

### `bit_transform`

As `ranges::transform` in [[alg.transform]](https://eel.is/c++draft/alg.transform), with:

- *Constraints:* `R`, or `R1` and `R2`, are sequences of ours, windows included; `O` is an output iterator of ours;
  `invocable<F&, bool>`, or `invocable<F&, bool, bool>`, is modeled and its result converts to `bool`. A function
  taking `bool&`, or answering a `string`, is turned away.
- *Effects:* The unary form calls `f(false)` and `f(true)` once each; the binary form calls `f` once at each of the
  four pairs. The answers make a truth table that is applied as one block expression, a block of the destination at
  a time as `bit_copy` writes it: for the unary form `(x & T) | (~x & U)`, where `T` and `U` are all ones or all
  zeros as `f(true)` and `f(false)` answer. The binary form goes as far as the shorter input. `out` may be
  `ranges::begin(r1)`, as [[alg.transform]](https://eel.is/c++draft/alg.transform) allows.
- *Complexity:* Exactly two applications of `f` for the unary form and four for the binary, whatever `N`, an empty
  input included; O(`N`/`D`) block operations.
- *Remarks:* The positions written are those `ranges::transform` writes; a function with side effects sees two or
  four calls where `ranges::transform` makes one per element.

### `bit_fill`

As `ranges::fill` in [[alg.fill]](https://eel.is/c++draft/alg.fill), with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* A whole sequence fills every block of its storage; a window sets its own positions, its end blocks
  masked, and leaves every other position as it was.
- *Complexity:* O(`N`/`D`).

### `bit_reverse`

As `ranges::reverse` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* `R` is a whole sequence of ours whose storage is writable: an owner that is not `const`, or a whole
  view of bits that are not `const`. A window, which shares its end blocks with positions outside it, is turned
  away.
- *Effects:* Position `i` takes position `N - 1 - i`: the blocks in reverse order, each block's bits reversed, and
  the padding, which then lies at the bottom, shifted out. The padding stays clear.
- *Returns:* `ranges::end(r)`.
- *Complexity:* O(`N`/`D`), without allocating.

### `bit_reverse_copy`

As `ranges::reverse_copy` in [[alg.reverse]](https://eel.is/c++draft/alg.reverse), with:

- *Constraints:* As for `bit_copy`.
- *Effects:* Each block of the destination is read from the top of `r`, its bits reversed, and written as `bit_copy`
  writes it.
- *Complexity:* As for `bit_copy`.

### `bit_rotate`

As `ranges::rotate` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Position `i` takes position (`i` + `m`) mod `N`, for `m = middle - ranges::begin(r)`, so a whole turn
  is no turn. The blocks rotate by whole blocks, then a funnel shift moves the remaining bits across each boundary.
- *Returns:* `{ranges::begin(r) + (ranges::end(r) - middle), ranges::end(r)}`.
- *Complexity:* O(`N`/`D`), without allocating.
- *Remarks:* It moves no proxy through the standard library, so it is exact over proxy references where libstdc++'s
  `ranges::rotate` before version 16 is not ([GCC PR 121913](https://gcc.gnu.org/PR121913)). The `rotr(n)` of
  [P3103R2](https://wg21.link/P3103R2) is `bit_rotate(v, v.begin() + n)`, and its `rotl(n)` is
  `bit_rotate(v, v.end() - n)`.

### `bit_rotate_copy`

As `ranges::rotate_copy` in [[alg.rotate]](https://eel.is/c++draft/alg.rotate), with:

- *Constraints:* As for `bit_copy`.
- *Preconditions:* `middle` is in `[ranges::begin(r), ranges::end(r)]`, checked by an `assert`.
- *Effects:* Copies `[middle, ranges::end(r))` and then `[ranges::begin(r), middle)`, each as `bit_copy` does.
- *Complexity:* As for `bit_copy`.

### `bit_shift_left`, `bit_shift_right`

As `ranges::shift_left` and `ranges::shift_right` in [[alg.shift]](https://eel.is/c++draft/alg.shift), with:

- *Constraints:* As for `bit_reverse`.
- *Preconditions:* `0 <= n`, checked by an `assert`.
- *Effects:* None where `n == 0` or `n >= N`. Otherwise `bit_shift_left` moves position `i + n` to `i`, and
  `bit_shift_right` position `i` to `i + n`, for every `i < N - n`. The `n` positions vacated are `false`, where
  [[alg.shift]](https://eel.is/c++draft/alg.shift) leaves them valid but unspecified.
- *Returns:* As [[alg.shift]](https://eel.is/c++draft/alg.shift): for `bit_shift_left`,
  `{ranges::begin(r), ranges::begin(r) + (N - n)}` where `n < N` and `{ranges::begin(r), ranges::begin(r)}`
  otherwise; for `bit_shift_right`, `{ranges::begin(r) + n, ranges::end(r)}` where `n < N` and
  `{ranges::end(r), ranges::end(r)}` otherwise.
- *Complexity:* O(`N`/`D`).
- *Remarks:* A sequence's front is a bit string's low end, so `bit_shift_left` is the storage's `>>=` and
  `bit_shift_right` its `<<=`.

### `bit_sort`

As `ranges::sort` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting), `false` ordered before `true`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `N - k` positions `false` and the last `k`
  `true`. Every `false` is like every other, so the result is `ranges::stable_sort`'s too.
- *Complexity:* O(`N`/`D`).

### `bit_is_sorted`, `bit_is_partitioned`

As `ranges::is_sorted` in [[alg.sorting]](https://eel.is/c++draft/alg.sorting) and `ranges::is_partitioned` in
[[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Returns:* For `bit_is_sorted`, whether no `false` follows the first `true`; for `bit_is_partitioned`, whether no
  `true` follows the first `false`.
- *Complexity:* O(`N`/`D`): two searches, the second from where the first stopped.

### `bit_partition`

As `ranges::partition` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a writable sequence of ours, a window included.
- *Effects:* Counts the `k` positions that are `true`, then sets the first `k` positions `true` and the rest
  `false`.
- *Returns:* `{ranges::begin(r) + k, ranges::end(r)}`.
- *Complexity:* O(`N`/`D`).

### `bit_partition_point`

As `ranges::partition_point` in [[alg.partitions]](https://eel.is/c++draft/alg.partitions), over `identity()`, with:

- *Constraints:* `R` is a sequence of ours, a window included.
- *Preconditions:* `r` is partitioned, as [[alg.partitions]](https://eel.is/c++draft/alg.partitions) requires.
- *Returns:* `ranges::begin(r) + i`, where `i` is the first position that is `false`, or `ranges::size(r)`.
- *Complexity:* O(`i`/`D`): a search up to the block holding the first `false`, where
  [[alg.partitions]](https://eel.is/c++draft/alg.partitions) bisects.

### `bit_includes`, `bit_intersects`, `bit_disjoint`

As `ranges::includes` in [[includes]](https://eel.is/c++draft/includes), and as whether `ranges::set_intersection`
in [[set.intersection]](https://eel.is/c++draft/set.intersection) would write a key, or none, with:

- *Constraints:* `S` is a set of ours, a view included. The second parameter is `const type_identity_t<S>&`, so `S`
  deduces from `s1` alone and `s2` converts to it: a flag set takes a flag second,
  `bit_includes(perms, fs::perms::owner_read)`, and `bit_fixed_set<8>` with `bit_fixed_set<9>` is turned away.
- *Returns:*
  - `bit_includes`: whether every key of `s2` is a key of `s1`, as `ranges::includes(s1, s2, s1.key_comp())`;
  - `bit_intersects`: whether some key is a key of both;
  - `bit_disjoint`: `not bit_intersects(s1, s2)`.
- *Complexity:* O(`K`), where `K` is the number of blocks of the wider. The blocks both storages hold are compared
  pairwise, and `bit_includes` also asks whether `s2` holds a key in the blocks above `s1`'s. A width of zero
  fixed in the type is answered without a block: `true` for `bit_includes`, `false` for `bit_intersects`.
- They answer at any two run-time widths.
- *Remarks:* `x` is a subset of `y` where `bit_includes(y, x)`, and a proper one where `not bit_includes(x, y)` as
  well; every other relation between two sets is spelled from these three.

### `bit_set_union`, `bit_set_intersection`, `bit_set_difference`, `bit_set_symmetric_difference`

As `ranges::set_union`, `ranges::set_intersection`, `ranges::set_difference` and
`ranges::set_symmetric_difference` in [[set.union]](https://eel.is/c++draft/set.union),
[[set.intersection]](https://eel.is/c++draft/set.intersection),
[[set.difference]](https://eel.is/c++draft/set.difference) and
[[set.symmetric.difference]](https://eel.is/c++draft/set.symmetric.difference), returning the set rather than
writing through an output iterator, with:

- *Constraints:* `S` is a set of ours whose operator `|`, `&`, `-` or `^` respectively answers an `S`: an owner. A
  view, which has no result to hold, is turned away. `s2` converts to `S`, as for `bit_includes`.
- *Returns:* `s1 | s2`, `s1 & s2`, `s1 - s2` and `s1 ^ s2`: the set that
  [range-v3](https://github.com/ericniebler/range-v3)'s `views::set_union(s1, s2, s1.key_comp())` and its siblings
  would hold, of the first argument's type, whatever widths held the keys.
- *Complexity:* That of the operator.
- *Remarks:* The exception specification is `noexcept(declval<const S&>() | declval<const S&>())`, with the
  operator each one calls.

### `bit_lexicographical_compare_three_way`

As `lexicographical_compare_three_way` in [[alg.three.way]](https://eel.is/c++draft/alg.three.way), over two ranges
rather than two iterator pairs, the shape [P2022R3](https://wg21.link/P2022R3) proposes for
`ranges::lexicographical_compare_three_way`, with:

- *Constraints:* As for `bit_equal`.
- *Returns:* `lexicographical_compare_three_way(ranges::begin(r1), ranges::end(r1), ranges::begin(r2),
  ranges::end(r2))`, a `strong_ordering`. Over two sequences, `false` precedes `true` and a prefix precedes the
  longer sequence. Over two sets it is `set`'s order, the keys compared lexicographically in iteration order:
  ascending keys position by position, and descending ones as their masks compared as numbers.
- *Complexity:* As for `bit_equal`.
- *Remarks:* `lexicographical_compare(r1, r2)` is `bit_lexicographical_compare_three_way(r1, r2) < 0`.

## `<xstd/bits/bit_iterator.hpp>`

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

The iterator operations over any iterator of ours, each in a header of its own under `<xstd/bits/bit_iterator/>`. A
sequence's iterator is random access and steps by an offset, so over it each is its `std::ranges` counterpart. A
set's iterator is bidirectional and steps a key at a time, so over it each skips whole blocks by their popcount and
selects within the block that holds its target: select is `*bit_next(s.begin(), n)`, and the rank of `k` is
`bit_distance(s.begin(), s.lower_bound(k))`.

An *iterator of ours* is an iterator of a sequence or a set of ours, `const` or not, a view's or a window's
included; `vector<bool>::iterator` and `set<size_t>::iterator` are turned away. Only the forms that take a count are
provided: there is no `bit_advance(it, bound)`, `bit_next(it)`, `bit_next(it, bound)` or `bit_prev(it)`.

### `bit_advance`

As `ranges::advance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours.
- *Preconditions:* As [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops)'s. For the first form over a set,
  the `n` keys are there to step over, checked by an `assert`.
- *Effects:* Over a sequence, `ranges::advance(it, n)` and `ranges::advance(it, n, bound)`. Over a set, moves `it`
  `n` keys on in the set's order, or `-n` back, skipping whole blocks by their popcount and selecting the key within
  the block that holds it, from the block's top where the walk goes down; the second form stops at `bound`.
- *Returns:* For the second form, the steps not taken, with the sign of `n`, as `ranges::advance(it, n, bound)`
  answers them.
- *Complexity:* Constant over a sequence. Over a set, linear in the number of blocks between `it` and where it
  stops, rather than one step per key.
- A view's iterators step over the keys it views, its owner's or lent blocks, and an empty set's `begin()` is its
  `end()`.

### `bit_distance`

As `ranges::distance` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* For the first form, `I` is an iterator of ours. For the second, `R` models `ranges::common_range`
  and its iterator is an iterator of ours.
- *Preconditions:* `[first, last)` denotes a range.
- *Returns:* For the first form, `ranges::distance(first, last)` over a sequence, and over a set the number of keys
  in `[first, last)`. For the second, `ranges::distance(r)` where `R` models `ranges::sized_range`, as a set does,
  and otherwise `bit_distance(ranges::begin(r), ranges::end(r))`: so
  `bit_distance(subrange(s.lower_bound(a), s.lower_bound(b)))` counts the keys in `[a, b)`.
- *Complexity:* Constant over a sequence and over a sized range. Over a set's iterators, linear in the number of
  blocks between them: a popcount of the whole blocks and of the masked ends.
- *Remarks:* Counting a set's keys is a distance and not a count: `ranges::count(s, k)` asks how often `k` occurs,
  which `s.contains(k)` answers.

### `bit_next`, `bit_prev`

As `ranges::next` and `ranges::prev` in [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops), with:

- *Constraints:* `I` is an iterator of ours, and for `bit_prev` it models `bidirectional_iterator`, as
  `ranges::prev` asks.
- *Effects:* `bit_next` calls `bit_advance(it, n)`, or `bit_advance(it, n, bound)`, on its copy `it`; `bit_prev`
  calls them with `-n`.
- *Returns:* `it`.
- *Complexity:* As for `bit_advance`.

## `<xstd/bits/bit_concepts.hpp>`

```cpp
namespace xstd {
  // what holds the bits: one block, or a sized contiguous range of them
  template<class Bits>
    concept bit_blocks = see below;

  // what a set owner asks of the mapping that places its keys in bits
  template<class Mapping, class Key>
    concept bit_index_mapping = see below;
  template<class Mapping, class Key>
    concept sized_bit_index_mapping = see below;
  template<class Mapping, class Key>
    concept bit_mask_mapping = see below;

  // blocks an owner takes as they are, and positions converted between any two widths
  template<class To, class Blocks>
    concept bit_constructible_from = see below;
  template<class From, class To>
    concept bit_convertible_to = see below;
}
```

The umbrella exports each header below, in the three groups its comments mark. A concept whose definition names an
exposition-only concept of the library's `detail` namespace writes `see below` there, and the bullets under it say
what that concept asks. The three mapping concepts are cv-transparent in `Key`, as xstd-ints's concepts are: a
cv-qualified key is asked about as the bare one.

### `bit_blocks`

```cpp
template<class Bits>
concept bit_blocks = see below;
```

- `Bits` models it where it is either of:
  - a *block*: a type that models xstd-ints's `unsigned_integer`, `const` or not, and is not `volatile`;
  - a *range of blocks*: `Bits` models `ranges::sized_range` and `ranges::contiguous_range`,
    `remove_reference_t<ranges::range_reference_t<Bits>>` is a block, and `blocks[n]` is valid for an lvalue
    `blocks` of type `Bits` and an `n` of type `ranges::range_size_t<Bits>`. The block is asked of the reference
    type, as `ranges::range_value_t` drops the cv.
- A block models it: `uint8_t`, `uint64_t`, `const uint64_t`.
- A range of blocks models it: `array<uint16_t, 3>`, `vector<size_t>`, `span<uint32_t>`, `span<const uint32_t, 2>`,
  `inplace_vector<uint16_t, 3>` where the library provides it, and the built-in arrays `uint64_t[4]` and
  `const uint16_t[3]`.
- `vector<bool>` does not, nor do `deque<uint32_t>` and `list<uint32_t>`, which are not contiguous.
- `bitset<64>`, `bit_set` and `bit_array<64>` do not: each *has* bit storage and is not bit storage.
- `volatile uint32_t`, `const volatile uint32_t`, `span<volatile uint32_t>` and `volatile uint32_t[2]` are turned
  away. A register's accesses must each stay one volatile access, and the block algorithms copy and merge them.
- *Remarks:* It is the constraint on the storage `bit_set_view` and `bit_span` view and on `bit_blocks_extent_v`,
  so a register array is turned away at the view's name rather than inside a block algorithm. An owner holds its
  blocks by value and takes only a cv-unqualified block: `basic_bit_array<const uint32_t, 32>` is not a type.

### `bit_index_mapping`

```cpp
template<class Mapping, class Key>
concept bit_index_mapping = requires (remove_cv_t<Key> key, size_t index) {
  { Mapping::to_index(key) } -> same_as<size_t>;
  { Mapping::from_index(index) } -> same_as<remove_cv_t<Key>>;
};
```

- A type models it if `to_index` preserves order, `a < b` exactly where `to_index(a) < to_index(b)`, and
  `from_index` inverts `to_index` on the keys the mapping places.
- `bit_key_mapping<Key>` models it for every unsigned block type `Key`.
- `bit_range_mapping<float_round_style, round_indeterminate, 5>`, `bit_find_mapping<int, Keys>` over a sorted list
  with gaps, `bit_flag_mapping<launch>` and `bit_flag_mapping<byte>` model it for their keys, and so does a
  program's specialization of `bit_key_mapping` for a strong index type.
- A mapping does not model it for a key of another type, however alike in width, signedness or header:
  `bit_key_mapping<size_t>` for `unsigned char`, `bit_range_mapping<int, -50, 100>` for `long`,
  `bit_flag_mapping<byte>` for `uint8_t`, and `bit_flag_mapping<filesystem::perms>` for `filesystem::perm_options`.
- `from_index` must answer exactly `Key`: a mapping for a wider key takes a narrower one in through `to_index`, and
  hands back a value of the wrong type.
- `bit_key_mapping<size_t>` models it for `const size_t` and `volatile size_t`, and `bit_flag_mapping<launch>` for
  `const volatile launch`, but `bit_key_mapping<size_t>` does not for `const unsigned char`.

### `sized_bit_index_mapping`

```cpp
template<class Mapping, class Key>
concept sized_bit_index_mapping = bit_index_mapping<Mapping, Key> and requires (remove_cv_t<Key> key) {
  { Mapping::size } -> convertible_to<size_t>;
  { Mapping::is_key(key) } -> same_as<bool>;
};
```

- A type models it if it models `bit_index_mapping<Mapping, Key>`, places exactly `Mapping::size` keys at the
  positions `0` to `size - 1`, and `is_key(k)` is whether `k` is one of them.
- `bit_range_mapping<int, -50, 100>` and `bit_range_mapping<float_round_style, round_indeterminate, 5>` model it,
  as do `bit_find_mapping<int, Keys>`, `bit_key_mapping` of an enumeration listed in `enum_traits`, dense or with
  gaps, `bit_flag_mapping<launch>` and `bit_flag_mapping<byte>`, and a program's mapping that offsets a strong
  index into a closed range.
- `bit_key_mapping<Key>` for an unsigned `Key` does not: the identity leaves the universe open, though it says
  which values are keys. Neither does a program's `bit_key_mapping` for a strong index that names no size.
- A size alone does not make every value below it a key: a list of keys with gaps leaves values between them,
  which `is_key` turns away.
- *Remarks:* A set whose mapping models it closes its universe at `Mapping::size` positions.

### `bit_mask_mapping`

```cpp
template<class Mapping, class Key>
concept bit_mask_mapping =
  sized_bit_index_mapping<Mapping, Key> and
  see below and
  requires (remove_cv_t<Key> mask, Mapping::block_type block) {
    { Mapping::to_block(mask) } -> same_as<typename Mapping::block_type>;
    { Mapping::from_block(block) } -> same_as<remove_cv_t<Key>>;
  };
```

- `see below` is that `typename Mapping::block_type` models `bit_blocks` and `bit_blocks_extent_v` of it is not
  `dynamic_extent`: one unsigned block, or a fixed number of them. It is never the signed type under an
  enumeration such as `launch`.
- A type models it if `to_block(mask)` holds every bit of `mask` at its position and `from_block(block)` is the
  mask with those bits, so that every value of `Key`, one bit or several, is a set of its one-bit keys.
- `bit_flag_mapping<byte>`, `bit_flag_mapping<byte, 2>`, `bit_flag_mapping<launch>` and
  `bit_flag_mapping<bitset<16>>` model it: `bit_flag_mapping<byte>::to_block(byte{0x02})` is `0x02`, and
  `from_block(0x03)` is `byte{0x03}`.
- `bit_key_mapping<size_t>`, `bit_range_mapping<uint8_t, 0, 8>` and `bit_key_mapping` of a listed enumeration do
  not: each ranks its values rather than reading one as a mask.
- `bit_flag_mapping<byte>` does not for `uint8_t`, the same bits under another type, nor for `const uint8_t`; it
  does for `const byte`, and `bit_flag_mapping<launch>` for `const volatile launch`.
- *Remarks:* A fixed set whose mapping models it reads every value of its key as a mask and converts to and from
  it; `bit_flag_set` is the alias over such a set.

### `bit_constructible_from`

```cpp
template<class To, class Blocks>
concept bit_constructible_from =
  see below and
  constructible_from<To, from_blocks_t, Blocks>;
```

- `see below` is that `remove_cvref_t<Blocks>` is bit storage a container can own: it models `bit_blocks` and
  `regular`, and it is an unsigned integer or a range whose subscript yields `ranges::range_reference_t<Blocks>`
  through a mutable object and `ranges::range_const_reference_t<Blocks>` through a `const` one, the alias of
  [P2278R4](https://wg21.link/P2278R4). A storage whose `const` subscript yields a writable reference is refused.
- A type models it if `To(from_blocks, b)` takes the blocks `b` as `To`'s bits, position `i` being bit `i` of `b`.
- An owner's own block container at a run-time width models it: `bit_vector` and `bit_set` from `vector<size_t>`,
  and `bit_inplace_set<100>` from its `block_container_type`.
- Blocks holding at least a fixed width model it: `bit_fixed_set<64>` from `array<uint64_t, 1>`, and
  `bit_array<20>` from `array<uint8_t, 3>`.
- Another block type does not, `bit_vector` from `vector<uint8_t>`, nor does a field too narrow,
  `bit_fixed_set<256>` from `array<uint64_t, 3>`, nor a container an inplace owner does not hold,
  `bit_inplace_set<100>` from `array<uint64_t, 2>` and `bit_inplace_vector<100>` from `vector<size_t>`.
- What has bit storage without being it does not: `bit_fixed_set<64>` from `bitset<64>`, and `bit_vector` from
  `bit_set`. Each converts through `bit_convert` instead.
- A view owns nothing to take blocks into: `bit_set_view<uint64_t>` from `uint64_t` does not model it.
- A built-in array is read and never owned: `bit_fixed_set<64>` is constructible from `from_blocks` and a
  `const uint32_t(&)[2]`, and `bit_constructible_from<bit_fixed_set<64>, uint64_t[1]>` is `false`.

### `bit_convertible_to`

```cpp
template<class From, class To>
concept bit_convertible_to = requires (From&& from) {
  { bit_convert<To>(std::forward<From>(from)) } -> same_as<remove_cv_t<To>>;
};
```

- It holds exactly where `bit_convert<To>(from)` is valid: the positions map, though a source of run-time width may
  still throw. The source comes first, as in `convertible_to`, so `bit_convertible_to<To> auto` constrains a source.
- A fixed width converts into one at least as wide: `bit_fixed_set<64>` to `uint64_t`, `bitset<20>` to
  `bit_array<20>`, `array<uint8_t, 3>` to `bit_fixed_set<24>`, `bit_array<64>` to `bitset<64>`,
  `bit_set_view<uint64_t>` to `uint64_t`, `bit_array<5>` to `uint8_t`, `bit_fixed_set<64>` to `bit_fixed_set<65>`,
  and `uint64_t[4]` to `bit_array<256>` and `bitset<257>`.
- A run-time width converts into a fixed one: `bit_set` to `uint64_t`, `bit_vector` to `bitset<70>` and to
  `bit_array<64>`, `bit_inplace_set<64>` to `bit_fixed_set<10>`, and a `bit_subspan` window to `bit_array<100>`.
- Anything converts into a run-time width: `bit_span<array<uint8_t, 3>>`, `bit_array<64>` and
  `bit_span<vector<uint64_t>>` to `bit_vector`, `bitset<70>` to `bit_small_vector<64>`, `bit_vector` to
  `bit_inplace_set<64>`, `uint64_t` to `bit_set`, and a window to `bit_vector`.
- A reference or a `const` source is the conversion of the type it names: `const bit_set&` and `bit_set&&` to
  `bit_vector`. A cv-qualified target converts as the bare one: `bit_array<64>` to `const uint64_t` and to
  `volatile bitset<64>`, and `bit_vector` to `const bit_set`.
- A volatile scalar converts by one read: `volatile uint32_t&` to `bit_array<32>`, and `const volatile uint64_t&` to
  `bit_set`.
- Narrowing one fixed width into another would truncate, and is no conversion: `bit_array<9>` to `uint8_t` or
  `const uint8_t`, `bit_fixed_set<65>` to `uint64_t`, `uint32_t` to `bit_array<20>`, `uint64_t` to `bitset<63>`,
  `bitset<65>` to `bit_array<64>`, and `uint64_t[4]` to `bit_array<255>` are turned away.
- A view is read from and never written into: `uint64_t` to `bit_set_view<uint64_t>`, and `bit_vector` to
  `bit_span<array<uint8_t, 3>>` or to a window, are turned away. So is a built-in array as a target, which no
  function returns: `bit_array<256>`, `array<uint64_t, 4>` and `bit_vector` to `uint64_t[4]`.
- What has no bit storage converts to nothing: `vector<bool>` to `bit_vector`, and `bit_vector` to
  `vector<uint64_t>`.
- Volatile storage read by more than one access is no source: `volatile bit_array<64>&`, `volatile bitset<64>&`,
  `volatile uint32_t(&)[2]` and `volatile array<uint32_t, 2>&` are turned away.
- With `<xstd/bits/ext/boost/dynamic_bitset.hpp>` included, `boost::dynamic_bitset<>` converts to `bit_vector` and
  `uint64_t`, `bit_set` and `bitset<70>` to `boost::dynamic_bitset<>`, and one `boost::dynamic_bitset` into
  another does not.

## `<xstd/bits/bit_type_traits.hpp>`

```cpp
namespace xstd {
  // the width a storage type names
  template<bit_blocks Bits>
    constexpr size_t bit_blocks_extent_v = see below;

  // the pair that rebinds an owner's block or width
  template<unsigned_integer Block, class Owner>
      requires see below
    using bit_rebind = see below;
  template<size_t N, class Owner>
      requires see below
    using bit_resize = see below;

  // the choices made through them
  template<size_t N>
    using least_block_t = see below;
  template<class Owner>
      requires see below
    using bit_least = bit_rebind<least_block_t<see below>, Owner>;
  template<size_t N>
    using fast_block_t = see below;
  template<class Owner>
      requires see below
    using bit_fast = bit_rebind<fast_block_t<see below>, Owner>;
  template<class Owner>
      requires see below
    using bit_align = bit_resize<see below, Owner>;
  template<class Key>
      requires see below
    using underlying_block_t = see below;
  template<class Owner>
      requires see below
    using bit_underlying = bit_rebind<underlying_block_t<typename Owner::key_type>, Owner>;
}
```

The width a storage type names, the two transformations that rebind an owner's block or width, then the choices
made through them, in the umbrella's order. Each transformation is an alias template and is specified as
[[meta.trans]](https://eel.is/c++draft/meta.trans) specifies one: its *Constraints*, then the type it names.
Two phrases name what the constraints admit:

- A *rebindable owner* is an owner of ours that names its block: a specialization of `basic_bit_array`,
  `basic_bit_inplace_vector`, `basic_bit_vector`, `basic_bit_set`, `basic_bit_fixed_set`, `basic_bit_inplace_set`,
  `basic_bit_valarray`, `basic_bit_fixed_valarray`, `basic_bit_inplace_valarray`, or of the Boost extension's
  `basic_bit_small_vector`, `basic_bit_small_set` and `basic_bit_small_valarray`, the short names included.
- A *resizable owner* is a rebindable owner whose type carries its width `N`: every one but `basic_bit_vector`,
  `basic_bit_set` and `basic_bit_valarray`. For an inplace or a small owner `N` is its capacity.

Every transformation is cv-transparent, as `make_unsigned` is: an `Owner` that is cv-qualified is transformed as
`remove_cv_t<Owner>` is, and the type named carries `Owner`'s cv. A view, `bitset`, a block and a range of blocks
own no type to transform and are turned away by the constraint, not deep inside the alias.

### `bit_blocks_extent_v`

- *Constraints:* `Bits` models `bit_blocks`, so a `volatile` block or range of them names no extent.
- *Value:* For `B` = `remove_cv_t<Bits>` and `D` the `xstd::numeric_limits<Block>::digits` of its block type:
  - `D`, where `B` is a block;
  - `D * K`, where `B` is `array<Block, K>` or the built-in array `Block[K]`, the element's cv stripped;
  - `D * E`, where `B` is `span<Block, E>` and `E` is not `dynamic_extent`;
  - `dynamic_extent` otherwise, as for a `vector`, an `inplace_vector` or a `span` of dynamic extent.
- `bit_blocks_extent_v<uint8_t>` is 8, `bit_blocks_extent_v<const uint64_t>` 64, `bit_blocks_extent_v` of
  `array<uint16_t, 3>`, of `const array<uint16_t, 3>` and of `const uint16_t[3]` 48, of `span<const uint32_t, 2>`
  64, and of `uint64_t[4]` 256, as of `array<uint64_t, 4>`.
- *Remarks:* It is the default width of `bit_set_view` and `bit_span`, so `bit_set_view<span<uint32_t>>` is
  `bit_set_view<span<uint32_t>, dynamic_extent>`, and the width the `from_blocks` deduction guides deduce.

### `bit_rebind`

- *Constraints:* `remove_cv_t<Owner>` is a rebindable owner. `Block` models `unsigned_integer`, so `bool` and `int`
  are turned away.
- *Result:* The same owner over `remove_cv_t<Block>`, every other argument kept: the width, the key, the mapping
  and the comparison. An allocator `A` becomes `allocator_traits<A>::rebind_alloc<remove_cv_t<Block>>`.
- `bit_rebind<uint8_t, basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint8_t, 9>`, and
  `bit_rebind<uint32_t, basic_bit_fixed_set<size_t, uint8_t, 9, bit_key_mapping<size_t>, greater<size_t>>>` is
  `basic_bit_fixed_set<size_t, uint32_t, 9, bit_key_mapping<size_t>, greater<size_t>>`.
- `bit_rebind<uint8_t, bit_vector>` is `basic_bit_vector<uint8_t, allocator<uint8_t>>`, and
  `bit_rebind<uint16_t, bit_small_vector<9>>` is
  `basic_bit_small_vector<uint16_t, 9, boost::container::new_allocator<uint16_t>>`.
- The owner rebound holds the same bits: `bit_rebind<uint8_t, bit_vector>` constructed from a `bit_vector`'s
  iterators has its size and its elements, in `vector<uint8_t>` blocks.
- `bit_rebind<const volatile uint8_t, bit_vector>` is `bit_rebind<uint8_t, bit_vector>`: a block is held by value.
- *Remarks:* The form is that of `simd`'s `rebind_t`, the new block first.

### `bit_resize`

- *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
- *Result:* The same owner `N` bits wide, every other argument kept, an allocator included.
- `bit_resize<16, basic_bit_array<uint8_t, 9>>` is `basic_bit_array<uint8_t, 16>`, `bit_resize<64, bit_fixed_set<9>>`
  is `bit_fixed_set<64>`, `bit_resize<0, basic_bit_inplace_set<size_t, uint8_t, 9>>` is
  `basic_bit_inplace_set<size_t, uint8_t, 0>`, and `bit_resize<128, basic_bit_small_vector<uint64_t, 9>>` is
  `basic_bit_small_vector<uint64_t, 128, boost::container::new_allocator<uint64_t>>`.
- `bit_vector` and `bit_set`, whose widths are run-time values, have none to change and are turned away.
- *Remarks:* The form is that of `simd`'s `resize_t`, the new width first.

### `least_block_t`, `bit_least`

- `least_block_t<N>` is the narrowest fixed-width block holding `N` bits in one: `uint8_t` where `N <= 8`, `uint16_t`
  where `N <= 16`, `uint32_t` where `N <= 32`, and `uint64_t` otherwise, of which `N` bits then take several.
  `least_block_t<0>` is `uint8_t`.
- `bit_least<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a resizable owner, whose width picks the block.
  - *Result:* `bit_rebind<least_block_t<N>, Owner>`, where `N` is `Owner`'s width.
  - `bit_least<basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint16_t, 9>`, and `bit_least<bit_fixed_set<9>>`
    is `basic_bit_fixed_set<size_t, uint16_t, 9>`. `bit_set` is turned away.
- *Remarks:* The name is `<cstdint>`'s: `uint_least8_t` is the smallest type of at least 8 bits. `bit_enum_set` and
  `bit_flag_set` are `bit_least` over a fixed set.

### `fast_block_t`, `bit_fast`

- `fast_block_t<N>` is the fastest fixed-width block of at least `N` bits in one, by the thresholds of
  `least_block_t`: `uint_fast8_t` where `N <= 8`, `uint_fast16_t` where `N <= 16`, `uint_fast32_t` where `N <= 32`,
  and `uint_fast64_t` otherwise.
- `bit_fast<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
  - *Result:* `bit_rebind<fast_block_t<N>, Owner>`, where `N` is `Owner`'s width.
  - `bit_fast<basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint_fast16_t, 9>`. `bit_set` is turned away.
- *Remarks:* The platform decides how wide each `uint_fastN_t` is, so `bit_fast` names a block and promises no size.

### `bit_align`

- *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
- *Result:* `bit_resize<K * D, Owner>`, where `N` is `Owner`'s width, `D` the digits of its block and `K` the
  ⌈`N`/`D`⌉ blocks that `N` bits take: the width rounded up to whole blocks, so that no block carries an unused
  tail. A width already whole, zero included, stays as it is.
- `bit_align<basic_bit_array<uint8_t, 9>>` is `basic_bit_array<uint8_t, 16>`, and `bit_align` of
  `basic_bit_array<uint8_t, 16>` and of `basic_bit_array<uint8_t, 0>` is the type itself. `bit_set` is turned away.
- The transformations compose, and the order matters: `bit_align<bit_least<bit_fixed_set<3>>>` is
  `basic_bit_fixed_set<size_t, uint8_t, 8>`, where `bit_least<bit_align<bit_fixed_set<3>>>` rounds to the digits
  of `size_t` first, and is `basic_bit_fixed_set<size_t, uint64_t, 64>` where `size_t` is 64 bits.
- *Remarks:* A fixed-width owner whose width fills its blocks is a structural type
  ([[temp.param]](https://eel.is/c++draft/temp.param)), so `bit_align<bit_array<N>>` and
  `bit_align<bit_fixed_set<N>>` are, at any `N`.

### `underlying_block_t`, `bit_underlying`

- `underlying_block_t<Key>`:
  - *Constraints:* `Key` is an enumeration, or models xstd-ints's `integer`; its underlying type, or `Key` itself,
    is an integral type; and xstd-ints's `make_unsigned_t` of that type models `unsigned_integer`.
  - *Result:* `make_unsigned_t<U>`, where `U` is the enumeration's underlying type, or `Key` itself: the block a
    field or an ABI of `Key` already uses.
  - An enumeration on `uint32_t` gives `uint32_t`, one on `int16_t` gives `uint16_t`, and one on `char` gives
    `unsigned char`. `underlying_block_t<int>` is `unsigned`, of `int8_t` is `uint8_t`, of `uint32_t` is `uint32_t`
    and of `int64_t` is `uint64_t`.
  - `bool`, `char`, `char8_t` and `wchar_t` have no such counterpart, nor has an enumeration on `bool`, and a class
    such as `bitset<8>` is turned away.
- `bit_underlying<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a rebindable owner with a `key_type` for which `underlying_block_t` names
    a type: a set keyed by an enumeration or an integer.
  - *Result:* `bit_rebind<underlying_block_t<typename Owner::key_type>, Owner>`, the width kept.
  - For a flag set at its mask's full width the result is the type itself, `bit_least` having chosen that word; for
    `bit_fast` of it, or a narrower `bit_flag_set<E, 9>`, it is the set in `E`'s own word.
    `bit_underlying<bit_flag_set<int, 5>>` is `basic_bit_fixed_set<int, unsigned, 5, bit_flag_mapping<int, 5>,
    greater<int>>`, in the word an `int` field stores.
  - The rule is by key type, not by mapping: `bit_underlying<bit_set>` is `bit_set`, and
    `bit_underlying<bit_least<bit_fixed_set<9>>>` is `bit_fixed_set<9>`.
  - The set's block is then the mask's representation bit for bit, so `bit_convert`, `from_blocks` and `bit_cast`
    to and from that word agree with `to_underlying` of the mask.
  - A sequence such as `bit_array<9>` has no key and is turned away, and so are `bit_flag_set<bitset<16>>` and a set
    keyed by an enumeration on `bool`.

## `<xstd/bits/bit_functional.hpp>`

```cpp
namespace xstd {
  // [comparisons], comparisons
  struct bit_equal_to;
  struct bit_not_equal_to;
  struct bit_greater;
  struct bit_less;
  struct bit_greater_equal;
  struct bit_less_equal;

  // [comparisons.three.way], class compare_three_way
  struct bit_compare_three_way;

  // [unord.hash], class template hash, beside it the digest of the same bits
  template<hash_algorithm Hash, class Flavor, class T>
      requires see below
    constexpr auto bit_hash_append(Hash& h, const Flavor& f, const T& x) -> void;
  template<hash_algorithm Hash, class Flavor, class R>
      requires see below
    constexpr auto bit_hash_append(Hash& h, const Flavor& f, const R& r) -> void;
  template<class T, hash_algorithm H = boost::hash2::xxhash_64, class Flavor = boost::hash2::default_flavor>
      requires see below
    class bit_hash;
  template<class T, hash_algorithm H = boost::hash2::xxh3_128, class Flavor = boost::hash2::little_endian_flavor>
      requires see below
    class bit_digest;
}
```

Each comparison and the hash has a header of its own under `<xstd/bits/bit_functional/>`, and the umbrella exports
them in [[comparisons]](https://eel.is/c++draft/comparisons)'s order, then
[[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way), then
[[unord.hash]](https://eel.is/c++draft/unord.hash) with the digest beside it. `hash_algorithm` is xstd-misc's. The
comparisons are transparent and agree with the hash: what `bit_equal_to` calls equal, `bit_hash` hashes alike, and
the orderings order equivalent exactly what it calls equal. So a `std::unordered_set<K, bit_hash<K>, bit_equal_to>`
or a `std::set<K, bit_less>` finds a key through a view of it or through an owner over narrower blocks.

### `bit_equal_to`, `bit_not_equal_to`

```cpp
struct bit_equal_to {
  using is_transparent = void;

  template<class T, class U>
      requires requires (const T& x, const U& y) { xstd::ranges::bit_equal(x, y); }
    constexpr auto operator()(const T& x, const U& y) const noexcept -> bool;
};
```

As `equal_to<>` and `not_equal_to<>` in [[comparisons]](https://eel.is/c++draft/comparisons), with:

- *Constraints:* `xstd::ranges::bit_equal(x, y)` is a valid expression: `T` and `U` are of one reading and one width
  kind, as `bit_equal` admits them. The constraint is the call itself, so the two cannot drift.
- *Returns:* `xstd::ranges::bit_equal(x, y)`, and for `bit_not_equal_to`, `not xstd::ranges::bit_equal(x, y)`.
- `bit_fixed_set<70>{1, 65}` equals `basic_bit_fixed_set<size_t, uint8_t, 70>{1, 65}`, and `bit_vector` compares with
  `basic_bit_vector<uint8_t>`.
- `bit_array<8>` does not compare with `bit_vector`, nor `bit_fixed_set<8>` with `bit_set` or with `bit_array<8>`,
  and `vector<bool>` is turned away: a static width never compares with a run-time one, which `bit_hash` keeps apart.

### `bit_greater`, `bit_less`, `bit_greater_equal`, `bit_less_equal`

```cpp
struct bit_less {
  using is_transparent = void;

  template<class T, class U>
      requires requires (const T& x, const U& y) { xstd::ranges::bit_lexicographical_compare_three_way(x, y); }
    constexpr auto operator()(const T& x, const U& y) const noexcept -> bool;
};
```

As `greater<>`, `less<>`, `greater_equal<>` and `less_equal<>` in [[comparisons]](https://eel.is/c++draft/comparisons),
each of the same shape as `bit_less`, with:

- *Constraints:* `xstd::ranges::bit_lexicographical_compare_three_way(x, y)` is a valid expression, which admits the
  pairs `bit_equal_to` takes.
- *Returns:* `is_gt(c)`, `is_lt(c)`, `is_gteq(c)` and `is_lteq(c)` respectively, where `c` is
  `xstd::ranges::bit_lexicographical_compare_three_way(x, y)`.
- `bit_less()(bit_fixed_set<70>{1}, basic_bit_fixed_set<size_t, uint8_t, 70>{1, 65})` is `true`, and a
  `std::set<bit_fixed_set<100>, bit_less>` finds a key through `bit_set_view` and through `uint8_t` blocks.
- The pairs `bit_equal_to` turns away are turned away.

### `bit_compare_three_way`

As `compare_three_way` in [[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way), of the same shape
as `bit_less`, with:

- *Constraints:* As for `bit_less`.
- *Returns:* `xstd::ranges::bit_lexicographical_compare_three_way(x, y)`, a `strong_ordering`.
- It answers `strong_ordering::equal` exactly where `bit_equal_to` answers `true`.

### `bit_hash_append`

- *Constraints:* `Hash` models `hash_algorithm`, and:
  - for the first form, `T` is an owner of ours, of either reading, or a set view. A sequence view, `bit_span` or a
    window, is turned away, as `span` has no hash, though `bit_equal` compares it with its owner;
  - for the second, `const R` models `ranges::contiguous_range` and `ranges::sized_range`, and its value type is a
    type the first form takes whose width the type fixes, or a built-in array or an `array` of such, nested to any
    depth: the planes of a board game's position, as `bit_fixed_set<64>[2][6]` or a `span<const bit_fixed_set<64>>`.
- The *bit string* of `x` is ⌈`N`/8⌉ bytes for its width `N`, byte `j` holding the positions `[8j, 8j + 8)` least
  significant bit first and the bits past `N` zero. For a set of run-time width it stops after the byte that holds
  the last key, so it is empty for an empty set: equal sets need not share a width.
- *Effects:* For the first form:
  - where the type of `x` fixes a width of zero, `boost::hash2::hash_append(h, f, '\x00')`, the one byte Hash2
    writes for an empty `array`;
  - otherwise, updates `h` with the bit string of `x`; then, where the width is a run-time value, an inplace
    capacity included, calls `boost::hash2::hash_append_size(h, f, n)`, where `n` is a sequence's size in bits and a
    set's bit string's length in bytes.

  For the second form:
  - where the type of `r` fixes a count of zero, `boost::hash2::hash_append(h, f, '\x00')`;
  - otherwise appends each element in order, by the first form or, for a nested array, by this one; then, where the
    type of `r` fixes no count, calls `boost::hash2::hash_append_size(h, f, ranges::size(r))`. A built-in array, an
    `array` and a `span` of static extent fix one.
- The block type is no part of the message, and `f` orders the bytes of the appended size alone. Under
  `little_endian_flavor`:

  | `x` | message |
  | :-- | :-- |
  | `basic_bit_fixed_set<size_t, uint64_t, 10>{1, 3, 5, 9}` | bytes `2A 02` |
  | `bit_fixed_set<0>()`, `bit_array<0>()` | byte `00` |
  | `basic_bit_vector<uint64_t>(10)`, its even positions `true` | bytes `55 01`, then the size 10 |
  | `basic_bit_set<size_t, uint64_t>{1, 3, 5}` | byte `2A`, then the size 1 |
  | `basic_bit_set<size_t, uint64_t>{1, 3, 64}` | bytes `0A 00 00 00 00 00 00 00 01`, then the size 9 |
  | an empty `basic_bit_vector<uint64_t>` or `basic_bit_set<size_t, uint64_t>` | the size 0 |
  | `array<bit_fixed_set<64>, 0>()` | byte `00` |

- *Remarks:* Wherever the storage holds the bit string, a little-endian target and a block of a built-in type whose
  value bits fill its object, the string goes in as one `update` over the storage. Elsewhere, a class-type block
  and a constant evaluation included, the same bytes are assembled, up to 256 an `update`. A Hash2 digest does not
  depend on how its message is split into updates, so the digest is the same either way.
- *Remarks:* Where each element of `r` is an owner whose object is its bit string, as `bit_fixed_set<64>` in
  `uint64_t` blocks is, and each nested `array` measures as wide as its elements, the whole range goes in as one
  `update`. A `bit_fixed_set<50>`, eight bytes for a seven-byte string, goes in an element at a time, to the same
  digest.
- *Remarks:* It is usable in a constant expression.
- Where `xstd::ranges::bit_equal(x, y)` is `true`, `x` and `y` append the same message: an owner and its view, two
  owners over different blocks, and a run-time set at any two widths. So what compares equal hashes equal.

### `bit_hash`

```cpp
template<class T, hash_algorithm H, class Flavor>
  requires see below
class bit_hash {
public:
  using is_avalanching = bool_constant<hash_algorithm_is_avalanching_v<H>>;
  using is_transparent = void;

  bit_hash() = default;
  constexpr explicit bit_hash(uint64_t seed);
  constexpr bit_hash(const unsigned char* p, size_t n);

  template<class U>
      requires see below
    constexpr auto operator()(const U& v) const -> size_t;
};
```

As xstd-misc's `hash`, with:

- *Constraints:* `T` is a type `bit_hash_append` takes, by either form.
- One algorithm, `H`, defaulted to `boost::hash2::xxhash_64` on every target, where `size_t` takes Hash2's fold of
  its result. There is no `H32` and no `hash_type`; `is_avalanching` is read from `H`, so it is `true` under
  xxHash and SipHash and `false` under FNV-1a.
- `is_transparent` is declared: equal bits hash equal whatever blocks or storage hold them, so a lookup may cross
  containers of one reading.
- There is no specialization for a string key.
- `operator()`:
  - *Constraints:* `U` is `T`, or `xstd::ranges::bit_equal(x, y)` is valid for a `const T& x` and a `const U& y`;
    and `U` is a type `bit_hash_append` takes. So `bit_hash<bit_array<64>>` does not take a `bit_span`.
  - *Effects:* Copies the prototype to `h` and calls `bit_hash_append(h, Flavor(), v)`. The prototype is unchanged.
  - *Returns:* `boost::hash2::get_integral_result<size_t>(h)`.
  - `v` is appended as the `U` it is, not converted to `T`.
- It hashes the bits, not the value as its standard model has it: no key and no count, so `bit_hash<T, H>()(x)` and
  xstd-misc's `hash<T, H>()(x)` differ.
- Equal bits hash equal: a `bit_fixed_set<100>` and its `bit_set_view`, the same set over `uint8_t` blocks, and a
  `bit_vector`, a `basic_bit_small_vector<uint8_t, 9>` and a `basic_bit_inplace_vector<uint64_t, 256>` of one value.
- Where `hash<T>` is enabled for an owner or a set view `T`, it is this hash at its defaults: `bit_hash<T>()(x)` is
  `hash<T>()(x)`.

### `bit_digest`

```cpp
template<class T, hash_algorithm H, class Flavor>
  requires see below
class bit_digest {
public:
  using result_type = typename H::result_type;

  bit_digest() = default;
  constexpr explicit bit_digest(uint64_t seed);
  constexpr bit_digest(const unsigned char* p, size_t n);

  constexpr auto operator()(const T& v) const -> result_type;
};
```

As xstd-misc's `digest`, with:

- *Constraints:* `T` is a type `bit_hash_append` takes, by either form.
- `operator()`, *Returns:* `h.result()`, where `h` is a copy of the prototype after `bit_hash_append(h, Flavor(), v)`.
- There is no specialization for a string key.
- Equal bits digest equal across block types: under `fnv1a_64`, `basic_bit_fixed_set<size_t, Block, 200>{1, 64,
  199}` digests to `0xB72B'9E62'AF48'F732` at every block type, and under `xxhash_64` to `0xF8C0'7F80'B85C'9EA1`.
- `bit_hash<T, H, Flavor>()(x)` is `boost::hash2::get_integral_result<size_t>` of the state whose result
  `bit_digest<T, H, Flavor>()(x)` is.

## `<xstd/bits/bit_ranges.hpp>`

```cpp
namespace xstd {
  // [ranges.syn], the from_range tag, whose counterpart for an argument's blocks is from_blocks
  struct from_blocks_t { explicit from_blocks_t() = default; };
  inline constexpr from_blocks_t from_blocks{};
}
```

The umbrella exports `<xstd/bits/bit_ranges/from_blocks.hpp>`.

### `from_blocks_t`, `from_blocks`

As `from_range_t` and `from_range` in [[ranges.syn]](https://eel.is/c++draft/ranges.syn), with:

- The tag says at the call site that an argument's blocks are read as bits, rather than as keys or elements:
  `bit_fixed_set<32>(from_blocks, 5u)` is `{0, 2}`, the positions the value five has.
- Its default constructor is `explicit`, so `{}` cannot stand in for the tag.
- Every owner of ours takes it: a fixed width reads any blocks that hold at least its width, and a run-time width
  adopts its own block container. `bit_constructible_from` names the blocks an owner takes as they are.
- The deduction guides of `basic_bit_array` and `basic_bit_fixed_set` deduce the block and the width
  `bit_blocks_extent_v` names, from an unsigned integer, an `array` of them or a built-in array of them, as
  [[span.deduct]](https://eel.is/c++draft/span.deduct) takes an array's bound: `basic_bit_array(from_blocks,
  uint16_t{...})` is `basic_bit_array<uint16_t, 16>`, and `basic_bit_fixed_set(from_blocks, array<uint8_t, 3>{...})`
  is `basic_bit_fixed_set<size_t, uint8_t, 24>`. An empty `array` deduces width zero.
- A signed integer or an `array` of them deduces nothing, and a run-time width takes no block of a fixed width:
  `bit_vector` is not constructible from `from_blocks` and a `uint64_t`.
- A type that has bit storage without being it, a `bitset<64>` or a `bit_array<64>`, is not read through the tag.
  It converts through `bit_convert`.

## `<xstd/bits/bit.hpp>`

```cpp
namespace xstd {
  // [bit.cast], function template bit_cast, whose counterpart across bit storages is bit_convert
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;
}
```

The umbrella exports `<xstd/bits/bit/bit_convert.hpp>`.

### `bit_convert`

The counterpart of `bit_cast` in [[bit.cast]](https://eel.is/c++draft/bit.cast) across bit storages: where `bit_cast`
reinterprets an object, `bit_convert` maps positions, and position `i` of the source is position `i` of the result.
Let `F` be `remove_cvref_t<From>` and `T` be `remove_cv_t<To>`. The constraints use these phrases:

- A *fixed width* is a type that fixes in its type how many positions it has, its *width* `W`: an owner of ours
  whose storage fixes its width, as `basic_bit_array`, `basic_bit_fixed_set` and `basic_bit_fixed_valarray` do; a
  whole view whose storage fixes its width, `W` being the storage's, as for `bit_set_view<uint64_t>` and
  `bit_span<array<uint8_t, 3>>`; a block, an `array` of blocks or a built-in array of blocks, `W` being their
  `bit_blocks_extent_v`; and, on a little-endian target, `bitset<N>`, whose `W` is `N`.
- A *fixed target* is a fixed width that is neither a view nor a built-in array.
- A *run-time target* is an owner of ours that stores its width, a capacity included: the vectors, the run-time and
  inplace sets and valarrays, and the Boost extension's small owners.
- A *source* is a fixed width, an owner of ours, a whole view of ours of run-time width, or a window.

The rules:

| `T` | `F` | result |
| :-- | :-- | :-- |
| a fixed target | a fixed width of `W(F) <= W(T)` | the `W(F)` positions, the positions `[W(F), W(T))` clear |
| a fixed target | a source that is no fixed width | its positions below `W(T)`, zero-extended; a position at or past `W(T)` throws |
| a run-time target | a source | the source's positions at the source's width, as below |
| a type an extension makes a target | a source the extension admits | as the extension specifies |

- *Constraints:* `F` and `T` meet one of the rules, and `remove_reference_t<From>` is not `volatile` unless it is a
  scalar type.
- *Effects:* Where `remove_reference_t<From>` is a `volatile` scalar, reads it once and returns
  `bit_convert<T>(static_cast<F>(from))`. Otherwise, into a run-time target:
  - A sequence target takes the source's *width*: a sequence's size, a window's size, a fixed width's `W`, and a
    set's universe, its whole blocks up to its `max_size()`. A set target covers that width in whole blocks of its
    own, capped at its capacity where it has one. The positions past the source's width are clear.
  - Where `From` is a non-`const` rvalue whose `extract()` yields a container `C`, `bit_constructible_from<T, C>`
    holds, and `T`'s capacity, where it has one, is every bit of `C`, the blocks move: `C` is extracted into the
    `from_blocks` constructor without copying a block, a sequence target then takes the source's width, and the
    source is left at width zero. The extracted container's type carries its allocator, so another allocator is
    another `C`.
  - Otherwise the positions are copied, and the source is unchanged.
- *Returns:* The object of type `T` holding those positions.
- *Throws:*
  - `overflow_error` where `T` is a fixed target, `F` is no fixed width, and a position at or past `W(T)` is set, as
    `bitset::to_ulong` throws in [[bitset.members]](https://eel.is/c++draft/bitset.members);
  - `bad_alloc` where `T` is an inplace sequence whose capacity is short of the source's width, or a set target
    capped at its capacity would lose a key, which is what the target's own growth throws, before a block is
    written;
  - any exception the target's allocator throws.
- *Remarks:* The exception specification is `fixed-width<T> and fixed-width<F>`: nothing throws between two fixed
  widths, and a run-time end can allocate, refuse a capacity or throw `overflow_error`.
- *Remarks:* It is usable in a constant expression. There the copy is made by shifts, and outside one by a single
  copy of the bytes where both block types are built-ins without padding on a little-endian target.
- *Remarks:* `To`'s cv is dropped, as a cast's scalar prvalue drops it. `To` is never a view, which would write bits
  it does not own, nor a built-in array, which no function returns. A `span` of blocks lends its width rather than
  having it, so it is neither end; a `bit_span` over it converts.
- *Remarks:* A `bitset` is read and written through its object on the terms `bit_cast` reads one, so it converts on
  a little-endian target only. Its `to_string()` writes position 0 last, so its text reads reversed against the
  positions written first to last.
- `bit_convert<uint64_t>(bit_fixed_set<64>{0, 5, 63})` is `(1ULL << 63) | (1ULL << 5) | 1`, and
  `bit_convert<uint8_t>(bit_array<5>)` is what `to_ulong` gives for a `bitset<5>` of the same positions.
- `bit_convert<array<uint64_t, 2>>(bit_fixed_set<100>{0, 63, 64, 99})` is `{(1ULL << 63) | 1, (1ULL << 35) | 1}`,
  which `bit_cast` reads from the set's object too.
- `bit_convert<bit_fixed_set<10>>(bit_set{3, 9})` holds `{3, 9}`, and `bit_convert<bit_fixed_set<10>>(bit_set{3,
  10})` throws `overflow_error`. `bit_convert<bitset<0>>(bit_set())` converts, and `bit_set{0}` throws.
- `bit_convert<bit_vector>(bit_set{3, 64, 129})` has the set's whole blocks as its size, 192 over 64-bit blocks, and
  `bit_convert<bit_vector>(bit_fixed_set<100>{99})` has 100. A `bit_vector` of 70 converts to a set over 72 positions
  in `uint8_t` blocks.
- `bit_convert<bit_vector>(std::move(s))` for a `bit_set s` takes `s`'s blocks, and `s` is then empty; from a
  `basic_bit_set<size_t, uint8_t>` it copies, and the source keeps its bits.

## `<xstd/bits/ext/boost/dynamic_bitset.hpp>`

```cpp
namespace xstd {
  // exported, and valid with boost::dynamic_bitset<Block, AllocatorOrContainer> at either end
  template<class To, class From>
      requires see below
    constexpr auto bit_convert(From&& from) noexcept(see below) -> remove_cv_t<To>;
}
```

`<xstd/bits.hpp>` and `<xstd/bits/ext/boost.hpp>` do not include this header, so the dependency on
[Boost.DynamicBitset](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/) is opted into by name. Including
it is what makes the conversions valid; it declares no name of its own and exports `bit_convert`.

### `bit_convert` with `boost::dynamic_bitset`

As `bit_convert`, with `boost::dynamic_bitset<Block, AllocatorOrContainer>` as either end:

- **As a source**, a `dynamic_bitset` is a sequence of its `size()`, of run-time width, read through
  `boost::to_block_range`, each of its blocks landing on its own positions at any target block width:
  - into a run-time target, as from any run-time sequence: a sequence target takes its size, a set target whole
    blocks of its own, and an inplace target too small throws `bad_alloc`;
  - into a fixed target, by value: zero-extended, and `overflow_error` for a position at or past the target's width.
- **As a target**, `bit_convert<boost::dynamic_bitset<Block, AllocatorOrContainer>>(from)` takes a source of ours
  that is an owner, a whole view or a fixed width, a `bitset` and a block among them. The result is the source's
  width, as a sequence target takes it, written through `boost::from_block_range` in blocks of its own width.
- A window is no source for a `dynamic_bitset`, and one `dynamic_bitset` converts into no other.
- *Throws:* Into a `dynamic_bitset`, any exception its allocation throws. Neither direction is `noexcept`.
- *Remarks:* Boost hands its blocks out nowhere, so both directions copy: an rvalue `dynamic_bitset` source keeps
  its bits, and nothing is adopted even where its container is an owner's own.
- *Remarks:* Since Boost 1.90 the container parameter converts like any other, a `small_vector` of blocks included,
  and both directions are usable in a constant expression.
- *Remarks:* As with `bitset`, its `to_string()` writes position 0 last.
- A `dynamic_bitset<uint8_t>` of 13 with positions `{0, 5, 12}` converts to a `bit_vector` of 13 with them, and
  back to an equal `dynamic_bitset<uint8_t>`. `bit_convert<boost::dynamic_bitset<>>(bit_set{3, 64, 129})` has the
  set's whole blocks as its size, and `bit_convert<boost::dynamic_bitset<uint8_t>>(bit_array<20>())` 20.
