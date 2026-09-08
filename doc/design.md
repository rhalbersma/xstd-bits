> **Superseded — kept for reference, not maintained.**
>
> The live design document is [`design.md`](../design.md) in the repository root: it is the one the
> `[design.md#anchor]` comments throughout the headers point at, and the one kept current. This file
> predates the adaptor refactor and the inplace column, and describes two storage vehicles carrying three
> interfaces where there are now three of each. Where the two disagree, the root document is right.
>
> It is retained because its framing of the naming rule and of the views is still worth reading; folding
> what survives into the root document and deleting this file is [#115](https://github.com/rhalbersma/xstd-bits/issues/115).

# xstd-bits

## Two vehicles, three Standard sections

The whole library is two storage vehicles carrying the same three interfaces.

|                     | array vehicle              | vector vehicle                |
| ------------------- | -------------------------- | ----------------------------- |
| storage             | `block_array<Block, N>`    | `block_vector<Block, Alloc>`  |
| `<array>`/`<vector>`| `bit_array<N, Block>`      | `bit_vector<Block, Alloc>`    |
| `<set>`             | `bit_static_set<N, Block>` | `bit_set<Block, Alloc>`       |
| `<bitset>`          | `bitset<N, Block>`         | `dynamic_bitset<Block, Alloc>`|

Both storage rows are `xstd::block_sequence<Blocks, N>`, which is public rather than
a detail: it is one class parameterized on what it packs into, with the width as a
template argument when that is a constant and as a data member when it is not. The
two aliases are named and ordered after what they hold, so `block_array<Block, N>`
and `block_vector<Block, Alloc>` read as `std::array` and `std::vector` do.

Rows are Standard sections, columns are vehicles. The four containers are named by
one rule: `bit_` and the container it packs, for `container` in {`array`, `vector`,
`static_set`, `set`}.

`bit_` is a storage-strategy prefix, and the Standard already has the other one.
`std::flat_set` and `std::flat_map` keep a sorted sequence of the elements that are
there, so they are sparse in the universe of possible keys; `bit_set` keeps one bit
per position in that universe, so it is dense but packed. Same container, same
interface, different representation, and the prefix is what says which -- which is
why it has to lead. `static_bit_set` reads as a qualified `bit_set` and breaks the
parallel with `flat_set`; `bit_static_set` is `bit_` applied to a `static_set`, the
way `flat_set` is `flat_` applied to a `set`.

Within the rule the unqualified name goes to the dynamic member of each pair, as
`std::vector` has it against `std::array`, so the fixed-size set is `bit_static_set`
and the dynamic one is `bit_set`.

The qualifier is `static` rather than `finite` because `finite` selected nothing:
`bit_set<Block, Alloc>` is a finite set of positions too, as every bit set is. What
separates them is that `N` is a compile-time constant, which is *static*, and
static/dynamic is one of the two axes the design is built on -- so the name reads off
the design rather than off a true-but-non-distinguishing adjective. Recorded against
it: P0843 renamed `boost::static_vector` to `std::inplace_vector` partly because
*static* is overloaded in C++. Accepted anyway, because `inplace` names where the
storage lives, which is the interesting property for a vector with static
capacity and runtime size, where ours is a genuinely fixed compile-time extent.

`bitset` and `dynamic_bitset` are outside the rule on purpose: they are not `bit_`
anything, they are the legacy types reproduced under their own names.

The bottom row is not a third interface. It exists to show that the same two
vehicles reproduce the legacy types at no cost -- `xstd::bitset` against
`std::bitset`, `xstd::dynamic_bitset` against `boost::dynamic_bitset` -- so a thin
interface over the array implementation throws nothing away.

Every container the library provides is one header named after the type it
declares, directly under `xstd/bits`. A directory per Standard section was tried
and dropped: with one entity behind each, `xstd/bits/set/bit_static_set.hpp` and
`xstd/bits/bitset/bitset.hpp` spent a path component to say what the filename
already said, and the section umbrellas above them re-exported a single header
apiece. The Standard's own sections are still the organizing idea -- they are the
rows of the table above -- they are just not directories.

```
xstd/bits.hpp                 the front door, over every container and the views
xstd/bits/bit_array.hpp       one header per container, named for the type
xstd/bits/bitset.hpp
xstd/bits/bit_static_set.hpp
xstd/bits/detail/             the vehicles and the block operations, namespace xstd::detail::bits
xstd/bits/ranges/             set_view, sequence_view, bit_extent
xstd/bits/ext/                the adaptors, asked for by name
```

`detail` for the vehicles, directory and namespace both. That is Boost's
convention, and it is the only one available to a library outside `namespace std`.
The three standard libraries cannot spell it that way, because inside `std` an
ordinary identifier is the user's to collide with, so each hides behind reserved
names instead: libstdc++ in `std::__detail` and `__gnu_cxx`, libc++ in `__cpo`,
`__detail` and a `__`-prefixed directory per domain, and the MSVC STL in `_Ugly`
names with no such namespace at all. Boost is in its own namespace, as this
library is, and uses `detail` some eight thousand times across 272 directories.

The nesting is Boost's, inverted. xstd is a federation of repositories -- xstd-misc,
xstd-ints, xstd-bits -- sharing one uniform `xstd` namespace, so unlike
`boost::json::detail` there is no library namespace to nest `detail` inside. The
library name takes the other position instead, `xstd::detail::bits`, partitioning
the shared namespace the same way round the other way. Without it
`xstd::detail::array` would reserve the name `array` across the whole federation,
alongside the `xstd::detail::delegates_to_std` that xstd-ints already declares.

It also reserves `impl/` for something else, which is why that name was wrong here:
`boost/json/impl/array.hpp` opens `boost::json` and holds out-of-line definitions
of the public interface, where `boost/json/detail/array.hpp` opens
`boost::json::detail`. What lives under this directory is machinery, not deferred
definitions.

The rename away from `bit/` was forced anyway: flattening made `bits/bit/array.hpp`
and `bits/bit_array.hpp` derive the same include guard, and the way out of that is
not the `XSTD_SUBDIR_BIT_SUBDIR_ARRAY_HPP` spelling this tree just got rid of.

## There is no bitset-like concept

A sequence of bits is one of exactly two things: a set of `size_t`, or a sequence
of `bool`. `std::bitset` is neither, or rather it is both at once under a third
vocabulary that names the same operations differently:

| bitset spells | a set calls it | a sequence calls it |
| ------------- | -------------- | ------------------- |
| `count()`     | `size()`       | -                   |
| `size()`      | `max_size()`   | `size()`            |
| `set(pos)`    | `insert(pos)`  | `v[pos] = true`     |
| `reset(pos)`  | `erase(pos)`   | `v[pos] = false`    |
| `reset()`     | `clear()`      | `fill(false)`       |
| `test(pos)`   | `contains(pos)`| `at(pos)`           |
| `none()`      | `empty()`      | -                   |

So the tests carry two contracts, not three-plus-a-bitset-concept: the `[set]`
contract and the sequence contract. A legacy bitset enters either one through a
view.

## Views

A view here is span-shaped, not string_view-shaped: non-owning, but **mutable
through**. Its job is to rewire misnamed members into the nomenclature of the
interface that is struggling to get out, not to freeze them.

`xstd::set_view(b)` presents any bit sequence as a `std::set<size_t>`: the
container typedefs, bidirectional iterators, `empty`/`size`/`max_size`,
`insert`/`erase`/`clear`, `find`/`count`/`contains`/`lower_bound`/`upper_bound`/
`equal_range`, and `==`/`<=>`.

`xstd::sequence_view<Bits, Extent>(b)` presents the same storage as a sequence of
`bool`, span-shaped with a `dynamic_extent` default: `operator[]`, `at`, `front`,
`back`, `size`, `empty`, `fill`, and `==`/`<=>`. One template rather than a
static/dynamic pair, for the reason `std::span` is one template: the two differ
only in whether `size()` is a constant expression.

The proxies are a closed pair: `*it` yields a proxy reference, `&ref` yields the
iterator back, and the value -- a key or a `bool` -- is reached only by converting
the reference. Both the mutable and the const path are proxies, the const one
differing only in refusing assignment.

That shape is not novel and it is not the standard's, because the standard has no
opinion here and its implementations disagree about all of it:

|                                          | libstdc++ | libc++ |
| ---------------------------------------- | --------- | ------ |
| `vector<bool>::const_reference` is `bool` | yes       | no     |
| `vector<bool>::reference` has `operator&` | no        | yes    |
| `bitset::reference` has `operator&`       | no        | yes    |
| `bitset`'s const `operator[]` gives `bool`| yes       | no     |

libc++ already builds its bit references this way and libstdc++ does neither half
-- but that is a statement about `sequence_view` alone, and it is worth being exact
about which of the two views the standard library prefigures.

`std::bitset::reference`, and libc++'s `__bit_reference` behind `vector<bool>`,
have `value_type = bool` and `iterator_category = random_access_iterator_tag`.
That is the sequence reading, and `sequence_view` recapitulates it -- the proxy pair,
the extent, the writing through. Nothing here is new.

The set reading is absent from both. Neither offers `value_type = size_t`, and
neither iterates the 1-bits: to walk them you call `find(first, last, true)`
repeatedly and subtract to recover positions. libc++ even computes the primitive
this needs, quickly -- `__find_bool` and `__count_bool` specialize `find` and
`count` over `__bit_iterator` a word at a time, which is where Hinnant's 76.9x
comes from -- but exposes it only as a search returning a bool-sequence iterator.

`set_find<Bits>::next` is that same operation surfaced as iteration, with
`operator*` yielding the position rather than `true`. So `sequence_view` is libc++'s
design made a guarantee, and `set_view` is the reading it does not offer, built on
the primitive it already has. The two together are why a bit sequence needs no
third vocabulary.

Two customization points, both defaulted so that `std::bitset`,
`boost::dynamic_bitset` and our own types all work unspecialized:

- `set_find<Bits>` -- where the elements are: `first`, `last`, `next`, `prev`.
- `set_ops<Bits>` -- how one goes in and comes out: `insert`, `erase`, `clear`.

`set_find` keeps its explicit-specialization design rather than plain ADL, because
`std::bitset`'s only associated namespace is `std`, where a program may not add
declarations, and because `boost::dynamic_bitset`'s own member `begin()`/`end()`
shadow any same-named free function. `set_ops` needs neither: a member call is
always found, so its default just probes for `insert`/`erase`/`clear` and falls
back to `set`/`reset`.

### The prize is word-at-a-time

Howard Hinnant, then libc++'s maintainer, made the case for this data structure in
2012 ([On `vector<bool>`](https://howardhinnant.github.io/onvectorbool.html)). Two
things there bear directly on this library.

The name. He proposed lifting the specialization out into `std::bit_vector<A>`
rather than leaving it pretending to be a `vector` of `bool` -- "it should not
pretend to be one". The vector column of the grid above is that proposal.

The numbers. His measurements are of algorithms specialized to operate a word at a
time against the same algorithms walking bits one by one:

| `find` | `count` | `fill` | `copy` | `swap_ranges` | `rotate` | `equal` |
| ------ | ------- | ------ | ------ | ------------- | -------- | ------- |
| 76.9x  | 22.7x   | 2.5x   | 2.8x   | 15.4x         | 1.69x    | 62.5x   |

which is the whole argument for the structure, and also the standing warning
against a view that quietly walks bits. Every relation on `set_view` therefore
tries, cheapest first: the Bits' own member, then its bitwise operators, then the
element-wise scan. `std::bitset<N>` has no `is_subset_of`, but `(a & ~b).none()`
asks the same question a word at a time, and only a Bits with neither pays for the
scan.

What the essay does not say is anything about `operator&` on the proxy or about
`const_reference`. Those are libc++'s own choices, visible in the table above but
unargued there.

There is an obvious objection to `set_view` existing at all: the 1-bits are
already reachable through the sequence reading, as `find(first, last, true)`
repeatedly, and libc++ makes exactly that fast. The objection is right about the
primitive -- `set_find<Bits>::next` *is* that search, and `__find_bool` is its
word-at-a-time implementation. What it misses is what surfacing the primitive as
a search rather than as iteration costs:

- the position comes back only by subtracting from `begin()`
- the value type is `bool`, so the elements iterated are not the keys
- there is no `[set]` interface over it -- no `lower_bound`, no `contains`, no
  ordering by key
- and stepping back to the *previous* 1-bit is not a `find` at all

`set_view` is what those four cost, paid once and in one place.

Open: whether `is_subset_of`, `is_proper_subset_of` and `intersects` stay members
of `set_view`. They are not in `[set]`, and lazy set intersection and union may be
the better home for what they do -- and would be the place to capture the rest of
Hinnant's numbers, since a lazy intersection could stay word-at-a-time end to end
rather than materializing.

## Tests mirror the headers

Every public header is answered by exactly one test source at the mirrored path,
enforced at configure time. A leaf test states what one entity does; an umbrella
test states what holds *across* the entities behind it, which no leaf can see.

Sweep depth is a separate axis. The exhaustive suites run over every set of
cardinality 1 through 4, which is a translation-unit splitting device, not a
statement about a header -- so those sources sit beside the leaves in the section
directory, named for the cost they carry (`constant`, `linear`, `quadratic`,
`cubic`, `quartic`) as their Boost.Test suites already are.

The type lists live in one header per contract rather than being copy-pasted per
source, the way `exact_width_types.hpp` does it in xstd:

- set-like: `std::set<size_t>`, `std::flat_set<size_t>`, `bit_static_set<N, Block>`,
  `bit_set<Block, Alloc>`, and `set_view` over each legacy bitset.
- sequence-like: `std::array<bool, N>`, `bit_array<N, Block>`, `std::vector<bool>`,
  `bit_vector<Block, Alloc>`, and `sequence_view` over each legacy bitset.
- `[bitset]` members: `std::bitset<N>`, `xstd::bitset<N, Block>`,
  `boost::dynamic_bitset<>`, `xstd::dynamic_bitset<Block, Alloc>`.

Because the views are complete and mutable, the set primitives split once, by what
they need rather than per type: nothing has to be carved out for a type whose view
lacks a member.

The prime sieve is a benchmark of dynamic containers only, so that it compares
like with like. A fixed-size `bit_static_set<N>` sifting a universe it was sized
for is not measuring the same thing as a `std::set` growing and shrinking.
