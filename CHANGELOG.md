# Changelog

Each release of xstd-bits, newest first. A version is a tag `vX.Y.Z` matching `project(VERSION)` in
[CMakeLists.txt](CMakeLists.txt); an entry is written from the pull requests merged since the previous tag, grouped
by what a user of the library meets. The numbers are this repository's pull requests.

## 0.1.0 (unreleased)

The first release: packed bit containers that each speak the vocabulary of the standard container they pack. There
are three readings of a block of bits, and each comes at a fixed width, within an inline capacity and at a run-time
width:
- the set, read as `std::set<std::size_t>`;
- the sequence, read as `std::vector<bool>`;
- the valarray, read as `std::valarray<bool>`.

Views read bits someone else owns, and the `std::ranges` algorithms over them run a block at a time.

It needs a C++23 compiler, and CMake 3.30 or later for the supplied CMake project. It also needs:
- xstd-ints and xstd-misc, which the supplied project fetches at their v0.1.0 tags (#531);
- Boost.Hash2;
- Boost.Container, where the standard library has no `std::inplace_vector`.

### The sequences: `<xstd/bits/bit_array.hpp>`, `<xstd/bits/bit_inplace_vector.hpp>` and `<xstd/bits/bit_vector.hpp>`

- `bit_array<N>`, `bit_inplace_vector<N>` and `bit_vector` answer `std::array<bool, N>`'s,
  `std::inplace_vector<bool, N>`'s and `std::vector<bool>`'s synopses line for line. Each has the standard's
  constructors and deduction guides (#231, #260, #501). They depart only where a bit forces it: no nodes, a proxy
  reference, and iterators that outlive a reallocation (#183).
- **Exception guarantees:** an inplace vector gives the standard's overflow guarantees (#276). Erasing never allocates
  (#306). Copy and move assignment and `append` give the strong guarantee (#315, #316). Insertion happens in place
  where the capacity holds the growth (#445), and growth goes through the storage's own `resize` (#450).
- `basic_bit_vector<Blocks>` takes any `bit_storage` as its block container, as `std::flat_set` takes its container:
  a `std::pmr::vector`, Boost's `small_vector` or one of your own (#497).
- `bit_array` is a structural type at widths its blocks fill, so it can be a template argument (#288, #289). Size-like
  members that the type fixes are `static` `integral_constant`s (#335).
- Each family has erasure and hashing, and `bit_array` the tuple interface and `to_bit_array` (#472).

### The sets: `<xstd/bits/bit_set.hpp>`

- `bit_fixed_set<N>`, `bit_inplace_set<N>` and `bit_set` answer `std::set<std::size_t>`'s synopsis, with its
  comparators (#259, #336).
  - Lookups are total over the key type, as they are on `std::set` (#90).
  - Inserting a value that is no key is refused, at every width (#361).
  - A shift truncates at `max_size()` (#303).
- **Keys of your own:** any unsigned integer, or a signed range through an offset mapping, can be the key, through a
  `bit_key_mapping` and the `basic_` forms (#344, #351).
  - `bit_enum_set<Enum>` is keyed by an enumeration's listed values (#346).
  - `bit_flag_set<Mask, N>` is keyed by a bitmask type's one-bit values and converts to and from the mask (#347,
    #350, #355, #356). It is checked against every bitmask type the standard defines (#358).
- **Set algebra:** the sets have an integer's bitwise operators, with checked named members beside them (#505).
  Containment and overlap are algorithms, below.
- A fixed-width set is packed with one PEXT per block (#514).
- `basic_bit_set` takes any `bit_storage`, as the vectors do (#497).

### The valarrays: `<xstd/bits/bit_valarray.hpp>`

- `bit_fixed_valarray<N>`, `bit_inplace_valarray<N>` and `bit_valarray` answer `std::valarray<bool>`: the
  elementwise operators `std::bitset`'s were, a row combined whole (#391).
- `[valarray.sub]`'s four subscripts and their helper arrays are here (#418). Selectors and masks also reach a
  `std::valarray` (#526).

### The views: `<xstd/bits/bit_span.hpp>` and `bit_set_view`

- `bit_span` and `bit_subspan` read bits another object holds as a sequence, whole or from any bit offset. A width can
  be carried in the type: `first<Count>`, `last<Count>` and `subspan<Offset, Count>` (#221, #226).
- `bit_set_view` reads them as a set (#221, #243).
- The views convert, combine and compare as `std::span` and `std::string_view` do (#384).

### The algorithms and iterator operations: `<xstd/bits/bit_algorithm.hpp>` and `<xstd/bits/bit_iterator.hpp>`

- The algorithms sit outside the containers and work a block at a time, each named after the `std::ranges` algorithm
  whose arguments and result it takes, in `xstd::ranges` with a `bit_` prefix (#387, #471). That covers:
  - the searches and quantifiers;
  - the copy family, `bit_fill`, `bit_sort` and `bit_partition`;
  - `bit_rotate`, `bit_reverse` and the shifts (#352, #417);
  - `bit_equal` and the three-way comparison, across block types and storages (#381);
  - `bit_find_if` (#512).
- **Over two sets:** `bit_includes`, `bit_intersects` and `bit_disjoint`, and the four set operations, which return a
  set (#394, #395).
- **Iterator operations:** `bit_advance`, `bit_next`, `bit_prev` and `bit_distance` step a sequence by offset, and a
  set by select and rank, a block at a time (#417, #469).

### Conversions: `<xstd/bits/bit.hpp>` and `<xstd/bits/ext/boost/dynamic_bitset.hpp>`

- `bit_convert` moves the positions of any bit storage into any other, at any two widths (#326, #419). Narrowing a
  run-time width into a fixed one throws `std::overflow_error` for a position that does not fit.
- `std::bitset` and `boost::dynamic_bitset` interoperate through `bit_convert` rather than being read in place (#321).
- `bit_compress`, `bit_expand` and `has_single_bit` work over owners of any width, as `[bit.permute]` and
  `[bit.pow.two]` have them (#510).

### Concepts, type traits, function objects and tags

- `<xstd/bits/bit_concepts.hpp>` and `<xstd/bits/bit_type_traits.hpp>` give every concept and transformation a header
  of its own (#363). `bit_convertible_to` is the one public conversion concept (#370).
- **Choosing the block:** `bit_least`, `bit_fast` and `bit_align` choose it by type transformation (#354).
  `bit_rebind` and `bit_resize` change an owner's block and a fixed owner's width (#372).
- Every concept and type trait is cv-transparent, as xstd-ints' are, and volatile storage is refused at the constraint
  (#443, #447).
- **`<xstd/bits/bit_functional.hpp>`:**
  - `<functional>`'s comparisons work across block types and storages (#461).
  - `xstd::hash` hashes a container as its standard model, through Boost.Hash2, and `std::hash` is built on it (#365,
    #366, #368).
  - `bit_hash` and `bit_digest` hash the packed bits, and avalanche as `xstd::hash` does (#463).
- **Tags:** `from_blocks` reads an argument's blocks as bits. `indeterminate` leaves a fixed owner's blocks unwritten
  for a later assignment (#505).

### `<xstd/bits/ext/boost.hpp>`

- `bit_small_set<N>`, `bit_small_vector<N>` and `bit_small_valarray<N>` keep up to `N` bits inline in Boost's
  `small_vector` and the rest on the heap (#214, #472).

### Platforms

GCC 15, 16 and trunk; Clang 22, 23 and trunk, with libstdc++ and with libc++; Apple Clang from Xcode 16.4 and 26.6; MSVC
and clang-cl on Visual Studio 2022, 2026 and 2026 Preview. Also MinGW 15 and 16, Emscripten for wasm32, i686 (#398),
and big-endian s390x under QEMU (#495). A test builds the library under `-fno-exceptions`, where every throw aborts
(#511), and a hardened release keeps the preconditions a hardened standard library checks (#446). The README's matrix gives the exact versions.

### How it is checked

- **Spec and audit:** each counterpart's clauses are read one declaration at a time into spec tests, including the
  uses that must be rejected (#269–#278, #308). The paragraphs are audited in CI (#279–#282).
- **Standard library suites:** libc++'s, libstdc++'s and the MSVC STL's own tests run against the vectors and the sets
  (#501, #519–#527, #531, #533).
  - They found three library bugs, each fixed where it was found.
  - GCC trunk's tests run weekly as an early warning (#525).
- **Mutation testing:** every mutant of `bit_algorithm/` and of the block storage is killed or listed with its reason
  (#455, #516, #518).
- **Fuzzing and allocation failure:** differential fuzzing against the standard models (#304), and every modifier's
  exception guarantee tested with a refusing allocator (#295).
- **Codegen:** the assembly the block kernels compile to is checked (#509).
- **Layering and coverage:** every public header builds alone (#514, #517), and line and branch coverage holds at 100%
  (#60).
- **Reference:** every public entity is specified in a reference, published as a site (#475–#477).
