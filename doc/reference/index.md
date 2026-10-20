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
