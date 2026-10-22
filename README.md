# Bitwise extensions to the C++ Standard Library

> "The reasonable man adapts himself to the world: the unreasonable one persists
> in trying to adapt the world to himself. Therefore all progress depends on the
> unreasonable man."
>
> — George Bernard Shaw, *Man and Superman* (1903), "Maxims for Revolutionists"

[![Project Status: WIP](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)
[![License](https://img.shields.io/badge/license-Boost-blue.svg)](https://opensource.org/licenses/BSL-1.0)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/rhalbersma/xstd-bits/badge)](https://scorecard.dev/viewer/?uri=github.com/rhalbersma/xstd-bits)

xstd-bits is a header-only C++ library for bit twiddling, suited to both a sequence of bools and an ordered set of
integers. It takes over the roles of the well-established `std::bitset`, `std::vector<bool>` and
`boost::dynamic_bitset`, split along the lines of the standard containers each of them resembles:
`std::array<bool, N>`, `std::inplace_vector<bool, N>`, `std::vector<bool>`, `std::valarray<bool>` and
`std::set<std::size_t>`. By leveraging C++23 techniques and idioms, xstd-bits answers those containers' interfaces line
for line, while its algorithms work a whole block at a time: code that reads like the standard library runs a word at a
time, as hand-written flag words do. xstd-bits is easy to extend with your own key and block types, can view bits
stored anywhere, and interoperates out of the box with the standard library's ranges and hashing, Boost.Hash2,
Boost.ContainerHash and `boost::dynamic_bitset`.

## Rebooting the bitset franchise

Packed bits were in C++ from the start: a bitset was among the first library components proposed for
standardization, and the SGI STL shipped a packed vector of bools. The standard froze their early shapes, and thirty
years of re-implementations show the demand they were meant to meet never went away.

### The `bits` origin story

> "A `bitset` can be seen as either an array of bits or a set of integers. [...]
> Common usage suggests that dynamic-length `bitsets` are seldom needed."
>
> — Chuck Allison, [ISO/WG21/N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf), November 25, 1991

The first committee proposal for what became `std::bitset<N>` named both readings, and the standard shipped one type
that is both.

An unsigned integer is raw bit storage. Used directly, it is a *flag word*: a row of flags set,
cleared and tested with `&`, `|`, `^`, `~` and shifts, where the number it spells is beside the point.

`std::bitset` generalized the flag word to any width, and in doing so mixed three vocabularies:
that of a **sequence** of `bool` (`operator[]` returning a proxy `reference`, `test`/`set`/`reset`/`flip`
by position), that of a **set** of positions (`&`, `|`, `^`, `count`, `any`/`none`/`all`, and Boost's
`is_subset_of`/`intersects`), and that of a **string** of `'0'`s and `'1'`s. Its `to_ulong`/`to_ullong` and
its `unsigned long long` constructor are not arithmetic — it has no `+`, `-`, `*` or `/` — but access to
the storage when it fits one word. Being all of these, it is none of them cleanly: it has no iterators,
compares only at one width, and prints its bits in the opposite order from how it indexes them.

Its follow-up, [N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf)
(May 26, 1992), put bit 0 leftmost, as char 0 is in a string; the standard shipped the opposite, so `bitset<4>(1)`
prints as `"0001"`. The rest of the shipped interface shows its age too:

- In C++98, getting a `std::string` out took `b.to_string<char, std::char_traits<char>, std::allocator<char>>()`;
  `b.to_string()` arrived with C++11.
- Its integer constructor is not `explicit`, so `b == 5` compiles.
- `==`, `~`, `<<` and `>>` are members, while `&`, `|` and `^` are non-member templates.

### `bit_vector` redux

The SGI STL had a packed vector of bools, and called it [`bit_vector`](https://stl.boost.org/bit_vector.html):
"essentially a `vector<bool>`", a Sequence with `vector`'s interface and one bit per element, behind a proxy
`reference`. It was a class of its own for one reason, which its documentation gave in a warning: "The name
`bit_vector` will be removed in a future release of the STL. The only reason that `bit_vector` is a separate class,
instead of a template specialization of `vector<bool>`, is that this would require partial specialization of
templates."

Once compilers caught up, that is what happened, and `std::vector<bool>` has been explaining itself ever since. A
specialization changes what `vector<T>` promises for one `T`: at `T = bool`, `operator[]` returns no `bool&`, `data()`
no `bool*`, and code written against `vector<T>` breaks there. Scott Meyers's *Effective STL* (2001) gave it an item
of its own, Item 18: "Avoid using `vector<bool>`."

Howard Hinnant's [On `vector<bool>`](https://howardhinnant.github.io/onvectorbool.html) (2012) separated the two
questions: is an array of bits a good data structure, and should it be named `vector<bool>`? "Yes. No." Under a
`bit_vector` of its own, the range-for that fails to compile on `vector<bool>` fails the same way, but now the coder
sees a dynamic array of bits, whose bits no reference can bind to: "names are important." And with the algorithms
adapted to read a word at a time, the same data structure is a speed optimization as well as a space one.

In hindsight the workaround had the better design: a packed container chosen by its own name, with its proxy stated up
front, beside a `vector<bool>` that stays a vector. `xstd::bit_vector` restores the name the STL promised to remove.

### Thirty years of re-implementation

The demand outlived both designs. Boost added
[`boost::dynamic_bitset`](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/) for the width `std::bitset`
could not grow; LLVM keeps [`BitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/BitVector.h),
[`SmallBitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/SmallBitVector.h) and
[`SparseBitVector`](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/ADT/SparseBitVector.h), Qt
[`QBitArray`](https://doc.qt.io/qt-6/qbitarray.html), and EASTL a
[`bitvector`](https://github.com/electronicarts/EASTL/blob/master/include/EASTL/bitvector.h) of its own. Each one pays
again for what the standard's versions leave out.

### The reboot

xstd-bits separates the three. Each becomes the bit-packed counterpart of the standard container it
already resembled, speaking that container's vocabulary, with one interface across every storage. The
hybrid itself is not reproduced: there is no bitset type here. `std::bitset<N>` and `boost::dynamic_bitset`
interoperate through `xstd::bit_convert` instead.

| reading  | standard counterpart                                                       | packed here as                                              |
| :------- | :------------------------------------------------------------------------- | :---------------------------------------------------------- |
| sequence | `std::array<bool, N>`, `std::inplace_vector<bool, N>`, `std::vector<bool>` | `bit_array`, `bit_inplace_vector`, `bit_vector`             |
| valarray | `std::valarray<bool>`                                                      | `bit_fixed_valarray`, `bit_inplace_valarray`, `bit_small_valarray`, `bit_valarray` |
| set      | `std::set<std::size_t>`                                                    | `bit_fixed_set`, `bit_inplace_set`, `bit_set`               |
| string   | `std::string`                                                              | `bit_string` (planned)                                      |

The valarray reading is the sequence read elementwise: `std::bitset`'s `&`, `|`, `^`, `~` and shifts on a row of
bools are what `std::valarray<bool>` spells, so they live there, beside its comparisons that answer an array of bools,
its `sum`, `min` and `max`, and its `shift`, `cshift` and `apply`. The sequences keep their counterparts' interfaces
and nothing more.

Every one of them reads its raw blocks through `from_blocks`, and converts to and from anything else that has
bit storage through `xstd::bit_convert` — the general form of what `to_ullong` does for one word. Position `i`
stays position `i` at any two widths: a fixed width widens into a wider fixed one, zero-extended, and never
narrows, a run-time width converts into a fixed one by value and throws `std::overflow_error` for a position the
target cannot hold, and anything converts into an inplace, small or dynamic owner, which adopts the blocks outright
from an rvalue holding the same container. Because positions are kept, a `std::bitset`'s `to_string()` reads
reversed against its positions.

xstd-bits is **twelve containers**: two readings of a block of bits — an ordered set of `std::size_t` and a sequence of `bool` — over four storages, which differ in whether size and capacity are static or dynamic: both static, a dynamic size within a static capacity, a dynamic size kept inline up to a static capacity and on the heap past it, and both dynamic; and the sequence's elementwise reading, the valarray, over all four. Three views give the set or the sequence reading of bits that something else owns.

## Header offerings

### Which one do I want?

`std::bitset` is a Swiss Army knife: one tool that does everything passably. xstd-bits is a set of chef's knives instead, each shaped for one job and all forged from the same block storage, so the choice is made once, where the variable is declared. Three questions make it.

**1. Do you ask *which* positions hold, or *what* each position holds?** Membership, keys, flags and iterating over what is present are the **set** reading, which speaks `std::set`. Indexing, a `bool` at each place, and rotating or reversing a row are the **sequence** reading, which speaks `std::vector<bool>`. A row combined whole, `a & b`, `~a`, `a << n` and `a < b` element by element, is the **valarray** reading, which speaks `std::valarray<bool>`, and is where `std::bitset`'s bitwise operators went.

**2. How is the width known?** And **3. Do you own the bits?**

| the width is…                                     | set                     | sequence                             | valarray                  |
| :------------------------------------------------ | :---------------------- | :----------------------------------- | :------------------------ |
| fixed at compile time                             | `bit_fixed_set<N>`      | `bit_array<N>`                       | `bit_fixed_valarray<N>`   |
| at most `N`, never more                           | `bit_inplace_set<N>`    | `bit_inplace_vector<N>`              | `bit_inplace_valarray<N>` |
| usually at most `N`, inline; more on the heap     | `bit_small_set<N>`      | `bit_small_vector<N>`                | `bit_small_valarray<N>`   |
| known only at run time                            | `bit_set`               | `bit_vector`                         | `bit_valarray`            |
| whatever bits someone else owns                   | `bit_set_view`          | `bit_span`, and `bit_subspan` for a part of it | `bit_span` over one, as `std::span<bool>` over a `std::valarray<bool>` |

Three specializations of the fixed set cover the keys a set most often holds:

- the enumerators of an enumeration, `bit_enum_set<Enum>`;
- the one-bit values of a bitmask type, such as `std::filesystem::perms`, `bit_flag_set<Mask, N>`, which converts to and from the mask so existing code keeps its constants;
- a key type of your own, through a `bit_key_mapping` and the `basic_` forms.

The block is a separate and later choice. Every name above holds `std::size_t` blocks. `bit_least<X>` and `bit_fast<X>` take the smallest and the fastest block that holds the width, and `bit_align<X>` rounds the width up to whole blocks; `bit_rebind<Block, X>` names any block outright.

### Each one is the packing of a standard container, and speaks that container's vocabulary

The relationship is the one `std::flat_set` has to `std::set`: **a different representation under the same interface**, departing from it only where the representation forces a departure. `xstd::bit_vector` answers `std::vector<bool>`'s synopsis line for line, less its two `flip()`s and its deprecated static `swap`; `xstd::bit_array<N>` answers `std::array<bool, N>`'s, `xstd::bit_inplace_vector<N>` answers `std::inplace_vector<bool, N>`'s, and the three sets answer `std::set<std::size_t>`'s. Each is held to its counterpart by a checklist that spells that counterpart's synopsis out as a `requires`-expression and is asserted **on the counterpart first**, so a line the model itself cannot answer can never be asked of the packing.

The yardstick is the [current working draft](https://eel.is/c++draft/), not the standard the library compiles as. Where C++23 and the draft disagree the draft wins, and [design.md](doc/design.md) records which paper moved each line.

Three things the packing genuinely forces, and nothing else:

- **No nodes.** A position is a bit in a block, so there is nothing to unlink and hand over: `node_type`, `extract`, `insert(node_type&&)` and `merge` have no meaning here. `std::flat_set` drops the same four for the same reason.
- **A proxy reference.** A bit has no address, so `operator[]` returns a proxy, `pointer` names nothing, and `data()` goes with it. `std::vector<bool>` makes exactly this trade. The views lose what `std::span` builds on an address: its constructors from a contiguous iterator, an array or a contiguous range of `bool`, which a view over the blocks or the owner stands in for, and `size_bytes()` and `as_bytes`, a window's bits sharing their bytes with bits it does not view. The proxy has the const-qualified assignment of [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html) and the three hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html). `[vector.bool]`'s extras are left out, by choice rather than force: `flip()` on the container and on the proxy, which negating is `v[i] = not v[i]` and the valarray's `!`, and the static `swap(reference, reference)` the standard now deprecates.
- **Different invalidation — mostly the other way.** An iterator here is a container and an index, not a pointer into the blocks, so growth that reallocates the blocks leaves it valid. That is `std::set`'s guarantee over storage that is `std::flat_set`'s.

Everything else is addition rather than subtraction: the bitwise operators, the byte exchange, and the views that give you a second reading of bits you already own. What `std::ranges` spells as an algorithm stays one, a block at a time and outside the containers: over a sequence `xstd::ranges::bit_count`, `bit_all_of`, `bit_any_of`, `bit_none_of`, `bit_find`, `bit_find_last`, `bit_mismatch`, `bit_is_sorted`, `bit_is_partitioned` and `bit_partition_point` read it, `bit_copy`, `bit_copy_backward`, `bit_move`, `bit_swap_ranges`, `bit_reverse_copy`, `bit_rotate_copy`, `bit_transform`, `bit_fill`, `bit_sort` and `bit_partition` write it at any offset, and `bit_reverse`, `bit_rotate`, `bit_shift_left` and `bit_shift_right` permute it whole; over either reading `bit_for_each`, `bit_equal` and `xstd::ranges::bit_lexicographical_compare_three_way`, the last two across block types and storages; over two sets `xstd::ranges::bit_includes`, `bit_intersects` and `bit_disjoint`, and `bit_set_intersection`, `bit_set_union`, `bit_set_difference` and `bit_set_symmetric_difference` combining them; and over any iterator of ours `xstd::ranges::bit_advance`, `bit_next`, `bit_prev` and `bit_distance`, an offset step on a sequence and select and rank a block at a time on a set. Each takes the arguments and gives the result of the `std::ranges` algorithm it is named after, less the projections, predicates and comparators a range of bits has no use for; the iterator operations always take a count. `bit_array` keeps `fill`, which `std::array` has; no other container has it.

**A port adds the prefix.** Each `std` namespace becomes the matching `xstd` one, and the name takes `bit_`: `std::ranges::copy(v, out)` is `xstd::ranges::bit_copy(v, out)`, `std::ranges::count(v, b)` is `xstd::ranges::bit_count(v, b)`, and `std::ranges::advance(it, n, bound)` is `xstd::ranges::bit_advance(it, n, bound)` whatever `it` iterates. `xstd::ranges::bit_lexicographical_compare_three_way` takes two ranges, as [P2022](https://wg21.link/P2022)'s proposed `std::ranges::lexicographical_compare_three_way` would, although the standard so far has only the iterator-pair `std::lexicographical_compare_three_way`. On a set, counting keys is a distance: `std::ranges::distance(s)` is `xstd::ranges::bit_distance(s)`, and the keys in `[first, last)` are `bit_distance(first, last)`, where `std::ranges::count(s, k)` asks how often `k` occurs, which `s.contains(k)` answers. The one exception is the four set operations: `bit_set_intersection`, `bit_set_union`, `bit_set_difference` and `bit_set_symmetric_difference` return a set, what range-v3's views of those names hold, rather than writing through an output iterator.

### Out of range means what each counterpart means by it

The **set reading** takes a key, and a key outside the domain is a lookup that answers no rather than an error: `contains`, `find`, `count`, `lower_bound`, `upper_bound`, `equal_range` and `erase` are total, as they are on `std::set`. So is a value that is no key at all, an enumerator missing from the list or a flag value of several bits: it is no element, and where the key type has an order of its own, the bounds place it among the keys by that order. Writing one is refused at every width alike: `insert`, `emplace`, the ranged forms and an enumerator's `^=` throw `out_of_range` for a value its mapping ranks past the last key, on a growing set as on a fixed one, whose `max_size()` is then the mapping's `size`; only a key of an open universe reaches the storage's own limit, `length_error` on the heap and `bad_alloc` past an inplace set's capacity. A shift names no one key, so `<<=` keeps what lands below `max_size()` and drops the rest, as `std::bitset`'s does past `N`. The **sequence reading** indexes, so out of range is out of bounds there: `at(n)` throws `out_of_range` at every width and through every handle, and everything else is the precondition `std::vector`, `std::array` and `std::span` already make it — stated with an `assert`, at the member you called.

`constexpr` is not on that list of advantages, and has not been since [P3372R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3372r3.html) made the standard's own containers `constexpr` throughout. It is table stakes now; what the packing buys is density and the bit-parallel operations over it.

A constant can also be a template argument, as a `std::array<bool, N>` can, wherever the width fills its blocks: `template<xstd::bit_align<xstd::bit_fixed_set<64>> Mask> struct S;` takes a mask by value, and so does `bit_align<bit_array<N>>`. A width that leaves unused bits in its last block is not a structural type, since those bits must stay clear for `==`, `<=>` and hashing to hold; [design.md](doc/design.md#structural-at-aligned-widths) has the reasoning.

Where the type fixes a size, the member that reports it is a constant, after the [static `constexpr std::integral_constant` idiom](https://www.think-cell.com/en/career/devblog/the-new-static-constexpr-std-integral_constant-idiom): `size`, `empty` and `max_size` on `bit_array<N>` and on a `bit_subspan` of static extent, `capacity` and `max_size` on `bit_inplace_vector<N>`, and `max_size` on `bit_fixed_set<N>` and `bit_inplace_set<N>`, whose `size()` counts keys. `a.size()` and `A::size()` still answer a `size_type`, `noexcept`, so every call the counterpart allows means what it does there; `A::size` is also a `std::integral_constant<std::size_t, N>`, which `decltype` reaches through any reference as a constant expression. Two things follow: `auto n = A::size;` deduces the `integral_constant`, so `auto n = a.size();` is the spelling for a `size_type`, and `a.size` without parentheses compiles, naming the constant. [design.md](doc/design.md#constant-sizes) has the details.

### The headers

Twelve containers: two readings of a block of bits, each over four storages, and the elementwise reading of
the sequence over all four. The reading picks the vocabulary, the storage picks whether size and capacity
are static or dynamic. Each name is a class with the constructors of the standard
container it packs. These twelve owners, the three views and the algorithms over them are the public surface;
the adaptor each reading is built on is internal, under `<xstd/bits/detail/>`.

Every concept and type trait is cv-transparent, as xstd-ints' are: `T const` and `T volatile` answer as `T` does.
Storage is the exception, refused when volatile, since a block algorithm merges the accesses a register keeps
apart; a volatile register is converted by one read instead, `xstd::bit_convert<xstd::bit_array<32>>(status)`
([cv-transparency](doc/design.md#cv-transparency)).

The headers come at three levels:

- `<xstd/bits.hpp>` includes the whole library, the Boost extensions aside.
- An umbrella per standard header the library has counterparts for, named after it with a `bit_` prefix and
  sectioned by that header's synopsis: `<xstd/bits/bit_vector.hpp>` for `<vector>`, `<xstd/bits/bit_algorithm.hpp>`
  for `<algorithm>`, and so on. `<xstd/bits/bit.hpp>` extends `<bit>` itself, as xstd-ints' does.
- Beneath each umbrella, in the folder of its name, a header per entity or per family of tightly related entities,
  such as the three quantifiers or the four set operations.

The Boost extensions sit apart, under `<xstd/bits/ext/>`. The umbrellas are:

<div class="headers" markdown>

| Header | Description |
| :----- | :---------- |
| `<xstd/bits/bit_array.hpp>` | `bit_array`, the sequence of `bool` at a fixed width, with `to_bit_array`, the tuple interface and hashing. |
| `<xstd/bits/bit_inplace_vector.hpp>` | `bit_inplace_vector`, the sequence of `bool` within an inline capacity, with erasure and hashing. |
| `<xstd/bits/bit_vector.hpp>` | `bit_vector`, the sequence of `bool` on the heap, with erasure and hashing. |
| `<xstd/bits/bit_set.hpp>` | The ordered sets of keys at a fixed width, within an inline capacity and on the heap, the set view, the key mappings, the enum and flag sets, erasure and hashing. |
| `<xstd/bits/bit_valarray.hpp>` | The valarrays of `bool` on the heap, at a fixed width and within an inline capacity, with their slices and helper arrays. |
| `<xstd/bits/bit_span.hpp>` | `bit_span` and `bit_subspan`, the sequence reading of bits another object holds, whole or from any bit offset. |
| `<xstd/bits/bit_algorithm.hpp>` | The `std::ranges` algorithms over bits, a block at a time, and the tests of whether two sets intersect or are disjoint. |
| `<xstd/bits/bit_iterator.hpp>` | `bit_advance`, `bit_distance`, `bit_next` and `bit_prev`, an offset step on a sequence and select and rank on a set. |
| `<xstd/bits/bit_concepts.hpp>` | What holds bits, what a set asks of its key mapping, and what an owner takes as blocks or converts. |
| `<xstd/bits/bit_type_traits.hpp>` | The transformations that give an owner another block or width, and the blocks they choose. |
| `<xstd/bits/bit_functional.hpp>` | The comparisons across block types and storages, and the hash and digest of the bits. |
| `<xstd/bits/bit_ranges.hpp>` | `from_blocks`, the tag that reads an argument's blocks as bits. |
| `<xstd/bits/bit.hpp>` | `bit_convert`, the positions of any bit storage into any other, at any two widths. |
| `<xstd/bits/ext/boost.hpp>` | The three readings in Boost.Container's `small_vector`, inline up to a capacity and on the heap past it. |
| `<xstd/bits/ext/boost/dynamic_bitset.hpp>` | `bit_convert` to and from `boost::dynamic_bitset`. |

</div>

[The reference](doc/reference/index.md) lists each group's headers and specifies each entity.

A `bit_` prefix names what works on the packed bits: the containers, the concepts and transformations over blocks, the algorithms, and `bit_hash`. The unprefixed name works on the value as the standard model sees it, as `xstd::hash` does. The prefix names *what* is hashed, the packed representation, not whether the code is optimized: the hooks behind `xstd::hash` expand bits to bools with `_pdep_u64` or AVX-512 where the target has them, and still append the model's bytes.
The headers under a `detail/` directory are implementation and carry no stability promise.

A header directory takes the `bit_` prefix for the reason its names do: what it holds works on the packed bits.
`<xstd/bits/bit_algorithm/>` holds the packed counterparts of `<algorithm>`'s, as `bit_vector` is the packed `std::vector<bool>`, and `<xstd/bits/bit_concepts/>` and `<xstd/bits/bit_type_traits/>` hold the concepts and transformations over blocks.
`<xstd/bits/bit/>` alone keeps a standard header's name, since that name already says it: it extends `<bit>`, as xstd-ints' `bit/` does.

## Migration path

`std::vector<bool>` and `std::set<std::size_t>` become `bit_vector` and `bit_set` by changing the type: each answers its counterpart's interface, so the code around it stays. A `std::bitset<N>` combined with `&`, `|`, `^`, `~` and shifts becomes a `bit_fixed_valarray<N>`, and a `boost::dynamic_bitset` a `bit_valarray`. The sources whose interface differs, `std::bitset`, integer flag words, enumeration flags and Qt's `QFlags`, `boost::dynamic_bitset` and itsy_bitsy, each have a before-and-after table in [doc/migration.md](doc/migration.md).

## Usage

### Hello World: generating (twin) primes

The code below demonstrates how a standards conforming `set` implementation can be used to implement the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes). This algorithm generates all [prime numbers](https://en.wikipedia.org/wiki/Prime_number) below a number `n`. It is the library's worked example and its benchmark, and it lives in [`examples/include/opt/set/sieve.hpp`](examples/include/opt/set/sieve.hpp) — outside `include/`, because a workload is a subject rather than library code. The listing below is that header's body, not a paraphrase of it; the names are in namespace `opt`.

```cpp
template<class X>
auto sift(X& primes, std::size_t m)
{
        primes.erase(m);
}

template<class X>
auto generate_candidates(std::size_t n)
{
        return std::views::iota(2UZ, n) | std::ranges::to<X>();
}

// Iterate a snapshot and guard with contains(): sift() erases, which invalidates a vector-backed X's cached end().
template<class X>
auto sift_primes0(std::size_t n)
{
        auto primes = generate_candidates<X>(n);
        auto const candidates = primes;
        for (auto p
                : candidates
                | std::views::take_while([&](auto x) { return x * x < n; })
        ) {
                if (not primes.contains(p)) {
                        continue;
                }
                for (auto m = p * p; m < n; m += p) {
                        sift(primes, m);
                }
        }
        return primes;
}
```

Two details are worth pausing on, because both were learned the hard way. The inner walk is `m += p` rather than `views::iota(p * p, n) | views::stride(p)`: `views::stride` is a C++23 adaptor that libc++ has not implemented, and the library supports all three standard libraries. And the outer loop iterates a **snapshot** while guarding with `contains()`, because `sift()` erases from `primes`, which invalidates a cached `end()` for any vector-backed container — `std::flat_set` among them.

Given a set of primes, generating the [twin primes](https://en.wikipedia.org/wiki/Twin_prime) means keeping every prime that has a neighbour exactly 2 away:

```cpp
template<class X>
auto filter_twins(X const& primes)
{
        using key = std::ranges::range_value_t<X>;
        auto twins = X();
        auto first = std::ranges::begin(primes);
        auto const last = std::ranges::end(primes);
        if (first == last) {
                return twins;
        }
        auto prev = static_cast<key>(*first++);
        if (first == last) {
                return twins;
        }
        auto self = static_cast<key>(*first++);
        for (; first != last; ++first) {
                auto const next = static_cast<key>(*first);
                if (self - 2 == prev or self + 2 == next) {
                        twins.insert(self);
                }
                prev = self;
                self = next;
        }
        return twins;
}
```

This returns **both** members of each twin pair — `{3, 5, 7, 11, 13, ...}`, [OEIS A001097](https://oeis.org/A001097) — rather than the lesser of each pair, `{3, 5, 11, 17, ...}`, [A001359](https://oeis.org/A001359). Both are called "the twin primes" in the wild, so the choice is worth stating.

The calling code for these algorithms is listed below. Here, the pretty-printing as a `set` using `{}` delimiters is triggered by the nested `key_type`. Note that `xstd::bit_fixed_set<N>` acts as a **drop-in replacement** for `std::set<int>` (or `std::flat_set<int>`).

```cpp
int main()
{
    constexpr auto N = 100UZ;
    using X = xstd::bit_fixed_set<N>; /* or xstd::bit_set, std::set<std::size_t>, std::flat_set<std::size_t> */

    auto const primes = opt::sift_primes0<X>(N);
    assert(std::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto const twins = opt::filter_twins(primes);
    assert(std::format("{}", twins)  == "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}");
}
```

Those two assertions are the ones [`test/src/bits/std_set/sieve.cpp`](test/src/bits/std_set/sieve.cpp) makes, over `std::set`, `std::flat_set`, `bit_fixed_set<N>` and `bit_set` alike.

### Sieves that need no bound

`generate_candidates` materializes every candidate below `n` before sifting one, which is what makes the sieve above `O(n)` in space and why it cannot answer "what is the next prime". The same header carries two variants that drop the bound, and they drop it in opposite directions:

```cpp
auto sieve = opt::incremental_sieve();
sieve.next();  // 2, then 3, 5, 7, ... forever, with no n anywhere

auto primes = opt::sift_primes_segmented<xstd::bit_set, xstd::bit_fixed_set<1 << 15>>(n);
```

The **incremental** sieve ([O'Neill 2009](https://www.cs.hmc.edu/~oneill/papers/Sieve-JFP.pdf)) keeps one entry per prime found — the next composite that prime will strike — so its space is `O(π(n))` and it generates without end. It is also about **29× slower**, which is the honest price of unboundedness and the reason it is measured rather than recommended.

The **segmented** sieve is the one that pays. Base primes below `√n` once, then a single reusable window walked over the rest, so peak memory is `O(√n + W)` whatever `n` is. `Window` is a template parameter carrying its own extent, which makes `bit_fixed_set<W>` the natural argument: a compile-time width that allocates nothing in the loop. At `n = 2^20` it is **1.7× faster** than the bounded sieve as well as far thriftier — the window stays in L1 for a whole segment where a megabit sieve is walked with a stride. Less memory *and* less time is the unusual direction for that trade to run.

All three agree, and the test asserts that rather than the README claiming it.

### A packed `std::vector<bool>`

`xstd::bit_vector` is `std::vector<bool>` with the packing declared in its name rather than hidden in a specialization: the same interface, so the code around it stays, and the same proxy reference, so the `std::ranges` algorithms run over it unchanged. What it adds is the algorithms that read the blocks rather than the bools:

```cpp
#include <xstd/bits/bit_algorithm.hpp> // bit_count, bit_rotate
#include <xstd/bits/bit_vector.hpp>    // bit_vector
#include <algorithm>                    // count, equal
#include <cassert>                      // assert
#include <format>                       // format
#include <vector>                       // vector

auto main() -> int
{
        auto v = xstd::bit_vector{true, false, true};
        v.push_back(true);
        v.resize(6);
        v[1] = not v[1];
        assert(std::format("{}", v) == "[true, true, true, true, false, false]");

        assert(xstd::ranges::bit_count(v, true) == std::ranges::count(v, true));
        xstd::ranges::bit_rotate(v, v.begin() + 4);
        assert(std::format("{}", v) == "[false, false, true, true, true, true]");

        auto const w = std::vector<bool>(v.begin(), v.end());
        assert(std::ranges::equal(v, w));
}
```

A sequence formats in brackets, as `std::vector<bool>` does, where a set formats in braces. `std::ranges::count(v, true)` walks the proxies a bool at a time, and `xstd::ranges::bit_count(v, true)` answers the same question, with the same signed result, a popcount per block; `xstd::ranges::bit_rotate(v, middle)` is `std::ranges::rotate(v, middle)` done over the blocks. [design.md](doc/design.md#the-sequence-aggregates) has the measurements.

### Hashing, and choosing the algorithm

Every owner but the valarrays, whose `==` answers element by element as `std::valarray`'s does, and every set view, hashes, and equal values hash equal whatever holds them. Behind it is [Boost.Hash2](https://www.boost.org/doc/libs/release/libs/hash2/), and there are two hashers, by what they hash:

- **`xstd::hash<T, H, Flavor>` hashes the value**, as its standard model does. It is [xstd-misc](https://github.com/rhalbersma/xstd-misc)'s, and runs Hash2's `hash_append`, which each type here meets through a hook that appends exactly the message Hash2 builds for the model: `std::array<bool, N>` for `bit_array<N>`, `std::vector<bool>` for `bit_vector` and `bit_small_vector`, `std::inplace_vector<bool, N>` for `bit_inplace_vector<N>`, and `std::set<Key, Compare>` for every set. So `xstd::hash<T, H, Flavor>()(x) == xstd::hash<Model, H, Flavor>()(model(x))`, and a bit container is a drop-in member of a Hash2-hashed aggregate. It costs what the model costs: a byte per bool, a word per key.
- **`xstd::bit_hash<T, H, Flavor>` hashes the bits** of a key `T`, from [`<xstd/bits/bit_functional/bit_hash.hpp>`](include/xstd/bits/bit_functional/bit_hash.hpp). Its message is the value's bits as bytes, bit `i` at bit `i % 8` of byte `i / 8` with the unused high bits clear, followed by the width only where the width is a run-time value; a set of run-time width stops at the byte of its last key and appends that byte count instead, since equal sets need not share a width. Wherever the storage already holds those bytes, little-endian blocks of `std::uint8_t` up to `unsigned __int128`, that is one `update` over the storage; elsewhere, and in a constant expression, the same bytes are assembled. The block type is no part of the message, so `bit_hash` is transparent. Its call operator answers a `std::size_t`. `xstd::bit_digest<T, H, Flavor>`, from [`<xstd/bits/bit_functional/bit_digest.hpp>`](include/xstd/bits/bit_functional/bit_digest.hpp), answers the algorithm's whole `result_type` over the same message, for a digest that stands in for its key: by default XXH3's 128 bits over little-endian bytes, the same on every target, where `bit_hash` defaults to native bytes as `std::hash` runs it. `xstd::bit_hash_append(h, f, x)` is the same message for a user's own hook, and also takes a contiguous range of one static-width container, nested arrays included, as the planes of a board game's position: where each plane's object is its bytes, the whole range goes in as one `update`.

`std::hash<T>` is `xstd::bit_hash<T, boost::hash2::xxhash_64>`, which is also `bit_hash`'s default: unseeded, which suits keys your program makes. It is xxHash at every length, never FNV-1a, because FNV-1a does not avalanche: the high bits of the last byte it reads never reach the low bits of its result, which are the bits a hash table indexes by. For keys an adversary can choose, name SipHash and seed it per container, as Hash2 advises:

```cpp
#include <xstd/bits/bit_functional/bit_hash.hpp> // xstd::bit_hash
#include <xstd/bits/bit_set/bit_set.hpp>
#include <boost/hash2/siphash.hpp>  // siphash_64
#include <cstdint>
#include <unordered_set>

using hash = xstd::bit_hash<xstd::bit_set, boost::hash2::siphash_64>;

auto const seed = std::uint64_t{/* drawn at random, per container */};
auto table      = std::unordered_set<xstd::bit_set, hash>(0, hash(seed));
```

The same spelling works for every other owner and set view, with `xstd::hash` in place of `xstd::bit_hash`, and with `boost::unordered_flat_set` as with `std::unordered_set`. A sequence view hashes no more than `std::span` does. [design.md](doc/design.md#the-hashing-invariant) has what each hasher appends and why the default is xxHash.

## Requirements

- A conforming [C++23](https://wg21.link/N4950) compiler
- CMake 3.30 or later when using the supplied CMake project
- [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc), fetched automatically via CMake `FetchContent` when not already installed
- [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support
- [Boost.Container](https://github.com/boostorg/container) for the inplace column's blocks where the standard library has no `std::inplace_vector`, and for the `ext/` column
- [Boost.DynamicBitset](https://github.com/boostorg/dynamic_bitset), only where `<xstd/bits/ext/boost/dynamic_bitset.hpp>` is included: the `dynamic-bitset` feature in `vcpkg.json`, and a `Boost::dynamic_bitset` the consumer links itself

This library depends on the C++ Standard Library, on [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc) (both fetched automatically via CMake `FetchContent` when not already installed), on [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support, and on [Boost.Container](https://github.com/boostorg/container) for the inplace column where the standard library has no `std::inplace_vector`. It is continuously being tested with the following conforming [C++23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf) compilers, against all three mainstream standard libraries (libstdc++, the MSVC STL, and libc++). Following the model of [apt.llvm.org](https://apt.llvm.org/), we support the latest two stable releases of each compiler, plus its current development branch.

Two standards are in play and they answer different questions. **C++23 is what the library compiles as** — that is the language it requires, and the tests build at C++26 as well — on GCC 16, GCC 17-SVN and MinGW 16, the three rungs whose libstdc++ carries `<inplace_vector>`. The inplace column exists at either standard and on every standard library: its blocks are a `std::inplace_vector` where the library has one and a `boost::container::static_vector` everywhere else. Only over `std::inplace_vector` are the inplace owners usable in a constant expression, and `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined exactly there, for code to test with `#ifdef`; over `static_vector` their copies are not `noexcept` either, as `static_vector`'s own are not, while their `swap` is `noexcept` over both, as `[inplace.vector.modifiers]` declares it. **The current [working draft](https://eel.is/c++draft/) is what the interfaces are measured against**, because a counterpart's synopsis is a moving target and the newest one is the one worth answering. So `std::bitset`'s `basic_string_view` constructor ([P2697R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2697r1.pdf)) and its `charT` Constraints ([LWG 4294](https://cplusplus.github.io/LWG/issue4294)) are here even though C++23 has neither, and the proxy carries the hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html) and not the static `swap(reference, reference)` that paper moved to `[depr.vector.bool.swap]`.

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## Installation


The library is header-only and its CMake target carries everything a consumer needs: the include directories, the `xstd-ints`, `xstd-misc`, `Boost::container` and `Boost::hash2` dependencies, and `cxx_std_23`. Link `xstd::bits` and you are done — there is no `target_include_directories` or `CMAKE_CXX_STANDARD` to set on your side.

All three methods below are built by the [Consumption workflow](.github/workflows/consumption.yml), on every pull request, so what is written here is what is tested.

### `find_package`, against an installed copy

```cmake
find_package(xstd-bits 0.1.0 CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE xstd::bits)
```

Installing needs no test dependencies:

```sh
cmake --preset no-tests-vcpkg          # or --preset no-tests, if Boost.Hash2 is already findable
cmake --build --preset no-tests-vcpkg
cmake --install build/no-tests-vcpkg --prefix /where/you/want/it
```

The installed package config calls `find_dependency` for `xstd-ints`, `xstd-misc` and `boost_hash2`, so those three have to be findable from the consuming project too. The version file is written `SameMinorVersion`, so `find_package(xstd-bits 0.1.0)` accepts 0.1.x and rejects 0.2.0.

### `add_subdirectory`, against a vendored copy

```cmake
add_subdirectory(external/xstd-bits)
target_link_libraries(my_target PRIVATE xstd::bits)
```

`xstd::bits` is an alias of the real target, so it spells the same thing here as under `find_package`. The tests and benchmarks are guarded by `PROJECT_IS_TOP_LEVEL` and do not configure when the library is a subdirectory, whatever your `BUILD_TESTING` is set to.

### `FetchContent`, against the repository

```cmake
include(FetchContent)
FetchContent_Declare(
    xstd-bits
    GIT_REPOSITORY https://github.com/rhalbersma/xstd-bits.git
    GIT_TAG        main          # pin a commit for a reproducible build
)
FetchContent_MakeAvailable(xstd-bits)
target_link_libraries(my_target PRIVATE xstd::bits)
```

`xstd-ints` and `xstd-misc` are fetched in turn if they are not already installed, at the commits `CMakeLists.txt` pins. Boost.Hash2 is not fetched — it is a `find_package(... REQUIRED)`, so it has to be installed, whether through vcpkg, your distribution, or a Boost tree you already have.

### The vcpkg manifest

[`vcpkg.json`](vcpkg.json) is this repository's own manifest, not a published port: it is what `VCPKG_ROOT`-based presets install from when you build **this** library. Its `test` feature — Boost.Test, Boost.Dynamic Bitset, Boost.Unordered, Google Benchmark and range-v3 — is a default feature because building the repository normally means building its tests. The `no-tests-vcpkg` preset turns that off with `VCPKG_MANIFEST_NO_DEFAULT_FEATURES`, so a packaging or install build pays for Boost.Hash2 and nothing else. Consuming the library by any of the three methods above does not read this manifest at all.

## Continuous Integration

We continuously test the stable, qualification, and development branches of the
major [C++23](https://wg21.link/N4950) toolchains (compilers and standard
libraries) in both Debug and Release mode:

| Platform | Compiler | Standard Library | Stable | Qualification | Development | Status |
| :------- | :------- | :--------------- | :----- | :------------ | :---------- | :----- |
| Linux | GCC | libstdc++ | 15 | 16 | 17-SVN | [![GCC](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml) |
| Windows | MinGW | libstdc++ | 15 | 16 | — | [![MinGW](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml) |
| Linux | Clang | libstdc++ | 22 (libstdc++ 15) | 23 (libstdc++ 16) | 24-SVN (libstdc++ 17-SVN) | [![Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml) |
| Linux | Clang | libc++ | 22 | 23 | 24-SVN | [![Clang-libc++](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml) |
| macOS | Apple Clang | libc++ | 17.0.0 (Xcode 16.4) | 21.0.0 (Xcode 26.6) | — | [![Apple Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml) |
| Windows | Clang-CL | MSVC | 19.1.5 (VS 2022) | 20.1.8 (VS 2026) | 20.1.8 (VS 2026-Preview) | [![Clang-CL](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml) |
| Windows | MSVC | MSVC | 2022 | 2026 | 2026-Preview | [![MSVC](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml) |
| Web | Emscripten | libc++ | 6.0.12 | — | — | [![Emscripten](https://github.com/rhalbersma/xstd-bits/actions/workflows/emscripten.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/emscripten.yml) |

The `Web` row runs one Emscripten release on the stable rung, in Debug: wasm32 under node, where `size_t` is 32 bits but 64-bit integers keep their 8-byte alignment. The GCC workflow adds the other 32-bit leg, i686 with GCC `-m32`, where `size_t` is 32 bits and there is no `__int128`.

`Apple Clang` has no `Development` entry because Apple doesn't publish Apple Clang dev snapshots the way LLVM does; that leg tests the latest stable Xcode release from each of the two supported series. `MinGW` has none either: WinLibs publishes no GCC trunk build between stable branches, and the shared workflow drops that rung with a notice rather than failing, so the row names the two that actually run. The `Linux | GCC` row keeps its `17-SVN` — that ladder is unaffected.

`MSVC` has its `Stable` entry back, and what it cost is worth recording. Two things stood between MSVC 17 (VS 2022) and this library. The first was alias-template deduction: `bit_set_view`, `bit_span` and `bit_subspan` were aliases, and `error C2976: 'xstd::bit_set_view': too few template arguments` is what MSVC 17 made of that. Making the views class templates with their own deduction guides settled it, for reasons of its own. The second was a ledger the rung's absence had opened: with no MSVC 17 leg to answer to, `decay_copy` became `auto(x)` ([P0849R8](https://wg21.link/P0849R8), which MSVC 17 does not implement) and a `typename` came off a constrained type-parameter's default argument. Both are back — a three-line helper in each of two adaptors, and one keyword under a `NOLINT` — which is the whole price of the rung. [design.md#the-views-are-the-adaptors](doc/design.md#the-views-are-the-adaptors) records the trade.

Every leg above passes. The library no longer uses the C++23 range adaptors libc++ has not implemented (`views::cartesian_product`, `views::adjacent`, `views::pairwise_transform`, `views::stride`), and the tests probe for `<flat_set>` rather than assuming it, so the `Clang | libc++` and `Apple Clang` rows and the `Clang-CL` VS 2022 legs build and run like the rest.

Beyond the compilers, CI holds coverage at 100%, tests the tests by mutation, consumes the library through
`find_package`, `add_subdirectory` and `FetchContent`, runs the sanitizers, analyzes the code with Clang-Tidy, MSVC's `/analyze` and CodeQL, and builds the
documentation site:

[![Coverage](https://codecov.io/gh/rhalbersma/xstd-bits/branch/main/graph/badge.svg)](https://codecov.io/gh/rhalbersma/xstd-bits)
[![Mutation](https://github.com/rhalbersma/xstd-bits/actions/workflows/mutation.yml/badge.svg)](doc/mutation.md)
[![Consumption](https://github.com/rhalbersma/xstd-bits/actions/workflows/consumption.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/consumption.yml)
[![Sanitizers](https://github.com/rhalbersma/xstd-bits/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/sanitizers.yml)
[![Clang-Tidy](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-tidy.yml)
[![MSVC-Analyze](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc-analyze.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc-analyze.yml)
[![CodeQL](https://github.com/rhalbersma/xstd-bits/actions/workflows/codeql.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/codeql.yml)
[![Documentation](https://github.com/rhalbersma/xstd-bits/actions/workflows/pages.yml/badge.svg)](https://rhalbersma.github.io/xstd-bits/)

## License


<pre>
         Copyright Rein Halbersma 2014-2026.
Distributed under the <a href="http://www.boost.org/users/license.html">Boost Software License, Version 1.0</a>.
   (See accompanying file LICENSE_1_0.txt or copy at
         <a href="http://www.boost.org/LICENSE_1_0.txt">http://www.boost.org/LICENSE_1_0.txt</a>)
</pre>
