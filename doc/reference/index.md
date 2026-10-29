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
  type carries, the blocks held inline; or *dynamic*, on the heap. Each has a standard backend: `array`,
  `inplace_vector` and `vector`. Any other container is a `bit_storage` argument; the aliases over other libraries'
  containers, such as Boost's `small_vector`, are under [`ext/`](ext_boost/index.md) and are not related here.
- An *owner* holds its storage, and a *view* refers to storage another object holds.
- A *reading* is what the bits mean: a *sequence* of `bool`, a *set* of keys, or a *valarray*, the sequence read
  elementwise.

Each container answers a standard counterpart's synopsis, and is specified as that counterpart *with* what the
packing changes. Members named `adaptor_type`, `adapted_type`, `derived_type`, `reads_as`, `owns_storage`,
`is_windowed` and `static_extent` answer the library's own traits and are not specified here.

Each umbrella has a page of its own, with its synopsis and its headers, and each header a page beneath it:

- [`<xstd/bits/bit.hpp>`](bit/index.md)
- [`<xstd/bits/bit_algorithm.hpp>`](bit_algorithm/index.md)
- [`<xstd/bits/bit_array.hpp>`](bit_array/index.md)
- [`<xstd/bits/bit_concepts.hpp>`](bit_concepts/index.md)
- [`<xstd/bits/bit_functional.hpp>`](bit_functional/index.md)
- [`<xstd/bits/bit_inplace_vector.hpp>`](bit_inplace_vector/index.md)
- [`<xstd/bits/bit_iterator.hpp>`](bit_iterator/index.md)
- [`<xstd/bits/bit_ranges.hpp>`](bit_ranges/index.md)
- [`<xstd/bits/bit_set.hpp>`](bit_set/index.md)
- [`<xstd/bits/bit_span.hpp>`](bit_span/index.md)
- [`<xstd/bits/bit_type_traits.hpp>`](bit_type_traits/index.md)
- [`<xstd/bits/bit_valarray.hpp>`](bit_valarray/index.md)
- [`<xstd/bits/bit_vector.hpp>`](bit_vector/index.md)
- [`<xstd/bits/ext/boost.hpp>`](ext_boost/index.md)
- [`<xstd/bits/ext/boost/dynamic_bitset.hpp>`](ext_boost_dynamic_bitset/index.md)

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
templates extend, and the templates that choose a container's block type, storage and allocator, like what the
standard has no word for, are new.

The Counterpart column leads with the clause of the working draft each entity answers, then says how it extends or
generalizes what that clause specifies. The rows follow the draft's order of those clauses, and the names within a
row the draft's order of their counterparts. A row with no counterpart sits beside its nearest kin, citing the kin's
clause where one is shared.

<div class="relation" markdown>

| Entity | Kind | Counterpart |
| :-- | :-- | :-- |
| `indeterminate`<br>`indeterminate_array`<br>`indeterminate_t` | New | [[dcl.attr.indet]](https://eel.is/c++draft/dcl.attr.indet) as a constructor tag: a fixed set's blocks left unwritten, as the attribute leaves a variable's storage |
| `bit_flag_set` | Generalizes | [[bitmask.types]](https://eel.is/c++draft/bitmask.types) as the set of a bitmask type's one-bit values |
| `bit_enum_set` | New | none: the standard has no set of an enumeration's values |
| `least_block_t`<br>`fast_block_t` | Generalizes | [[cstdint.syn]](https://eel.is/c++draft/cstdint.syn) chosen by a width rather than named by one |
| `bit_blocks`<br>`bit_constructible_from`<br>`bit_convertible_to`<br>`bit_index_mapping`<br>`bit_mask_mapping`<br>`sized_bit_index_mapping` | New | none |
| `bit_align`<br>`bit_blocks_extent_v`<br>`bit_fast`<br>`bit_least`<br>`bit_underlying`<br>`underlying_block_t` | New | none |
| `bit_storage_rebind` | New | [[allocator.traits.types]](https://eel.is/c++draft/allocator.traits.types) as `rebind_alloc` rebinds an allocator, for a storage's block and its count of blocks |
| `bit_compare_three_way`<br>`bit_equal_to`<br>`bit_not_equal_to`<br>`bit_greater`<br>`bit_less`<br>`bit_greater_equal`<br>`bit_less_equal` | Generalizes | [[comparisons.three.way]](https://eel.is/c++draft/comparisons.three.way) [[range.cmp]](https://eel.is/c++draft/range.cmp) across block types and storages |
| `hash` | Extends | [[unord.hash]](https://eel.is/c++draft/unord.hash) specializations for every owner but the valarrays, and for the set view |
| `bit_hash` | Generalizes | [[unord.hash]](https://eel.is/c++draft/unord.hash) of the packed bits rather than the model's value, under a Boost.Hash2 algorithm the caller names |
| `bit_hash_append` | New | none in the standard: a Boost.Hash2 `hash_append`, after [N3980](https://wg21.link/n3980) |
| `bit_digest` | Proposed | [N3980](https://wg21.link/n3980)'s `uhash`, returning the algorithm's whole result |
| `is_range`<br>`is_tuple_like` | New | none in the standard: specializations of Boost.ContainerHash's traits |
| `bit_convert` | Generalizes | [[bit.cast]](https://eel.is/c++draft/bit.cast) from [P0476R2](https://wg21.link/p0476r2): position for position across widths and readings, rather than byte for byte at one size |
| `bit_convert` with `boost::dynamic_bitset` | New | none in the standard: conversion to and from Boost.DynamicBitset |
| `has_single_bit` | Generalizes | [[bit.pow.two]](https://eel.is/c++draft/bit.pow.two) from [P0556R3](https://wg21.link/p0556r3): whether one position is set in an owner of any width, rather than one bit in an unsigned integer |
| `shl`<br>`shr` | Proposed | [P3793R2](https://wg21.link/p3793r2)'s total shifts, beside [[bit.rotate]](https://eel.is/c++draft/bit.rotate): a set's keys moved by any count, where the set's own `<<` and `>>` take a count below a static width as their precondition |
| `bit_compress`<br>`bit_expand` | Generalizes | [[bit.permute]](https://eel.is/c++draft/bit.permute) from [P3104R5](https://wg21.link/p3104r5), over two owners of one type and any width rather than one unsigned integer |
| `bit_array` | Generalizes | [[array]](https://eel.is/c++draft/array) of `bool`, packed |
| `basic_bit_array` | New | [[array]](https://eel.is/c++draft/array) customizing the block type |
| `to_bit_array` | Generalizes | [[array.creation]](https://eel.is/c++draft/array.creation) returning a `bit_array` |
| `tuple_size`<br>`tuple_element`<br>`get` | Extends | [[array.tuple]](https://eel.is/c++draft/array.tuple) specializations and an overload for `basic_bit_array` |
| `erase`<br>`erase_if` | Generalizes | [[vector.erasure]](https://eel.is/c++draft/vector.erasure) [[inplace.vector.erasure]](https://eel.is/c++draft/inplace.vector.erasure) [[set.erasure]](https://eel.is/c++draft/set.erasure) for the packed sequences and sets |
| `bit_vector` | Generalizes | [[vector.bool]](https://eel.is/c++draft/vector.bool) packed into blocks, which `vector<bool>` leaves to the implementation |
| `basic_bit_vector` | New | [[vector.bool]](https://eel.is/c++draft/vector.bool) customizing the storage, and with it the block type and the allocator, and capping the width |
| `bit_inplace_vector` | Generalizes | [[inplace.vector]](https://eel.is/c++draft/inplace.vector) of `bool`, packed |
| `basic_bit_inplace_vector` | New | [[inplace.vector]](https://eel.is/c++draft/inplace.vector) customizing the block type, as `basic_bit_vector` over inline blocks |
| `bit_set` | Generalizes | [[set]](https://eel.is/c++draft/set) of positions, one bit per possible key, packed into blocks |
| `basic_bit_set`<br>`basic_bit_fixed_set`<br>`basic_bit_inplace_set`<br>`bit_fixed_set`<br>`bit_inplace_set` | New | [[set]](https://eel.is/c++draft/set) customizing the key mapping and the storage, and with it the block type and the allocator: at a fixed width or within an inline capacity |
| `bit_set_view` | New | none: no standard view reads a range as a set |
| `bit_find_mapping`<br>`bit_flag_mapping`<br>`bit_key_mapping`<br>`bit_range_mapping`<br>`enum_traits` | New | none; `bit_key_mapping` is specialized per key as `char_traits` is, [[char.traits]](https://eel.is/c++draft/char.traits) |
| `bit_storage` | New | [[flat.set.overview]](https://eel.is/c++draft/flat.set.overview) as `flat_set` constrains its `KeyContainer`, for the blocks an owner keeps |
| `bit_span` | Generalizes | [[views.span]](https://eel.is/c++draft/views.span) over blocks read as a sequence of `bool` |
| `bit_subspan` | Generalizes | [[span.sub]](https://eel.is/c++draft/span.sub) at any bit offset |
| `bit_advance`<br>`bit_distance`<br>`bit_next`<br>`bit_prev` | Generalizes | [[range.iter.ops]](https://eel.is/c++draft/range.iter.ops) a block at a time over a set |
| `from_blocks_t`<br>`from_blocks` | Generalizes | [[ranges.syn]](https://eel.is/c++draft/ranges.syn) from [P1206R7](https://wg21.link/p1206r7): reading an argument's blocks as bits, where `from_range` reads its elements as values |
| `bit_all_of`<br>`bit_any_of`<br>`bit_none_of`<br>`bit_for_each`<br>`bit_find`<br>`bit_find_last`<br>`bit_count`<br>`bit_mismatch`<br>`bit_equal` | Generalizes | [[alg.nonmodifying]](https://eel.is/c++draft/alg.nonmodifying) a block at a time, without the projections and predicates a range of bits has no use for |
| `bit_copy`<br>`bit_copy_backward`<br>`bit_move`<br>`bit_swap_ranges`<br>`bit_transform`<br>`bit_fill`<br>`bit_reverse`<br>`bit_reverse_copy`<br>`bit_rotate`<br>`bit_rotate_copy`<br>`bit_shift_left`<br>`bit_shift_right` | Generalizes | [[alg.modifying.operations]](https://eel.is/c++draft/alg.modifying.operations) a block at a time, without the projections a range of bits has no use for |
| `bit_sort`<br>`bit_is_sorted`<br>`bit_is_partitioned`<br>`bit_partition`<br>`bit_partition_point` | Generalizes | [[alg.sorting]](https://eel.is/c++draft/alg.sorting) a block at a time, without the projections, predicates and comparators a range of bits has no use for |
| `bit_includes`<br>`bit_set_union`<br>`bit_set_intersection`<br>`bit_set_difference`<br>`bit_set_symmetric_difference` | Generalizes | [[alg.set.operations]](https://eel.is/c++draft/alg.set.operations) returning a set where the standard writes through an output iterator |
| `bit_intersects`<br>`bit_disjoint` | New | [[alg.set.operations]](https://eel.is/c++draft/alg.set.operations) none: the standard has no test of whether two sets share an element |
| `bit_lexicographical_compare_three_way` | Proposed | [[alg.three.way]](https://eel.is/c++draft/alg.three.way) over two ranges, as [P2022R3](https://wg21.link/p2022r3) proposes `ranges::lexicographical_compare_three_way` |
| `bit_valarray`<br>`bit_slice`<br>`bit_slice_array`<br>`bit_gslice`<br>`bit_gslice_array`<br>`bit_mask_array`<br>`bit_indirect_array` | Generalizes | [[numarray]](https://eel.is/c++draft/numarray) of `bool`, packed, with the slices and helper arrays that select from it |
| `basic_bit_valarray`<br>`basic_bit_fixed_valarray`<br>`basic_bit_inplace_valarray`<br>`bit_fixed_valarray`<br>`bit_inplace_valarray` | New | [[numarray]](https://eel.is/c++draft/numarray) customizing the storage, and with it the block type and the allocator: at a fixed width or within an inline capacity |
| `bit_rebind`<br>`bit_resize` | Generalizes | [[simd.traits]](https://eel.is/c++draft/simd.traits) from [P1928R15](https://wg21.link/p1928r15), for owners rather than `simd` types |

</div>

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
- **The dynamic storage, and the containers under `ext/`:**
    - [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/), and
      [N2050](https://wg21.link/n2050), *Proposal to Add a Dynamically Sizeable Bitset to the Standard Library
      Technical Report Revision 1* (J. Allsop, A. Meredith, G. Prota), which proposed it for the standard.
    - LLVM's [`BitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/BitVector.h) and
      [`SmallBitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/SmallBitVector.h),
      EASTL's [`bitvector`](https://github.com/electronicarts/EASTL/blob/master/include/EASTL/bitvector.h) and Qt's
      [`QBitArray`](https://doc.qt.io/qt-6/qbitarray.html).
    - Boost.Container's
      [`small_vector`](https://www.boost.org/doc/libs/release/doc/html/boost/container/small_vector.html), which
      `ext/boost/`'s small aliases adopt, and `static_vector`, which holds the inplace storage below C++26.
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
  Jabot, Eric Niebler, Casey Carter), whose `from_range` tag it mirrors. itsy_bitsy's
  [`bit_sequence`](https://github.com/ThePhD/itsy_bitsy/blob/d5b6bf9509bb2dff6235452d427f0b1c349d5f8b/include/itsy/bit_sequence.hpp)
  takes its storage through `std::in_place` instead.
- **`indeterminate`:** [P2795R5](https://wg21.link/p2795r5), *Erroneous behaviour for uninitialized reads* (Thomas
  Köppe), whose `[[indeterminate]]` the tag mirrors; [P1020R1](https://wg21.link/p1020r1), *Smart pointer creation
  with default initialization* (Glen Fernandes, Peter Dimov), the standard's `for_overwrite`; and Boost.Container's
  `default_init_t`.
- **`bit_convert`:** [P0476R2](https://wg21.link/p0476r2), *Bit-casting object representations* (JF Bastien).
