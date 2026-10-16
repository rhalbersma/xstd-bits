# Design notes

Why the code is shaped the way it is. The headers carry one line each; the reasoning lives here, and a
one-line comment ending in `[design.md#anchor]` points at the section that explains it.

Decisions still in flight live on the [open issues](https://github.com/rhalbersma/xstd-bits/issues), and
[#80](https://github.com/rhalbersma/xstd-bits/issues/80) is the closed design plan the current shape came
out of. This file holds what has landed, and, under [Future directions](#future-directions), what is designed and
waits on something.

## Storage, readings, refinements

The library has two layers and one extension point.

**Storage** is blocks of unsigned integers, owned by `bit_block_container`. It knows widths, blocks and the
primitives every reading is built from — `first_difference`, `any_above`, `any_block_set` and the like —
and it compares only memberwise: it names no ordering, because the readings order the same bits
differently.

**A reading** is what the bits mean: a set of positions, or a sequence of `bool`. Each
is a tag type (`set_reading_tag`, `sequence_reading_tag`) and an adaptor over the
storage that decides what `==`, `<=>`, iterators and `operator[]` mean, with the orderings themselves in
`detail/comparisons.hpp` as free functions over the storage's primitives.

**A refinement** extends a reading without touching it: a future `string_reading_tag`
([the-string-reading](#the-string-reading)) derives from `sequence_reading_tag`, so every view and algorithm that accepts a sequence accepts a string, and
`sequence_three_way` serves both. The views check a reading with `std::derived_from`, which is what keeps
the set of readings open.

Each reading is held to the specification of the container it imitates, paragraph by paragraph: the
inventories in `doc/audit/` and the check that runs them are that denominator.

## Storage and containers

### owned-bit-storage

The exposition-only `owned_bit_blocks` asks whether a type is bit storage a container can **own**: `bit_blocks`
([is-and-has](#is-and-has)) that is also regular, and read-only through a `const` object. For a range that is a
regular, sized, contiguous, subscriptable range of unsigned integers whose `const` subscript does not write.
Regular is what lets `bit_block_container` default its `==` over the width and the blocks, in that member
order, so two run-time widths part on the width before a block is read. `std::array` and `std::vector` both
qualify, and so does `std::inplace_vector` — a runtime width over static capacity, for free. A built-in array of
blocks, `std::uint64_t[4]`, is bit storage and not owned bit storage: it is not regular, having neither copy
assignment nor an `==` over its blocks, so no owner holds one and no `bit_block_container` has it as `Blocks`. The tag
constructors, `xstd::bit_convert` and the views read it as they read the `std::array` of the same blocks.

**`bit_blocks` is two cases joined.** One block is an `xstd::unsigned_integer`, const or not, but never volatile:
that concept is cv-transparent, for the reasons xstd-ints'
[design notes](https://github.com/rhalbersma/xstd-ints/blob/main/doc/design.md#integer-types) give, and `bit_blocks`
refuses `volatile` on top of it, since a block algorithm copies, merges and vectorizes the accesses a memory-mapped
register must keep one by one. The exposition-only `bit_block_range` is a sized contiguous range of such blocks that
subscripts, and reads the `volatile` off the range's reference type, since `range_value_t` drops it;
`xstd::bit_blocks` is either, and is the one of the three a user names. Code that treats the scalar and
the range differently branches on `xstd::unsigned_integer`, and `bit_block_container` asks `bit_block_range` of its
`Blocks`, which is how a bare block is refused at the constraint while contiguity is named rather than implied.

**It is exposition-only because no storage joins the library from outside.** The storage under every owner is
a `bit_block_container` over `owned_bit_blocks Blocks`, while the views take any `bit_blocks`. The split is
the one between owning and borrowing: `std::span<B>` is bit storage a view borrows, and it is neither regular --
two spans comparing equal would mean the same blocks, not the same bits -- nor read-only through `const`. Each
owner fixes its storage by its name, and none takes one as an argument: the grid's four columns, the array, the
bounded vector, the vector and the small vector, are the whole of what an owner holds. A user's own container of
blocks is read by a view over it, which asks only `bit_blocks`. So `owned_bit_blocks` and `resizable_bit_blocks`
state what the library's own storages provide, as the standard's exposition-only concepts state what its own
templates ask, and a user never meets them in a signature. Meyers' *Effective STL*, Item 2, "Beware the illusion of
container-independent code", is the reason not to open it: the storages already part on what growth past capacity
throws, on whether they are constant-evaluable (only `std::inplace_vector`), on whether a copy is `noexcept` (not
`boost::container::static_vector`'s) and on what a capacity of nought costs, and an owner generic over them all could
promise only what they share.

**A run-time width asks one thing more.** `resizable_bit_blocks` refines `owned_bit_blocks` with what
the vehicle calls to change its block count: `resize(count, value)`, `push_back`, `insert` at the end, `clear`
and `max_size`. The storage requires it only at `N == std::dynamic_extent`, so `std::array<B, K>` still serves a
fixed width while `bit_block_container<std::array<B, K>, std::dynamic_extent>` is refused at the constraint
rather than inside `resize`. The test that the contract is the whole of what a storage has to provide
is `test::minimal_blocks`: a storage written outside the library with exactly those members and no others, which
runs the set reading's full primitive suite through the set adaptor over it. `boost::container::static_vector`
satisfies both concepts as it comes, and it holds the bounded column's blocks wherever `std::inplace_vector` is
missing, so it runs under coverage. Boost.Container force-inlines an asserting `operator[]`, which at `-O0` would
put a branch no passing test takes on every subscript in the vehicle; the test tree defines
`BOOST_CONTAINER_DISABLE_FORCEINLINE`, which leaves the call, and its branch, in Boost's header.

**Whether growth throws is the storage's own answer.** `std::vector::resize` can throw `std::bad_alloc`, and a
storage with an inline capacity -- `std::inplace_vector`, `boost::container::static_vector` -- throws its own
`bad_alloc` past it, which reaches the caller unchanged. Growth keeps whatever guarantee the storage's own `resize`
gives, because the width moves only after the storage has grown: over any of the standard or Boost vectors, whose
`resize` on unsigned blocks either completes or changes nothing, a failed growth leaves the owner as it was. A
storage whose `resize` is `noexcept` never takes that path.

The element clause is `xstd::unsigned_integer`, and **not** the wider `bit_mask`, which would be the concept
if the operators were all a block is asked for. They are not. Beyond them the body wants the `<bit>` intrinsics
— `popcount`, `countr_zero` and `countl_zero`, each constrained on `xstd::unsigned_integer` in
`detail/intrinsic.hpp` and reached at some thirty sites — a `numeric_limits<block_type>::digits` for
`bits_per_block`, and block arithmetic: `shl(unit, count) - unit`, and the `block & (block - 1)` step the set
reading's block walk takes, where `bit_mask` omits `-` deliberately, subtraction and set difference
being indistinguishable to a concept.

The two agree on every block the library ships, and both refuse `bool`, the character types and every signed
type, so `std::vector<int>` stays out either way. They part on the class types that are fields of bits without
being numbers, `std::bitset` among them: `bit_mask` admits those and this concept does not, which is
the point. `std::array<std::bitset<64>, 4>` is asserted refused, a ready-made negative case
([concepts-are-tested-on-ready-made-types](#concepts-are-tested-on-ready-made-types)) for exactly the gap
between the two spellings. Widening the clause would move that refusal from an unsatisfied constraint to a hard
error inside the template — the failure mode the subscript clause below exists to prevent, and there is no
reason to accept it on the element clause when the narrower concept states the truth.

The asymmetry it records is the layering, not an accident. `unsigned_integer` goes **in** and the container
gives the common vocabulary ([the-common-vocabulary](#the-common-vocabulary)) — and, once the non-assigning operators land, `bit_mask` — **out**: it
asks more of a block than it offers its own user, consuming numbers and yielding a field of bits, shedding the
arithmetic on the way up. That is also why nesting cannot work: a `bit_block_container` will have every
operator and still no `popcount`, no `digits` and no `- 1`.

The concept has a header of its own under `<xstd/bits/bit_concepts/>`, beside `bit_blocks`, which it refines: the
one says what a `Blocks` **is**, the container is the vehicle built over it, and a reader asking the first question
need not open the 1500 lines answering the second. Between them those headers include `<concepts>`, `<ranges>`, the
one xstd-ints concept and the alias of [the-const-reference](#the-const-reference). `borrowed_block_span`, the span a view writes through, is
left in `detail/borrowed_block_span.hpp`. `num_blocks_v` stays with the vehicle, being a block-count computation
rather than a statement about what a `Blocks` is.

Subscript is spelled out rather than left to `std::ranges::contiguous_range`, which does not imply it.
`contiguous_range` gives `data()` and a `contiguous_iterator`, and a `contiguous_iterator` is a
`random_access_iterator`, so `i[n]` **is** required — of the *iterator*. The range itself is under no such
obligation: a plain buffer wrapper can satisfy the other four requirements and have an iterator that subscripts
happily while `c[n]` does not compile. No *ready-made* type isolates that gap — every std candidate that fails
this concept fails for some other clause first (`span` and `subrange` are not `regular`, `string_view`'s and
`vector<int>`'s elements are not unsigned integers, `vector<bool>` is not contiguous) — and the gap is not worth
a class written only to be
asked about ([concepts-are-tested-on-ready-made-types](#concepts-are-tested-on-ready-made-types)), so it is
stated here from [range.refinements] rather than pinned by a test. `bit_block_container` reaches for the
range's subscript in 140 places, `block_mask` among them, so without this the concept admits storages the class
cannot be instantiated over — the shortfall surfacing as a hard error inside the template rather than as an
unsatisfied constraint, which is the failure mode
[a-requires-clause-names-its-arguments](#a-requires-clause-names-its-arguments) exists to prevent.

The mutable and the const subscript are two requires-expressions with a parameter list each, not one expression
over `C&`, `C const&` and an index together. They are separate requirements — a storage can offer one and not
the other — and each list then names exactly what its own requirement uses, `c` and `n`, rather than a shared
`r`/`c` pair in which half the names are dead in half the expressions. The index is `n` like every position in
this library, and its type is `C::size_type` rather than a bare `std::size_t`: a contiguous block *container*
is a container, `a[n]` is how [sequence.reqmts] spells the operation, and `size_type` is the name a container
gives the index. `std::array`, `std::vector` and `std::inplace_vector` each name it, so the clause costs the
three storages nothing and asks anything else for the typedef a container has. It is written without a
`typename`, P0634 making one implicit in a requirement-parameter-list and `readability-redundant-typename`
reporting it where it is written — which is how the first CI run of #143 failed. The quoted GCC diagnostic
below echoes a `typename` all the same: that is GCC's printing of the parameter, not what the header says.

Diagnostics were measured, not assumed, and they are *almost* the same either way: given a storage with the
mutable subscript alone, both forms point at the failing requirement, and Clang's note is identical. GCC's
differs only in the parameter list it echoes, remeasured at the current spelling: `in requirements with 'C& r',
'const C& c', 'typename C::size_type n'` joined, against `in requirements with 'const C& c', 'typename
C::size_type n'` split. A narrower echo, not a better error. The split is for the reader.

The requirement is written against the iterator's own reference type, `range_reference_t<C>`, rather than
against `range_value_t<C>&`, because the point is not that subscript yields *a* reference but that it yields
*the same* one iteration does. The rest of that is semantic and no concept can check it, so it is stated
here as the standard states it for `random_access_iterator`'s `i[n]`:

> `c[i]` is `*(std::ranges::begin(c) + i)`

and asserted in the tests, by address rather than by value, for every storage shipped. An unchecked
semantic requirement that no test pins is a comment.

Naming it a *block container* follows from the same fact. A contiguous container of blocks generalizes the C array
of blocks, and its `operator[]`, the C array's defining operation, yields a block; a concept claiming a range *is* blocks while unable to index one would be
describing something else. The word is deliberately close to the standard's *contiguous container*
([container.reqmts]/68) without claiming it: that term drags in the whole *Container* table — `empty()`,
`max_size()`, `cbegin`/`cend`, member `swap`, seven nested typedefs — and `bit_block_container` needs
almost none of it. At a static width it needs none; at a run-time width it needs `max_size`, `resize`,
`push_back` and `clear`, which are *sequence* container operations, not `Container` ones. Neither path draws the
line where [container.reqmts]/68 draws it, so the concept states its own five requirements and borrows nothing.

The name says *block container*, not *storage*: what an adaptor sits on is a different question, asked by
`bit_block_container_type` and answered by name rather than by members
([one-storage](#one-storage)).

### the-const-reference

The const subscript is checked against `range_const_reference_t`: the standard's where the library has it, and
where it does not, ours — P2278R4's two aliases transcribed from the standard, in a header of their own:

```c++
template<std::indirectly_readable It>
using iter_const_reference_t = std::common_reference_t<std::iter_value_t<It> const&&, std::iter_reference_t<It>>;

template<std::ranges::range R>
using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;
```

That is [const.iterators.alias] and [ranges.syn] verbatim, down to the constraints, and it is ten lines because
the paper defines the alias by a formula rather than by magic.

**One header, and the `fallback` namespace is what makes the conditional safe.** The transcription above sits
there unconditionally, and below it `detail/range_const_reference.hpp` selects
`std::ranges::range_const_reference_t` where `__cpp_lib_ranges_as_const` says the library has the paper and
`fallback::range_const_reference_t` where it does not. Nothing shadows anything: on a library that ships P2278
both spellings exist and are separately reachable, which is exactly what lets the tests below check one against
the other. What the check needs is that the fallback be separately *nameable*, which is the namespace; a second
file bought nothing and is not there.

The reason not to use `std::ranges::range_const_reference_t` is that a third of the matrix does not have it.
**libc++ has implemented P2278R4 on no branch, trunk included**: `__cpp_lib_ranges_as_const` is still a
commented-out line in its `<version>`, beside `__cpp_lib_ranges_chunk` and unlike the `chunk_by` defined next to
it, so the commenting tracks implementation rather than being a stale block. The headers are simply absent —
`range_const_reference_t` occurs nowhere in `__ranges/concepts.h`, and `__ranges/as_const_view.h` and
`__ranges/const_access.h` do not exist. Apple's toolchain is downstream of libc++, so no Xcode will have it
first, and this is not a wait-for-the-next-rung situation: naming the standard alias directly fails on Xcode
16.4.

**A conditional is only as good as its other arm, and that is the whole argument for the transcription.** The
obvious arms, `std::ranges::range_const_reference_t` and `range_reference_t<C const>`, ask two different
questions: P2278 asks what `C`'s own iterator yields once const-ified, the second what a
`C const` iterates. Measured, they part on exactly one thing:

| storage | `range_const_reference_t<C>` | `range_reference_t<C const>` |
| --- | --- | --- |
| `vector<Block>`, `array<Block, N>`, `inplace_vector<Block, N>` | `Block const&` | `Block const&` |
| a regular, span-like handle | `Block const&` | `Block&` |

Within `contiguous_range` that makes P2278 strictly the tighter arm, and the one thing it catches is a
**shallow-const** blocks type: one whose `c[n]` hands back a writable `Block&` through a `C const&`, which would
let a `bit_block_container const` be written through. That conditional would therefore have admitted such a
storage on libc++ and refused it everywhere else — a concept meaning two things on two thirds of the matrix.
Transcribing does not remove the conditional; it makes the other arm *the same type* rather than a second
contract, which is the only reason a conditional is the right shape here at all.

Two spellings were considered and not taken. `range_value_t<C> const&` is what the alias collapses to *for a
contiguous range*, needs nothing, and was measured equal on every storage here — but it states a coincidence of
this concept's other clauses rather than the requirement, and would be wrong the moment the alias were reused
anywhere that is not contiguous. A `conditional_t` over `is_const`/`add_const` reconstructs "const-ify the
reference" by hand, and gets proxies wrong: measured on `std::vector<bool>`, the dance says `bool const&` where
the paper says `bool`, because `common_reference_t` consults `basic_common_reference` and `add_const_t` cannot.
Both are lookalikes; the formula is the thing.

What pins it is `TheConstReferenceIsP2278s`. Where the library *does* have the paper — the gcc, msvc and
clang-with-libstdc++ rungs — the fallback must equal `std::ranges::range_const_reference_t`, asserted on both
storages and on `vector<bool>`, so the transcription is checked against three vendors rather than against my
reading of the wording. That check is the reason the fallback is namespaced apart instead of written inline in
the `#else`: were it only the unselected arm, the rungs that could verify it would never name it, and the rung
that names it could not verify it. The rest holds everywhere, whichever arm was taken: the reference is `const` for
each storage shipped, and the fallback gives `bool` for `vector<bool>`, which is the assertion that fails first
if anyone ever "simplifies" the definition into the dance.

### the-one-vehicle

`bit_block_container` lives in `detail/bit_block_container.hpp`, with the concept, `num_blocks_v` and
the detector that names it. It holds primitives only: each reading's equality and orderings are free functions
over them in `detail/comparisons.hpp` ([the-ordering-primitive](#the-ordering-primitive)). It has no per-storage aliases: each owner spells its storage once, in its base clause --
`basic_bit_array<Block, N>` derives from the sequence adaptor over
`bit_block_container<std::array<Block, num_blocks_v<Block, N>>, N>`.
The name is in `xstd::bits::detail` with the rest of `detail/`, so nothing outside the library names it; it is the
device that turns two readings over three storages into two plus three, and a factoring device is machinery
rather than vocabulary. A user reaches every width through `bit_fixed_set<N>` or `basic_bit_array<Block, N>`, and a
test that needs the container itself spells its storage, as `bit_block_container<std::vector<std::uint8_t>>`.
Each public header includes only the storage it names, so `bit_array` does not see `std::vector`, and the
choice between `std::inplace_vector` and `boost::container::static_vector` sits in `detail/bounded_blocks.hpp`,
which only the two bounded headers include.

**The name says what it does to its argument.** It takes a `owned_bit_blocks` — a range that *is*
blocks ([owned-bit-storage](#owned-bit-storage)) — and adds the bit interface. In goes
storage that answers about blocks, out comes something that answers about bits. Concept and class differ by one
word, and it is the word that changes.

That is also what earns it the name the adaptors are constrained on. It is the one storage they admit
([one-storage](#one-storage)), the only one there is: `std::bitset` and `boost::dynamic_bitset` are comparison
targets rather than adapted storages ([owning-is-ours](#owning-is-ours)). It lives under `detail/` because nobody
outside spells it ([the-interface-line](#the-interface-line)). What the three have in common as *members* is
[the-common-vocabulary](#the-common-vocabulary). A name such as `block_sequence` would hide that: a *sequence of
blocks* reads as storage that happens to have been adapted, where a *bit container* reads as the thing the
readings are built on.

`bit_block_container<Blocks, N>` is the single storage vehicle. `N` is the width when that is a constant
and `std::dynamic_extent` when the width is carried at run time.

It owns the **unused-tail invariant** — every bit at or above `size()` reads zero — which is what makes
whole-block comparison, popcount and the block-at-a-time scans mean anything at all.

It has no iterators and no proxies. Those are readings, and a reading here would be picking one; it would
also make `std::ranges` see a sequence of blocks rather than of bits.

The hand-unrolled one- and two-block cases are where the static performance lives, so they stay
compile-time branches. The general path they fall through to is written over `m_blocks` as a range, and
therefore serves the dynamic width unchanged.

### concepts-are-tested-on-ready-made-types

Concept tests name `std::array`, `std::vector` and `std::inplace_vector` and nothing else. A class written only
to be asked about proves what its author put in it: it is the concept restated in class syntax, it passes by
construction, and it goes stale silently when the concept moves. The three std containers are the storages this
library is actually instantiated over, so an assertion about them is an assertion about the library.

What that gives up is the negative cases a shim isolates one clause at a time, and here three of the four
survive on ready-made types alone: `std::vector<bool>` is not contiguous, `std::vector<int>` is signed, and
`std::array<std::bitset<64>, 4>` is the field of bits that is not a number — the one that separates
`unsigned_integer` from `bit_mask` ([owned-bit-storage](#owned-bit-storage)). Only the
range-subscript clause has no ready-made counterexample, and it is argued in prose there instead.

`counting_blocks` in `test/src/bits/detail/bit_block_container.cpp` is not an exception to this. It is a
swap-and-move fixture with instrumented operations ([swap-goes-through-adl](#swap-goes-through-adl)), which no
std container can be, and it carries no concept assertion of its own: it is instantiated as a
`bit_block_container`'s `Blocks`, and that instantiation is the only check it needs to satisfy.

### the-common-vocabulary

`test::bitset::vocabulary`, in `test/include/test/bitset/vocabulary.hpp`, is what the three bit containers answer
**in their own names**, with no trait in
between: the intersection of `std::bitset`'s vocabulary, `boost::dynamic_bitset`'s and
`bit_block_container`'s. Measured against all three rather than guessed, and larger than it first looks —
nineteen requirements:

| | |
|---|---|
| `std::regular` | copy, move, `==` |
| observers | `size`, `count`, `test(n)`, `all`, `any`, `none` |
| mutators | `set()`, `set(n)`, `reset()`, `reset(n)`, `flip()`, `flip(n)` |
| bitwise | `&=`, `\|=`, `^=`, `<<=`, `>>=` |

What falls outside is what makes it an intersection. Each of these is absent from at least one of the three, so
asking for it would drop a model:

| absent from | |
|---|---|
| `bit_block_container` | a subscript to a bit, its `operator[]` being a block's ([test-not-subscript](#test-not-subscript)), unary `~`, `set(n, value)` |
| `std::bitset` | `-=`, `is_subset_of`, `find_first`, member `swap` |
| `boost::dynamic_bitset` | `to_string` |

The asymmetry is the point, and it runs in two directions at once. `bit_block_container` provides the
**union** of what the two readings ask of it — a set reading needs `find_first` and `find_next`, a sequence
reading needs positional writes — while the concept asks only what the three *containers* have in
common. Union below, intersection across.

**Nothing is constrained on it, and that is deliberate.** The adaptors admit their storage *nominally*, by
`bit_block_container_type` ([one-storage](#one-storage)), and the structural question is a
different question: `std::bitset` and `boost::dynamic_bitset` both model the common vocabulary and neither
is a storage this library wraps. Constraining an adaptor on the concept would turn *this is not one of ours*
into *your type lacks `count()`*, which is the wrong diagnosis about the wrong type.

The concept is therefore a **description, pinned by the tests, not a gate**. It says what the three have in
common and fails loudly if one of them drifts.

### the-primitive-basis

Two layers, and the lower one is the whole design. `bit_block_container` provides the **primitives** — the
operations a reading needs to be efficient rather than merely correct — and each adaptor **assembles** them into
the standard-like API its reading presents. No third layer sits between them, and [one-storage](#one-storage)
says why.

The primitives are the point, and the reason is measurable: they are exactly what the counterparts do not have.
What each provides natively, on libstdc++:

| primitive | `bit_block_container` | `std::bitset<N>` | `boost::dynamic_bitset` |
|---|---|---|---|
| block read | yes | only `N <= 64`, through `to_ullong` | **no** |
| `find_first` | yes | `_Find_first`, where the library has it | yes |
| `find_next` | yes | `_Find_next`, where the library has it | yes |
| `find_prev` | yes | **no** | **no** |
| `count`, `all`, `any`, `none` | yes | yes | yes |

`std::bitset` and `boost::dynamic_bitset` are an **incomplete basis**: enough to implement the full views API
correctly, never enough to implement it efficiently. Neither walks backwards, and boost hands over no blocks at
all, so a view over one would have to walk in reverse a position at a time — quadratic over a full
traversal, where ours reads a block at a time ([one-function-per-tier](#one-function-per-tier)). The two
reserved names are not portable either: `_Find_first` and `_Find_next` are libstdc++ and MSVC extensions,
absent on libc++ ([the-two-reserved-names](#the-two-reserved-names)).

**The shortfall is not an oversight.** `std::bitset` was designed to extend the **bitwise operators** to an
arbitrary fixed width: it is a wide unsigned integer with per-bit accessors, not a container.
[template.bitset]'s synopsis is that and nothing besides — `&=`, `|=`, `^=`, `<<=`, `>>=`, `~`, `set`, `reset`,
`flip`, `test`, `count`, `size`, `all`, `any`, `none`, `operator[]`, and the conversions to and from strings and
integers. No `begin`, no `end`, no search. So `find_first`, `find_next` and `find_prev` were never *missing*
from `std::bitset`; they were never in scope. That also explains why `_Find_first` and `_Find_next` are
libstdc++ and MSVC extensions rather than standard ([the-two-reserved-names](#the-two-reserved-names)): the
implementations added what the standard had no reason to ask for.

The set and sequence readings ask a wide integer questions it was
never meant to answer, and answering them efficiently is what the storage below is for.

So the conclusion the table argues for is not a shim over three spellings of one thing. It is **one storage
that is a complete basis**, and no counterpart is adapted as a storage. Adapting one would mean hand-writing
`find_first` and `find_next` over whatever it offers, `num_blocks` and `block` behind a
`requires (N <= ullong_digits)` guard, and no `find_prev` at all, since neither counterpart has one to forward
to. The counterparts are comparison targets instead ([owning-is-ours](#owning-is-ours)), and with no second basis
to reconcile, no layer reconciles one ([one-storage](#one-storage)).

### the-bytes-they-agree-on

A comparison target can become a **value** the set reading converts to and from, and at a static width it is an
exact one. A field of `N` bits has the same `N`, and under this reading that width is a capacity, so position
`n` here is bit `n` there with nothing left to choose: nothing truncates, nothing grows, nothing throws, and the
round trip is the identity both ways. `bit_fixed_set<N>` therefore reads blocks that *are* bit storage through
the tag constructor **`bit_fixed_set<N>(xstd::from_blocks, b)`**, and hands a field back, as it crosses with
anything else that *has* bit storage, through the free function **`xstd::bit_convert`** ([is-and-has](#is-and-has)). Named in both directions, and not because either could fail — a set of positions
and a field of bits are two readings of the same bits, and this library makes a reader pick one rather than
letting a conversion pick for them.

**An explicit constructor and an explicit conversion operator are the wrong tool for the door.** `explicit`
guards against a conversion nobody asked for; the failure available here is a reader misreading one that *was*
asked for. Admitting a sequence of blocks makes that concrete, because a contiguous range of unsigned integers
already means something at this reading:

```cpp
auto const blocks = std::array<std::uint64_t, 4>{ 5, 0, 0, 0 };

bit_fixed_set<256>(std::from_range, blocks)   // {0, 5} — the values are keys
bit_fixed_set<256>(blocks)                    // {0, 2} — the values are blocks
```

Behind an explicit constructor both compile, both succeed, and they disagree, a tag being the whole of what
separates them. A name is what tells them apart at the call site, where the reader is. `std::bitset` spells its own
exit `to_ullong` for the same reason and offers no conversion operator at all, and Boost.dynamic_bitset spells this
pair `from_block_range` and `to_block_range`. Here the way in is the tag, which `bit_block_container` spells
`assign_bits` one layer down, and the way out is `xstd::bit_convert`, one free function for every pair of widths, so
no reading carries a member for it.

Neither is spelled with a type. `std::bitset` appears nowhere in the set adaptor: the two are templates
constrained on `bits::detail::bit_layout<B, N>`, which admits two families: one whose layout the language states, and
one read on the terms `std::bit_cast` reads any object.

| family | admitted because | what it costs |
|---|---|---|
| fixed blocks (`fixed_blocks_source`) | bit `n` of a block is 2^n, *by the language* | nothing: no assumption |
| field of bits (`container_source`) | trivially copyable, little-endian, room for `N` bits | the caller's word that its bytes are its bits |

`unsigned long long` is therefore not a special case in the header but an instance of the first family, which is
what `to_ullong` and the constructor taking one reduce to. `std::bitset<N>` is an instance of the second, and
crosses to `bit_fixed_set<N>` with neither side named in the constraint. Its width is the `N` of its type, and no
member of it is called to find one.

The stated family is **one concept over a block or a fixed number of them**, and it is what makes raw blocks
convertible. An unsigned integer states its own layout; a **contiguous sequence of unsigned integer blocks** states
the rest of it, block `j` holding the positions `[j·digits, (j+1)·digits)`. A scalar is the sequence of length one,
which is why `fixed_blocks_source` is one concept: `fixed_bit_blocks` whose width covers `N` and that a value owns.
A scalar crosses as the range of one block it is, so one copy serves both. Everything in it is read by **shifts on values**, never by `std::bit_cast` on an
object, so no padding is reachable and endianness never enters: `b[j] >> k` is the same number on either byte
order. Only a foreign field of bits, whose internals this library cannot name, is read as an object.

So `std::array<uint64_t, 4>` crosses, and so does `std::array<uint32_t, 8>` over the same two hundred and
fifty-six positions, because the byte is the common ground where the block is not. The width must be **named by the
type** — `xstd::bit_blocks_extent_v` answers it for a block, a `std::array` and a built-in array, and
`std::dynamic_extent` for a vector — because "nothing truncates" is a promise made at compile time, and a run-time
size cannot keep it. It must cover `N` but need not equal it, at one block as at many. A type with no
`bit_blocks_extent_v` is not in this family; it crosses as a field of bits or not at all.

A built-in array of blocks is in the family as a **source** only. It is read and never owned, and its blocks are the
bits of the `std::array` of the same blocks, so it is read through the same shifts and the same copy. No function
returns one, so it is never a target: `bit_convert` writes into a `std::array` and not into a `std::uint64_t[4]`.

Asking `fixed_bit_blocks` **before** `owned_bit_blocks` is load-bearing rather than tidy. That concept asks
`std::regular`, which asks `constructible_from`, which re-enters the very constructor whose constraint this
is — a concept depending on itself. `fixed_bit_blocks` asks `bit_blocks` first, and an adaptor's iterator is a proxy
and so never contiguous, so the cheap question answers false for every reading here before the recursive one is put.

One spelling repays reading twice, and it reads differently at each reading — which is the point, and the trap.
`bit_fixed_set<32>(xstd::from_blocks, 5u)` is the set of positions the **value** five has, `{0, 2}`, not the set `{5}`.
`bit_array<32>(xstd::from_blocks, 5u)` is a packed array of bool, so it is `true, false, true` and twenty-nine more
`false`. Same bits, two vocabularies. The name is what carries the distinction: each is asked for by a
spelling that says which reading of the argument is meant, which is exactly what `explicit` could not do.

The currency is **bytes**, not blocks. Byte `j` holds the positions `[8j, 8j + 8)` least significant bit first,
which is what every contiguous bit container lays them out as whatever its block width, so two widths over the
same positions agree byte for byte. That is what makes this a copy rather than a walk over positions, and one
function is the primitive: `copy_bits`, blocks to blocks at any two block widths, a run of bytes being blocks of
`unsigned char`. The storage's `assign_bytes` and `to_bytes`, the two families' `bit_bytes` and `bytes_bits`,
`bit_convert`'s copy between owners and `boost::dynamic_bitset`'s gather all go through it or its two byte
accessors, and the hash reads the same bytes through the same accessor.

It is said **twice**, and the reason is measured rather than tasteful. Said in **shifts**, they name where a
position goes instead of assuming a byte order, so they are right on either endianness and they are a constant
expression — neither of which a `memcpy` is. They are also a byte at a time, and over the eight kilobytes of
2^16 positions that costs **9.70µs against 0.07µs**, a factor of about a hundred and forty-five. So the shifts
keep the two cases a copy cannot take, a constant expression and a big-endian target, and the copy takes the
case that is neither, which is every rung this ladder runs. `if consteval` picks the first,
`if constexpr (endian::native == endian::little)` the second; neither is a branch a coverage slot can miss,
because neither is a branch at run time.

That pair of cases is worth taking one branch at a time, because three of them are not the same question.

A **scalar** is the range of one block and takes whatever a range takes, and that is measured rather than
assumed: a value is at most a handful of bytes, the loop unrolls, and `memcpy` timed identically at every width
— **0.31ns either way** on `uint8`, `uint32` and `uint64`. A branch of its own would buy nothing.

A **sequence of blocks** takes the copy only where the bytes of a value are the bytes of the field, which is a
little-endian target whose block type has no padding: the shifts count by `digits` where a copy counts by
`sizeof`, and those agree only when every bit of the object is a value bit. No standard type has such padding,
and the test is there so that one could not quietly turn a copy into the wrong answer. The block must also be a
built-in: a class such as `absl::uint128` or `boost::int128::uint128` lays out its value as its author chose,
which no rule of the language makes its bytes, so it crosses by shifts. That is one predicate,
`block_copies_as_bytes`, asked by every copy in the library, the hash's included, so no two of them can disagree
on when a copy is the answer. Its shape is load-bearing
too: written as `if !consteval` with a return, the shifts below it become unreachable code in a run-time
instantiation, which MSVC reports as C4702 and this build treats as an error. Two alternatives, so neither arm
is dead.

The blocks are **value-initialised first**, which is what clears the tail above `N` and so makes
set → blocks → set the identity at a width the sequence is wider than. The other direction is not the identity
and is not meant to be, exactly as it already is not for an integer too wide for the width.

A **zero width** exchanges no byte, and so reads none. `std::bitset<0>` occupies a byte that represents no
position, so a `std::bit_cast` of it reads an uninitialised one and is no constant expression; the way back casts an
object of zero bytes, which reads nothing of the source either. The guard is `byte_count<N>` rather than `N`,
those being zero together and `byte_count` being what the two conversions actually range over.

That factor is what decides a question this design keeps inviting: whether a foreign bitset should be **read**
block-wise in place rather than converted. In place is not portably possible — `std::bit_cast` yields a copy, so
reading someone else's words needs a pointer into them, which is `_M_p`, `__seg_` or `_Myptr` by turns. It also
turns out not to be worth wanting. Iterating the set positions of a `std::bitset<2^16>` at 2.7% density costs
**41.1µs** asking every position, against **libstdc++'s own in-place `_Find_first`/`_Find_next`** and
**converting once and then iterating**, which come out level: five runs put their ratio at 1.06, 0.94, 0.95,
1.06 and 1.01, so on par to within about six percent. The absolute microseconds are not quotable — this box is
bimodal, and both of those two move between roughly 7.5µs and 18.5µs *together*, which is what makes the ratio
the only honest figure and a single run of either a trap. So there is no `bit_readable` to add: converting
costs what reading in place would, the conversion is the door, and the readings' own block-wise algorithms are
what waits behind it.


The second family is **trusted on `std::bit_cast`'s terms**, and asked for exactly what those terms need.
`container_source<B, N>` holds where `B` is trivially copyable, which is what `std::bit_cast` asks before it reads an
object; where the target is little-endian, so that byte `j` of a word holds the word's bits `[8j, 8j + 8)`; and where
the object has room for the width, `N <= sizeof(B) * 8`. Nothing of `B` is called and nothing is constructed. Its
first ⌈`N`/8⌉ bytes are its bits, read by one copy at run time and through `std::bit_cast` in a constant expression;
writing one copies those bytes into `sizeof(B)` zero bytes and casts them back, so the bytes past the width are clear
without a default constructor to clear them.

Where its width comes from is the same choice made the same way. A foreign type names a width only as
`std::bitset<N>` does, in its type, and `bit_width_v` takes that `N` from the template argument rather than from a
`size()`. Any other foreign type has `std::dynamic_extent`, so `xstd::bit_convert` matches no fixed width against it,
and it is a source only where a target supplies `N`, as `bit_block_container`'s tag constructor does.

The checks are for **Murphy, not Machiavelli**. Each guards against what a real implementation or a real caller gets
wrong by accident: a big-endian target, where bit `n` of a word lands at the far end of it, and a type too small for
the width, whose last positions would be read from past the object. Both are refused at compile time. What is not
guarded is a type built to deceive — one that keeps its bits reversed, or in a word it does not keep clean — because
telling that apart from the real thing means running the type's own members, which asks of every candidate a `set`,
a `count` and a `size` that a constant evaluation can reach, and a width bounded by the evaluation's step budget.
Like `std::bit_cast`, the caller vouches that the type is plain bits. The consequence is stated rather than hidden:
a pointer or a `std::span` passed where a target supplies `N` is trivially copyable and has room for as many bits as
its object, so it is admitted, and what is read is the bytes of the address and not what it points at.

What is **not** asked is `has_unique_object_representations_v`, and that is measured rather than preferred. GCC
13 through 16 answer false for any class with an empty non-static data member, even one `[[no_unique_address]]`
makes free, where clang answers true at identical layout — `sizeof` 8 and `offsetof` 0 on both. Every container
this library defines has such a member, so that trait would make this concept false on every GCC rung and true
on every clang rung. An empty *base* keeps the trait on GCC where an empty member does not, which is the remedy
if it is ever wanted; trivially copyable is what `std::bit_cast` asks, and the concept asks no more.

A run-time width takes no field of `N` bits through the tag, and that is the policy and not an omission: a field of
`N` bits names one `N` at compile time and a growing set has no single one to mean. It crosses through
`xstd::bit_convert` instead, which carries the width along ([interop-not-a-bitset](#interop-not-a-bitset)).

**Both readings carry the exchange**, and it is said **once, in the container**. The width is
`bit_block_container`'s own property rather than any reading's, so that is where `bit_extent`, the two
`exchanges_bits` predicates, `assign_bits` and `to_bytes` live; a reading forwards the tag constructor to them in
one line and adds only what its own vocabulary requires, and `xstd::bit_convert` reads `to_bytes` through
`storage_access`, the one door both adaptors open to the library's free functions. Said in each adaptor instead,
the `dynamic_extent`-to-zero guard — needed because `dynamic_extent` is `SIZE_MAX` and `byte_count` of that is two
exabytes — would be a copy per adaptor of one idea. The differences between the readings are each forced by
something the reading already is.

The **sequence** reading takes it whole, on `bit_layout` exactly as the set reading does — with one constraint
the set reading has no need of. A `bit_subspan` is a *window*: a bit offset and a size of its own into storage it
does not span, so its position zero is not the storage's and its bytes are not the storage's bytes. Asking
`has_static_width` alone would wave it through, because a window over a static container reports that
*container's* extent rather than its own size, and `to_bytes` would then hand back the wrong bits. So
`bit_convert` reads a window as a run-time width of its own (`bit_width_of` asks a `view` for `is_windowed`),
a block at a time from its offset through `block_at`, so that its first position is position zero of the copy. A
`bit_span`, which is not a window, spans the whole container and converts like an owner.

Two block widths over the same `N` are two spellings of one field of bits, so they cross on this rule with
neither side named: `basic_bit_array<uint8_t, 64>` converts to and from `basic_bit_array<uint64_t, 64>` because the
byte is the common ground where the block is not.

The set reading and the sequence reading cross **directly**, both ways: `xstd::bit_convert<xstd::bit_array<64>>(set)`
is one copy of the blocks, because each *has* bit storage at a layout the library knows ([is-and-has](#is-and-has)).
Neither is read as a `container_source`: each has bit storage at a layout this library lays out itself, and crosses
through its blocks rather than through its object.

#### the tag that deduces a width

The way in is a tagged constructor, not a static member, because a static member names its own type and nothing
can be deduced through it, and because a constructor is what `emplace_back` and the other in-place constructions
reach. The tag is `std::from_range`'s twin -- `xstd::from_blocks_t`,
an explicitly defaulted constructor, and the object `xstd::from_blocks` -- because the job is the same one: saying at
the call site which reading of the argument is meant. `basic_bit_array(xstd::from_blocks, board)` is
`basic_bit_array<std::uint64_t, 64>`, and `basic_bit_fixed_set(xstd::from_blocks, blocks)` over a
`std::array<std::uint8_t, 3>` is `basic_bit_fixed_set<std::size_t, std::uint8_t, 24>`: the guides deduce the block type and the
width, with `std::size_t` as the key, from an unsigned integer, a `std::array` of them or a built-in array of them, the
storage whose width the type carries. The guide for a built-in array takes it by reference, `Block const (&)[K]`, so
it keeps its bound rather than decaying to a pointer, and deduces what the `std::array<Block, K>` guide does.

`bit_block_container` takes the same tag. Its adopting constructor moves in owned blocks whose type holds exactly
the width, as well as the growable blocks a run-time width adopts, and stores a width only where the storage has
one; so `bit_block_container(xstd::from_blocks, std::array<std::uint8_t, 3>{})` deduces
`bit_block_container<std::array<std::uint8_t, 3>>` through that constructor's own guide. A templated constructor
reads any other bits of a layout its width exchanges. Two explicit guides remain, both for what is not a range of
owned blocks: one block deduces `std::array<Block, 1>`, and a built-in array `Block[K]` deduces `std::array<Block, K>`.

None of this reaches a run-time width, which has no width in its type for a guide to find.

#### is-and-has

Two things cross this door, and the library keeps them apart. A type **is** bit storage when its own blocks are the
bits: an unsigned block, or a sized contiguous range of them that subscripts, which is `xstd::bit_blocks`. A type
**has** bit storage when its internal storage is such blocks at a layout known or trusted: every owner and every
view over a whole width here, `std::bitset<N>`, `boost::dynamic_bitset` through its extension header, and bit
storage itself, which trivially has itself.

Each relation gets one operation and one concept. Blocks that *are* bit storage for `X` are taken as they are by
the tag constructor, `X(xstd::from_blocks, b)`, where the tag says the blocks are bits rather than keys or
elements; `xstd::bit_constructible_from<X, B>` names it. Anything that *has* bit storage converts to anything else
that does through `xstd::bit_convert<To>(from)`, which maps positions and may copy, shift or throw;
`xstd::bit_convertible_to<From, To>` names it. Between two fixed widths the target is at least as wide as the
source, the blocks are copied through the byte currency with the positions past the source's width clear, and a
view is read from and never written into. So `bit_convert<xstd::bit_array<64>>(set)`,
`bit_convert<std::bitset<64>>(set)` and `bit_convert<std::uint64_t>(set)` are one rule, and a `std::bitset` never
goes through the tag, which would blur a type that has storage with one that is it.

#### the same job `flat_set` gives `extract` and `replace`

`std::flat_set` has a door of its own onto its representation: `extract() &&` hands the underlying
`KeyContainer` out and leaves the set empty, and `replace(container_type&&)` takes a sorted-unique one back.
Together they are how a `flat_set` is built cheaply from a vector you already have, and how you get the sorted
vector back out afterwards. At a static width, `from_blocks` and `xstd::bit_convert<B>` are this library's answer
to the same need, and the two pairs differ in four ways that all follow from what the representation *is*.

| | `flat_set::extract`/`replace` | `from_blocks`/`bit_convert<B>` |
|---|---|---|
| what crosses | one fixed type, `container_type` | any `B` a **concept** admits |
| cost | a move of the container | a copy of `N` bits |
| what `extract` leaves behind | an empty set -- it is `&&`-qualified and destructive | nothing; a fixed width is copied out and the set is untouched |
| what `replace` promises | a **precondition**: sorted, unique, no duplicates. Violating it is UB | nothing to promise; every field of `N` bits is a valid set |

The first row is the substantive one. `container_type` is a single type fixed by the class template, so
`extract`/`replace` cross with `vector<Key>` and nothing else -- another `flat_set` over a `deque<Key>` is a
different type with a different door. What crosses is named by concepts instead: the tag takes any block or array of
blocks that is bit storage, and `xstd::bit_convert` carries an
`unsigned long long`, a `std::array` of blocks and a `std::bitset<N>` of the right width on
one rule, the last of them read on the terms `std::bit_cast` reads any object (the field of bits earlier in this
section).

The fourth row is why the naming could be lighter here than the standard's. `replace` is `void` and cannot check
its precondition without doing the linear work the call exists to avoid, so the standard makes it UB and the name
carries the warning. `from_blocks` has no precondition at all: the domain of an `N`-bit field and the domain of a
set over `[0, N)` are the same set of values, and the map between them is a bijection. So the name is not there
to warn about validity -- it is there to say **which reading of the bits you meant**, which is
`(from_blocks, 5u)` reading `{0, 2}` and not `{5}`, and that is a different job from `replace`'s.

The third row is the one to watch when porting. `extract()` is destructive and `&&`-qualified because moving the
container out is the whole point; `bit_convert<B>` copies a fixed width, because at `N` bits there is nothing to
move that is cheaper than the copy. A loop that calls `extract` once and `bit_convert` once is not the same loop, and
the `flat_set` one is the one that has to be written carefully.

The run-time widths take `from_blocks` too, with a different argument: not a block or an array to copy, since
`std::bitset` names one `N` and a growing set has no single value for it, but the block container itself, which they
adopt. The constructor is `flat_set`'s container constructor under the tag -- `block_container_type` by value, moved
in, with an allocator-extended form beside it -- and `flat_set`'s pair completes it: `extract() &&` hands out
`block_container_type` -- the `std::vector`, `bounded_blocks` or `small_vector` of blocks -- and leaves the owner
at width zero, and `replace(block_container_type&&)` takes one back. All three are a move and nothing else, on every
reading's run-time-width owner and on no view, since a view has nothing of its own to hand. The static widths' tag
constructor requires a width in the type and the adopting one a width in the object, so the two never meet.

Each owner deduces through guides of its own. `basic_bit_vector(from_blocks, std::move(v))` deduces
`basic_bit_vector<Block, Allocator>` from a guide spelled on `std::vector<Block, Allocator>`, and the bounded names
deduce their capacity from `bounded_blocks<Block, K>`, adopted at `N = K * digits`: an alias template is
transparent, so `K` deduces through it from a `std::inplace_vector<Block, K>` or a
`boost::container::static_vector<Block, K>`, whichever the library holds the blocks in. There is no
guide over every resizable storage, and `std::vector` has no such guide to match. A storage the library does not
name has no owner of its own; the adaptor over it adopts it the same way, which is how the tests run
`test::minimal_blocks`.

`replace` has no precondition to state, which is where it parts from `flat_set`'s: the width becomes the blocks'
whole width, every bit a position, so there is no tail for it to find dirty and no order for it to find broken. The
price is that a width is not carried through the round trip -- `extract` hands out whole blocks, the unused tail
clear, and `replace` reads them as whole blocks -- so a sequence of 70 comes back as one of 128 with the last 58
false. The set reading loses nothing, its width being only capacity; the sequence reading `resize`s
afterwards when the exact width matters. Adoption reads its blocks the same way. None is untagged, for the reason blocks take a tag: a
range of unsigned integers already means something to each reading's constructors, and the name is what says blocks.

The conversion operator reaches its storage through the `storage()` accessor and never through `m_bits`. The
member is a `Bits*` wherever a reading refers rather than owns, so naming it directly compiled for an owner and
was a hard error for a view — while the constraint answered *yes* either way. That is the worst shape a concept
can have: the question says the conversion exists and the call then fails to compile. The constructor is the
owner's alone, since writing through a view would write bits it does not own.

### padding

`static_used_bits` is the mask of the last block that is not padding. `num_bits` is `align_up(N)`, so
`num_bits - N` lies in `[0, bits_per_block)` and the shift is always in range.

Width zero needs no case of its own: it holds no block, so it has no padding, and the mask of all ones that
`num_bits - N == 0` gives it names nothing, since every arm that applies the mask asks first whether there is
padding to clear. `used_bits()` is the same mask at a run-time width, asked only where a block exists.

The width member is a `std::size_t`, widened to the blocks' alignment where they out-align one:

```cpp
template<class Blocks>
using stored_width_t = std::conditional_t<
        (alignof(std::size_t) >= alignof(Blocks)), std::size_t,
        std::conditional_t<(alignof(std::uint64_t) >= alignof(Blocks)), std::uint64_t, std::ranges::range_value_t<Blocks>>>;
```

A storage out-aligns a `size_t` in two ways, and each arm answers one. A storage holding its blocks inline does so
by the blocks' own alignment, and past a `std::uint64_t`'s that is a 128-bit block, whose `block_type` is both wide
enough to hold any width and exactly the size of the gap it fills:
`bit_block_container<std::inplace_vector<xstd::uint128, K>, N>` is that cell. A 32-bit target whose 64-bit integers
keep their 8-byte alignment, as wasm32's do, reaches the `std::uint64_t` arm through any storage holding one: a
`std::vector` whose allocator carries a `std::int64_t` is 8-aligned against a 4-byte `size_t` whatever its blocks
are, so its `block_type` may be an `unsigned char` that cannot hold the width, where a `std::uint64_t` both holds it
and fills the gap. Either way `bit_block_container` is then its two members and nothing else, at the same size the
padding cost. `std::array` reaches none of this, a static width carrying no member at all, and neither does a
`std::vector` over a stateless allocator, aligned as a pointer whatever it holds. The `static_assert` in
`bit_block_container` holds the two facts that make the chosen type the right carrier, at least a `size_t`'s size
and at least the blocks' alignment, so a storage over-aligned for some other reason fails loudly rather than
truncating a width. Every reader goes through `size()`, which converts once, so the arithmetic stays a `size_t`'s.

### default-construction

A defaulted default constructor plus an NSDMI, rather than two constructors constrained on the extent:
`std::vector` default-constructs empty, and so does a run-time width here -- width zero, no blocks, no
allocation, as `std::vector<bool>` has it. The two members live in a base, `bit_members`, whose specialization for a
capacity of nought declares them with no initializer and `[[no_unique_address]]`, since a default member initializer
makes a defaulted default constructor nontrivial ([the-bounded-column](#the-bounded-column)); everywhere else the
blocks stay a plain member, whose tail padding an overlappable one would expose to `-Wpadded`.
A static width holds exactly the blocks its width fills, which is none at width zero: `num_blocks_v` and `blocks_for`
give the same count, and the static arms that index a block directly are the ones selected at one block or two.

### structural-at-aligned-widths

`[array.overview]/4` makes `std::array<bool, N>` a structural type, usable as a non-type template argument. A
structural class has every base and every non-static data member public and non-`mutable` ([temp.param]/7), and two
values name the same specialization exactly when their members do, word for word. That is only sound where every word
pattern is a value. At a width that is not a multiple of the block's digits, the last block has unused high bits,
and `==`, `<=>` and hashing compare whole blocks on the promise that those bits stay clear. Public blocks would let a
caller set them, so those widths keep their blocks non-public and are not structural.

At an aligned width, where N fills its blocks, there is no invariant to protect: every bit is a position, and the
layout is exactly the `std::array<Block, K>` plus an empty width tag. Access control cannot depend on a template
argument, but the choice of base can. `bit_block_container` takes `structural_bit_members`, a struct whose `m_size` and
`m_blocks` are public, when its blocks are a `std::array` whose `xstd::bit_blocks_extent_v` is exactly N bits (`structural_blocks_v`,
answered as `bit_block_container::is_structural`), and `bit_members`, whose members are protected, otherwise. Each adaptor
keeps its `m_bits` in an `adapted_bits` base on the same terms, public only for an owner over structural storage. The
names are the same in either base and the adaptors reach them through the same `using`-declarations, so nothing else
in the library notices which one it got; the public members are not an interface, only what makes the owner
structural.

That gives `bit_align<bit_array<N>>` and `bit_align<bit_fixed_set<N>>` at any width, and the plain owners
at a multiple of the block's digits. Width zero is among them: its blocks are a `std::array<Block, 0>`, structural by
`[array.overview]/4` like any other extent, with no bit in it to keep clear. The bounded owners cannot follow at any width,
since neither `std::inplace_vector` nor `boost::container::static_vector` is structural, and the views hold pointers
kept non-public as before.

### the-moved-from-state

A run-time width's move leaves the source at width zero with no blocks: the same object the default constructor
makes, and one on which every member without a precondition works, `push_back` and `insert` included. The implicit
moves cannot say that: they move the blocks and copy the width, so a moved-from `bit_vector` would answer
`size() == 100` over no blocks and a `push_back` would write past the end. The moves are written out for the run-time
widths alone, constrained on `not has_static_size`, and defaulted for the static ones, which keeps those trivial.
A capacity of nought is defaulted too: its width is zero without being stored, so there is nothing to reset.

Zero blocks is what makes the state reachable without allocating. Restoring one block in the source would have
asked `std::vector` for memory inside a move, which is what `noexcept` on every cell's move rules out. The source's
blocks are `clear()`ed after the move, because `std::inplace_vector` and `boost::container::small_vector` keep
their moved-from elements where `std::vector` does not. Move assignment takes the source's blocks into a local
before writing anything, so a self-move puts back exactly what it took, with no branch on `this == &other` for a
test to have to take.

### the-strong-assignments

Both assignments of a run-time width give the strong guarantee, where the standard asks a container only for the
basic one. Blocks are trivially copyable, so the one step that can throw is an allocation, and it comes first: blocks
that fit the capacity already held are copied over the old ones, which allocates nothing, and otherwise the copy is
built in storage of its own and moved in whole; the width is written last. A defaulted copy assignment would write
the width first, it being the first member, so a refused allocation would leave a width over blocks that do not hold
it, and reads past the old blocks would go out of bounds. The move assignment copies only between unequal allocators that
stay, and anywhere else moves the blocks without throwing. An allocator that propagates on copy assignment comes
over with a copy built under it, moved in where the allocator propagates on move assignment and swapped in where it
propagates on swap; one that does neither leaves that copy only the basic guarantee, which is all `std::vector`'s
gives. The copy is moved in rather than swapped wherever both would do: Boost 1.83 swaps a `small_vector` that
holds its blocks inline with one that does not element by element, growing the inline one, which allocates. A
capacity in the type keeps the defaulted copy assignment, which cannot allocate and so stays trivial where its
blocks are.

### growth

Growth is the run-time width's alone, and every member of it leaves the unused tail clear. `resize(n, value)`
resizes the blocks to what `n` needs, filled with `value`, moves the width, and masks the new last block;
growing with ones also sets the old last block's tail, clear by the invariant, since those are the first new
positions -- and sets it only after the blocks have grown, so that a growth `std::inplace_vector` refuses with
`bad_alloc` leaves the value as it was. `push_back` and `pop_back` are `resize` by one, `clear` is `resize(0)` -- the object a default
constructor makes -- and `append(block)` is boost's: the block's bits become the next `bits_per_block`
positions, split across two blocks where the width is not aligned, and pushed as the first block at width
zero. It keeps boost's strong guarantee: the split pushes the new block before it writes the old last
block's tail, as boost's does, so a refused push leaves that tail clear. `reserve`, `capacity` and `shrink_to_fit` are in bits and exist where the blocks have them:
`std::vector` and `std::inplace_vector`, not `std::array`.

`clear()` here is the sequence reading's, width to zero, which is what `std::vector<bool>` and
`boost::dynamic_bitset` mean by it; the set reading's `clear()` is `fill(false)` and never reaches this
member, so the landmine #80 recorded -- probing `clear()` on boost and emptying the width -- cannot recur.

`bounded_blocks<Block, num_blocks_v<Block, N>>` is the third storage: a run-time width under a compile-time capacity
of exactly `N` bits, held in a `std::inplace_vector` where `__cpp_lib_inplace_vector` says the library has one and in
a `boost::container::static_vector` everywhere else. Either satisfies `owned_bit_blocks` as it is, but each counts
blocks, so it refuses growth only a whole block at a time; a capacity that stops inside the last block is the
container's to hold. `check_capacity` does that on every growth path -- `resize`, `push_back`, `append`, `reserve`
and the sized constructor -- and throws `std::bad_alloc`, as `std::inplace_vector` specifies, before anything is
written, so the storage is never asked for a block past the capacity.

The adaptors take growth by detection on the storage: growth is a container's business and no view's, so it exists on an owner and on nothing else. `sequence_adaptor` is
`std::vector<bool>` where its storage grows -- the count and count-value constructors, the range and
`initializer_list` constructors and assignments, `assign`, `resize`, `clear`, `push_back`, `pop_back`,
`emplace_back`, with `reserve`, `capacity` and `shrink_to_fit` where the blocks have them -- and
`std::array<bool, N>` where it does not, each member requiring the storage member it forwards to. `set_adaptor`
takes none by name: a set grows by `insert`, and the storage's `growing_insert` grows a run-time width to hold
the key ([asking-is-total](#asking-is-total)).

## Scans

### inclusive-is-the-primitive

Inclusive and exclusive name the only thing separating the two forward scans: whether `n` itself is a
candidate. That is a property of the scan rather than of a reading, which is why this is not called
`lower_bound` — that is the set reading's word, and this layer serves the sequence reading equally.
`bit_fixed_set::lower_bound` is where the set name belongs, and it maps here one for one.

The inclusive form is the primitive and the exclusive one is derived, because the reverse does not close:

```
exclusive_find_next(n) == inclusive_find_next(n + 1)     for every n
find_first()           == inclusive_find_next(0)
```

Exclusive-first would need `find_first() == exclusive_find_next(-1)`, and `size_t` has no such value —
which is exactly what would force a branch on `x == 0`. **Both derivations are `+ 1` and never
`− 1`**, so nothing wraps at zero. Boost's `find_next` and libstdc++'s `_M_do_find_next` are both the
exclusive form, correctly: their iteration idiom is `find_first()` then `find_next(i)`, which never needs
the inclusive one.

`n == size()` rather than `n >= size()`: the precondition is `n <= size()`, so `size()` is the only value
the scan cannot start from. `is_valid` does not say this — it is `n < size()` — and `size()` is a
legitimate argument, meaning "no such position".

### offset-guards

Neither forward scan guards on `offset != 0`. `>> 0` is the identity, so the masked test is correct at a
block boundary too, and advancing past that block afterwards is right either way. Neither reference
implementation guards it — boost shifts by `ind` then falls to `m_do_find_from(blk + 1)`, libstdc++ masks
by `~0 << whichbit` then does `__i++` — and the guard skips no work: at offset 0 it merely moves the same
test into the loop's first iteration. [#88](https://github.com/rhalbersma/xstd-bits/issues/88) measured it
as pure cost.

The reverse scan's `reverse_offset != 0` guard is a different question, and is not about the shift:
`left_bit - offset` is in `[0, left_bit]`, so shifting is always in range and by zero is the identity. It
is about which block the range scan must start at — at `index` when the whole block is in range, below it
when the masked block came up empty. Naming the fallback block outright, as the two-block case does, makes
that distinction disappear.

### two-block-case

At two blocks the tail is one named block rather than a range: the general walk costs more than the whole
scan is worth at this width. `index` is a run-time value — `n` is — but the block count is not, so the
second half is a test and not a loop.

**Indexed rather than branched**: `m_blocks[index]`, not an `if` on `index`, so the common path is one
computed load. Only when the starting block comes up empty does it matter which block we started in, and
then only to decide whether block 1 is still ahead. Writing this as a branch instead cost **10 instructions
at `-O3 -march=native`**.

### index-walks

A plain index walk rather than `drop` + `find_if`, and a descending one rather than `reverse` + `drop` +
`find_if`. The iterator forms would have to recover the block's position with `distance()`; the index *is* that
position, so it never needs recovering. The reverse form would compose two adaptors only to have `distance()`
undo them.

### the-blit

`bit_block_container::block_at(pos)` is a block's worth of bits at any position:
the bits `[pos, pos + digits)`, assembled from `block(pos / digits) >> r` and the next block `<< (digits - r)`
where `r = pos % digits`. Two things make it total where it is called. A shift by `digits` is undefined, so
an aligned read is the block itself with no second term; and the last block has nothing above it, so the
read stops there rather than asking for a block past the end. What the block reaches beyond the width is the
clear tail, and it is the caller's to trim. It is the one primitive under every unaligned read: the sequence
adaptor's `append_range` from a sequence read by block ([the-range-members](#the-range-members)), and the bulk
operations on a window ([windows](#windows)).

`bit_block_container::block_at(pos, value, mask)` is its write side, on our storage alone as `set_block`
is: the bits of `value` under `mask` land at `[pos, pos + digits)`, split over two blocks where `pos` is not
aligned, the tail kept clear. Every masked write goes through it: the storage's ranged `set`,
`xstd::bit_fill` over a window, the copy algorithms at any two offsets, and a window's bulk operators, each walking the blocks a range spans with the mask of what each
holds, whole blocks and a partial one at the end.

### the-funnel-shift

Under `block_at` and both shift operators is one operation: two adjacent blocks spliced into a double-width block
and shifted down. `bit_block_container::straddled_block(index, L_shift, R_shift)` is that splice, and the
three sites read as three uses of it rather than three spellings.

It takes the shift **and** its complement, named as both operators already name them, because both already hold
the pair as loop invariants: passing only one would have the other derived back inside from what it was derived
from outside. That round trip is one hoisted instruction, but it is also the only thing that makes the codegen
differ -- with the pair passed, GCC emits the same instruction mix as the splice written out at each site (954
lines, 60 subtractions, six `$64` immediates, against 1016, 67 and twelve with one shift passed), differing only
in register assignment. The two
adding to the block width is the whole contract, and the assert says so. There is no one-shift overload:
`block_at` holds only the offset and spells the complement at its single call site.

The three do not look alike, which is why the one operation under them is easy to miss. `block_at(n)` takes
`(index, offset)` from `n`.
`operator>>=` reads `(i + n_blocks, R_shift)` — literally `block_at(i * digits + n)`, reached without
recomputing the division. `operator<<=` reads *one block below* its destination, `(i - n_blocks - 1,
R_shift)`, because it walks down while writing up; its loop names `L_shift` and the complement is the
offset, which is what hid the third one.

**It asserts rather than guards, and that is the design content.** Every caller already handles its own edge,
and they are different edges: `block_at`'s is a full-width shift or a missing block above, each operator's is
the destination block with nothing beyond it, and — the one that matters — both operators branch on the
*aligned* case **outside** their loop, into `std::shift_left` or `std::shift_right`. A guarded primitive
called once per block would drag that test into the loop and cost the `memmove` fast path, which is the one
thing those operators get for free. Keeping the check where it is preserves it, and the splice still exists
once instead of three times.

## Contracts

### total-versus-precondition

`bit_block_container::exclusive_find_prev` is deliberately **not** total. Where `inclusive_find_next`
answers `size()` for "nothing at or above", this one has a precondition instead, and that is the whole of why it
is three instructions cheaper at every width: it never materializes a not-found value.

It is safe because **reverse iteration supplies the guard the function does not**. `rend()` is
`make_reverse_iterator(begin())`, and `std::reverse_iterator` stops there, so `operator--` is never applied
at `begin()`. A hand-written loop gets no such help: the forward idiom terminates itself against `size()`,
the reverse one runs off the bottom.

Totality is the **container's** contract to keep, not this layer's to absorb —
[#86](https://github.com/rhalbersma/xstd-bits/issues/86) settled the same division one layer down.

### the-wraparound-assert

`exclusive_find_prev` asserts `is_valid(n - 1)`, mirroring `exclusive_find_next`'s `is_valid(n)`: each says
the position actually scanned from is one this container has.

`n - 1` is that position here, and **the wraparound does the work at the bottom** — `0 - 1` is `SIZE_MAX`,
which no width admits — so this states `1 <= n <= size()` without a second predicate. `is_valid` alone
would be wrong: `size()` is a legitimate argument, meaning "from the end".

### the-cheapest-contract

`bit_block_container` states each operation in its cheapest form and lets the caller that needs more pay
for more. `exclusive_find_next` requires `is_valid(n)`; `exclusive_find_prev` requires a set position strictly
below `n` — which `any()` does not establish, a container whose set positions all lie above `n` having none
below it.

**A caller that needs a total answer restores totality where the width is already in hand.** Reverse iteration
does it with `rend()` ([total-versus-precondition](#total-versus-precondition)), and the set reading's `upper_bound`
guards the forward step with the width it already holds. Neither guard is in the storage, because every other
caller of either step already knows it does not need them.

This is the direction that matters. A caller may widen a contract; the storage may not narrow one. Putting the
guard in the storage would cost the three instructions at every call site to serve the one that asked.

## One storage

### one-storage

**There is no storage trait.** The adaptors take a `bit_block_container` and ask it directly, so a question about
a storage is answered by the storage, in its own name. The alternative is a `bit_traits<Bits>`, specialized beside
each storage and carried as a fourth template parameter, `set_adaptor<Bits, Own, Traits>`, inside a layer of
generic scans that synthesizes whatever a specialization does not declare, choosing a block tier or an element
tier by whether the entry is there.

Such a layer serves a plurality of storages, and there is none. `std::bitset` and `boost::dynamic_bitset` are
comparison targets rather than adapted storages ([owning-is-ours](#owning-is-ours)), and the adaptors are
constrained to `bit_block_container` nominally, so a storage is ours because the constraint names it and not
because its members answer. Over the one storage a trait would name exactly one thing, the adaptors would name it
twice, and `bit_traits<bit_block_container<...>>` would be twenty entries forwarding to members its only caller
already has.

**What that is worth is not the line count.** The element tier is the half of such a layer that only a foreign
storage could ever reach, and it is reachable code carrying reachable branches — a reverse walk one position at a
time, quadratic over a traversal, behind a probe that our storage always fails. Synthesized scans also have to be
total, because a fallback cannot know what its caller checked; the storage's members state preconditions instead
([the-cheapest-contract](#the-cheapest-contract)). And every question has one answer at one address, so a reader
chasing `find_first` reaches the loop rather than a dispatcher over a probe.

**What it costs is the extension point.** Specializing a trait for `MyStorage` is how an outside storage would
join, and nothing stands in for it: a storage joins by being a `bit_block_container`, which lives under `detail/`
and is spelled by the library alone. That is a real narrowing and it is deliberate — a trait would pay for a
generality with no second instance ([the-interface-line](#the-interface-line)).

**The detector is local.** The obvious `is_specialization_of` names a template through a `template<class...>`
parameter, and this storage is a type then a value, so it does not apply. The adaptors are constrained instead on
`bit_block_container_type`, a concept over a variable template specialized on `bit_block_container<Blocks, N>`,
which strips the const a view over a const owner names:

```cpp
template<class T>
inline constexpr bool is_bit_block_container_v = false;

template<class Blocks, std::size_t N>
inline constexpr bool is_bit_block_container_v<bit_block_container<Blocks, N>> = true;

template<class T>
concept bit_block_container_type = is_bit_block_container_v<std::remove_const_t<T>>;

template<bit_block_container_type Bits, storage Store = storage::owned, class Derived = void, class Key = std::size_t, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, class Compare = std::less<Key>>
class set_adaptor;
```

A generic concept for the shape would have to be spelt `<class U, U...>` rather than `<class, auto...>`, which
would also take a value typed by the type, and would still need the const stripped; two specializations and a
concept state the one fact directly. A constrained parameter is no obstacle: `owned_bit_blocks Blocks` binds it as
a bare `class` would.

### what-the-readings-share

Almost everything the readings ask of a storage is the same operation under another name, which is why
the vocabulary is smaller than the APIs suggest:

| what a reading calls | what the storage calls it | |
|---|---|---|
| set `size()` (cardinality) | `count()` | |
| set `max_size()` (width) | `size()` | |
| `contains(n)` | `test(n)` | the same operation |
| set `erase(n)` | `reset(n)` | the same operation |
| set `clear()` | `fill(false)` | |
| sequence `size()` / `operator[]` | `size()` / `assign(n, value)` | |
| a view's static extent | `extent` | |

Three members are left over, and they are the three no reading can spell for itself:

- **`growing_insert(n)`** is the only operation that can *grow*. The partial `insert(n)` asserts `is_valid(n)`
  and answers whether the position was new; the total one resizes first where the storage can. They are
  deliberately two names: a silent substitution of one for the other is a precondition quietly dropped.
- **`assign(n, value)`** is the positional write, spelled apart from `set` on purpose. `set(bool)` and
  `set(std::size_t)` are ambiguous for a literal `0`, and `set(n, value)` is one of the five absences that
  make the common vocabulary the intersection of the three vocabularies rather than the union
  ([the-common-vocabulary](#the-common-vocabulary)).
- **`fill(value)`** is bulk, and `clear` is `fill(false)`. Not an overload of `set` for the same reason.

### why-nested

The free functions live in `xstd::bits::detail` rather than in `xstd`, and the nesting is load-bearing.

Since C++20 ([temp.names]/3, P0846) an unqualified call with explicit template arguments — `shl<Block>(b, n)`,
or the `scan_first<Traits>(c)` this rule was written for — parses its `<` as a template argument list and then
performs ADL **with those arguments included**, so `std` and `boost`, the associated namespaces of the types
in play, would join the overload set. `[namespace.std]` bars *users* from adding to `std` but not
implementations, and boost is under no such constraint.

Down here nothing is visible unqualified from `xstd`, so the qualification is enforced by **scoping** rather
than by remembering a prefix at every call site, and no class is needed to stand in for the namespace.

### the-two-reserved-names

`std::bitset`'s two implementation hooks are **complementary, not paired** — which is the opposite of what
it looks like, and worth stating because the wrong guess silently costs a tier:

| | `_Getword` | `_Find_first` / `_Find_next` |
|---|---|---|
| libstdc++ | no — it is `_M_getword`, on an inaccessible base | yes |
| MSVC | yes | no |
| libc++ | no | no |

So neither implies the other, and each is worth a constrained entry on its own. Where `_Getword` is
reachable the walks run block-wise, including a `find_prev` neither library supplies; where `_Find_first` is
reachable the forward scans are native. Both return `N` when nothing is set, which is what a set reading's
end position is here, so no `npos` mapping is needed — unlike boost, whose `find_first` answers `npos`.

The third row is not the end of block access on libc++. A width that fits one `unsigned long long` reads
its single block through `to_ullong()`: portable, `constexpr`, no reserved name, and the constraint
`N <= numeric_limits<unsigned long long>::digits` puts the `overflow_error` it could throw out of reach, so
`std::bitset<64>` runs the block walks on every library. Above one word it is `_Getword` or nothing:
shifting and calling `to_ullong` per block is O(N) each, worse than the element walk it would replace.

### proxies-compare-themselves

Neither ordering needs a comparator. A proxy reference converts implicitly to its `value_type`, and that
conversion is what satisfies `three_way_comparable` — checked for both this library's `sequence_reference`
and `std::vector<bool>::reference`, on which `lexicographical_compare_three_way` works with no comparator
at all. An explicit `[](bool a, bool b) { return int(a) <=> int(b); }` says nothing the conversion has not
already said.

### block-writes

`set_block` is the write side of `block()`, and was deliberately never one of the questions a reading asks:
block writes are only ever needed on storage we control, and the unused tail is `bit_block_container`'s
to keep.

## Orderings

### two-readings-disagree

"Lexicographic" is underspecified below the reading layer. Both readings are lexicographic; they order
over different sequences, and **they disagree**, as two pairs show:

| pair | set reading, ascending positions | sequence reading, bools from index 0 |
|---|---|---|
| `{0}` against `{1}` | `[0]` vs `[1]`: less | `[1,0]` vs `[0,1]`: greater |
| `{0,1}` against `{1}` | `[0,1]` vs `[1]`: less | `[1,1]` vs `[0,1]`: greater |

A storage serving two readings cannot hold one of their orderings under a neutral name without choosing for
its callers, so it holds none. The orderings sit beside it, **both separately named**:
`detail/comparisons.hpp` has a `set_three_way` and a `sequence_three_way` over the storage's
primitives, never one `three_way`, so a caller says which reading it means rather than being handed whichever
the storage happened to pick. Each is the reading's own blockwise answer to `std::lexicographical_compare_three_way`,
which is also what pins it: whatever it answers has to agree with that algorithm over that reading's own
iterators.

**Each is named for the reading it orders, the same names `set_reading_tag` and `sequence_reading_tag`
give.** Positions, and bools from index 0.

**A name is a reading's order, not an adaptor's.** Neither takes an adaptor type, only the storage, so
the order is shared by every reading that means it. The one in view is the sequence order: a future `bit_string`,
whose `string_reading_tag` derives from `sequence_reading_tag`, orders its bools from index 0 exactly as a
`bit_vector` does, and so reuses `sequence_three_way` as it stands rather than naming a fourth ordering. The bit
string `std::bitset::to_string()` prints is a different order, most significant first, and is not that reading.

**Recorded against a long-held hypothesis: that with the right bit order the set reading's `<=>` could be a
plain `lexicographical_compare_three_way` over the blocks.** It cannot, in any bit order. The hypothesis is the
whole case for a mirrored layout -- the set order "equivalent to doing the integer comparison `wL > wR` on the
underlying words" -- and stating the layout without its refutation would leave it free to be mirrored on that
reasoning, so the refutation lives here.

Take the two cases where **x holds the lowest differing position p**:

| pair | p | does y hold anything above p? | set order |
|---|---|---|---|
| `{0}` against `{1}` | 0 | yes, `1` | x is **less** |
| `{0,1}` against `{0}` | 1 | no | x is **greater** |

Same local observation, opposite answers, and `set_three_way` takes exactly those two
branches: `any_above` answers true in the first and false in the second. A lexicographic scan cannot tell them
apart, because it has already returned by the time it reaches `p`.

The reason is structural rather than a shortfall of any particular walk. A blockwise lexicographic compare
orders two vectors of the **same** length `N`, position by position, in some fixed order of positions. The set
order orders two sorted element lists of **different** lengths, where a shorter list that is a prefix is less.
Those lists are the *support* of the bit vectors and their lengths are popcounts, so the set order asks a
question about what lies above `p` that no fixed positional scan answers. Relabelling positions moves both
cases together, so the pair above can be built in whatever bit order is chosen; `any_above` is irreducible.

A bit string, the order `std::bitset` and `boost::dynamic_bitset` mean by `<`, is the one that does get to be
the standard algorithm over the blocks reversed: it is a fixed-length vector, so it has no prefix case to answer.
It is colexicographic on the elements, comparing from the largest position down; the set reading is
lexicographic, from the smallest up. Two different orders on sets, not two spellings of one.

### the-ordering-primitive

Both orderings are **free functions** over the storage rather than members of it, in
`detail/comparisons.hpp`: `set_three_way(x, y)` and not `x.set_three_way(y)`. An ordering is a question about
two values with neither as its subject, and the member spelling put one of them in a place the operation does
not have -- the same asymmetry a member `operator<=>` would carry. Each is a template constrained on
`bit_block_container_type`, in the storage's own namespace, so the adaptors reach it by ordinary lookup and a caller
holding two storages by ADL. Both adaptors are constrained on the same concept, so every storage they adapt has
these primitives, and `set_adaptor`'s `<=>` calls them with no fallback, as `xstd::bit_includes`,
`xstd::bit_intersects` and `xstd::bit_disjoint` walk the storage's blocks.

They are not the storage's because they are readings and it is not. What they are built from is: `first_difference`,
`any_above`, `any_block_set`, `padded_block`, `padded_first_difference`, `padded_any_above`, `block`, `test` and
the widths, all public, and the orderings and `set_equal` say everything else in those terms. `any_above` is a
member because it IS asked of one value: whether *this* storage holds anything above a position.
`first_difference` is symmetric -- its answer is an `xor`, which commutes -- and is a member because it is a
primitive over the blocks rather than a reading's vocabulary; the same is true of `padded_first_difference`.
`padded_set_three_way` and `padded_sequence_three_way`, the width-crossing arms of the two ascending orderings,
sit with them. `set_equal` takes the orderings' form, and for the same reason: equality is as much a question
about two values with neither as its subject as an ordering is, and `x.set_equal(y)` would spell a symmetry the
operation has and the call does not. It sits beside them, while the defaulted `operator==`, width first, stays the
storage's hidden friend.

`intersects` takes the same form, and the standard library says why: **`intersects` is to `set_intersection` what
`contains` is to `find`** — the predicate form of an algorithm. `find` is a member of `std::set`, asked of one
set with a key, and so `contains` is a member too. `set_intersection` is a free algorithm over *two* ranges,
so the question is free.

**Which reading keeps a member is then decided by the counterpart, not by taste.** `std::set`
has nothing of the kind, so `set_adaptor` keeps neither a member nor a friend, and a reading asks the question
through the algorithm `xstd::bit_intersects`, named for the `set_intersection` it predicts
([algorithms-not-members](#algorithms-not-members)) — which is also the spelling the analogy above asks for.
`xstd::bit_disjoint` asks the negation, so neither question is written with a `not`.

Neither inclusion nor intersection is a storage member. `xstd::bit_includes` and `xstd::bit_intersects` walk the two
storages' blocks themselves, keeping the storage's hand-unrolled one- and two-block arms through its public
`static_num_blocks`, and ask each pair of blocks through `block_includes` and `block_intersects`: the one-block
predicates, named for their unit as the public algorithms are, `bit_` for the whole and `block_` for one block.
`block_intersects` sits in `detail/`, since the containers' single-bit tests ask it too; `block_includes` sits in
`bit_algorithm/detail/`, since only `bit_includes` does. `block_includes(a, b)` takes `std::ranges::includes`'s order
of arguments: every position of `b` is one of `a`. A proper subset and either superset are no primitive at all:
`xstd::bit_includes(a, b)` is the superset, and each proper form is one inclusion without the other, both answered
across two widths.

Both orderings are answered a block at a time, from two pieces:

- **`first_difference`** returns the lowest block at which two values differ, together with that block's
  `xor`. `countr_zero` of that `xor` is then the lowest position at which they differ. Equal values answer
  the last block and a zero `xor`, from the unrolled arms and the loop alike: the loop runs to the block
  before the last and returns that one's `xor` unconditionally, as the two-block arm does.
- **`any_above`** asks whether one value holds anything strictly above a given position. The value's bit
  *at* that position is clear -- it is the one that lacked the differing bit -- so `block >> offset` leaves
  exactly what it holds above, and the shift is always defined because `offset < digits`. No two-step shift,
  and no guard against shifting by the width.

From there the two readings differ by one clause and nothing else:

| reading | at the lowest differing position |
|---|---|
| set | whoever HOLDS it is greater, **unless** the other holds nothing above it |
| sequence | whoever HOLDS it is greater, full stop -- position 0 is the first element, so nothing above it is consulted |

**Both are total across two widths.** `sequence_adaptor::operator<=>` calls its ordering for any two lengths, so
an ordering that asserted `x.size() == y.size()` would abort every `bit_vector` comparison of two lengths in a
debug build -- and, with the assert compiled out, answer *wrongly* rather than not at all. Measured against
`lexicographical_compare_three_way` over `std::vector<bool>` across 148,225 pairs (every length 0 to 70 plus the
block boundaries 127/128/129/191/192/193, five patterns each), that form gives **27,312 wrong answers** and the
total one none.

The total form is the set reading's, one clause lighter. `padded_sequence_three_way` pads the shorter operand's
missing blocks with zero exactly as `padded_set_three_way` does -- the invariant keeps everything above
`size()` clear, so a block that is not there reads the same as a block that is
([width-is-capacity](#width-is-capacity)) -- and where the set version asks `padded_any_above` at the deciding
position, the sequence version asks nothing: position 0 is the sequence's *first* element, so holding the
lowest differing position settles it outright. What the sequence reading needs instead is the case the set
reading cannot have: agreeing at every position the two share leaves only length, and the shorter is then a
proper prefix of the longer and so less, which is `size() <=> size()`.

An assert where an answer belongs is the pattern in [the cheapest contract](#the-cheapest-contract) read the
wrong way round. A precondition is free to be narrower than the naive form only where the caller can honour it;
this caller cannot, having two lengths whenever its user does.

**Why this beats iterating.** The `lexicographical_compare_three_way` form walks *set bits*; this walks
*blocks*, and a block step is an `xor` and a test rather than a load, a shift, a `countr_zero` and a branch.
Equal values are the clearest case: iteration confirms every element, so a full 1024-bit set costs 1024
bit-scans against 16 block `xor`s. Near-equal and dense values are the same story. Only an early difference
makes the two comparable, both exiting at once.

**It is one sweep, not two passes.** `first_difference` covers blocks `[0, i]`; `any_above` then covers
`[i, n)` on **one** operand, the one that lacked the bit. Block `i` is the only one touched twice, so the
pair costs `n + 1` block reads. And when the values are equal `any_above` is never reached at all, since
`first_difference` already settles it.

**The prefix clause is not removable.** Set order is not plain lexicographic over blocks under *any*
comparator. At `digits = 4`, `A = {1}` and `B = {5}` differ in block 0, where `A₀ = {1}` and `B₀ = {}`; a
prefix rule over blocks says `B < A`, but the sets truly compare `A < B`. `any_above` is exactly the repair,
and is the whole of what separates the two readings.

**The standard algorithm is the specification.** Whatever the block-at-a-time form answers has to agree with
`lexicographical_compare_three_way` over the reading's own iterators, which is
[the invariant](#the-ordering-invariant) itself, and the test is that the two paths agree. The efficient form
is then free to state preconditions the naive one does not ([the cheapest contract](#the-cheapest-contract));
what it may not do is answer differently.

### the-set-queries

A set answers one question of a key, as a member, and three of another set, as algorithms, each named after what
the standard library already spells:

| query | true where | spelled | named after |
| :--- | :--- | :--- | :--- |
| `contains(k)` | `k` is an element | member | `std::set::contains`, the predicate form of `find` |
| `xstd::bit_includes(x, y)` | every element of `y` is one of `x`'s | algorithm | `std::ranges::includes(x, y)` |
| `xstd::bit_intersects(x, y)` | some element is in both | algorithm | `std::ranges::set_intersection`, its result nonempty |
| `xstd::bit_disjoint(x, y)` | no element is in both | algorithm | `std::ranges::set_intersection`, its result empty |

`contains` is total, as `std::set`'s is: a key past the width, and a value that is no key of the mapping at all, are
no element. The three over sets are all-of, any-of and none-of, and every other relation between two sets is one of
them, its operands swapped, or a conjunction of two:

| relation | spelled |
| :--- | :--- |
| `x ⊇ y`, superset | `xstd::bit_includes(x, y)` |
| `x ⊆ y`, subset | `xstd::bit_includes(y, x)` |
| `x ⊋ y`, proper superset | `xstd::bit_includes(x, y) and not xstd::bit_includes(y, x)` |
| `x ⊊ y`, proper subset | `xstd::bit_includes(y, x) and not xstd::bit_includes(x, y)` |
| some element is in both | `xstd::bit_intersects(x, y)` |
| no element is in both | `xstd::bit_disjoint(x, y)` |

All three are free functions because `std::ranges` spells each as one, an algorithm over two ranges rather than a
member of either, and they take its order of arguments: `includes(x, y)` asks whether `y` lies within `x`. Each reads the
blocks a step at a time, at any two run-time widths, and is `noexcept`. The second argument is
`std::type_identity_t<S> const&`, outside deduction, so it converts to the first's type, and a flag set takes a mask
there ([flag-types](#flag-types)). [P0125R0](https://wg21.link/p0125r0) proposes the four containments for
`std::bitset` as members, and `boost::dynamic_bitset` has `is_subset_of`, `is_proper_subset_of` and `intersects`;
here each relation has the one spelling above, built from the algorithms `std::ranges` already names
([algorithms-not-members](#algorithms-not-members)).

Other libraries spell the six differently, and a dash marks a question a library has no name for:

| | subset | proper subset | superset | proper superset | overlap | no overlap |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| xstd-bits | `bit_includes(b, a)` | `bit_includes(b, a) and not bit_includes(a, b)` | `bit_includes(a, b)` | `bit_includes(a, b) and not bit_includes(b, a)` | `bit_intersects(a, b)` | `bit_disjoint(a, b)` |
| `boost::dynamic_bitset` | `a.is_subset_of(b)` | `a.is_proper_subset_of(b)` | — | — | `a.intersects(b)` | — |
| `std::bitset` | — | — | — | — | — | — |
| `std::ranges` | — | — | `includes(a, b)` | — | — | — |
| Python `set` | `a <= b`, `a.issubset(b)` | `a < b` | `a >= b`, `a.issuperset(b)` | `a > b` | — | `a.isdisjoint(b)` |
| Rust `HashSet` | `a.is_subset(&b)` | — | `a.is_superset(&b)` | — | — | `a.is_disjoint(&b)` |
| Swift `Set` | `a.isSubset(of: b)` | `a.isStrictSubset(of: b)` | `a.isSuperset(of: b)` | `a.isStrictSuperset(of: b)` | — | `a.isDisjoint(with: b)` |
| Rust `bitflags` | — | — | `a.contains(b)` | — | `a.intersects(b)` | — |

Checked against Boost's
[`dynamic_bitset.hpp`](https://github.com/boostorg/dynamic_bitset/blob/boost-1.92.0/include/boost/dynamic_bitset/dynamic_bitset.hpp)
in Boost 1.92.0, [[template.bitset]](https://eel.is/c++draft/template.bitset) and
[[includes]](https://eel.is/c++draft/includes) in the working draft, Python 3.14's
[set types](https://docs.python.org/3.14/library/stdtypes.html#set-types-set-frozenset), Rust 1.99.0's
[`HashSet`](https://doc.rust-lang.org/1.99.0/std/collections/struct.HashSet.html), Swift 6.4.0's
[`Set.swift`](https://github.com/swiftlang/swift/blob/swift-6.4.0-RELEASE/stdlib/public/core/Set.swift) and
bitflags 2.13.2's [`Flags`](https://docs.rs/bitflags/2.13.2/bitflags/trait.Flags.html).

Boost names the subset direction alone, and `std::ranges::includes` and bitflags' `contains` the superset direction
alone, each with its operands swapped for the other. Here the superset direction is the one named, in
`std::ranges::includes`'s order, and the subset is that call with its operands swapped. Boost and bitflags name only
the positive overlap and Python, Rust and Swift only the negative one; this library names both, a nonempty and an
empty `set_intersection`, so neither is spelled as the other's `not`.

### degenerate-widths

Two widths get their own `if constexpr` arm in the orderings, for the reason in
[per-instantiation-slots](#per-instantiation-slots) -- an arm that no input of *this instantiation* can take
is an uncovered branch, and gcovr scores instantiations separately:

- **Width zero** never differs, so both orderings answer `equal` outright. Without the arm, the `diff == 0`
  test would have an untakeable false branch, and `any_above` would be instantiated with no caller able to
  reach it.
- **Width one** has a single position, so the value lacking it is empty and `any_above` is constantly false.
  The set ordering answers `test(0) <=> other.test(0)` instead, which also skips instantiating `any_above`.
  The two readings happen to coincide here, the empty set being both the prefix and the smaller bool.

Everything wider shares the general arms. A single block still needs the block-loop specialisations -- a
one-block instantiation cannot take a loop's exit branch -- which is why `first_difference` and `any_above`
each spell out the one- and two-block cases the way `find_front` and `xstd::bit_intersects` do.

Each caller of a scan carries the width-zero arm ahead of the call, and `is_zero_width` is the one
predicate they all ask: at width zero every answer is zero -- the total answer, `size()` -- and the storage's
step, which asserts, is never instantiated for it. The set iterator's
equality takes an arm there too: a zero width has one position, so every iterator over it is the same one,
and `operator==` says so outright. The point is the loops an optimizer sees into. Three spellings of the
step failed: one that returned `size()` left every `while (it != last) ++it` provably unable to advance,
which crashes MSVC's optimizer (range-v3's symmetric difference over a zero-extent set inside a zero-trip
loop was the reproducer) and which gcc 15 diagnoses, through the counter overflow it implies in
`ranges::distance`, as undefined behaviour; one marked `std::unreachable()` cured both and made MSVC report
range-v3's code after it as unreachable (C4702), which `/external` does not silence; and one that simply
moved the position brought MSVC's crash back. With equality constant instead, every such loop's condition
is false before its first step, and the step itself stays as it is for every width.

### the-ordering-invariant

Every ordering in the library satisfies

```cpp
std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end()) == (a <=> b)
```

which is stated on the **reading**, never on the storage: `bit_block_container` has no `begin()`/`end()`,
and `boost::dynamic_bitset` has no public iterators, so it cannot be written against a backend at all.

`boost::dynamic_bitset::operator<` is that algorithm over reverse iterators, a third order: *a*'s highest bit
paired with *b*'s highest, second with second, then a tie broken on width. It is *not* magnitude ordering,
though it coincides with one at equal width: `"1"` and `"01"` are both the number 1, and boost orders them
strictly, the shorter first. No reading here orders that way.

Where a specialization offers nothing faster, the default is that standard algorithm over the reading's own
iterators — so the default cannot disagree with the specification, and only an optimization can.

### the-hashing-invariant

Every value the library compares, it hashes, and

```cpp
a == b  implies  hash(a) == hash(b)
```

under every reading. The cross-cutting protocols -- equality, ordering, formatting, ranges, hashing
-- follow the reading: the standard's own coverage, `std::bitset`, `std::vector<bool>` and
`std::string` hashing while `std::array`, `std::set` and `std::pair` do not, is history rather than design.

The engine is Boost.Hash2, and there are two hashers over it, which differ in **what** they hash.

**`xstd::hasher<H>` hashes the value**, as its standard model does. It is xstd-misc's, and runs Hash2's
`hash_append`, which finds each adaptor's `tag_invoke` hook. A hook appends exactly the message Hash2 1.92
builds for the model, so `xstd::hasher<H>()(x) == xstd::hasher<H>()(model(x))` under every algorithm and every
flavor, and a bit container hashes inside a user's Hash2-hashed aggregate as the standard container would:

| xstd type | model | message |
| :--- | :--- | :--- |
| `bit_array<N>` | `std::array<bool, N>` | N bytes `0x00` or `0x01`, no size; one `'\x00'` when N = 0, Hash2 having every append update |
| `bit_vector`, `bit_small_vector` | `std::vector<bool>` | N bytes, then `hash_append_size(N)`; an unspecialized `boost::container::vector<bool>` writes the same |
| `bit_bounded_vector<N>` | `std::inplace_vector<bool, N>` | n bytes, then `hash_append_size(n)`, as `boost::container::static_vector<bool, N>` |
| every set and set view | `std::set<Key, Compare>` | each key in iteration order, descending under `std::greater`, then `hash_append_size(count)` |

A sequence's bools are not read through its proxies. `detail/hash.hpp` expands each 64-bit word of the bits
into 64 bytes, through `detail/intrinsic.hpp`: one `_mm512_movm_epi8` where AVX-512BW is enabled, an
`xstd::bit_expand` per byte where xstd-ints finds PDEP fast, which is BMI2 on x86-64 short of the AMD processors that
microcode it and MSVC's `/arch:AVX2`, and otherwise, and in a constant expression, a 256-entry table of each byte's
eight bools. Eight words go to an `update`, and the last stops at the width: no padding bit is ever a bool. Chunking
does not change a Hash2 digest, so the expansion's grain is free. A set walks its keys, costing what `std::set`
costs, and appends each as `Key`, so its message follows the platform's `std::size_t` as the model's does.
`bit_block_container`, which is no reading, keeps its own hook over its blocks.

**`xstd::bit_hasher<H>` hashes the bits**, the representation rather than the model's value. Its message is
the canonical byte string of the bits: ceil(N / 8) bytes, bit `i` at bit `i % 8` of byte `i / 8`, unused high
bits zero. A static width appends nothing after it, as `std::array` appends no size: the suffix would carry
nothing, and equality never crosses types. A run-time width appends `hash_append_size` of the width in bits
for a sequence. A set of run-time width cannot: equal sets need not share a width
([width-is-capacity](#width-is-capacity)), so its string stops after the byte holding its last key, and its
suffix is that byte count. An empty static width appends the one `'\x00'`, by Hash2's rule. Wherever
`block_copies_as_bytes` holds -- a little-endian target, a built-in block, and value bits that fill its
object -- the first ceil(N / 8) bytes of the storage **are** that string, whatever the block type,
since the unused bits are clear; this is the identity `bit_convert` rests on. There the string goes in as one
`update` over the storage, from `std::uint8_t` blocks to `unsigned __int128` ones. Elsewhere -- a big-endian
target, a block with padding bits, a class-type 128-bit block whose layout no rule proves, a constant
expression -- the same bytes are assembled by shifts, up to 256 at a time. Equal values thus hash equal across
storages **and** block types, which is why `bit_hasher` declares `is_transparent`; `xstd::hasher` cannot, being
generic, where `42` and `std::int64_t{42}` hash differently.

The same message is public as `xstd::bit_hash_append(h, f, x)`, for a user's own hook. It also takes a
contiguous range of one static-width container -- a built-in array, a `std::array` or a `std::span`, nested
arrays flattened -- and appends each element's string in turn, then the count where the range's type fixes
none. Where each element's object is its string, an owner whose blocks hold no byte past ceil(N / 8), as a
`bit_fixed_set<64>` in `std::uint64_t` blocks, and a nested `std::array` measures as wide as its elements, the
whole range is one `update`. A board game's position holding six `bit_fixed_set<64>` planes a side hashes each
side in one `update` of 48 bytes, and `planes[2][6]` in one of 96. A `bit_fixed_set<50>`, eight bytes for a
seven-byte string, goes in a plane at a time, to the same digest. The containers are not made trivially
equality comparable to get this: that would have Hash2 hash their objects and bypass the model hooks.

`std::hash` is `bit_hasher<boost::hash2::xxhash_64>`, which is also `bit_hasher`'s default, and it is a
**default rather than a fact**. It is xxHash at every length, with no switch to FNV-1a for a short string,
because FNV-1a does not avalanche: each byte is xored into the low bits and multiplied up, so the high bits of
the last byte never reach the low bits of the result, and the low bits are what a hash table indexes by. Two
one-word sets differing only in their top key would land in the same bucket. xxHash mixes every input bit into
every output bit at the end, at a cost a short string barely notices. It is spelled `boost::hash2::xxhash_64`
on every target, `std::size_t` taking Hash2's fold of its result where it is narrower.

`std::hash` cannot take another algorithm: its `operator()` has one argument and its type no parameter for
one. The door to another algorithm is `bit_hasher<H>` for the bits and `xstd::hasher<H>` for the value, each
seeded where `H` takes a seed. `std::hash<T>` is `bit_hasher` at the default, so the two agree digit for digit.
At any other algorithm the digits differ and the invariant still holds, since it rests on what is appended and
not on who hashes it.

So a caller chooses as follows. The default, through `std::hash`, is for keys the program makes: it is
unseeded, and fast at every length. A caller who knows the keys better names another Hash2 algorithm.
**Keys an adversary can choose** -- read from
the network, a file or a user -- need SipHash, seeded per container, which is Hash2's own advice: an unseeded
algorithm lets an attacker who knows it build a table's worth of colliding keys in advance, and SipHash is a
keyed function built to make that infeasible without the seed, at a speed a hash table can afford.

```cpp
#include <xstd/bits/bit_hasher.hpp> // xstd::bit_hasher
#include <xstd/bits/bit_set.hpp>
#include <boost/hash2/siphash.hpp>  // siphash_64
#include <cstdint>
#include <unordered_set>

using hasher = xstd::bit_hasher<boost::hash2::siphash_64>;

auto const seed = std::uint64_t{/* drawn at random, per container */};
auto table      = std::unordered_set<xstd::bit_set, hasher>(0, hasher(seed));
```

Who hashes follows [views-follow-their-precedent](#views-follow-their-precedent): the set adaptor owned or
viewed, as `std::string_view` hashes; the sequence adaptor as an owner alone, as `std::span` does not, so its
hook is constrained on ownership and a `bit_span` hashes no more than it compares. Both range adaptors tell ContainerHash they are not ranges: Hash2 chooses between its
range overload and a hook by `enable_if`, a range with a hook is ambiguous, and the range overload could not
hash the proxy the iterators return anyway. The harness checks the invariant beside `==`, wherever a
`std::hash` exists, and each model's message under three algorithms and both fixed byte orders.

## Coverage

### per-instantiation-slots

gcovr counts branch slots **per template instantiation and never merges them**. A loop that a one-block
instantiation can never enter is a branch never taken, whatever every other instantiation does.

So a degenerate width does not get an unreachable loop; it gets **different code**, via `if constexpr` on
`static_num_blocks == 1` and `== 2` in `bit_block_container` and in the set queries that read it, and on `static_block_count` and `extent == 0`
in the storage's own scans. The block count is derived from the width rather than declared for exactly this
reason: nothing new has to be supplied to know it, the block layout being fixed already.

The same gate is why a cursor lives inside the walk it belongs to rather than beside the block index: at one
block the walk is discarded, and a cursor declared outside would never be written — which
`misc-const-correctness` reads, correctly, as a variable that should have been `const`.

**A test that only makes a call fail instantiates the path it never runs**, and the same rule scores it. Three
cases here each handed `append_range` a `views::iota | views::transform` whose sum saturates, to check that the
reserve refuses it. Each wrote its own lambda, so each was a distinct closure type and a distinct instantiation
of the packing tier — three of them, and in all three the reserve threw before the loop was entered, so every
branch in that tier was one no test took: five short of the gate, on the two lines that shape the loop. One
functor at namespace scope makes the three views one type, and a length the sequence can hold runs that
instantiation's loop for real. The general form: when a range or a storage enters a template only through the
path that refuses it, it arrives with the whole body's branches and none of them taken.

### an-assert-begins-its-line

The gate excludes an assert from both counts, by `--exclude-lines-by-pattern` and
`--exclude-branches-by-pattern` over `^\s*assert\(` -- anchored at the start of a line. An assert written
anywhere else on its line is not excluded, and `assert(cond)` expands to a conditional whose false arm no test
takes, so it is an uncovered branch per instantiation ([per-instantiation-slots](#per-instantiation-slots)).

`front()` and `back()` on the sequence reading are four lines apiece for this, and no other reason. Their two
preconditions, each written on the line of the call it guards, cost seven branches across the deducing-this
instantiations and take the gate from 100% to 99.2%, where the same asserts in `erase` and `index_of` -- each on a
line of its own -- cost nothing at all. The rule is worth stating because the failure is silent at the point of
writing: the code is right, the assert fires as intended, and only the gate says otherwise.

### while-not-for

Two descending walks are shaped as a `while` rather than a `for` with a fall-through, deliberately. The
precondition guarantees a set bit at or below, so a loop that could run out would carry an exit branch no
test can take — exactly what the coverage gate catches, and what `find_if` hid by keeping that branch inside
the standard library. Written as a `while`, both branches of the condition are exercised: empty blocks are
skipped, a non-empty one ends it.

### one-function-per-tier

Each tier of each scan is its own function. `readability-function-cognitive-complexity` counts `if constexpr`
like any other branch, and the guards that satisfy the coverage gate put both scans over the threshold when
the tier choice shares the body. The tier was the seam anyway.

## Platform workarounds

### libcxx-views-take

`all_but_last_are_ones` uses an iterator pair rather than `views::take`.

libc++ 18 — Xcode 16.4 on the matrix — writes the return type of `views::take`'s `iota_view` fast path as
`decltype(iota_view(*begin(rng), ...))`, so forming the adaptor's `operator|` over a range of 128-bit blocks
instantiates an `iota_view` over that block type, however unlike an `iota_view` a `std::vector` is. That
trips libc++'s *bigger than integer-like type* `static_assert`, which is a hard error rather than a
substitution failure.

`views::drop`, `views::reverse`, `views::transform` and `views::zip` are all clear. Only `take` carries it,
and only until Xcode 16.4 leaves the matrix.

### msvc-completes-the-accessor

`sequence_adaptor::operator<=>` is a **non-template friend**, so its constraint is checked where it is
declared: inside the class, while the class is still incomplete. Written over the accessor —

```c++
requires is_owner and requires { x.storage().sequence_lexicographical_compare_three_way(y.storage()); }
```

— the member access on `x.storage()` makes MSVC deduce `storage()`'s return type there, which means
instantiating a body that reads `self.m_bits`, which needs the enclosing class complete. MSVC 18 answers
**C2027, *use of undefined type `sequence_adaptor<...>`*** at each of `storage()`'s three returns, then
C7683 and C3313 as the cascade, and C2102 wherever `&self.storage()` appears. GCC and Clang defer all of it.

The constraint is about the **storage**, not about the accessor, so it says so:

```c++
requires is_owner and requires (bits_type const& b) { sequence_three_way(b, b); }
```

`bits_type` is complete and already in hand, and for an owner it is exactly what `storage()` returns, so
satisfaction is unchanged on every compiler. The call is spelled as a free function because the orderings
are free functions over the storage rather than its members ([the-ordering-primitive](#the-ordering-primitive)),
each declaring its return type so that checking the constraint instantiates no body. What makes MSVC complete
the class is naming a **member** of what the accessor returns, which is the shape the rule below is about; whether
a free-function call through the accessor would trip it too is unmeasured, the constraint not going through the
accessor at all.

Two neighbours look like the same shape and are not. The bulk operators take `this auto&& self`, so they are
templates and their constraints wait for a call, by which time the class is complete. And
`set_adaptor::operator==`, also a non-template friend, constrains on `x.storage() == y.storage()` — an
operator expression, which MSVC does not resolve eagerly the way it does a member access. The rule to carry
forward is narrow: **a non-template friend's constraint may not name a member of whatever the accessor
returns.**

### gcc-array-bounds

The shift operators assert `n_blocks <= last_block()`, which is implied by the `is_valid(n)` above it. It is
stated again because GCC does not carry the range through `xstd::div`'s aggregate return: without it,
`-Warray-bounds` reports the memmove inside `shift_right` at `-O1` and `-O2` whenever asserts are live.

No CMake build type is that combination — Debug is `-O0`, and the optimized ones carry `NDEBUG` — but
`-O2 -g` is one command away, and the bound is worth saying in any case.

### gcc-nttp-implicit-zeros

GCC 15 mangles a class-type template argument without the zeros it was never given
([GCC PR 121231](https://gcc.gnu.org/PR121231), [#396](https://github.com/rhalbersma/xstd-bits/issues/396)). A value
whose leading or interior blocks were zero-initialized and never written takes the name of another value: `{0, top}`
is mangled as `{top}`, which reads back as `{top, 0}`, so two different values share one specialization, within one
translation unit as well as across them. GCC 16 mangles correctly, as an ABI change (`-fabi-version=21`) that is not
backported, and by default also emits the GCC 15 name as a compatibility alias; that alias is the correct name of the
other value, so across translation units the linker can merge `f<{0, top}>` with `f<{top, 0}>` unless
`-fabi-compat-version=0` drops it. Clang 22 is correct in both.

The library meets it through its structural owners ([structural-at-aligned-widths](#structural-at-aligned-widths)).
A default-constructed owner filled by `insert`, `set` or `operator[]` leaves a block implicit whenever its keys skip
one, so a value built by insertion can name a different specialization from the equal value built from its blocks.
Over 64-bit blocks that takes a key at or past position 64 with an empty block below it; over 32-bit blocks the keys
`{0, 127}` of a 128-bit owner already leave two. `EqualValuesNameTheSameSpecialization` in
`test/src/bits/structural.cpp` builds `{0, last}` both ways and asserts one specialization, and on GCC before 16 it
skips that assertion for an owner of more than two blocks, whose interior blocks are the implicit ones. GCC mangles
correctly once every element has been stored, so writing every block explicitly during constant evaluation, under
`if consteval`, would keep every value the owners produce clear of the defect; that is untried, and needs a check
that it changes nothing at run time or under MSVC.

### uint128-printing

Never `BOOST_CHECK_EQUAL` on a block-typed value. It prints its operands on failure, and no standard library
defines `operator<<` for `__int128`, so a `graded_extents` sweep that includes `xstd::uint128` fails to
compile on libc++ where it happens to compile on libstdc++. `BOOST_CHECK` compares without printing.

## Naming

### test-not-subscript

`bit_block_container::test` reads a bit, and cannot be written through. `std::bitset`'s `operator[]` returns an
assignable proxy and `test` returns `bool`, so a subscript to a bit would promise an assignment that does not
compile. `bit_block_container`'s `operator[]` is the subscript of the blocks it wraps -- a `std::array`, a
`std::vector`, a `std::span` -- and yields a block, read through a `const` storage and written through a mutable
one, after which the caller restores the unused bits with `erase_unused`. Only the sequence containers lift the
subscript to a bit: `bit_array`'s `operator[]` is `std::array<bool, N>`'s, and returns the proxy.

The writable proxy belongs to the containers above, which is also where the checked reading lives —
`std::bitset::test` throws where this asserts, a difference the containers state as a guard rather than
one this name should try to carry.

### qualifier-prefixes

Contract qualifiers are prefixes, not suffixes — `inclusive_find_next`, `exclusive_find_prev`,
`growing_insert` — so that the contract reads before the operation and the cheaper form cannot be called by
mistake. `growing_insert` is the one that earns the rule twice over: it sits beside an `insert` that asserts
`is_valid(n)`, and a silent substitution of one for the other is a precondition quietly dropped.

## Test harness

### scratch-objects

`checker` holds two scratch objects, reset before each mutating check, rather than taking a fresh copy per
check. Same-width assignment reuses the storage, so this is two allocations for the whole checker instead of
one per operation — of which `positions()` alone did five per bit.

It is faster, and it leaves the optimizer one object to follow rather than thousands of construct/destroy
pairs. **Three GCC versions have each mis-analysed the copying shape in a different way** —
`-Wfree-nonheap-object` on 15, `-Wrestrict` on 17-SVN — and moving the copies around only moved the
diagnostic. The whole-container mutators get one method apiece for the same reason: at `-O3` GCC 15 inlines
a combined sweep into a single function and then reports `-Wfree-nonheap-object` on the vector copies, which
ASan, LSan and UBSan all say is not there.

The scratch objects are held **by reference**, owned by `check_ops`: by value they would be the only members
narrower than a pointer, and `-Wpadded` reports the tail padding that leaves.

### counted-not-asserted

Disagreements are counted rather than asserted per bit. A passing assertion per bit says no more than one,
and a failing one drowns the log. Two helpers turn the comparison into a count, so the cast
`readability-implicit-bool-conversion` asks for has two sites rather than thirty.

### seven-patterns

The sweep uses seven patterns, chosen so every pair lands on both sides of each branch: all clear and all set
for the whole-container shortcuts, the strided ones for partial blocks, and the endpoint ones for the first
and last block specifically.

The last pattern fills every block but the last, which is the only way to reach the right operand of `all()`'s
short circuit with a `false`: every other pattern either fails a block below the last, or fills the last one
too. It compares block indices rather than a precomputed bit count, so that a single-block width — for which
it selects nothing — does not fold into an unsigned comparison against zero.

### width-zero-comparisons

An unsigned comparison against zero is a diagnostic on both major compilers: `-Wtype-limits` on GCC, and C4296
on MSVC, which is what `is_valid()` carries its own note about. At a width of zero, `i < n` with `n` folding to
a constant is exactly that, so the sweeps use `views::iota` instead.

The same folding is why lambdas capture by reference throughout rather than naming what they use: under a
static width the compiler folds those to constants, and naming something usable in a constant expression is
what `-Wunused-lambda-capture` reports.

### the-audit

The clauses under `test/src/spec/` cite the paragraphs they check, which is a numerator. `doc/audit/` holds the
denominator: one file per audited specification, pinned to the draft commit it was read from, with a row for
every numbered paragraph of every leaf clause, saying which element it states and what the tests do with it:
`answered` where a test cites it, `declined` or `forced` where a note says why not or what instead,
`no-requirement` for text with nothing to test, and `gap` for what is testable and not yet cited.

`python3 tools/audit.py` reads every table there and every citation under `test/`, lists and ranges expanded,
and fails on an answered row nothing cites, a cited paragraph with no row, an unexplained `declined`, `forced`
or `no-requirement`, one that a test cites all the same, and any `gap` unless given `--allow-gaps`. It prints
the numerator against the denominator per clause, and the Audit workflow runs it on every pull request. When the
draft moves, re-pin the file, renumber the rows, and let the checker name the citations left behind.

## Views and containers

### the-two-adaptors

Two class templates carry the two readings: `set_adaptor` and `sequence_adaptor`. Each is
written against `bit_block_container` and against nothing else, so one adaptor serves
`bit_block_container` over `std::array`, `std::vector` and the bounded blocks alike, at both widths and
in both ownerships ([one-storage](#one-storage)). Each takes the parameters its own reading needs and no
others: `set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>` and `sequence_adaptor<Bits, Store, W, Derived>`, the set reading
never windowed and the only one with a key.

The two are internal: the owners and the three views are the public surface, and no public template
argument list, deduction guide or specialization spells an adaptor. Every public name is a class deriving from
one of them, passing itself as an argument so that the adaptor names it back: `basic_bit_fixed_set<Key, B, N, KeyMapping, Compare>` derives from
`set_adaptor<bit_block_container<std::array<B, K>, N>, storage::owned, basic_bit_fixed_set<Key, B, N, KeyMapping, Compare>, Key, KeyMapping, Compare>`. The short layer stays
an alias fixing the block, and for a set the key: `bit_fixed_set<N>` and `bit_array<N>` are those at `std::size_t`.
Deriving is what keeps a value-returning operation -- `& | ^ -`, `operator~`, the shifts, `xstd::bit_convert` --
handing back the container the caller named rather than the vehicle under it
([the-views-are-the-adaptors](#the-views-are-the-adaptors)).

### the-grid

**The containers are a grid, and nothing else.** Two readings over three storages, every cell occupied, every
one an owner and none windowed. Each is a class deriving from its reading's detail adaptor
over its storage, `basic_bit_array<B, N>` from the sequence adaptor over
`bit_block_container<std::array<B, num_blocks_v<B, N>>, N>`, and each writes out the constructors of the
standard container it packs ([the-container-adaptor](#the-container-adaptor)). A reading over a storage is
reached through its owner's name and no other, so the grid is a fact about the public surface rather than a
construction imposed on it.

**The readings are flat: neither nests inside the other.** A set of positions is no sequence of bools and a
sequence no set, and the relation views need is a predicate on the owner rather than a base class
([the-readings-do-not-mix](#the-readings-do-not-mix)).

**The grid is not written out as a template.** One `basic_bits<R, C, Block, N, Alloc>` taking a reading tag and a
container tag and naming one cell, with the `basic_` names aliases over it and the readings aliases over a general
`adaptor<R, ...>` keyed on the same tags, would pay twice for the uniformity. An alias template cannot declare a
deduction guide, so no owner could have one while the views, being classes, do. And a partial specialization
cannot be befriended: a friend declaration naming the template is a redeclaration, which cannot add a
`requires`-clause to single one reading out, so the only form left would make every reading a friend of every
other. Without the tags the friendships follow the constraint -- `set_adaptor` befriends `set_adaptor` and
`sequence_adaptor` befriends `sequence_adaptor` -- and a diagnostic says `basic_bit_array<unsigned long, 100>`
rather than a five-argument cell name.

What a tag would key on, each container names outright: its base clause says
`std::array<Block, num_blocks_v<Block, N>>` where a tag map would take `array_container_tag` to a storage's name.
**Each storage is answered where it is defined**, one header apiece, so a container includes only the vehicle it
uses: measured, a central switchboard has `bit_array.hpp` pulling `<vector>`, which is exactly what the vehicle
split prevents.

**The axis is closed to users and open to the library.** What a storage has to satisfy is the exposition-only
`owned_bit_blocks` ([owned-bit-storage](#owned-bit-storage)), which is a claim about blocks and says nothing about
where they live. `ext/boost.hpp` joins on that and nothing else: two containers over
`boost::container::small_vector`, added without a line of the core changing. No owner takes a storage argument, so a
new column is a new owner here, not a parameter a user sets.

### the-container-adaptor

The grid has two container adaptors, one per reading -- `set_adaptor` and `sequence_adaptor`
in `xstd::bits::detail`, in the manner of `std::stack<T, Container>` and `std::flat_set<Key, Compare, KeyContainer>`
-- each over a `bit_block_container` open to any `owned_bit_blocks` without a new class for it. They are
internal. The public surface is the owners and the three views, and an adaptor over a storage no owner
names is reached from inside the library and its tests alone.

**The owners are classes deriving from the adaptors, not aliases over them.** Each passes itself as its adaptor's
last argument over the `bit_block_container` its base clause spells, which `owned_bits_t` names; a second
class over the same storage is a distinct type all the same. Each writes out its constructors in the order and
notation of the clause it packs: [set.cons] for the three sets, less the allocator forms where the storage has
none; [vector.bool.pspc] for `basic_bit_vector` and `basic_bit_small_vector`; [inplace.vector.overview] for
`basic_bit_bounded_vector`; and for `basic_bit_array`, `std::array` being an aggregate, the default and initializer-list
constructors its aggregate initialization stands for. What the clause lacks and the adaptor offers -- the
`from_blocks` doors -- follows the standard ones, each marked as not in the clause. The copy and move constructors, both assignments and the
destructor are left to the compiler, which declares each as the adaptor underneath answers it, `noexcept` and
triviality included: a static width stays trivially copyable, and a polymorphic allocator's move assignment stays
potentially throwing, with nothing spelled on the owner to fall out of step. Each declares its own deduction guides and hidden `swap`, and
specializes `std::hash`, Boost's `is_range` and `is_tuple_like` where it is a range, and for `basic_bit_array`
`std::tuple_size` and `std::tuple_element`: a partial specialization on the adaptor never matches a class derived
from it.

The empty allocator base is keyed on its adaptor, which is what keeps two classes over one storage apart. It
carries the defaulted `operator==` the defaulted comparisons above it need; shared between two adaptors over one
storage, both would convert to the one empty base and compare equal whatever their bits. Keyed, the comparison
does not compile, as `std::set<int>` against `std::flat_set<int>` does not.

**Classes deduce without CTAD for alias templates.** Owners declared as aliases over the adaptor would reach every
guide, the `from_blocks` tag's among them, through the alias ([P1814](https://wg21.link/P1814)). A probe on every
leg (#228, stand-in types, one `static_assert` per case) finds three rules for that arrangement:

1. **A guide spells the storage as the alias does.** The return type computes the block count from the width,
   `adaptor<std::array<B, blocks_for<B>(W)>, W>`, and never names it, `adaptor<std::array<B, K>, …>`: `K` meets
   `blocks_for<B>(N)`, which [temp.deduct.type]/5.3 makes a non-deduced context.
2. **The one-block guide carries a defaulted count.** MSVC 17 (14.44) drops a guide whose only template parameter
   is the block type when it builds the alias's guides (`C2641`, `C2976`); `template<std::unsigned_integral B,
   std::size_t K = 1>`, spelled as the array guide is, is kept.
3. **Nothing deduces through a name that pins the block type.** `bit_array<N> = basic_bit_array<std::size_t, N>`
   as an alias of an alias deduces on neither MSVC 17 nor MSVC 18 (`C2641`), so a pinned name that something
   deduces through has to be written over the adaptor directly.

MSVC also refuses alias deduction through an alias `basic_bit_fixed_set` itself, with `C7602`, and that is what
makes the owners classes. A class takes its guides as its own and needs none of the three rules: the guides name
`basic_bit_array<Block, N>` rather than computing a block count, and `bit_fixed_set<N>` is an alias of
`basic_bit_fixed_set<std::size_t, std::size_t, N>`, one alias over a class, the depth every compiler deduces
through. The one-block guides need no defaulted count either. The views are a separate question from the owners
([the-views-are-the-adaptors](#the-views-are-the-adaptors)).

### owning-is-ours

Owning is ours and viewing is interop. An owning adaptor sits over a storage of this library,
`bit_block_container` over `std::array` or `std::vector`, and nothing else is supported or tested: the owning
`set_adaptor` and `sequence_adaptor` take their ordering from the storage's own
three-way members, with no synthesized fallback for a storage without them. A view reads blocks, and a foreign
bitset reaches either reading by conversion.

The split is what a foreign owner would cost against what it would buy. Every owner-only member -- construction
through the storage's constructors, growth, the saturating shifts, the checked element access, the ordering
-- would need an entry in each foreign trait and harness arms over each foreign storage, and the blit and the
range insertions of #80's 7d more again. It would buy one object where a copy gives the
same reading: `xstd::bit_convert<xstd::bit_fixed_set<N>>(my_std_bitset)` is the set reading of a `std::bitset`
someone else owns, and `bit_fixed_set` holds it in storage of ours. What is given up is the wrapped-equals-raw proof
that the wrapper adds and drops nothing; the views keep it for reads, writes, iteration, searches and hashing,
and construction, growth and the ownership protocol are proven over our storages alone.

A view over a foreign type would need a trait as its contract: `extent`, `size`, `at`, `count`,
`unchecked_assign`, `insert`, `fill`, and the searches and block reads where the type has them, with a `checked_*`
family and checked shifts that only a wrapper asks, and an `operator-=` and `operator-` on `std::bitset` that
`[namespace.std]` does not let a program declare. None of it exists: `bit_set_view` has `-=`, `std::bitset` has no
set difference of its own, and with no foreign storage to adapt, a trait would have nothing to abstract over
([one-storage](#one-storage)).

### ownership-is-not-an-axis

Owning versus viewing is storage lifetime, not a third axis of the model, and it collapses to one template
parameter: `storage::owned` stores `Bits`, `storage::borrowed` stores `Bits*` or, over span-backed storage, a
copy of it ([borrowed-blocks](#borrowed-blocks)). Always present and only its
type changes, so a plain `conditional_t` rather than `conditional_data_member_t`. One accessor, via deducing
`this`, gives deep const to the owner — `self.m_bits` propagates `self`'s const — and shallow const to the
view — `*self.m_bits` does not — for free.

Every mutator is then gated on the storage and nothing else: `requires requires { self.storage().op(…) }`
reads "the storage lets *this handle* write". A const owner's accessor hands back a `Bits const&`, which has no
`assign` to reach; a const view's hands back `Bits&`, which is what a view is for; a view over `Bits const`
hands back `Bits const&` again. Const and ownership are the same question, asked once, and answered by the type
the accessor returns. The exceptions are the constructors and, once storage grows, the growth members, which need an explicit
`requires (owns(Store))`: the requires-expression tests what the storage can do, not what this handle may do to
it, and a view over a `bit_block_container` over `std::vector` must not be able to resize what it does not own.

### views-follow-their-precedent

`bit_set_view` follows `std::string_view`: a value that happens not to own its bytes, so it has `==` and
`<=>`, and its ordering is exactly `std::set`'s. As [string.view.comparison] asks, it compares with whatever
converts to it: its owner, and a view of the same bits that are not const, since both views convert from
mutable to const bits as `std::span` does. Those are the only mixed comparisons; two owners compare only within
their own type, as `std::array` and `std::vector` do. `bit_span` follows `std::span`, which P1085 stripped of both
because "same referent" and "same contents" are both defensible readings of a handle. So `set_adaptor`
compares and hashes whatever it owns or views, and `sequence_adaptor` compares and hashes only as an owner. The non-member copies
— `~`, `&`, `|`, `^`, `-`, `<<`, `>>` — are the owner's alone in both readings: a copied view would write
through to what it views.

A view's empty base is `xstd::empty_base_type<>`, which carries **nothing** — no `==`, no `<=>` — so
`bit_span`'s incomparability is simply absent, with nothing to delete. That is the whole reason xstd-misc has
two empty types rather than one. `empty_member_type` keeps a defaulted `<=>` so an enclosing class can default
its comparisons over the member, and that is safe there: a member's associated classes are not the enclosing
class's, so the hidden friend is invisible to it. A base's ARE the derived class's, so the same defaulted
comparison would be found by ADL for every `bit_span` and would answer *equal* for any two of them, having only
the empty base to compare.

The alternative is an `incomparable_base` deriving from the one empty type and deleting the two comparisons it
brings. That works — the deleted pair takes `sequence_adaptor` exactly where the base's takes the empty type by a
derived-to-base conversion, so it wins overload resolution by [over.ics.rank]/4.4 and the answer is ill-formed
rather than wrong — but it needs BOTH deletions, and neither implies the other: `<=>` rewrites the four
relationals and never `==`; `!=` rewrites from `==` and never from `<=>`; and a defaulted `<=>` implicitly declares
a defaulted `==` beside it ([class.compare.default]). Deleting `==` alone leaves `<`, `>`, `<=` and `>=`; deleting
`<=>` alone leaves `==` and `!=`. Nor can it be written where it belongs: MSVC rejects a trailing requires clause
on a *deleted* friend (C7599) where it accepts one on a defaulted friend, so the pair cannot be constrained on
`not is_owner` in the adaptor and has to sit in a base of its own. Two empty types upstream need none of that.

What stays is the assertion. `not equality_comparable` and `not three_way_comparable` in the harness are
joined by the seven spellings, and the owner is asserted to answer all seven beside the view answering none, so
they are the view's shape rather than a concept nobody satisfies. Those seven go through named concepts rather
than bare `requires (View a, View b) { a == b; }`, because an absent or deleted overload is not assertable the
direct way: it is a **hard error** where the requires-expression names a concrete type — measured identically
on GCC and Clang — and a soft `false` only when the check reaches the type through a template parameter.

Both referring adaptors opt into `std::ranges::enable_view` and `enable_borrowed_range`, the two
specializations [range.view] and [range.range] invite for a program-defined type. The first makes
`bit_set_view(x) | views::take_while(…)` take the view as it is rather than wrapping it in an `owning_view`;
the second says what `span` says, that the iterators point at the storage and outlive the handle that made
them, which is what lets a caller return `bit_set_view(c).begin()` from a temporary.

### the-comparison-is-a-hidden-friend

Both adaptors spell `operator==` as a hidden friend. A defaulted comparison need not be a member:
[class.compare.default]/1 admits a non-static member **or a friend**, and `sequence_adaptor` defaults a
*constrained* friend. The set reading writes both of its overloads out instead, for a reason that is about coverage rather than about comparison and is the
last subsection here.

The choice is unobservable since C++20, which is what lets it rest on other grounds. Where a converting
constructor is not `explicit`, as
`std::bitset`'s from `unsigned long long` is not, a mixed comparison compiles; which spellings compile depends on
which parameter can take that conversion. Measured over a class template with an implicit width-carrying
constructor, per standard:

| | C++17 `x == u` | C++17 `u == x` | C++20 `x == u` | C++20 `u == x` |
| --- | --- | --- | --- | --- |
| member, as `std::bitset` has it | yes | **no** | yes | yes |
| hidden friend | yes | yes | yes | yes |
| namespace-scope function template | **no** | **no** | **no** | **no** |

The member's implicit object argument never converts, which is the C++17 asymmetry. P1185's **reversed
candidates** ended it: `u == x` now also considers `x == u`, reaching the member with the integer as the
argument that converts, so member and friend became indistinguishable. Defaulted `==` arrived in the same
standard, so there is no era in which the friend could be defaulted but behaved differently.

The third row is why "non-member" is not the useful distinction. A hidden friend of a class template is a
**non-template function**, one per instantiation, so both parameters take conversions normally. A
namespace-scope template deduces instead, and deduction never considers user-defined conversions -- it
rejects `x == u` *and* `u == x`, in every standard, the reversed candidate failing to deduce just as the
first one did. The hidden friend only ever accepts more. A namespace-scope template would also need the
forward-declaration and `friend operator==<>` dance for private access: three declarations in dependency order
for one operator.

#### the set reading writes its equality out

`set_adaptor` carries two `operator==` and neither is defaulted. The general one answers through `set_equal`,
because width is capacity for this reading and two sets holding the same positions are equal at any two widths
([width-is-capacity](#width-is-capacity)). The other, the static-width owner's, could be `= default` and is
`return x.storage() == y.storage();` instead.

The two spellings mean the same thing and do not cost the same. A defaulted comparison compares base classes
before members, and an owner here derives from `bits::detail::allocator_base_type` -- an empty class carrying
the storage's `allocator_type` where there is one and nothing at all where there is not. Its own defaulted
`operator==` can only answer true, so the defaulted form emits a call and a branch that no input can send the
other way. Probed with equal operands, with operands differing in the first block, with operands differing in
the second, and with an object against itself: that edge stayed at zero every time.

Which is a curiosity until a gate counts it. `gcovr` is told to drop branches on a line matching
`= default;`, which is there to excuse exactly this synthesized branch -- and the pattern matches a LINE, not
a declaration. `sequence_adaptor`'s comparison and `bit_block_container`'s each fit
on the one line that carries `= default;`, so the exclusion finds them. `set_adaptor`'s static-width overload,
defaulted, wraps over three, its trailing *requires-clause* being longer than everything else about it, and gcov
anchors a function's branches at the first line of its declaration -- where there is no `= default;` to match.
One construct, three sites, and the only one the gate sees is the one whose constraint is too long to fit on a
line.

So the two siblings carry the same unreachable branch, excused by their formatting rather than by being built
differently. Writing the member comparison out removes it instead of excusing it and needs no exclusion to pass,
and it is what the comment above it claims: a static owner's equality IS its one member's. Reformatting it onto
the pattern would work as well and teach nothing; writing all three out would spend two defaulted comparisons to
buy symmetry.

### the-views-are-the-adaptors

`bit_set_view<Blocks, N>` derives from `set_adaptor<Bits, storage::borrowed>` and `bit_span<Blocks, N>` from
`sequence_adaptor<Bits, storage::borrowed, window::all>`, where `Bits` is the storage the blocks map to
([the-views-are-named-by-their-blocks](#the-views-are-named-by-their-blocks)), each passing itself as the adaptor's last argument so that
the adaptor hands back the view; `bit_subspan` is the same shape with the window argument set. They are the
referring adaptors under the names of [the-public-names](#the-public-names), one header each beside the owners,
and not a second implementation of either reading. Views with iterators, proxies and customization points of
their own — a `set_find`, a `sequence_find`, a `block_access` and a `bit_extent` — would be a storage trait under
another name, and with the adaptors reading the storage alone there is nothing left for one to do.

**The views are classes, not alias templates.** An alias *is* the adaptor, so every opt-in the adaptor carries
would apply to the name for free, and a storage the library does not wrap would be diagnosed where the alias is
written. Both claims are correct, and a class pays for both rather than collecting them (below), because an alias
costs something neither buys back: the name itself.

**Deduction through an alias is what MSVC 17 cannot be relied on for.** It is class template argument deduction
for alias templates ([P1814](https://wg21.link/P1814)), which Clang has had since 19 and GCC since 10. On this
shape -- one pinned non-type argument plus a defaulted, constrained argument depending on the first -- MSVC 17
(VS 2022) has answered `C2976: 'xstd::bit_set_view': too few template arguments` alongside `C2641: cannot deduce
template arguments`, 151 times over both views in `msvc` and `msvc_analyze`, Debug and Release, where MSVC 18
(VS 2026) deduces through the same aliases. Microsoft's conformance table lists `P1814R0 CTAD for alias
templates` as **VS 2019 16.7**, gated only by a flag this project clears at `/std:c++23`; the table describes the
feature and not this shape of it, and a diagnostic quoted from the compiler is the better evidence. A probe (#237,
asked in #235) that puts the two view guides back on the detail adaptors and deduces through aliases of that
shape deduces every case on the MSVC 17 the stable rung resolves to, as on MSVC 18, the Preview and clang-cl, so
the 151 diagnostics belong to an older toolchain and the classes are a choice rather than a compiler's limit. The
probe tests the alias shapes, not today's headers. #235 weighed aliasing the views and took them to blocks instead
([borrowed-blocks](#borrowed-blocks)), the guides for which are the classes' own.

**The stable MSVC rung costs two workarounds**, both for MSVC 17 alone. It does not implement
[P0849R8](https://wg21.link/P0849R8) — `C3878` and `C2760` at `f(auto(pos))` — so the functor's argument is
copied through a three-line `decay_copy` helper in each of two adaptors rather than through `auto(x)`
([the-functor-takes-a-value](#the-functor-takes-a-value)). And it rejects `std::integral T = X::key_type` on the
default argument of a constrained type-parameter, `C2061: syntax error: identifier 'integral'`, where GCC, clang,
clang-cl and Apple clang take it, so `test/include/test/set/primitives.hpp` keeps that one `typename` under a
`NOLINT`; it is the one site in the tree where [P0634R3](https://wg21.link/P0634R3) permits the omission, the
remaining `typename X::value_type` sites being template arguments and functional casts, which P0634R3 does not
reach. [msvc.yml](.github/workflows/msvc.yml) and [msvc-analyze.yml](.github/workflows/msvc-analyze.yml) carry
`stable,qualification,development`.

That is the shape of the trade rather than a verdict on it. A rung costs whatever the vendor's oldest
front end cannot do, paid in workarounds that the other five toolchains carry without needing them; what it
buys is that the front end keeps building this library. Here the price is a helper and a keyword.

**The constraint is the MSVC compiler, not the VS 2022 platform.** `clang_cl` passes on all three rungs, 2022
included, because clang-cl is Clang and Clang has had P1814 since 19, so VS 2022's runner, STL and platform are
covered whatever the MSVC 17 front end asks.

**What deriving costs is restatement, and every entry is paid.** A derived class needs its own constructors,
its own two deduction guides, and its own `enable_view`, `enable_borrowed_range`, `std::hash` and `is_range`
specializations, because **a derived class is not its base to a partial specialization** — the base's opt-ins
say nothing about the derived name. Each view carries all of them, the trait specializations delegating to the
adaptor's through `adaptor_type` rather than restating a body. The constructors are inherited, with
`using base_type::base_type;` and `using base_type::operator=;`; the second matters, because a derived class's
implicit copy-assignment hides every base `operator=`, the `initializer_list` overload included. Inheriting
constructors inherits the primary's guides as well (P2582, which GCC implements), which would tie with restated
guides — so each view's pair is spelled on its own name, where the primary's cannot be what a consumer deduces.

**The diagnostics were the other entry, and restating the constraint settles them.** An alias is transparent,
so a storage the library does not wrap is diagnosed where the alias is *written*: `void f(my_set<int>)` is an
error at that declaration, quoting the unsatisfied constraint. A derived class that leaves the constraint to
its base is not: naming one in a declaration does not require a complete type, so the same line **compiles**,
the base is never instantiated, and the diagnosis waits for whoever first completes the type. It then arrives
twice, because a dependent base is named twice and cannot be named once -- in the base-specifier, and again in
the using-declaration that inherits the constructors. Measured on `set_adaptor` over a storage the library does
not wrap: 13 lines and one error through the alias, 22 lines and two errors through such a derived class.

Restating the constraint on the derived class's own parameter recovers all of that, and then some: it fails at
the declaration, once, in **fewer** lines than the alias, having no indirection to explain. Which is why all
three views spell their constraint, `xstd::bit_blocks Blocks`, on their own template
parameter instead of leaving it to the base. A four-line reduction holding a constrained class template, an
alias of it, and both derived forms reproduces the shape exactly, GCC and Clang agreeing to the line, so it is
the language rather than a diagnostic quirk. The failure mode is the derived class that skips the restatement,
and it would bite the views and only the views: the owners choose their own storage, so a storage the
library does not wrap cannot arise through them at all, the one parameter a user supplies being the `Block`,
constrained at every layer. A view takes the storage -- that is what a view is for
([owning-is-ours](#owning-is-ours)) -- so a view is exactly the place where a constraint left to the base would
go undiagnosed until use.

**The printed name is what the classes collect.** An alias is not what a compiler prints, so over aliases a
diagnostic about `bit_array<100>` would name `sequence_adaptor<bit_block_container<array<unsigned long, 2>, 100>,
...>`, a spelling the user did not write and cannot write back. `std::string` makes that trade and the world lives
with `basic_string<char, char_traits<char>, allocator<char>>` -- but `std::string` aliases a *class*, where two
layers of aliases would let the printed name fall through to the adaptor rather than stop at
`basic_bit_array`. Each owner is a class, so the name stops there: a diagnostic about `bit_array<100>` says
`basic_bit_array<unsigned long, 100>`, a name the user can write back.

One constraint sits in two places rather than one. The guide for a plain storage is viable for an owner too,
an owner being nothing but a storage under a wrapper, and would tie with the owner guide -- so it is
constrained to non-owners, as [an-owner-reads-as-its-storage](#an-owner-reads-as-its-storage) describes. It is
written on `set_adaptor`'s and `sequence_adaptor`'s own guides, and again on each view's restated pair, which
is where a consumer's deduction lands.

The sequence view pays the `span` half of [views-follow-their-precedent](#views-follow-their-precedent) by
being the adaptor: it has no `==` or `<=>`, and the harness checks the sequence reading through the iterators
instead.

**A trait the views specialize names the windows.** `sequence_adaptor` returns a window from `first`, `last` and
`subspan`, and the public name of that window is spelled in blocks the adaptor never sees: `bit_subspan<Blocks,
Extent, N>`, where the adaptor holds only the storage. So the adaptor asks `window_of<Derived, Bits, E>`, whose
primary answers the adaptor windowed, and `detail/views.hpp` specializes it for both views from declarations of
their templates, which carry no default arguments since those are given once, on each class. `bit_span.hpp` and
`bit_subspan.hpp` include it before defining their class -- the specialization has to exist when the base's member
declarations are instantiated, which is while the view is still incomplete. `blit_source` needs no entry
there: whatever derives from its `adaptor_type`, a view or a sequence owner, blits as that adaptor does. That is the hook back into the header above, and it
spares the adaptor a forward declaration of `bit_subspan`.

### the-views-are-named-by-their-blocks

Every public view is named by **blocks**, never by the storage built over them: an unsigned block, or a sized
contiguous range of unsigned blocks that subscripts, which is the public concept `xstd::bit_blocks`. A packed container
*has* bit storage and is not bit storage itself: `bit_set` is a range of keys, `bit_array` of proxies, and
`std::bitset` has one too without exposing it. `bit_block_container`
is a storage device and appears in no public template argument list. `detail/blocks.hpp` holds the map:

| blocks | a view stores |
|---|---|
| `std::uint64_t` | `bit_block_container<std::span<B, 1>>`, by value |
| `std::array<B, K>`, `std::vector<B>`, ... | a pointer to an owner's storage over the same blocks |
| `std::span<B, E>` | `bit_block_container<W>`, by value |

The views line up as `std::span<T, N>` does -- `bit_set_view<Blocks, N>` -- and `N` defaults to the width the blocks name, so a view needs it only over an owner
of a narrower width, such as `bit_span<std::array<std::size_t, 1>, 20>` over a `bit_array<20>`. A default in a
public argument list is public too, so that width is `xstd::bit_blocks_extent_v<Blocks>`, beside the concept: a
block's digits, all the bits of `std::array<B, K>`, `B[K]` or `std::span<B, E>`, and `std::dynamic_extent` for everything
else, a span of dynamic extent included. The storage keeps its own sentinel for that span's width, and it never
reaches an argument list.
`bit_subspan<Blocks, Extent, N>` takes the extent second, so `bit_subspan<Blocks, 4>` is a four-bit window as
`std::span<T, 4>` is four elements.

The guides answer in blocks. A block deduces itself and a const block itself const: `bit_set_view(board)` is
`bit_set_view<std::uint64_t>`. A range that is not one of ours has no storage object for a view to point at, so
it deduces the span that lends it: `bit_span(blocks)` over a `std::vector<std::uint32_t>` is
`bit_span<std::span<std::uint32_t>>`. An owner is named by its block and width instead, and deduces both from
the blocks it is handed: `basic_bit_fixed_set(from_blocks, block)` is a `basic_bit_fixed_set` at the block's
digits, stored as a block array of one.

`bit_blocks` is stricter than what itsy-bitsy's `bit_view<R>` adapts, which reaches its blocks through `R`'s
iterators and so takes a `std::deque` of blocks. Contiguity is what the rest leans on: a view borrows the blocks as a
`std::span`, a conversion between two things that have bit storage is one `memcpy` of the blocks, and the block-wise
algorithms run on raw blocks. So `bit_blocks<std::deque<std::uint32_t>>` is false, and asserted false.

### windows

`bit_subspan<Blocks, Extent, N>` derives from `sequence_adaptor<Bits, storage::borrowed, window::sub>`: the referring adaptor
windowed, and the name `first`, `last` and `subspan` hand back on a `bit_span` or on another window. It is a
class for the reason the other two views are ([the-views-are-the-adaptors](#the-views-are-the-adaptors)), and
being one is what lets the adaptor return it by name rather than by respelling its own parameters. It stores what
`std::span` stores, a pointer and a size, with the pointer's role split over a pointer and a position
because bits are not addressable: the sequence iterator's two fields and a count, 24 bytes beside the whole
view's 8. The offset is applied once, in a private `offset()` that answers zero for every other shape, so
`begin()`, `operator[]`, `at`, `front` and `back` are written the same way for all three.

The three members are [span.sub]'s, on a view and never on an owner, since `std::array` and `std::vector`
have no subviews either; they assert their preconditions rather than throw, and `std::dynamic_extent` is the
to-the-end sentinel. A window is a view in `std::ranges`' sense and borrowed like `span`, and like `span` it
neither compares nor hashes ([views-follow-their-precedent](#views-follow-their-precedent)).

A window's end blocks are shared with what lies outside it, so every write to one is masked blocks:
`xstd::bit_fill` over a window is `bit_block_container::set(pos, len, value)`, a block at a time through
`block_at`, and the copies, `bit_sort` and `bit_partition` write a window the same way
([algorithms-not-members](#algorithms-not-members)). Like `std::span`, no view combines: elementwise logic is
the valarray reading's, over an owner ([the-valarray-reading](#the-valarray-reading)), and no sequence has shifts,
window or whole ([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)).

**A window over a const storage writes nothing, and the predicate has to be asked of the right type to say so.**
`bits_type` is `Bits` with the const stripped, because a view over a const owner names `Bits const` and the
typedef wants the storage's own name. Asking whether *that* takes a write is always yes, so a writer is
constrained through the storage expression itself, which carries the const: `xstd::bit_fill` drops out of overload
resolution on a const window rather than failing inside its body. `set_adaptor`'s const view does the same: every
one of its mutators constrains through the storage expression.

### static-windows

`first<Count>()`, `last<Count>()` and `subspan<Offset, Count>()` are [span.sub]'s compile-time three, and they
hand back `bit_subspan<Blocks, Count, N>`: a window whose width is its type's, as `std::span<T, N>`'s is. The extent is
the adaptor's fifth parameter, defaulted to `std::dynamic_extent` so that a four-argument spelling means a dynamic
extent, and it is the adaptor's and not the derived class's because the derived class is incomplete when the base
lays out its members -- which is where the count has to disappear. It does: the count is a
`conditional_data_member_t`, so a static window is the pointer and the position, 16 bytes beside the dynamic
window's 24, and `size()` is the extent.

Where the type already says a request cannot fit, it is ill-formed rather than asserted: `first<21>()` over a
view of a twenty-bit array, or `subspan<10, 11>()` over the same, fail their constraints, as [span.sub] makes
them fail. Over a run-time width the count is checked at run time, which is where the storage's width is known.
The extent of `subspan<Offset>()` is what is left of a static one, and dynamic otherwise. Between extents the rule
is `std::span`'s: a static window converts to a dynamic one implicitly, and a dynamic one to a static one only
explicitly, asserting the sizes agree.

What is not here is a static *offset*. A window's start is a bit position, so unlike `std::span`'s it cannot be
folded into the pointer unless it is a whole number of blocks, and putting it in the type is a step past anything
the standard has; the position stays a member.

### constant-sizes

A size-like member whose value the type fixes is a static `std::integral_constant`, and `empty` a
`std::bool_constant`, after Jonathan Müller's
[static `constexpr std::integral_constant` idiom](https://www.think-cell.com/en/career/devblog/the-new-static-constexpr-std-integral_constant-idiom):

```cpp
static constexpr std::integral_constant<std::size_t, N> size = {};
static constexpr std::bool_constant<N == 0> empty = {};
```

| type | constants | functions |
|---|---|---|
| `bit_array<N>` | `size`, `empty`, `max_size` | -- |
| `bit_subspan` at a static extent | `size`, `empty`, `max_size` | -- |
| `bit_bounded_vector<N>` | `capacity`, `max_size` | `size`, `empty` |
| `bit_fixed_set<N>`, `bit_bounded_set<N>` | `max_size` | `size`, `empty`, which count the keys |

The run-time widths keep functions, and so do the whole views, `bit_span` and `bit_set_view`, whose width is
whatever they borrow. A set has no `capacity` at any width ([growth](#growth)).

One declaration gives three spellings. `A::size` is a value whose type carries `N`, for metaprogramming by type;
`A::size()` and `a.size()` call `integral_constant`'s `operator()`, which is `constexpr` and `noexcept` and returns
`std::size_t`. So every call `std::array`, `std::span` or `std::inplace_vector` allows compiles with the same result
type and `noexcept`, and the synopsis checklists hold as written. The one observable difference is `&A::size`, which
`[namespace.std]` does not let a program take of a standard library member in the first place. Through a parameter
`A const& a`, `std::remove_cvref_t<decltype(a)>::size` is a constant expression on every compiler; `a.size` is one
only under P2280, as `a.size()` on a `std::array` is.

The adaptors declare none of these members. A static data member cannot carry a `requires`-clause, and a member
function `size()` declared in the adaptor hides a base's `size` even where its own constraints fail, so the members
come from a base between the adaptor and its storage, chosen per column: `fixed_sizes`, `bounded_sizes` or
`run_time_sizes` under the sequence reading, `fixed_max_size` or `run_time_max_size` under the set reading. The
adaptor's using-declarations make them visible to its own unqualified calls, and a call `size()` there resolves
through `operator()` as it would through a function. `capacity` is not among them, a fixed column having none, so the
bounded column's own members name it as `members_type::capacity()`.

Two consequences follow from the members being constants rather than functions:

- `auto n = A::size;` deduces `std::integral_constant<std::size_t, N>`, not `std::size_t`. It converts implicitly,
  so this rarely matters, but `auto n = a.size();` is the spelling that gives `size_type`.
- `a.size` without parentheses compiles, and names the constant.

### a-set-needs-no-width

A set owner could hold its blocks at `blocks_extent`, and the question is worth answering because the reading lets
it: nothing a set reading says depends on its width. `max_size()` answers the storage's capacity, not the width;
`set_equal` and both orderings read a missing block as empty; the hash appends positions. A `bit_set` whose width
were always its blocks' would answer every question the same and hold one member less, eight bytes beside a
vector's twenty-four, and no tail for any member to mask.

What it would cost is a growth path of its own. `growing_insert` resizes a stored width by the position asked
for; at `blocks_extent` it would resize the blocks and have no width to move, and every storage member gated on
`has_stored_size` -- the moves, `replace`, `extract`, the allocator constructors -- would need a third arm for an
owner whose width is not a member. The sequence reading over the same storage does need the width, so
the saving would be the set reading's alone, over a storage the two readings otherwise share. It is not done;
the note is here so that the question does not have to be asked again from the start.

### the-range-members

`append_range` has two tiers. Where the source is a sequence adaptor of any shape, owner, view or window, over
a storage whose blocks are the destination's block type, the source's bits are read as blocks at the source's own
alignment through `block_at` and appended a block at a time through `bit_block_container::append(block)`,
which splits each block over the destination's own alignment; then the width is trimmed to the count, `resize`
clearing whatever the last block carried past it. Everything else, a `std::vector<bool>`, an `iota` under a
`transform`, a sequence over another block type, is packed a block at a time, which is boost's private
`bit_appender`, and trimmed the same way. A source inside the very storage being appended to is safe: the blit
reads positions below the old width alone, and no append writes one.

Under a static capacity `[inplace.vector.modifiers]/3` asks more of `append_range`: past the capacity, no
effects. A sized source gets that for free, the width it needs being reserved, and refused, before a block is
written. A source with no size finds the capacity only by overrunning it, so the bounded owner remembers its
width, and on any exception resizes back to it before rethrowing -- the resize clearing the tail as every
shrink does. `assign_range` has no such remark in `[sequence.reqmts]`, so its clear-and-append keeps only the
basic guarantee, and an overflowing source leaves what it appended.

`insert_range`, `insert` in its four shapes and `emplace` shift wherever the count is known before the first
element is read: a value, a count of them, a sized or forward range, or another sequence's blocks. Room is made
first, and that is the one step that can throw: a heap grows geometrically, as `push_back`'s does, so
`insert(end(), b)` is amortized constant as `std::vector<bool>`'s is, and an inline capacity too small for the
growth throws `bad_alloc` before anything has moved, as `[inplace.vector.modifiers]`/3 asks. The tail then moves up
a block at a time, descending, each write landing above every read still to come, and the middle is written into
the gap; a source that throws part way has its gap erased again before the exception leaves. A single-pass source
with no size, and a window of the sequence itself, which would move with its own tail, rebuild instead: the head
through `first(pos)`, the middle, the tail through `subspan(pos)`, into a fresh sequence that is then moved in, at
the blit's tier, with the strong exception guarantee that `std::vector::insert_range` gives on reallocation.
`subspan` pays for itself here, the head and the tail being exactly windows. Both `erase`s shift: the tail moves
down a block at a time, ascending, each write landing below every read still to come, and the width shrinks behind
it. A rebuild allocates, and `[vector.modifiers]`/5 lets an erase throw only what the element's copy, move or
assignment throws, which for a `bool` is nothing.

`[vector.bool]`'s `flip()` and its static `swap(reference, reference)` are left out. Negating every element is a
bulk operation, the valarray reading's `!` ([the-valarray-reading](#the-valarray-reading)), negating one is
`v[i] = not v[i]`, and two proxies swap through their own hidden friends, which the static member, deprecated by
[P3612R1](https://wg21.link/P3612R1), only renamed.

### what-a-sequence-may-add

[the-sequence-contract](#the-sequence-contract) is a floor: *`bit_vector` answers every line of `[vector.bool]`'s
synopsis*. A floor with nothing above it admits whatever the storage happens to offer -- the shifts among them,
declared, never argued for, and spelled backwards ([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)) -- so the
sequence adaptor needs a ceiling as well.

The ceiling: **the sequence adaptor adds an operation only where the sequence reading is what asks for it,
and the spelling is the one that reading already uses.** Two questions, and a candidate answers both or it
does not cross:

- *Does a sequence of `bool` want this?* The elementwise operators are `std::valarray<bool>`'s, the standard's
  one model for a bulk logical operation over bools, and no sequence container of the standard has them: they
  answer no here and are the valarray reading's instead ([the-valarray-reading](#the-valarray-reading)). So is
  `[vector.bool]`'s `flip()`, the one bulk operation `std::vector<bool>` has. A **difference** answers no
  anywhere: no standard sequence of bools spells `a and not b`, and `valarray<bool>`'s `operator-=` is
  arithmetic. A **shift** answers no twice over.
- *Is this the name that reading gives it?* `<<=` fails here even where the operation is wanted, because a
  sequence already spells moving elements `std::shift_left` and `std::shift_right`, in the opposite
  direction.

Being free is not an argument. Every operation the storage already has is free to forward, which is what
makes the forwarding tempting and the ceiling necessary: `-=` and the shifts were each one line over a
storage member that exists regardless, and the cost of a wrong one is not compile time but a caller who
reads `v <<= 1` as `std::shift_left`. What the storage provides is the **union** of the two readings'
demands ([the-two-adaptors](#the-two-adaptors)); each adaptor exposes its own reading's share, and the
shares are not the same set.

The four ways a reading answers an operation are all visible in the current surface:

| | |
|---|---|
| one name, a different thing per reading | `size()` -- the sequence's element count, the set's **cardinality** |
| one reading alone | `cshift` and `apply`, the valarray's, after `[valarray.members]` |
| one reading spelling it otherwise | the set's cardinality `size()` is `xstd::bit_count` over the sequence, an algorithm rather than a member |
| one reading declining it | `-=` and the shifts on the set, not the sequence |

### no-shifts-on-a-sequence

`std::vector<bool>` has no shifts, and neither has any sequence here. The storage keeps `<<=` and `>>=`
because the set reading asks for them, and means by them what `[bitset.members]`'s truncating shift means:
`<<=` translates, keeping the keys that land below `max_size()`, and `>>=` empties past the width. The sequence reading is the one with nothing to add. Worse,
it already spells moving elements, and spells it the other way round: `operator<<=` is implemented with
`std::shift_right` and `operator>>=` with `std::shift_left`, because a sequence's low index is its front
where a bit string's low bit is its right. Exposing the operators would have `v <<= 1` mean the opposite of
the algorithm whose name the sequence reading already owns. So the shifts stay off the sequence reading. A row
of bools reaches them through the valarray reading, which declares them `std::bitset`'s, in the bit string's
direction, beside `valarray`'s own `shift` ([the-valarray-reading](#the-valarray-reading)).

### algorithms-not-members

A container here has its standard counterpart's members, and no member that `std::ranges` spells as an algorithm.
Every block-wise operation the standard expresses as an algorithm over the elements lives in
`<xstd/bits/bit_algorithm/bit_*.hpp>`, one header each, exported together by `<xstd/bits/bit_algorithm.hpp>` and by
`<xstd/bits.hpp>`, as a free function named `xstd::bit_` and the algorithm's name. Each takes the arguments the
`std::ranges` algorithm takes and returns what it returns: `xstd::bit_count` a `range_difference_t`, signed as
`std::ranges::count`'s is, `xstd::bit_mismatch` a `std::ranges::mismatch_result`, `xstd::bit_rotate` a
`borrowed_subrange_t`, `xstd::bit_copy` a `std::ranges::copy_result` and `xstd::bit_for_each` a
`std::ranges::for_each_result`. The `std::ranges` call over the same container is the specification, and the tests
hold each algorithm to it, on `std::vector<bool>` and `std::set<std::size_t>` models of every column's inputs;
what the prefix adds is the blocks.

**Which algorithms get a `bit_` form.** One whose work on packed bits is a pass over blocks rather than a walk of
proxies: it beats a bit at a time because a block answers, moves or writes `digits` positions in one instruction,
or, on the set reading, because a loop can hold a block an iterator cannot. That is the whole list below, and it is
a closed list in the sense that an algorithm not on it is one of two kinds. **It reduces to one of these on
bools**: every `false` is like every other, so `stable_sort`, `nth_element`, `partial_sort` and `ranges::sort` with
`std::ranges::less` are all `bit_sort`, `stable_partition` is `bit_partition`, `count(v, false)` is
`ssize(v) - bit_count(v)`, `find_if(v, std::identity())` is `bit_find(v, true)`, `fill_n` and `copy_n` are
`bit_fill` and `bit_copy` over a `subspan`, `lexicographical_compare` is `bit_lexicographical_compare_three_way(a, b)
< 0`, and `upper_bound`, `lower_bound` and `equal_range` over a sorted sequence are `bit_partition_point` and
`bit_find`. **Or packing gains it nothing**: an algorithm taking a predicate or a function of an element the
algorithm cannot see as a block -- `count_if`, `find_if` with a lambda, `remove_if`, `unique`, `search` with a
comparator -- calls the caller's code once per element whatever the storage, and `std::ranges` already runs it
correctly over these ranges. `bit_transform` is the exception that proves the rule: a function of one or two
`bool`s has a truth table of two or four entries, so the algorithm asks it once per entry and applies the table as
one block expression. A rejected predicate would have no such table, since a predicate's type does not say what it
tests; the set reading's predicate algorithms are the one family still open, the walk of
[the-set-for-each](#the-set-for-each) carrying a predicate as cheaply as it carries a functor.

| algorithm | `std::ranges` equivalent | reading | a window |
| :--- | :--- | :--- | :--- |
| `bit_count(r)` | `count(r, true)` | sequence | yes |
| `bit_all_of(r)` | `all_of(r, std::identity())` | sequence | yes |
| `bit_any_of(r)` | `any_of(r, std::identity())` | sequence | yes |
| `bit_none_of(r)` | `none_of(r, std::identity())` | sequence | yes |
| `bit_find(r, value)` | `find(r, value)` | sequence | yes |
| `bit_find_last(r, value)` | `find_last(r, value)` | sequence | yes |
| `bit_mismatch(r1, r2)` | `mismatch(r1, r2)` | sequence | yes, a `bool` at a time |
| `bit_is_sorted(r)` | `is_sorted(r)` | sequence | yes |
| `bit_is_partitioned(r)` | `is_partitioned(r, std::identity())` | sequence | yes |
| `bit_partition_point(r)` | `partition_point(r, std::identity())` | sequence | yes |
| `bit_copy(r, out)` | `copy(r, out)` | sequence | yes, either side |
| `bit_copy_backward(r, out)` | `copy_backward(r, out)` | sequence | yes, either side |
| `bit_move(r, out)` | `move(r, out)` | sequence | yes, either side |
| `bit_swap_ranges(r1, r2)` | `swap_ranges(r1, r2)` | sequence | yes |
| `bit_reverse_copy(r, out)` | `reverse_copy(r, out)` | sequence | yes, either side |
| `bit_rotate_copy(r, middle, out)` | `rotate_copy(r, middle, out)` | sequence | yes, either side |
| `bit_transform(r, out, f)` | `transform(r, out, f)` | sequence | yes, either side |
| `bit_transform(r1, r2, out, f)` | `transform(r1, r2, out, f)` | sequence | yes, every side |
| `bit_fill(r, value)` | `fill(r, value)` | sequence | yes |
| `bit_sort(r)` | `sort(r)` | sequence | yes |
| `bit_partition(r)` | `partition(r, std::identity())` | sequence | yes |
| `bit_reverse(r)` | `reverse(r)` | sequence | no |
| `bit_rotate(r, middle)` | `rotate(r, middle)` | sequence | no |
| `bit_shift_left(r, n)` | `shift_left(r, n)` | sequence | no |
| `bit_shift_right(r, n)` | `shift_right(r, n)` | sequence | no |
| `bit_for_each(r, f)` | `for_each(r, f)`, and over `std::views::reverse(r)` | both | yes |
| `bit_equal(r1, r2)` | `equal(r1, r2)` | both | yes |
| `bit_lexicographical_compare_three_way(r1, r2)` | `lexicographical_compare_three_way` over both ranges | both | yes |
| `bit_includes(s1, s2)` | `includes(s1, s2)` | set | — |
| `bit_intersects(s1, s2)` | `set_intersection(s1, s2, out)`, writing a key | set | — |
| `bit_disjoint(s1, s2)` | `set_intersection(s1, s2, out)`, writing nothing | set | — |
| `bit_set_intersection(s1, s2)` | range-v3's `views::set_intersection(s1, s2)`, as a set | set | — |
| `bit_set_union(s1, s2)` | range-v3's `views::set_union(s1, s2)`, as a set | set | — |
| `bit_set_difference(s1, s2)` | range-v3's `views::set_difference(s1, s2)`, as a set | set | — |
| `bit_set_symmetric_difference(s1, s2)` | range-v3's `views::set_symmetric_difference(s1, s2)`, as a set | set | — |
| `bit_advance(it, n)`, `bit_advance(it, n, bound)` | `advance(it, n)`, `advance(it, n, bound)` | set | — |
| `bit_next(it, n)`, `bit_next(it, n, bound)` | `next(it, n)`, `next(it, n, bound)` | set | — |
| `bit_prev(it, n)`, `bit_prev(it, n, bound)` | `prev(it, n)`, `prev(it, n, bound)` | set | — |
| `bit_distance(first, last)` | `distance(first, last)` | set | — |

**Free, because the counterpart has no such member.** `std::vector<bool>` has no `count`, `std::array` no `rotate`
and `std::set` no `is_subset_of`, and a member would answer a question the counterpart's synopsis does not ask, in
a vocabulary of its own choosing: a nullary `count()`, an `all()` taking a `bool`, a `rotate(n)` reducing its
argument modulo the size. An algorithm takes the shape `std::ranges` already fixed, so a reader who knows
`std::ranges::count(v, true)` knows `xstd::bit_count(v)`. The standard libraries make the same split for
`vector<bool>`: libc++ answers `std::count`, `std::find` and `std::fill` a word at a time through overloads on its
`__bit_iterator`, not through members of the container. A user may not add overloads to `std::ranges`' algorithms,
so these take the `bit_` prefix instead, and a call says which of the two it is.

**One door to the blocks.** An algorithm reaches the storage through `storage_access`, the one friend both adaptors
and their iterators declare, as libc++'s overloads reach `vector<bool>`'s words through the `__bit_iterator` they are
written against. Nothing else about the container is opened: each algorithm asks the storage member that answers it,
`count`, `all`, `any`, `none`, `first_difference`, `reverse`, `rotate`, `fill`, the ranged `set` and `count`, or the
shifts, over a whole sequence's blocks, and reads and writes a window's blocks through both sides of `block_at`
([the-blit](#the-blit), [windows](#windows)); `bit_includes` and `bit_intersects` read the blocks themselves. An
iterator opens the same door: `storage_access` hands an algorithm the storage an iterator steps over and the
position it stands at, which is how `bit_copy` writes through an output iterator and `bit_advance` moves one.

**Whole sequences and windows.** `bit_reverse`, `bit_rotate` and the two shifts permute a whole storage in place,
and take an owner or a whole view, never a window or a `const` sequence: a window shares its end blocks with
positions outside it, and its permutation would be a masked walk of its own. Everything else takes a window,
reading its blocks at its offset through `block_at` and writing them masked to its own positions. `bit_mismatch`
takes two operands of one type, and compares two whole sequences a block at a time, at two run-time sizes as well,
stopping at the shorter's end as `std::ranges::mismatch` does; two windows it compares a `bool` at a time through
`std::ranges::mismatch` itself.

**The writes, at any two offsets.** `bit_copy`, `bit_copy_backward`, `bit_move`, `bit_reverse_copy`,
`bit_rotate_copy` and `bit_transform` read their source and write their destination a block of the destination at
a time: the source's bits at any offset, assembled from its own blocks where the two block types differ, land
through the write side of `block_at` under the mask of how many remain. So source and destination may sit at
different offsets in different storages over different block types, and nothing walks a proxy. Overlap follows the
standard's rules: `bit_copy` writes lowest first, so a destination below its source is safe, as
`std::ranges::copy` allows; `bit_copy_backward` writes highest first, for a destination above; and `bit_transform`
may write over its first input in place. `bit_swap_ranges` trades a block of the narrower of two block types at a
time. `bit_fill`, `bit_sort` and `bit_partition` are the storage's ranged `set`: sorting bools is counting the ones
and filling the zeros before them, partitioning them is the same fill the other way round, and both are stable,
every `false` being like every other. `bit_shift_left(v, n)` is `std::ranges::shift_left`, element `i` taking
element `i + n`, which is the storage's `>>=` because a sequence's front is a bit string's low end
([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)); `bit_shift_right` is the storage's `<<=`. Each leaves the
positions it vacates `false`, which `std::ranges` leaves valid but unspecified, and a shift by the size or more has
no effect, as the standard's has none.

**`bit_transform` asks its function once per entry of a truth table.** A function of one `bool` is one of four --
constant `false`, constant `true`, the identity or negation -- so the unary form calls `f(false)` and `f(true)` once
each, as the valarray's `apply` does ([the-valarray-reading](#the-valarray-reading)), and applies
`(x & T) | (~x & F)` a block at a time, `T` and `F` being blocks of all ones or all zeros by the answers. A function
of two `bool`s is one of sixteen, so the binary form calls `f` on the four pairs once each and applies
`(x & y & T11) | (x & ~y & T10) | (~x & y & T01) | (~x & ~y & T00)`. `std::bit_and<>`, `std::logical_or<>`,
`std::not_equal_to<>` and an arbitrary lambda all take that one path, there being nothing a functor's type says that
its four answers do not. The difference from `std::ranges::transform` is the number of calls, two or four rather
than one per element, which a function with side effects would see; its result over the sequence is the same.

**`bit_for_each` walks either reading** a block at a time and hands its functor each element by value
([the-set-for-each](#the-set-for-each), [the-sequence-for-each](#the-sequence-for-each),
[the-functor-takes-a-value](#the-functor-takes-a-value)); the walk against the order is the overload on
`std::views::reverse` over a range of ours, so the containers' `for_each` and `for_each_reverse` members became one
algorithm with the standard's spelling for both directions.

**`bit_equal` and `bit_lexicographical_compare_three_way` cross block types and storages.** Every owner's `==` and
`<=>` stays a hidden friend over its own type, as `std::array<T, 3>` does not compare with `std::array<T, 4>`;
containers of different types compare through these two, as through `std::ranges::equal` and
`std::lexicographical_compare_three_way`. Two operands of one storage type compare in their own blocks, by the
storage's `first_difference`, the set reading's `set_equal` and the reading's own three-way; any other pair compares
64-bit words assembled from each side's blocks at each side's offset. The set reading's order is `std::set`'s, the
keys in iteration order, lexicographic over ascending keys and the masks as numbers over descending ones. The
operands must be of one reading, a set's of one key, mapping and comparator, and of one width kind: two widths
fixed in the type and equal, or two run-time widths. The width kind is what `bit_hasher` hashes differently
([the-hashing-invariant](#the-hashing-invariant)), so whatever compares equal hashes equal, and
`xstd::bit_equal_to`, the transparent equality over `bit_equal`, pairs with the transparent `bit_hasher` for a
lookup across block types and storages: `std::unordered_set<K, xstd::bit_hasher<>, xstd::bit_equal_to>::find` takes a
view, or the same value over narrower blocks.

**The iterator operations are rank and select.** `std::ranges::advance`, `next`, `prev` and `distance` over a set's
iterators step one key at a time; `bit_advance(it, n)` skips whole blocks by their popcount until the block holding
the target key, then selects within it, counting from the top when it walks down, so it costs `n / digits` blocks
rather than `n` scans. Its bounded form stops at `bound` and answers the steps it did not take, signed as `n` is, as
`std::ranges::advance` does; `bit_next` and `bit_prev` are it over a copy. `bit_distance(first, last)` is a popcount
over the whole blocks between two iterators and the masked ends, so select is `*bit_next(s.begin(), n)` and the rank
of `k` is `bit_distance(s.begin(), s.lower_bound(k))`. The select within one block is xstd-ints' `xstd::bit_select`,
`pdep` and a `countr_zero` where BMI2 is fast and a broadword select elsewhere.

**The second set converts.** `bit_includes`, `bit_intersects` and `bit_disjoint` take `S const& s1, std::type_identity_t<S> const& s2`:
`S` deduces from the first argument alone, and the second converts to it. That puts a flag set's mask on the second
side, `xstd::bit_includes(perms, fs::perms::owner_read)`, and never first ([flag-types](#flag-types)). All three answer
at any two run-time widths ([width-is-capacity](#width-is-capacity)), and every other relation between two sets is
spelled from them ([the-set-queries](#the-set-queries)).

**The set operations return a set, and an operator spells each.** `std::ranges::set_intersection` and its three
siblings write through an output iterator, and that form already works on these sets, element by element. What the
blocks add is the result as a value, the shape range-v3 gives its lazy `views::set_intersection`,
`views::set_union`, `views::set_difference` and `views::set_symmetric_difference`, so `xstd::bit_set_intersection`,
`bit_set_union`, `bit_set_difference` and `bit_set_symmetric_difference` take their names and return the set those
views would hold, a block at a time. The operators `&`, `|`, `-` and `^` are their shorthand, the notation set
theory already uses, and each algorithm returns exactly what its operator does. Only an owner has a result to hold,
so a view, which writes through to blocks it does not own, takes neither; the second argument converts to the
first's type, as for the queries, so `xstd::bit_set_union(perms, fs::perms::owner_read)` adds a flag.

### rotation-and-reversal

`xstd::bit_rotate` and `xstd::bit_reverse` cross where the shifts did not, and the ceiling's two questions say why
([what-a-sequence-may-add](#what-a-sequence-may-add)). A sequence of `bool` wants them: `std::ranges::rotate` and
`std::ranges::reverse` are sequence algorithms, and over packed bits each is a pass over the blocks rather than a
walk of proxy swaps. So each is the algorithm it packs, under its name, with its signature and its result
([algorithms-not-members](#algorithms-not-members)). `xstd::bit_rotate(v, middle)` has the effect of
`std::ranges::rotate(v, middle)`: bit *i* takes bit *(i + n) mod N* for *n = middle - v.begin()*, so a whole turn,
`middle == v.end()`, is no turn, and it returns `{v.begin() + (v.end() - middle), v.end()}`, the range from where
the old first bool now stands. `xstd::bit_reverse(v)` has the effect of `std::ranges::reverse(v)`: bit *i* takes bit
*N - 1 - i*, and it returns `v.end()`.

[P3103R2](https://wg21.link/P3103R2) is the prior art. It gives `std::bitset` a `reverse()`, and a rotation
as the pair `rotl` and `rotr`, after `std::rotl` and `std::rotr` in `<bit>`, which name the direction by bit
significance: left is towards the high bit. That is the right spelling for a bit string, whose low bit is printed on
its right, and the wrong one for a sequence, whose low index is its front. In the sequence reading the paper's
`rotr(n)` is `xstd::bit_rotate(v, v.begin() + n)`, and its `rotl(n)` is `xstd::bit_rotate(v, v.end() - n)`, for `n`
up to the size. One algorithm says both, in the direction `std::rotate` already fixes, and leaves no left or right
for a reader to map onto front and back. It is the mismatch that keeps the shift operators off the sequence reading:
a direction named for a bit string reads backwards on a sequence.

An owner and a whole view take them and a window does not: a window shares its end blocks with
what lies outside it, and its rotation would be a masked walk of its own. The set reading does not take them
either; `std::set` has no counterpart, and a rotation of keys has no meaning there that a shift does not already
give.

The storage does the work, at every width without allocating, so each is `noexcept`. A rotation by
*n = q · digits + r*, reduced modulo the width first so the shifts' `n < size()` precondition never arises, is
`std::ranges::rotate` over the blocks by *q*, then a funnel shift by *r* through `straddled_block`, the first block
wrapping round into the last ([the-funnel-shift](#the-funnel-shift)). Over the blocks' whole width that leaves the
*n* bits that wrapped sitting above a gap as wide as the padding, so where there is padding one more pass moves
them down onto it through both sides of `block_at` ([the-blit](#the-blit)). A reversal is the blocks in reverse
order, each block's bits reversed by log2(digits) masked swaps, and the padding, now at the bottom, rotated out by
the same funnel shift.

libstdc++ before 16 gives a reason of its own not to forward to `std::ranges::rotate`. It holds the
element a closing rotation by one displaces as `auto`, which over a proxy reference is a proxy to the position
about to be overwritten, so at a turn coprime with a width of three or more the bit that should wrap round comes
out as a copy of its neighbour: `std::vector<bool>` and this library's sequences lose it alike, while `std::rotate`
and libc++ do not ([GCC PR 121913](https://gcc.gnu.org/PR121913), fixed in 16 and not backported). `xstd::bit_rotate`
is a pass over the blocks that never asks the standard library to move a proxy, so it is exact on every library, and
the tests build their own rotations by index for the same reason.

### the-valarray-reading

The containers here are principled versions of the standard's: `bit_vector` is what an unspecialized
`std::vector<bool>` would be, `bit_array<N>` `std::array<bool, N>` and `bit_bounded_vector<N>`
`std::inplace_vector<bool, N>`. None of those has a bitwise operator, so none of ours does. What `&=` means on a
row of bools is **elementwise logical**, `a[i] = a[i] and b[i]` at every position, and the one standard type
that spells it is `std::valarray<bool>`, R's logical vector. That is a reading of its own, the sequence read
elementwise, with its own four owners over the sequence's storages:

| | `std::valarray<bool>` packed |
| :--- | :--- |
| static size, `std::bitset<N>` read elementwise | `bit_fixed_valarray<N>` |
| run-time size within a capacity `N` | `bit_bounded_valarray<N>` |
| run-time size inline up to `N`, on the heap past it, in `<xstd/bits/ext/boost.hpp>` | `bit_small_valarray<N>` |
| run-time size on the heap, `boost::dynamic_bitset` read elementwise | `bit_valarray` |

All four are one `valarray_adaptor` over the storage, whose reading tag refines the sequence's: the algorithms of
[algorithms-not-members](#algorithms-not-members) and `xstd::bit_convert` take a valarray as they take a sequence, and
a `bit_span` views one as a `std::span<bool>` views a `std::valarray<bool>`. Its proxy and iterators are the
sequence's, handed out through a whole view of its own storage. On packed bits every elementwise operation is
the set operation's own instruction a block at a time, so the reading costs nothing to serve.

**The interface is `[valarray]`'s at `T = bool`, line by line.** `value_type`, and the proxy `reference` that
stands in for `T&`; the default, count, value-and-count (the value first, as `[valarray.cons]` orders them),
pointer-and-count, list and proxy constructors; assignment from an array, a `bool` filling every element, a list
and each of the four proxies; `operator[]`, a proxy where `valarray` hands out `T&` and a `bool` where it hands out
`const T&`, and the four subscripts of `[valarray.sub]` described below; `swap`, `size`, `sum`, `min`, `max`,
`shift`, `cshift`, `apply` in both shapes and `resize`, which assigns its value to every element as
`[valarray.members]` says; `!`, `&=`, `|=` and `^=` against an array and a scalar; `&`, `|`, `^`, `&&`, `||` and
the six comparisons over two arrays and over an array and a scalar on either side; `swap`, `begin` and `end` as
non-members, which makes it a random-access range, and no member `begin` or `end`. A static size has no count
constructors and no `resize`, and takes `std::array`'s list initialization and a built-in array of bools as
`bit_array` does; its pointer constructor and its constructors from a proxy assert that the count is the type's
own. Each array operator asserts that its operands have one size, the precondition `[valarray]` makes
undefined behaviour. Everything is `constexpr` as far as the storage is: the fixed one entirely, and structural at
whole blocks; the heap one in a constant expression as `std::vector` is; the bounded one where its blocks are a
`std::inplace_vector`. Besides `[valarray]`, the owners take `from_blocks` as every owner does.

**The deviations, each one forced by `bool` or chosen for it:**

- **`~` is logical not.** `valarray<bool>::operator~` applies `~` to each element, which promotes `bool` to `int`
  and converts `~0` and `~1` back to `true`: an array of all `true`, whatever it held. Nobody wants that, and
  `std::bitset`'s users mean complement, so `~` is `!` here.
- **`std::bitset`'s shifts are kept, in its sense.** `[valarray.cassign]`'s `<<=` and `>>=` shift each element's
  value, which on a `bool` is arithmetic again. Here `v <<= n` moves position `i` to `i + n`, zeros filling in, and
  `v >>= n` moves position `i + n` to `i`, each emptying the array at `n >= size()`, with `<<` and `>>` returning
  copies: the operators `std::bitset` and `boost::dynamic_bitset` code is written with. `valarray`'s own `shift(n)`
  stays beside them in its own sense, element `i` taking element `i + n`, so `v << n` is `v.shift(-n)`.
- **Empty reductions are defined.** `[valarray.members]` leaves `sum`, `min` and `max` of an empty array undefined;
  here `sum()` and `max()` are `false`, the identity of or, and `min()` is `true`, the identity of and.
- **No arithmetic.** `+`, `-`, `*`, `/`, `%`, unary `+` and `-`, and their compound forms are left out, as are the
  `<cmath>` functions: on `bool` each either promotes out of the type or is one of the logical operators again.
- **Comparisons answer arrays.** `==`, `!=`, `<`, `<=`, `>` and `>=` return the array of elementwise answers, as
  `valarray`'s do, so the type is not `equality_comparable`, has no `<=>` and no `std::hash`, exactly like
  `std::valarray`. Whole-array equality is `(v == w).min()`, or `xstd::bit_mismatch`.
- **`apply` is asked twice.** A function of one `bool` is one of four, so `apply(func)` calls `func(false)` and
  `func(true)` once each and applies the answer a block at a time: a constant, the identity or a negation.
- **The proxies take scalar compound assignments and no shifts.** `[template.slice.array]` and its siblings
  have `=` from a `T` but compound assignment only from an array; here `&=`, `|=` and `^=` take a `bool` too, as the
  owners' do. Their `<<=` and `>>=` shift each element's value, which on a `bool` is arithmetic, and a position
  shift over positions that need not be contiguous has no counterpart, so a proxy has neither.
- **A fixed size gathers into the bounded array.** A const subscript returns a valarray of the run-time size it
  selects, so `bit_fixed_valarray<N>` returns a `bit_bounded_valarray<N>` over the same block: no heap, and
  `constexpr` as far as the bounded one is. A gather longer than `N`, which a slice of stride zero, a gslice or an
  index array can make by repeating positions, throws `std::bad_alloc` as the bounded array does past its capacity.

**The subscripts.** `[valarray.sub]`'s four each come in two forms: on a const array a gather into a valarray of
its own, and on a non-const one a proxy, `bit_slice_array`, `bit_gslice_array`, `bit_mask_array` or
`bit_indirect_array`, over the same block type. A proxy refers to its array's blocks, as `std::slice_array` refers
to its `std::valarray`: it is made by the subscript alone, copies as a handle to the same elements, and must not
outlive the array, or a mask or an index array it was made from, any more than the standard's must. Through a const
proxy, `=`, `&=`, `|=` and `^=` take an array of the proxy's size or a `bool`, and assigning one proxy to another
assigns elements, never the reference. An array here is any owner of the valarray reading over the same block, or
another proxy, which gathers what it selects before any position is written, so `v[s] = v[t]` reads `v` before it
writes it. A mask is any owner of the valarray reading over the same block and of the same size; a
`std::valarray<bool>` would have to be packed into blocks the proxy then owns, so it is not taken. Every owner is
constructible from each proxy, implicitly as `[valarray.cons]` makes it, and assignable from it, so the compound
operators and the binary ones take a proxy on the right as they do in `std::valarray`.

Each subscript asserts its positions lie inside the array, and the mask its size. `[indirect.array.assign]` leaves a
write that names a position twice undefined, and the indirect proxy asserts against it, as every undefined case of
`[valarray]` here is asserted. A slice of stride zero and a gslice that repeats a position are left defined by the
standard, and write one position at a time, in order, so the last write to a position stands: libc++ does the same,
and libstdc++ leaves a stride of zero's compound writes undone, its loop bounded by the stride times the size.

**The gather and the scatter.** Each subscript runs on one of two paths. The mask path is a block at a time:
a gather takes `xstd::bit_compress(src_k, mask_k)` of each block, P3104's PEXT, and lays it at a running output
offset that grows by `popcount(mask_k)`, through the write side of `block_at` ([the-blit](#the-blit)); a scatter
reads the next `popcount(mask_k)` bits at a running input offset through `block_at` and deposits them with
`xstd::bit_expand`, PDEP, combining them with the block as the operator says: assignment keeps `d & ~m` and adds the
deposit, `&=` is `d & (e | ~m)`, `|=` and `^=` take the deposit as it is. A scalar is a deposit of the whole mask
or of nothing. A mask of all ones needs no permutation and passes the block through. The index path is a position
at a time, through `test` and `assign`.

| subscript | path |
| :--- | :--- |
| `v[std::slice]`, `v[bit_slice]` | a run at stride one, whole blocks copied from a block boundary and the rest through `block_at`; mask, a stride below the block width, one period of `stride / gcd(stride, digits)` masks made once with `xstd::bit_repeat` and loaded, the first and last blocks cut; index, a stride of zero or of a block or more |
| `v[std::gslice]`, `v[bit_gslice]` | mask, where the positions ascend, from the `bit_gslice`'s stored words; index, where they repeat or turn back |
| `v[mask]` | mask, the mask's own blocks |
| `v[std::valarray<std::size_t>]` | index |

**`bit_slice` and `bit_gslice`.** `std::slice` has no `constexpr` constructor, so a slice could never be taken in a
constant expression; `bit_slice` is the same three numbers, `constexpr` and regular, and `v[std::slice]` forwards to
`v[bit_slice]`. `std::gslice` works its positions out once, in its constructor, and keeps them private, so a subscript
by one had to work them out again, every time. `bit_gslice` keeps them: where its shape says they ascend, which is
whether each stride clears the span of every dimension inside it, a check on the lengths and strides alone, it stores
one bit per position as 64-bit words, which every block width reads as its masks, a block of up to a word inside one
word and a wider one across several; where they repeat or turn back, it stores the positions in row-major order. The
index is immutable and shared between copies, so a proxy holds its own copy and never outlives what it reads, which a
`std::gslice_array` referring to a `std::gslice` can. `v[std::gslice]` builds one per subscript.

**Not worse than handwritten.** The benchmark holds each subscript to three columns: `std::valarray<bool>`, this
library, and the same algorithm written by hand over raw 64-bit words with P3104's word functions, each gather
returning a clear array of its own as `v[s]` does. A gather collects its packed bits in a register and stores each output
block once, full: an or into memory per input block chains each update on the last store through store forwarding,
which at stride eight, eight blocks to an output word, held it to two thirds of the handwritten loop. Where xstd-ints runs PEXT and PDEP in hardware, a full mask goes through
them like any other; the shortcut that saves the portable kernels a full permutation mispredicts at high densities.

Handwritten means what a programmer writes at the call site with the information the library has there, and no more:
the block width, the kind of selection, and the stride or the mask words at run time, but not how dense the selection
is. The handwritten column is therefore the library's own algorithm as a raw loop over the same blocks of the same
owners, with no hints, and what it measures is the cost of the abstraction alone. On GCC 15 and Clang 22 that cost is
nil: both columns compile to the same instructions per block. What remains between them is code layout. The gather
stores an output block on a branch that depends on the data, and GCC places that store either in line, so the common
path jumps over it, or ahead of the loop head, so every path takes one branch; which it picks depends on the
surrounding function, not on the loop, and at five per cent density the difference is a cycle a block. Clang lays the
two columns out alike. `-freorder-blocks-algorithm=simple` gives GCC the better layout at every density and is the
user's to set for the whole program; `[[likely]]`, `[[unlikely]]` and `__builtin_expect_with_probability` each help one
density and hurt another, and profile-guided layout fits whichever density it was trained on, so the kernels carry none
of them.

A programmer who knows the density knows something the library does not, and can beat it: by a branch hint that suits
the data, or, for a sparse enough mask, by walking its set bits instead of permuting every block. That is not an
abstraction penalty, any more than a counting sort over small keys is a penalty of `std::sort`. Where such knowledge
should reach the library, it belongs in the interface, as a selection type with a path of its own, the way
`bit_gslice` carries a shape a `std::gslice` cannot; none is added without a workload that needs it.

The two permutations come from xstd-ints, which runs BMI2 where the target has it fast and portable kernels
elsewhere, choosing at compile time. Neither is public here: no `std::ranges` algorithm packs selected elements, so
there is no signature for an `xstd::bit_compress` over this library's ranges to borrow
([algorithms-not-members](#algorithms-not-members)), and the block-range kernels stay in `detail`, under the proxies.

`std::slice`, `std::gslice` and `std::valarray<std::size_t>` have no `constexpr` constructor, so a mask's subscripts
and a `bit_slice`'s are the ones that run in a constant expression; they do, on the fixed and bounded arrays.

**The kernels.** Each operator is one bitwise expression over a block of either side, after which the padding past
`size()` is cleared, so a count over whole blocks stays right. A scalar is a block of all ones or all zeros.

| operator | per block | padding cleared |
| :--- | :--- | :--- |
| `a & b`, `a && b` | `a & b` | no need |
| `a \| b`, `a \|\| b` | `a \| b` | no need |
| `a ^ b`, `a != b` | `a ^ b` | no need |
| `a == b` | `~(a ^ b)` | yes |
| `a < b` | `~a & b` | no need |
| `a > b` | `a & ~b` | no need |
| `a <= b` | `~a \| b` | yes |
| `a >= b` | `a \| ~b` | yes |

The storage applies one kernel to every block through `transform`, against another storage of one width or
against a scalar's block, and clears the padding once at the end, whether the kernel needed it or not.

### the-sequence-contract

`bit_vector` answers every line of `[vector.bool]`'s synopsis and `bit_array<N>` every line of `[array]`'s,
and the test says so declaration by declaration rather than as a claim: each line is asserted in the case of
the clause under `test/src/spec/` that describes it, over that clause's list, where `std::vector<bool>` and
`std::array<bool, N>` come first. A line the model itself fails is a wrong line, so the check is known to be
honest before ours is held to it; the C++23 range members are asserted only where the standard library has
them. Where a packed sequence parts from its model because its reference is a proxy -- no `data()`, random
access where the model is contiguous, a `tuple_element` that converts to `bool` -- the case says so through
`test::proxy_reference` in `test/include/test/reference.hpp`, the named relaxation of `test::real_reference`.

The sweep found what the range members had not needed. The allocator: `allocator_type` through the same empty
base `set_adaptor` has, `get_allocator`, and the allocator-extended constructors,
`[container.alloc.reqmts]`'s copy and move included, which `bit_block_container` gains beneath them.
Each takes `allocator_type const&` as the standard does, not a deduced `Alloc` matched to it: a matched template
refused every allocator that only converts -- a `memory_resource*` for a `polymorphic_allocator`, a rebound
`std::allocator`, a braced `{}` -- and so broke uses-allocator construction, a `std::pmr::vector` of owners failing
to compile at `emplace_back`. Where the storage has no allocator the parameter is an explicit tag, `no_allocator`,
and a `requires` removes the overload, so a static owner still has none. `[vector.erasure]`'s `erase` and `erase_if`
as non-members over the owner's `erase(first, last)`, `std::ranges::remove_if` running unchanged over the
proxies, which move and swap. And `std::array`'s aggregate initialization as an `initializer_list` constructor
on the static owner, the listed values leading and the rest false, a longer list being the error it is on
`std::array`.

`data()` is not offered, because packed bits have no address, and the `pointer` and `const_pointer` typedefs are
`void`: they name what a proxy cannot give, and the checklist leaves them out of `[container.reqmts]`'s typedefs
rather than inventing a pointer to a bit. `std::array`'s tuple interface is offered on `bit_array` as far as a proxy
carries it. `std::tuple_size` is `N`; `std::tuple_element<I>` names the proxy, `reference`, or `const_reference` for
a const array, rather than `bool`; and `get<I>` is four overloads found by ADL, one per value category, each
constrained on `I < N` and returning the proxy by value, `std::get` being the standard's to declare. The two
specializations are written for `basic_bit_array` itself, forwarding to the adaptor's, since a partial
specialization on the adaptor never matches a class derived from it. `std::hash` is asked of the vector alone,
`std::array` having none, while `bit_array` hashes as every owner does
([the-hashing-invariant](#the-hashing-invariant)).

### borrowed-blocks

A view over blocks that belong to no container of ours takes them directly: `bit_set_view(board)` over one
`std::uint64_t`, `bit_span(blocks)` over a `std::array`, a built-in array, a `std::vector` or a `std::span` of unsigned blocks. The
storage underneath is `bits::detail::borrowed_bits<Block, Extent>`, which is `bit_block_container` over a
`std::span<Block, Extent>`, and a deduction guide on each view names it from the argument: one block is
`std::span<Block, 1>`, a range is whatever `std::span(blocks)` deduces, which over a built-in array `Block[K]` is
`std::span<Block, K>`, a static width, as [span.deduct] takes the bound of `T (&)[N]`. One block is taken by
lvalue, a `bit_block_range` by lvalue or as a `borrowed_range`, so a contiguous range that does not subscript, such
as a `std::initializer_list`, is not lent at all, and `bit_set_view(std::uint64_t{5})` and `bit_span(std::vector<std::uint32_t>{})`
do not compile, and a `std::span` temporary does. Const blocks make const storage, and a view over const
storage reads and cannot write, as a view over a const owner cannot.

Every bit of the blocks is a position: bit `n` of block `i` is position `i * digits + n`, which is the order
`from_blocks` already reads an integer in. So the width is the blocks', and there is no tail for the storage to keep
clear -- the invariant a width of our own needs is vacuous here, which is what lets the blocks be anyone's. The
width takes one of two forms, both named by the span's own type through `default_extent_v`, which is
`xstd::bit_blocks_extent_v` wherever that is fixed, `blocks_extent` for a span of dynamic extent, and the
storage's capacity otherwise:

- A span of static extent `K` is a static width of `K * digits`, and runs the same arms as a `std::array`,
  the one- and two-block paths included. One block is `std::span<Block, 1>`.
- A span of dynamic extent is `blocks_extent`, a third value of `N` beside a bit count and `std::dynamic_extent`:
  a run-time width that is not a member, `size()` being the span's length times `digits`.

So `has_static_size` asks whether the width is a template argument and `has_stored_size` whether it is a member,
and the members that grow, the written-out moves and the allocator constructors are the second question's: a
borrowed run-time width neither grows nor moves anything but a span. `extent` still answers
`std::dynamic_extent` for `blocks_extent`, which is what every reading already asks to mean "not static".

The class admits the span beside `owned_bit_blocks`, and only at the span's own default width: a narrower
`N` over someone else's blocks would have a tail the storage could not keep clear. The span is refused as a
`owned_bit_blocks` on purpose and stays refused -- it is not `regular`, and its const subscript writes --
so `borrowed_block_span` is its own concept, with a mutable unsigned element.

**The view holds that storage by value, as `std::views::all` holds a view.** A view over an owner holds a
`Bits*`, which is `std::ranges::ref_view`: the owner is not a view, so the view refers to it. Span-backed
storage *is* a view, and `views::all` would copy it rather than point at it. Pointing at it would cost a named
object for the pointer to reach: `auto bits = borrow_bits(board);` before `bit_set_view(bits)`, with `bits` bound
to outlive the view. `storage_ref_t<Bits>` is the choice, made once in
`detail/storage_ptr.hpp`: a `Bits*`, or a `storage_copy` holding the span-backed storage. The copy is `mutable`,
because the blocks are not the view's and a const view writes through them as a const `std::span` does. So the
view is the size of the span, 8 bytes over one block or an array and 16 over a vector, and there is nothing
beside it to keep alive.

**The iterators hold the blocks, not the view.** The views are `borrowed_range`s: an iterator outlives the
view it came from, which is what `std::ranges::find(bit_set_view(board), 7)` returning a live iterator
needs. An iterator that held a pointer to the view's copy would dangle there. `storage_ptr_t<Bits>` is the
iterator's and the proxy's hold: a `Bits*`, or a `block_ptr` holding the blocks' address and, for a dynamic
extent, their count, from which `->` rebuilds the storage for the one call it makes. It holds no storage
object because `std::span<Block, 1>` has no default constructor and an iterator must have one. An iterator
over one block is a pointer and a position, as it is over an owner.

`borrow_bits` and `borrowed_bits` are in `detail/`: with the views taking the blocks directly, they are how a
view gets its storage and not something a caller needs to name.

### views-over-owners

An owner is not itself a storage — `bit_fixed_set` and `bit_array` are thin wrappers over a
`bit_block_container` — so a view over an owner is a view over the storage it wraps:
`bit_set_view(xstd::bit_fixed_set<64>&)` is `set_adaptor<bit_block_container<std::array<size_t, 1>, 64>, refers>`, and the pointer in
the iterator is to the `bit_block_container`, never to the `bit_fixed_set`. The owner hands its storage over through
`owned_storage<Owner>`, declared beside it and never defined for anything else, so
`owner_of<Owner, Bits, R>` reads "this owner wraps exactly the storage this view refers through, and is not
already committed to another reading". Const flows one way: a const owner gives a
view over `Bits const`, a mutable owner either.

The view's converting constructor takes the owner's private member directly, which is why an owner befriends
a referring adaptor — the one friendship in the tree that runs upward, from a container to the views over it,
and it grants access to a member and to nothing that member's type does not already expose. Which adaptors it
befriends is [the-readings-do-not-mix](#the-readings-do-not-mix). Storage stays private; nothing on an owner's
surface says `bit_block_container`.

### viewing-an-owner-is-implicit

`bit_set_view(my_set)` is a converting constructor on the **view**, not a conversion operator on the owner, and
the choice is forced rather than stylistic. Deduction runs through constructors and guides, wherever they are
written -- on each view, or on the adaptor behind an alias; a conversion function contributes nothing to class
template argument deduction either way. Measured on a model of both shapes: with only the
operator, `view(owner)` is `no matching function for call to 'adaptor(...)'`. The constructor has to exist
anyway, and once it does the operator is a second mechanism for a conversion already spelled. Three lesser
reasons agree. The constructor is where `owner_of<Bits, R>` already hangs, so the readings-do-not-mix rule is
stated once. Const falls out of deducing `Owner&` rather than needing an `operator view<Bits>() &` and an
`operator view<Bits const>() const&` kept in step by hand. And `Owner&` is an lvalue reference, so a temporary
owner never binds — the `string_view` foot-gun closed by the signature instead of by a `&`-qualifier someone has
to remember.

The standard's own split is about layering, not taste: `string` → `string_view` is an operator because
`<string_view>` must not depend on `<string>`, while `vector`/`array` → `span` is a constructor because `span`
is generic over its sources and names none of them. Owner and view here are the same class template, so there is
no layer to respect, and the second shape is the one that fits.

**Implicit from an owner, explicit from raw storage**, and `span` supplies the criterion. Its conditional
`explicit` is usually read as "static extent", which is not what it says — the span-to-span constructor spells
it `extent != dynamic_extent && OtherExtent == dynamic_extent`, so static-to-static stays *implicit* and only
dynamic-to-static is explicit. The rule is therefore: **explicit exactly where the conversion asserts a size the
source cannot prove**, which is why those constructors carry a hardened precondition
(`ranges::size(r) == extent`) and why `span(array<T, N>&)` and the C-array overload are implicit even at a
static extent — an `array` carries its `N` in the type.

Viewing an owner is the `array` row. The width comes from the owner's own `Bits`, `owner_of` requires the
storage to match exactly, and the lvalue parameter closes the lifetime hole: nothing is asserted
that is not already proven, and there is no precondition to violate. So that constructor is implicit — spelled
`explicit(false)` with a `NOLINT(misc-explicit-constructor)`, as the five other deliberate implicit conversions
in the tree are, because `misc-explicit-constructor` holds that every one-argument constructor must be explicit
and cannot know that this is the conversion the type exists for. Saying `explicit(false)` rather than omitting
the keyword is what makes the intent readable at the declaration instead of inferable from its absence.
`set_adaptor(Bits&)` stays explicit — not for any size claim, but because reaching past a container to the
storage underneath it is an act worth spelling, and it is the constructor a user adapting their own storage
reaches for deliberately.

### the-readings-do-not-mix

A view over an owner is a second reading of bits that already have one, and a view of the other reading would be
choosing for the caller. `bit_set` is a set of positions; spanning it as bools would present the
container's capacity as its contents. `bit_vector` is a sequence of bools; viewing it as a set would present
the indices of the true ones as the elements. Neither is wrong in the way a bug is wrong — the bits do support
the reading — but neither is what the owner's own name says it holds, and the owner is the thing the caller
named. So a set owner admits only `bit_set_view`, a sequence owner only `bit_span`.

Blocks that no owner holds are committed to neither reading, so they admit either view: that is what a view over
raw storage is for, and `std::bitset<N>` reaches it through `xstd::bit_convert` to its blocks.

The rule is one typedef on each adaptor, `reads_as`, naming one of the empty tag types `set_reading_tag` and
`sequence_reading_tag`, which every owner and view built on it inherits, and one concept over it, `reads<T, R>`,
asked with `std::derived_from`: the container's reading is `R` or refines it. `owner_of` asks it of the owner
for the view's reading, and the hash, `bit_convert`'s widths and the proxies' recognition ask the same concept.
The readings are types rather than enumerators so that the set of them is open: another reading, a string one
say, declares a tag of its own and touches nothing already there, and one that refines an existing reading
derives its tag from that one's, so it is accepted wherever its base is asked for. It has to live in the constraint
and not in the friendship alone. Dropping only the friendship leaves the constructor declared and viable, and
its `m_bits(&c.m_bits)` is a mem-initializer — not the immediate context — so the access check happens at
instantiation and nowhere earlier. Measured: `std::is_constructible_v<bit_set_view<Blocks>, bit_array<8>&>`
still answers **true**, and the actual construction fails with `'m_bits' is private within this context`
pointing into `set_adaptor.hpp`. A type trait that lies and an error inside a constructor the caller never
meant to reach are both worse than the constructor simply not being there, which is what the clause gives: no viable
deduction guide for `bit_span(bit_set{})`, and `is_constructible_v` false.

It is the constraint that does the work, so the friendships follow it rather than the other way round:
`set_adaptor` befriends `set_adaptor` and `sequence_adaptor` befriends `sequence_adaptor`.

What this gives up is real and small: `bit_span(some_bit_set)` does not compile. Nothing in the library or the tests
wants it — an assertion that it compiles would assert the mechanism, not a use — and a caller who genuinely wants
the other reading of an owner's bits is asking for a conversion between owners, which is an owner's to offer
explicitly and not a view's to perform silently. None exists today; if one ever should, it belongs beside the
containers, where it can be named and its cost seen.

### the-interface-line

**If a user never spells it, it lives in `detail/`.** The name or the header, either counts. That is the whole
rule, and it is a test rather than a judgement: `bit_fixed_set` is spelled, `bit_block_container` is not;
`set_adaptor`'s nested `basic_reference` is reached only through the `iterator` and `reference` typedefs and is
spelled by nobody.

What the rule keeps on the interface side: **the eleven owners and the three views** -- the set and sequence
readings over four storages each, the small ones among them under `ext/`, and the valarray reading over three --
with the concepts they are named by (`bit_blocks` and the mapping concepts), the tag, the conversion, the
transformations and the `bit_` algorithms over them. The common vocabulary is a
test concept rather than a public one: nothing is constrained on it, so it is a claim about `std::bitset` and
`boost::dynamic_bitset` rather than an interface ([the-common-vocabulary](#the-common-vocabulary)).

That is the whole of it. There is no storage extension point to publish ([one-storage](#one-storage)), `Bits`
having to be a `bit_block_container`. **The adaptors are the harder call.** Every container and every view derives
from one, so a name a consumer cannot avoid reading could be called interface whether or not anyone can write it,
and `storage`, `window` and the axis tags would come along because no adaptor can be named without them. What
settles it is that a consumer never has to ask. A container is what it is, and what it *does* -- bidirectional with
a `key_type`, or random-access over `bool` -- is askable in the standard's own vocabulary, without naming a base at
all. So the adaptors and their vocabulary are in `xstd::bits::detail`, and what `namespace xstd` holds is the
owners and views, their short aliases, the transformations over them, and the concepts.

On the other side, the two that have to be argued. The four proxy types are reached only through container
typedefs, so no user spells them. `bit_block_container` and its three aliases are the device that turns
two readings times three storages into two plus three ([the-one-vehicle](#the-one-vehicle)), and the
`basic_` layer already exposes the block parameter — `basic_bit_fixed_set<std::size_t, std::uint8_t, 24>` reaches the
capability without the storage being named. Recorded against: `bit_block_container` *is* instantiated by
name throughout `test/`, which is a real signal, and demoting it makes the test tree reach into `detail/`. The
counter is that a test is not a user; a test tree that mirrors the library, `detail/` included, is what testing
an implementation looks like.

**Enforced, not asserted.** `test/consumer/main.cpp` includes `<xstd/bits.hpp>` and, the `ext/` column being
opt-in, `<xstd/bits/ext/boost.hpp>`, and no other header of ours. It names the eleven owners, the set and sequence
readings' over the fixed, bounded, small and dynamic storages and the valarray reading's over the fixed, bounded and
dynamic ones, and the three views, and nothing from `detail/`. It pattern-matches each container against its
reading — bidirectional with a `key_type`, random-access over `bool`, or random-access over `bool` with an `==` that
answers an array rather than a `bool` — for all eleven and over the three views besides, which is the
claim [the-views-are-the-adaptors](#the-views-are-the-adaptors) rests on, asked from outside and in the
standard's vocabulary rather than in ours. The view names it reaches by deduction, which is both the only way
in and the thing the classes are for. The three `consumption`
configurations build it against the installed headers, so a name that stops being reachable from the umbrella,
or an interface header that starts needing one from `detail/`, fails there rather than in a user's build.

Two failures are what it guards against. The first is a public header missing from `bits.hpp`, which reaches
consumers only transitively, through whichever headers include it — the same thing the include order guards
against, our headers before Boost's and the standard's so that a transitive include is found rather than leaned
on. The second is worse, and no compiler leg can catch it: `CMakeLists.txt`'s `FILE_SET HEADERS` is hand-written,
and a header that reaches `include/` and `bits.hpp` without reaching it is never installed, which breaks
**every** installed consumer's umbrella include on every compiler. Every compiler leg builds from the source
tree, where the files are present; only the consumer translation unit builds against the install tree, which is
why it includes the umbrella rather than one container header. The file set is checked against `include/xstd/`
at configure time, the way `test/` checks that every public header is mirrored — a hand-written list that nothing
verifies is a list that drifts.

The rule reaches `include/` as a whole, not only `include/xstd/`. A header beside the library under `include/` is
never installed and so invisible to a `find_package` consumer, yet on the build-interface include path of every
`add_subdirectory` and `FetchContent` consumer, to whom its names are perfectly reachable. So the sieve the
containers are benchmarked with lives in `examples/`, in namespace `opt`, and the file-set check globs all of
`include/`, so a stray directory is a configure error rather than a silent extra ([the-sieve](#the-sieve)).

`ext/` is the one interface piece the umbrella leaves out. It costs nothing here — a consumer that wants the
small column includes the one header for it — and it keeps `boost::container::small_vector` off the path of
every consumer who does not.

### the-public-names

Two layers of names, over a third nobody spells. The adaptors under `detail/` carry the reading and take the
storage, the parameters each reading needs and no more ([the-two-adaptors](#the-two-adaptors)). The
`basic_` layer chooses the storage and leaves the block open, `basic_string`-style:
`basic_bit_array<Block, N>`, `basic_bit_vector<Block, Allocator>` and their sequence siblings. A set's `basic_` name
leads with its key, as `std::set<Key>` does, then the storage, then the defaulted policies, the key's mapping first:
`basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>`, `basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>`,
`basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>` and `basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>`,
with `KeyMapping` defaulting to `bit_key_mapping<Key>` and `Compare` to `std::less<Key>`. The block leads the storage in
every column, so a `basic_` name hands its base clause the arguments in the order it was given them --
`basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>` derives from `set_adaptor<bit_block_container<std::array<Block, K>, N>,
storage::owned, basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>`, straight
through. Taking `<N, Block>` in the static and bounded columns and transposing at the call would gain nothing:
`Block` carries no default in those columns, so it is free to lead, and leading is what
`std::array<T, N>`, `std::inplace_vector<T, N>` and `std::span<T, Extent>` all do with the pair. The restricted
layer fixes `std::size_t` and `std::allocator`: `bit_fixed_set<N>` and `bit_array<N>` keep one
parameter, and `bit_set` and `bit_vector` keep none, so the flagship is `xstd::bit_set`, without the `<>`. A set's
short name fixes its key to `std::size_t` as well: `bit_fixed_set<N>` is `basic_bit_fixed_set<std::size_t, std::size_t, N>`.

The key is spelled wherever the `basic_` form is, because it changes the interface: `key_type`, what iteration
yields, and what `insert`, `find` and `contains` accept. The positions stay the storage, and `KeyMapping` is the key's
*mapping*: an order-preserving bijection from a finite universe of keys onto the positions `[0, N)`, given by two
static members, `to_index(key)` and `from_index(index)`, and, where the mapping closes the universe, a `size` that is
that `N` and an `is_key(key)` that says whether a value is one of those `N` keys. Two public concepts, each in its own header under `<xstd/bits/bit_concepts/>`, say so. `bit_index_mapping<M, Key>` asks for
`M::to_index` taking a `Key` to a `std::size_t` and `M::from_index` giving back exactly a `Key`, and carries what no
syntax check can see: `a < b` exactly where `to_index(a) < to_index(b)`, and `from_index(to_index(k)) == k` for every
`k` in the universe. `sized_bit_index_mapping<M, Key>` adds `M::size` and `M::is_key`, returning exactly `bool`, and
with them `to_index(k) < size` for every key. `is_key(k)` is the universe tested, which is `to_index`'s precondition:
a range's `First <= k < First + N`, computed as the unsigned distance from `First` so that no bound overflows the key
type, a list's binary search, the same one `to_index` ranks by, and a flag's one bit set below `N`. The open identity
has an `is_key` too, true for every key that names a position, which is every key no wider than `std::size_t`, and
`bits::detail::is_key<M>(k)` asks it of any mapping, answering `true` where the mapping declares none; an unsized
mapping need not, its universe being open. An owner with a width or a capacity in its type must equal that size, checked by a `static_assert`. Every set
owner constrains its `KeyMapping` with the first and asks the second wherever the universe bounds the width. The
family is four templates, each taking its key first. `bit_key_mapping<Key>` is the default an owner takes and the
customization point, specialized beside a key type as `std::char_traits` is beside a character type.
`bit_range_mapping<Key, First, N>` maps consecutive keys by subtraction, `bit_find_mapping<Key, Keys>` maps a sorted
list of keys by binary search, and `bit_flag_mapping<Key, N>` maps one-bit values by their bit
([flag-types](#flag-types)). Only the unsigned identity leaves the universe open. `bit_key_mapping<Key>` is the identity with no `size` for every `xstd::unsigned_integer` key, from
`std::uint8_t` to the 128-bit types: a key narrower than `std::size_t` names fewer positions, and one wider must name
a position that fits, as a precondition. A signed key has no default: offsetting by its most negative value would put
`0` in the middle of the universe, so a `bit_set<int>` holding `{0}` would take 2³¹ bits. It names its range instead,
through `bit_range_mapping<Key, First, N>`: the `N` consecutive keys from `First`, a contiguous range, at positions
`0` to `N - 1`, with a `size` of `N`. The arithmetic is modular in the key's unsigned counterpart, an enumeration's
through `std::to_underlying` and its underlying type's, so `First` may be the type's minimum and an enumeration whose
values run without a gap needs no list. A strong index type specializes
`bit_key_mapping` or is given a mapping of its own. `to_index` must preserve order, so that ascending positions
are ascending keys. Every member that takes a key maps it through `to_index`, and the iterator and its proxy hand out
`from_index` of the position, so a set formats as its keys do; set algebra, comparison, hashing and the block
exchange work on blocks and never see a key. The views stay keyed by `std::size_t`: a view reads positions it does
not own, and names no key of its own.

An enumeration is keyed by rank, not by value. Its author declares the values once, beside the enumeration:
`template<> struct xstd::enum_traits<E> { static constexpr std::array values = {E::a, E::b, ...}; };`, ascending by
underlying value and each value once. `enum_traits` only lists; `bit_key_mapping<E>`, specialized for every
enumeration that declares a list, picks the mapping that reads it. Where the list turns out contiguous, each value one
above the last, which is decided at compile time, it derives from `bit_range_mapping<E, values[0], size>`, and
`to_index` is a subtraction in the underlying type's unsigned counterpart, so a dense enumeration starting anywhere,
negative included, pays no search. Otherwise it derives from `bit_find_mapping<E, values>`, which checks the list
strictly ascending with a `static_assert`: `size` is its length, `from_index(i)` is `values[i]`, and `to_index(e)` is
`e`'s rank, found by binary search. Gaps cost nothing, so
`{pawn = 1, knight = 3, bishop = 4, rook = 8, queen = 9, king = 100}` takes six bits rather than a hundred. A value
not in the list ranks at `size` or above under either, and every owner refuses to write it there with
`std::out_of_range`: a fixed set because `size` is its width, and a growing one, whose storage could hold the
position, because `size` closes the universe all the same. A growing owner's `max_size()` is that `size` as well, so
a left shift drops what it carries past the last key, as it does at a static width. A lookup asks `is_key` before
`to_index`, so `contains`, `count`, `find` and `erase(k)` answer *absent* for any
value of the key type, as `std::set`'s do; the bounds of a value that is no key bisect the keys under `key_compare`,
which puts `First - 1` before a range's first element where the wrapped distance alone would have put it after the
last, and an unlisted value between two listed ones between them. A key type with no order of its own, a
`std::bitset`, has no place between two keys to give, and its bounds keep `is_key` as their precondition. `basic_bit_fixed_set<E, Block, N>` with no mapping spelled keys it the same way. An enumeration that lists nothing
but ends in a sentinel, `enum class E { a, b, c, count };`, is keyed by that sentinel, below. One that declares
neither has no default mapping, and so no set, unless its author specializes `bit_key_mapping<E>` directly, say as a
`bit_range_mapping` from its first enumerator. The same `bit_find_mapping` keys sparse integers, such as identifiers
drawn from a fixed table, to as many positions as there are identifiers.

The count comes from the declaration because C++ has no portable way to count enumerators. Many enumerations
already carry one: `enum class piece : std::uint8_t { pawn, knight, bishop, rook, queen, king, count };` says
"from the first enumerator, `count` of them", and type_safe's `flag_set_traits` reads such a sentinel too. So
`bit_key_mapping<E>` reads it where the author declared it. For an enumeration with an enumerator named `count` and
no `enum_traits<E>::values`, it derives from `bit_range_mapping<E, E{}, N>`, `N` being `count`'s distance from
`E{}`: `to_index` is a subtraction, `size` is `N`, and the sentinel, at position `N`, is never a key, so
`bit_enum_set<piece>` is one byte holding six keys and `insert(piece::count)` throws `std::out_of_range` as any
other non-key does. The name is exactly `count`, one spelling, since every further spelling (`size`, `COUNT`,
`_size`) is another way for an unrelated enumerator to be read as a sentinel, and an inclusive alias such as
Chromium's `kMaxValue = kLast` would be a second rule rather than a synonym. The range starts at `E{}`, the value
`0`; an enumeration starting elsewhere lists its values or spells its `bit_range_mapping`. A `count` below zero is
rejected by a `static_assert`, and a `count` of `0` is the empty universe, as `bit_range_mapping<E, E{}, 0>` is. The
sentinel means that every value from `0` to `count - 1` is a key, and C++ cannot check that: an enumeration
`{a = 0, b = 2, count = 3}` gets a three-key range in which the undeclared value `1` is a key. A list is the
enumeration's own account of its keys and keeps priority, so an enumeration that lists its values keeps the list's
mapping whether it ends in `count` or not, and the two specializations never compete.

The sentinel is read, never required. Requiring one would make the width a value of `E`, which every exhaustive
`switch` must then handle and which cannot be added to an enumeration someone else owns; an enumeration without one
behaves as it would if the rule did not exist. Counting by parsing compiler-generated function names, as `magic_enum`
does, works only within a guessed value range and is no part of the language. `enum_traits<E>` remains the general
way: it leaves `E` as it is, takes gaps and any first value, and can be declared by whoever needs it; C++26
reflection, `std::meta::enumerators_of(^^E)`, can generate `values` later without changing what `bit_key_mapping`
reads.

`bit_enum_set<Enum>` is `bit_least<basic_bit_fixed_set<Enum, std::size_t, N, bit_key_mapping<Enum>>>`, `N` being
the mapping's `size`, for any enumeration whose `bit_key_mapping` models `sized_bit_index_mapping`. The block is
`bit_least`'s ([block-and-width-transformations](#block-and-width-transformations)), the narrowest of `std::uint8_t` to
`std::uint64_t` that holds the `N` keys, since an enum set mostly lives inside other structures as a flag field: a
three-value set is one byte, not eight. Wider than 64 values, it is several `std::uint64_t`. The alias takes no
block, so the block policy is stated once, by `bit_least`. A field of fixed wire width is
`basic_bit_fixed_set<E, std::uint32_t, N>`, whose mapping defaults to the same `bit_key_mapping<E>`, and a set over
another mapping of the enumeration is `bit_least<basic_bit_fixed_set<E, std::size_t, N, M>>`, which spells the
mapping and keeps the block policy. `least_block_t` is public, in `<xstd/bits/bit_type_traits/bit_least.hpp>`, so that the policy has
a name outside the alias. A deduction guide on the class template, not on the alias, deduces that same type from a
braced list of enumerators, `basic_bit_fixed_set{E::a, E::b}`; alias deduction is not relied on, being unreliable
on older compilers. An enumerator meets a set through the set's own type: `|`, `&`, `^` and `-` take a set on one
side and an enumerator on the other, and `|=`, `&=`, `^=` and `-=` an enumerator on the right, the enumerator acting
as the one-element set holding it. The value-returning forms are hidden friends of the set, so two enumerators
never reach them, and nothing is declared on the enumeration: `E::a | E::b` does not compile, and two enumerators
combine as `bit_enum_set<E>{E::a, E::b}`. Each is constrained to an enumeration `key_type` whose mapping is no mask
mapping ([flag-types](#flag-types)), where a value is a set of keys rather than one, so a set of integers
gains no mixed operator, and with `key_type` an enumeration nothing converts an `int` into one, so `insert(1)`,
`contains(1)` and `find(1)` do not compile.

`Compare` keeps `std::set`'s place, after the key's mapping and before the allocator, and is defaulted because it
leaves every member signature as it is. Position order is structural, so a comparator can only choose a direction:
`std::less<Key>` and `std::less<>` ascend, `std::greater<Key>` and `std::greater<>` descend, and any other type is
rejected by the owners' template heads. The mapping comes first because the direction is defined over the order
`to_index` preserves. Under `std::greater` the set behaves as `std::set<Key, std::greater<Key>>` specifies: `begin()`
is the highest set position and `++` steps down, `lower_bound` is the largest key not above its argument and
`upper_bound` the largest below it, `key_comp()` and `value_comp()` return `std::greater`, and `xstd::bit_for_each` walks
in that order, and against it over `std::views::reverse`. The iterator carries the direction rather than wrapping
`std::reverse_iterator`, which would step back on every dereference; both directions end at `size()`. Equality, set
algebra, hashing and the block exchange are unchanged, since they see the same blocks. `<=>` is
`[associative.reqmts]`' lexicographic comparison of the keys in iteration order, which over descending keys is the masks
compared as unsigned numbers, highest block first, a missing high block reading as zero; `numeric_three_way` beside
`set_three_way` does that a block at a time. The transparent forms add the heterogeneous `find`, `count`, `contains`,
`lower_bound`, `upper_bound`, `equal_range` and `erase`: a key of another type has no position, so they bisect the
positions for the run of keys equivalent to it under `Compare`. The short names keep `std::less`, and the views ascend.

The unmarked name goes to the flagship — `bit_set` is the dynamic set
benchmarked against `std::set` and `std::flat_set` — and the qualifier marks the special case, `bit_fixed_set`.
The sequence row is named after the `std` container it packs, `bit_array` for `std::array<bool, N>`. The rows
therefore mark different columns, and that is correct by each row's own analogy rather than an inconsistency
to fix.

**Why the prefix leads.** `bit_` is a storage-strategy prefix, and the Standard already has the other one:
`std::flat_set` keeps a sorted sequence of the elements that are there, so it is sparse in the universe of
possible keys, where `bit_set` keeps one bit per position in that universe, so it is dense but packed. Same
container, same interface, different representation — and the prefix is what says which, so it has to come
first. `fixed_bit_set` would read as a qualified `bit_set`; `bit_fixed_set` is `bit_` applied to a
fixed set, the way `flat_set` is `flat_` applied to a `set`.

**Why `fixed` and `bounded`.** The qualifier names what the user gets, not where the blocks live. `fixed` is a
width fixed at compile time; `bounded` is a run-time size under a compile-time capacity. `finite` would select
nothing, since `bit_set` is a finite set of positions too. `static` is overloaded in C++, which is part of why
P0843 renamed `boost::static_vector` to `std::inplace_vector`. `inplace` names one storage, and the bounded
column is not tied to one: `boost::container::static_vector` holds the same blocks where the standard library
has no `std::inplace_vector`. The qualifier comes after `bit_`, as `small` does in `bit_small_set`.

One header per restricted name, holding its `basic_` form beside it, each over one storage: `bit_set` and
`bit_vector` over `std::vector<Block, Allocator>`, beside `bit_fixed_set` and
`bit_array` over `std::array<Block, num_blocks_v<Block, N>>`, and `bit_bounded_set` and `bit_bounded_vector`
over `bounded_blocks<Block, num_blocks_v<Block, N>>` ([the-bounded-column](#the-bounded-column)). The
header is the name's home and the only place it is spelled; `bits.hpp` includes them all.

**What `bit_` names.** The prefix marks what works on the packed bits: the containers, the concepts and
transformations over blocks (`bit_concepts/`, `bit_type_traits/`, `bit_align`, `bit_least`), and `bit_hasher`. The unprefixed name
works on the value as the standard model sees it, as `xstd::hasher` does. So `bit_hasher` is the
representation-level hash and `hasher` the value-level one. The prefix names **what** is hashed, the packed
representation, not whether the code is optimized: the hooks behind `xstd::hasher` expand bits to bools with
`_pdep_u64`, AVX-512 or a table, and are as optimized, yet still append the model's bytes.

### block-and-width-transformations

Two type transformations, one header each under `<xstd/bits/bit_type_traits/>`, take an owner and return the same
owner with one argument changed, every other passed through, as `std::simd`'s `rebind_t` and `resize_t` do:
`bit_rebind<Block, X>` puts it over another block, its allocator rebound with it through
`std::allocator_traits<A>::rebind_alloc<Block>`, and `bit_resize<N, X>` gives it another width. Four more are named
choices written through that pair, the block or width computed from `X`'s own:

| Transformation | Block | Width |
| :-- | :-- | :-- |
| `bit_rebind<B, X>` | `B` | `X`'s own |
| `bit_resize<M, X>` | `X`'s own | `M` |
| `bit_least<X>` | `least_block_t<N>`: the narrowest of `std::uint8_t` to `std::uint64_t` holding `N`, else `std::uint64_t` | `N` |
| `bit_fast<X>` | `fast_block_t<N>`: `std::uint_fast8_t` to `std::uint_fast64_t` by the same thresholds | `N` |
| `bit_align<X>` | `X`'s own | `align_up(N, digits)`, whole blocks of `X`'s block |
| `bit_underlying<X>` | `underlying_block_t<X::key_type>`: the key's underlying type, enumeration or integer, made unsigned | `N` |

`bit_rebind` and `bit_underlying` take every owner: `basic_bit_array<Block, N>`,
`basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>`, `basic_bit_bounded_vector<Block, N>`,
`basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>`, `basic_bit_vector<Block, Allocator>`,
`basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>`, and the Boost extension's
`basic_bit_small_vector<Block, N, Alloc>` and `basic_bit_small_set<Key, Block, N, KeyMapping, Compare, Alloc>`, short
names included. `bit_resize`, `bit_least`, `bit_fast` and `bit_align` read or write `N`, so they take the six whose
type carries one, the bounded and small ones changing their capacity. So `bit_rebind<std::uint8_t, bit_vector>` is
`basic_bit_vector<std::uint8_t, std::allocator<std::uint8_t>>`, `bit_least<bit_fixed_set<9>>` is
`basic_bit_fixed_set<std::size_t, std::uint16_t, 9>`, two bytes where `bit_fixed_set<9>` is eight, and
`bit_align<bit_array<9>>` is `bit_array<64>`, whose last block carries no unused tail. A type with nothing to change,
a view, a standard bit container or a bare block, and a run-time owner asked for a width, is rejected by the alias's
constraint rather than deep inside it.
The names are `<cstdint>`'s: `std::uint_least8_t` is the smallest type of at least 8 bits and `std::uint_fast8_t` the
fastest, and the platform decides how wide that is. glibc on x86-64 makes `std::uint_fast16_t` and
`std::uint_fast32_t` 64 bits wide where MSVC makes them 32, so `bit_fast` names a block and promises no size.

`bit_underlying<X>` asks more of `X`: a set whose `key_type` is an enumeration or a built-in integer type with an
unsigned counterpart, which excludes `enum : bool`, `bool`, the character types and every class key, a `std::bitset`
mask among them. Its block is the word an existing field or ABI already stores those flags in,
`std::make_unsigned_t` of the enumeration's underlying type or of the integer type itself, and named
`underlying_block_t<K>` beside `least_block_t` and `fast_block_t`: `underlying_block_t<int>` is `unsigned`, which is
what an `int` field of flags is read as. The rule is by type, not by mapping, so a set of positions has one too:
`bit_fixed_set<N>` is keyed by `std::size_t`, whose word is its own, and
`bit_underlying<bit_least<bit_fixed_set<9>>>` is `bit_fixed_set<9>` again. That is what a flag set wants at a
boundary: `bit_underlying<bit_fast<bit_flag_set<E>>>` is back in `E`'s own word,
`bit_underlying<bit_flag_set<E, 9>>` holds its nine flags in that word rather than the two bytes `bit_least` chose,
and either way the set's block is the mask's representation bit for bit, so `xstd::bit_convert`, `from_blocks` and
`std::bit_cast` to and from that word are copies. For a flag set at the mask's full width, the default, `bit_least`
already chose that word, and `bit_underlying` is the identity; it is the transformation that says why, rather than
one that happens to agree. The precedent for the form is `std::make_unsigned_t<T>`, a type in and a related type
out, and C++26's `std::simd`, whose `rebind_t<U, V>` changes the element type of a `basic_simd` and `resize_t<N, V>`
its width, each keeping the other, and `bit_rebind` and `bit_resize` take the names and the argument order of that
pair. Each owner specializes one exposition-only trait that names its block, its width where its type has one, and
itself over another block or width; the two aliases are written once against that trait, and the four choices once
against the two. Spelling the same choices as namespaces, `aligned::bit_fixed_set<N>` beside
`xstd::bit_fixed_set<N>`, would re-declare every class template's parameter list in each namespace with its own
defaults and constraints, and such copies drift: an `aligned::basic_bit_fixed_set` taking an unconstrained `Compare`
where the class takes only a direction surfaces a misuse inside the class rather than at the name. A transformation
has no parameter list of its own to keep in step, and so cannot drift; it also applies to a type a user has already
named, mapping and comparator included, rather than asking for them again.

They compose, and the order matters. `bit_align<bit_least<bit_fixed_set<3>>>` picks the byte first and then fills
it, `basic_bit_fixed_set<std::size_t, std::uint8_t, 8>` in one byte: least first, then align, is the compact form
with no padding and no unused tail. `bit_least<bit_align<bit_fixed_set<3>>>` rounds to the 64 bits of the default
block first, and the least block of 64 bits is `std::uint64_t`, eight bytes. `bit_align<bit_fast<X>>` is the fast
block filled the same way. Each is idempotent.

Past 64 bits the least block is `std::uint64_t` and the set takes several of them, so a wide set still scans a word
at a time. The choice has prior art. [N4202](https://wg21.link/n4202), *Strongly Typed Bitset*, proposed a block
parameter for `std::bitset` and a `small_bitset<N>` over the smallest of `std::uint_least8_t` to `std::uint_least64_t`;
it chooses by `N % 64`, so a 72-bit `small_bitset` is nine one-byte blocks where `bit_least<bit_fixed_set<72>>` is two
words. [type_safe](https://github.com/foonathan/type_safe)'s `flag_set<Enum>` stores its flags in the smallest of
`std::uint_least8_t` to `std::uint_least64_t` that holds them, and takes no more than 64. `bit_enum_set` is
`bit_least` over an enumeration's fixed set, as above.

A block need not be a power of two. One that is, of a byte or more, tiles any width: no position and no byte
straddles two blocks, so every owner and view takes it. Any other width, C23's `unsigned _BitInt(17)` or
`unsigned _BitInt(127)` spelled `xstd::bit_uint<N>`, holds one fixed-width value: `bit_array` and `bit_fixed_set`
take it for a width up to its own, in a single block, and the bounded, small and growing owners refuse it, since a
second block would put a position across two of them. Clang on x86-64 stores such a block in the next standard
integer size, so `_BitInt(23)` takes the four bytes of a `std::uint32_t` and nine padding bits the library never
reads; what the block buys is the exact width in the type, not fewer bytes.

Other libraries either pick the block themselves, from the width, or take it from the user as a template argument:

| Library | Who picks the block | Rule | Whole-block width | Changing the block |
| :--- | :--- | :--- | :--- | :--- |
| xstd-bits | the user, by transformation or argument | `std::size_t` by default; `bit_least<X>` the least type holding `N`, `bit_fast<X>` the fast one, `bit_underlying<X>` the key's underlying type made unsigned; `bit_enum_set` and `bit_flag_set` are `bit_least` already; any block as the `Block` argument of a `basic_` template | `bit_align<X>` | by transformation, on any fixed-width owner, its key, mapping and order kept; the transformations compose |
| type_safe `ts::flag_set<Enum>` | the library | `std::uint_least8_t` to `std::uint_least64_t`, the least holding the number of flags; no more than 64 flags | no | no |
| itsy_bitsy `bitsy::bit_view<Range, Bounds>`, `bitsy::bit_sequence<Container>`, `bitsy::dynamic_bitset<T>` | the user | the value type of the range or container adapted, or `T`, with no default | — | by naming another range, container or `T` |
| Chromium `base::EnumSet<E, Min, Max>` | the library | `uint8_t`, `uint16_t` or `uint32_t` by the number of values from `Min` to `Max`, and a `std::bitset` past 32 | no | no |
| Mozilla `mozilla::EnumSet<T, Serialized>` | the user, with a default | `Serialized`, by default the enumeration's underlying type made unsigned, which is `bit_underlying`'s rule; any unsigned type, or a `mozilla::BitSet` | no | by spelling the argument again |
| MSVC STL `std::bitset<N>` | the library | `unsigned long` up to its 32 bits, else `unsigned long long` | no | no |
| libstdc++ `std::bitset<N>` | the library | `unsigned long`: 64 bits on LP64 targets, 32 on LLP64 ones such as MinGW | no | no |
| libc++ `std::bitset<N>` | the library | `std::size_t` | no | no |
| Boost `boost::dynamic_bitset<Block, AllocatorOrContainer>` | the user, with a default | `Block`, by default `unsigned long` | — | by spelling the argument again |
| LLVM `llvm::Bitset<NumBits>` | the library | `uintptr_t`, as for `llvm::BitVector` | no | no |

A dash marks a width chosen at run time. Checked against type_safe's
[`flag_set.hpp`](https://github.com/foonathan/type_safe/blob/292e8c127037e33d92f8f52dab0f1184993942d0/include/type_safe/flag_set.hpp)
at `292e8c1`, itsy_bitsy's
[`bit_view.hpp`](https://github.com/ThePhD/itsy_bitsy/blob/d5b6bf9509bb2dff6235452d427f0b1c349d5f8b/include/itsy/bit_view.hpp)
and [`bit_sequence.hpp`](https://github.com/ThePhD/itsy_bitsy/blob/d5b6bf9509bb2dff6235452d427f0b1c349d5f8b/include/itsy/bit_sequence.hpp)
at `d5b6bf9`, Chromium's
[`enum_set.h`](https://github.com/chromium/chromium/blob/b0840fa3d3ce4939cecdb19aa2125bbffbe069ca/base/containers/enum_set.h)
at `b0840fa`, Firefox's
[`EnumSet.h`](https://github.com/mozilla-firefox/firefox/blob/3c71a541b0e7ac6e086907b38670dabca9174c45/mfbt/EnumSet.h)
and [`BitSet.h`](https://github.com/mozilla-firefox/firefox/blob/3c71a541b0e7ac6e086907b38670dabca9174c45/mfbt/BitSet.h)
at `3c71a54`, MSVC STL's [`<bitset>`](https://github.com/microsoft/STL/blob/da52dc0fcadbd83690cfd1ed32408d2847439493/stl/inc/bitset)
at `da52dc0`, libstdc++'s [`<bitset>`](https://github.com/gcc-mirror/gcc/blob/releases/gcc-16.2.0/libstdc++-v3/include/std/bitset)
in GCC 16.2.0, libc++'s [`<bitset>`](https://github.com/llvm/llvm-project/blob/llvmorg-23.1.3/libcxx/include/bitset)
and LLVM's [`Bitset.h`](https://github.com/llvm/llvm-project/blob/llvmorg-23.1.3/llvm/include/llvm/ADT/Bitset.h) and
[`BitVector.h`](https://github.com/llvm/llvm-project/blob/llvmorg-23.1.3/llvm/include/llvm/ADT/BitVector.h) in LLVM
23.1.3, and Boost's
[`dynamic_bitset.hpp`](https://github.com/boostorg/dynamic_bitset/blob/boost-1.92.0/include/boost/dynamic_bitset/dynamic_bitset.hpp)
in Boost 1.92.0.

None of the others changes the block of a type already named or rounds a width to whole blocks on request: where the
block is an argument the type is spelled again with another, and where the library picks it the user cannot.

### flag-types

A flag type replaces a bitmask type such as `std::filesystem::perms`: code written against the mask keeps the
spelling of its constants, only the variable's type changes, and the set vocabulary comes on top.
`bit_flag_set<Mask, N>` is the set of `Mask`'s one-bit values below `N`: its `key_type` is `Mask` itself, each key a
value with exactly one bit set, ranked at its bit by `bit_flag_mapping<Mask, N>`, and `N` defaults to the mask's
width. It is no class of its own. There is one fixed set, `basic_bit_fixed_set`, and the flag type is an alias of it,
`bit_least<basic_bit_fixed_set<Mask, std::size_t, N, bit_flag_mapping<Mask, N>, std::greater<Mask>>>`, in the least
block holding `N` as `bit_enum_set` is. What makes a fixed set a flag type is its mapping. `bit_mask_mapping<M, Key>`
refines `sized_bit_index_mapping` with `M::to_block`, which takes any value of `Key`, one-bit or not, to the block
holding its bits at their positions, and `M::from_block`, which reads one back; `bit_flag_mapping` models it and no
other mapping does. Under that concept alone, `basic_bit_fixed_set` declares the members a flag type has: the
conversion from the mask and to it, the union of a braced list and the mixed operators; through the conversion,
`xstd::bit_includes`, `xstd::bit_intersects` and `xstd::bit_disjoint` take a mask as the other set. A set of positions or of listed enumerators has none of them, and a flag set
over another block, `bit_fast<bit_flag_set<Mask>>` or `basic_bit_fixed_set<Mask, std::uint8_t, 16, bit_flag_mapping<Mask, 16>>`
over two bytes, or in ascending order, has all of them.
`examples/include/xstd/filesystem.hpp` is one line, `using perms = bit_flag_set<std::filesystem::perms, 16>;`,
sixteen bits rather than the twelve permissions so that `std::filesystem::perms::unknown`, `0xFFFF`, survives the
round trip; its four high bits are keys like the others.

`Mask` is an enumeration, a built-in integer type or a `std::bitset<M>` of any width `M`, the three forms
[bitmask.types]/1 allows. Its word is the enumeration's underlying type made unsigned, the integer type's unsigned
counterpart, or for a bitset the least block holding `M`, and past the widest block an array of them:
`std::bitset<12>` is read as a `std::uint16_t`, and `std::bitset<147>` as a `std::array<std::uint64_t, 3>`. Every
conversion between the mask and the set goes through that word, position `i` staying position `i`: a cast for an
enumeration or an integer, and for a bitset the bytes `xstd::bit_convert` moves a bitset by, which reach a word wider
than `M` with its high bits clear. `bit_mask_mapping` asks the word to be fixed bit blocks, one unsigned block or a
fixed array of them, which is `xstd::bit_blocks`' own two cases with the width in the type, and never the signed type
under an enumeration. Of the three shapes weighed for a mask wider than a block, this is the word array: the set's
`from_blocks` already takes any fixed blocks at least `N` wide and drops what lies above, so the conversion, `&` and
`-` need nothing new, and an enumeration or an integer keeps its one-cast path. Dropping the mapping's `to_block` and
`from_block` for an equal-width `bit_convert` between set and mask would have cost that path and still needed a
widening step below `M`, and a bit-precise word, `xstd::bit_uint<M>`, exists on Clang alone. A rank and its key are
one bit counted across the blocks of the word, and `fits`, no bit at or above `N`, compares the word with the one
that keeps only the bits below `N`, so `bit_flag_set<std::bitset<147>, 140>` refuses a mask with bit 145 set on the
way in and truncates it under `&` and `-`. `bool` and the character types are integral but no integer type, and are
not masks. The 128-bit integers are masks too: `bit_flag_set<xstd::uint128>` is 128 flags over two 64-bit blocks on
GCC and Clang alike, in the GNU dialect the library builds in and under a strict `-std=c++23`, where they are no
`std::integral` and `xstd::integer` admits them instead.

An enumeration is a mask when `xstd::bit_mask` says so, its operators returning the enumeration, and also whenever
its `|` compiles, whatever that returns. Qt's flag enumerations return a `QFlags<Enum>` from `|`, and a C enumeration
with no operators of its own returns the `int` of the built-in one; both are bitmasks by intent, and writing
`bit_flag_set<E>` already says so. A scoped enumeration with no `|`, such as `std::filesystem::file_type` or
`std::regex_constants::error_type`, is an enumerated type and no bitmask, and is turned away: it is the likeliest
accident. An unscoped enumeration that is no bitmask still has the built-in `|`, and is admitted; that is the rule's
cost, paid only by someone who names it in `bit_flag_set`.

**The width an enumeration defaults to depends on how it is declared.** With an unsigned underlying type, it is
every bit of that type: Qt's `WindowType` and glibc's `EPOLL_EVENTS` put a flag on bit 31 that way. With a signed
underlying type, it is the integer's width less the sign bit, below: 31 for an `int`-based enumeration such as
`std::launch`, exactly as for `int` itself. With no fixed underlying type there is no default, and `bit_flag_set<E>`
does not name a type: such an enumeration's values are only those of the smallest bit-field holding its enumerators
([dcl.enum]/8), and a flag type over more bits than that forms a value outside the enumeration on `~` or on
iterating its high keys, which is undefined behaviour and which UBSan reports. `E{0}` is well-formed exactly where the
underlying type is fixed ([dcl.init.list]/3.8), and that is the test. The range itself cannot be computed: Clang
rejects an out-of-range `static_cast<E>` in a constant expression and GCC accepts it, and C++ cannot list
enumerators. So the user writes the width, as `bit_flag_set<Qt::AlignmentFlag, 9>` for an alignment whose
enumerators reach bit 8. libc++'s `regex_constants::syntax_option_type` and `match_flag_type`, and libstdc++'s
`ios_base::fmtflags`, `iostate` and `openmode`, are such enumerations. C++26 reflection lifts the restriction:
`std::meta::enumerators_of(^^E)` gives the enumerators, and with them the range.

**An integer mask keys its one-bit values, not its numbers.** `bit_flag_set<std::uint32_t>`'s keys are `1`, `2`,
`4`, … `0x8000'0000`, exactly as an enumeration's are; it is `bit_flag_mapping`, not the key type, that decides,
and `bit_fixed_set<N>`, keyed by the positions `0` to `N - 1` through `bit_key_mapping<std::size_t>`, is a different
set over the same `std::size_t`. `xstd::bit_mask` admits only the unsigned integer types, but [bitmask.types]/1
says "an integer type" without a sign, and an implementation picks one: the flag type also takes a signed `Mask`,
and keeps its sign bit out of the universe. `N` is at most
`std::numeric_limits<Mask>::digits`, which counts no sign bit, and defaults to it: 31 for `int`, 7 for
`std::int8_t`, every bit for an unsigned type. An enumeration on a signed underlying type is counted the same way,
so `int` and an `int`-based enumeration agree about bit 31. So every value a signed flag type gives back is
non-negative, and a value with the sign bit set is a value wider than `N`, below: it narrows under brace
initialization, `bit_flag_set<int>{0x8000'0000U}`, and fails the conversion's assertion in a constant expression and
at run time. The arithmetic is the unsigned counterpart's: a rank is its `countr_zero` and rank `i` is `1` shifted
left `i` places in that word, cast back, which is why the sign bit, the one position a shift there would reach, is
excluded rather than handled.

A bit set's order comes from bit positions, and a signed value order cannot match it once the sign bit is set:
`std::greater<E>` ranks a flag on the sign bit lowest, where the walk from the highest position visits it first, so
iteration, `lower_bound` and `<=>` would disagree with `std::set<E, std::greater<E>>`. Four ways to keep such a flag
were weighed and rejected, each costing machinery or a comparator users cannot name: rotating the storage by one
bit so that the sign flag ranks lowest, a cyclic walk starting at the sign bit, a signed index type for `to_index`,
and a position comparator in place of `std::less` and `std::greater`. The one real case found, POSIX's `sa_flags`
with glibc's `SA_RESETHAND` at `0x80000000`, is an `unsigned` constant stored in an `int` field: the word is `int`,
the flags are `unsigned`, and `bit_flag_set<unsigned>` keys it, with a cast on each side of the field.

The standard's own bitmask types are the case in point. [ios.base] declares `fmtflags`, `iostate` and `openmode`
as bitmask types and leaves the rest to the implementation: libstdc++ makes each an unscoped enumeration with no
fixed underlying type and the operators overloaded, libc++ `unsigned int`, and MSVC `int`, its constants of an
unscoped enumeration beside it. libstdc++'s enumerations take no default width, so the spelling for all three is
`bit_flag_set<std::ios_base::fmtflags, 16>`, every standard constant of every implementation below bit 16, and code
written against the stream carries over with only the variable's type changed:
`fmtflags f = os.flags(); f -= ios::basefield; f |= ios::hex; os.flags(f);`.

1. **Implicit conversions both ways.** Values from the standard's functions flow in,
   `xfs::perms p = fs::status(path).permissions();`, and ours flow out, `fs::permissions(path, p)`. The converting
   constructor takes any value of the mask, so a composite such as `fs::perms::owner_all` is an ordinary argument,
   and a braced list is the union of its values: `{m}` is the conversion from `m`, and `{a, b}` of two one-bit
   values is the set of both, which is what [set.cons] makes of it too.
2. **The mask's constants are the flag type's.** `fs::perms::owner_read | p` is a flag type, so the standard's
   spelling carries over unchanged and no constant is written twice. This is the split Qt's `QFlags<Enum>` makes, a
   set type over the enumeration that names its values, where Rust's `bitflags`, Python's `enum.Flag` and Swift's
   `OptionSet` make the element the type itself.
3. **The set reading.** Iteration yields the one-bit values, highest first; `contains`, `insert`, `erase`, `find`,
   the bounds, `rbegin`, `clear`, `swap`, the hash and `max_size()`, which is `N`, come with the adaptor.

The flag type is itself an `xstd::bit_mask`, a [bitmask.types] type with `~`, `|`, `&`, `^` and their compound
forms closed over it, which the tests assert of each instantiation.

Other libraries offer one of an enum set or a flag type, seldom both:

| Library | Enum set | Flag type | Element | Iterates? |
|---|---|---|---|---|
| xstd-bits | `bit_enum_set<E>`, keyed by rank | `bit_flag_set<Mask, N>`, over an enumeration, an integer or a `std::bitset` | the mask's one-bit values | both iterate |
| C++ standard | — (`std::bitset<N>` is indexed, not keyed) | bitmask types ([bitmask.types]: `fs::perms`, `ios_base::fmtflags`) | — | no |
| Qt | — | `QFlags<Enum>` | `Enum` | no |
| Chromium `base` | `EnumSet<E, Min, Max>` | — | — | yes |
| Rust | `enumset` crate | `bitflags` crate | `Self` | yes |
| Python | — (a plain `set` of `Enum` members) | `enum.Flag`, `enum.IntFlag` | `Self` | yes, named members only |
| Java | `EnumSet<E>`, keyed by ordinal | — | — | yes |
| Swift | — (a hashed `Set<E>`) | `OptionSet` | `Self` | no |
| C# | — (`HashSet<E>`) | `[Flags]` enum | — | no |

Java's `EnumSet` keys by ordinal, which is `bit_enum_set`; Qt's `QFlags` keys by the mask's own values, which is
`bit_flag_set`. Only Rust, through two crates, and this library offer both, and here both are the same set reading.

WG21 has worked on bitsets and bitmasks in pieces. Each paper below is cited where its point matters; here they are
together, with what each contributed and where this library takes it up:

| Paper | Year | What it proposed | Where this library meets it |
| :-- | :-- | :-- | :-- |
| [N2050](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2050.pdf) | 2006 | A dynamically sizeable bitset for TR2, after `boost::dynamic_bitset`, with set queries such as `is_subset_of` beside `push_back` and `resize`; libstdc++ still ships it as `std::tr2::dynamic_bitset`. | `bit_set` and `bit_vector`. The paper joins a set and a sequence in one type; here they are two readings of one storage. |
| [N3110](https://wg21.link/n3110) | 2010 | Problems with the library's bitmask types, `ios_base::fmtflags`, `iostate`, `openmode` and the `regex_constants` flags: operators declared as members, an `operator~` that was a no-op or recursed, an underlying type that should be fixed so that `~` stays in range, and the point that the standard must not require enumerations, since [bitmask.types] allows integers and `std::bitset` too. | Flag types over an enumeration, an integer or a `std::bitset`. `~` stays within `N` on the unsigned block, and an enumeration with no fixed underlying type spells its `N`. `bit_flag_set<std::ios_base::fmtflags, 16>` is one spelling across three standard libraries. |
| [N4202](https://wg21.link/n4202) | 2014 | *Strongly Typed Bitset*: a storage parameter `T` for `std::bitset`, `small_bitset<N>` over the least `uint_least*_t` holding `N`, `fast_bitset<N>`, and `underlying()`. | The `Block` parameter, `bit_least`, `bit_fast`, `from_blocks` and `bit_convert`. N4202 leaves the bits at or above `N` implementation-defined; here they are always zero, so `==`, hashing and `bit_cast` are exact. |
| [P0125R0](https://wg21.link/p0125r0) | 2015 | `std::bitset::is_subset_of` and `is_superset_of`, members that may short-circuit, the names N2050 used. | `xstd::bit_includes`, `bit_intersects` and `bit_disjoint`, free functions after `std::ranges::includes`. |
| [P3103R0](https://wg21.link/P3103R0), [R2](https://wg21.link/P3103R2) | 2024 | *More bitset operations*: `rotl`, `rotr`, `reverse`, `countl_zero(pos)` and `countr_zero(pos)` and their `one` forms, and `one()`. | `xstd::bit_rotate` and `bit_reverse`, in `std::rotate`'s direction. Iteration, `lower_bound` and `upper_bound` cover the positional counts. |
| [P4313R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4313r1.html) | 2026 | *Bitmask operations for enums*: `\|`, `&`, `^`, `~` and their compound forms for an enumeration annotated `[[=std::bitmask_type]]`, as operator templates in the global namespace, each returning the enumeration. | Complementary: P4313 gives the enumeration its operators, and `bit_flag_set` gives it the set reading, `contains`, `size`, iteration and the containments, which the paper leaves out. Its global-namespace operators are what this library never declares for a user's type ([the-adl-firewall](#the-adl-firewall)). |

An enumeration with P4313's operators is an `xstd::bit_mask`, and `bit_flag_set<E>` takes it unchanged, where lookup
finds the operators. `xstd::bit_mask` is a concept in `namespace xstd`, so its `a | b` sees what unqualified lookup
finds at the concept's definition and what argument-dependent lookup finds at the use. An enumeration in the global
namespace brings the global operators in by ADL; one in a named namespace finds them only if they were declared
before the concept, since ADL never searches the global namespace for it. P4313R1 puts the operators in the global
namespace and says nothing about an enumeration declared in a named one, so including `<bitmask>` after the xstd
headers would leave such an enumeration with operators its own callers find and no flag type. The tests emulate the
annotation with a trait and pin all three cases: global, named with the operators first, and named with them last.

**A value wider than `N` meets the flag type as the enumeration always has.** Its positions at or above `N` are
truncated by `&`, `-` with the flag type on the left, their compound forms and `==`, which finds such a value
unequal, and are precluded by an `assert` on the way in: the conversion, `|`, `^`, their compound forms and `-` with
the mask on the left. `xfs::perms(xstd::from_blocks, 0xFFFF)` takes the block as it is, `bitflags`'
`from_bits_retain`, and `xstd::bit_convert<std::uint16_t>(p)` reads it back; a block wider than `N` has its bits at or
above `N` dropped, as every fixed set's `from_blocks` does.

**A flag type orders as the mask it replaces, highest flag first.** The alias has no `Compare` parameter: it always
passes `std::greater<Mask>` to the fixed set, and over a descending set the adaptor's `<=>` is `numeric_three_way`, the block
compared as an unsigned number: the order the bitmask enumeration's own relational operators give, the order of a
bitset's `to_ulong()`, and the `Ord` that Rust's `bitflags` derives. It stays the container's lexicographic
comparison over its own iteration, since walking from the highest position down, the first difference is the
highest bit that differs. So iteration, `front()`, `back()` and `lower_bound` run from the highest flag down: for
`perms`, `set_uid` through `others_exec`, the order in which `ls -l` reads the mode. `std::greater<std::bitset<M>>`
is never called, a bitset having no `<`: the adaptor orders by position and only hands the comparator out through
`key_comp()`. A transparent comparator would turn on the heterogeneous `contains(K)`, and is not offered either.

**The mixed operators are generated, because hand-writing them goes wrong.** With only the homogeneous operators,
`p ^ std::filesystem::perms::owner_write` is ambiguous: ours wants a conversion on the right, the mask's own
`operator^` one on the left. The flag type declares `==`, `|`, `&`, `^` and `-` against the mask in both orders,
each an exact match for both operands, so it wins outright; `==` is one declaration, its reversed form being the
language's. Each is a hidden friend template whose mask parameter must deduce exactly as `Mask`, so that a flag
type, converting to the mask, never deduces as one, and each takes the flag type by value, an identity conversion
that beats the adaptor's key-typed operators, which reach the flag type through its base and would otherwise read a
multi-bit value as one key. The adaptor's operators that read an enumerator as the one key it is are constrained off
wherever the mapping is a mask mapping, so they never compete. The compound forms against the mask are hidden friends
taking the mask as it is, beside the adaptor's member set forms, which a mask cannot reach and so need no redeclaring.

**`contains(k)` is `std::set`'s membership of one flag.** A value of several bits, or of none, is no key, so it is no
element either: `contains` answers `false`, `count` zero, `find` `end()` and `erase` zero, and over an enumeration the
bounds place it by the mask's order. Writing one stays `bit_flag_mapping`'s precondition, there being no single position to write. All-of
is `xstd::bit_includes(p, m)`, any-of `xstd::bit_intersects(p, m)` and none-of `xstd::bit_disjoint(p, m)`: each takes its second argument
as `std::type_identity_t` of the first's type, outside deduction, so a mask converts there and the flag type declares
nothing of its own for them ([algorithms-not-members](#algorithms-not-members)). A mask never stands first, so
`m ⊇ p` is `xstd::bit_includes(xfs::perms(m), p)`, the conversion written out. There is no `operator[]` and no
proxy for a `bool`: a set changes one flag through `insert(k)` and `erase(k)`, as `std::set` does. There is no
nullary `count()` either: `size()` answers it.

**A user may still derive a class of their own** from `bit_flag_set` to add names, but its operators then return
the fixed set, as a class derived from any standard container's would.

Where it is not drop-in:

- **The variable's type changes.** `fs::perms p` becomes `xfs::perms p`; the constants, the functions and the rest
  of `fs::` are untouched.
- **It is a class, not an enumeration.** `std::to_underlying(p)` and `static_cast<unsigned>(p)` do not compile;
  `switch (p)` and `fs::perms(p)` do, through the conversion.
- **An integer mask meets only its own type.** The mixed operators deduce their mask parameter exactly, so an
  operand of another type, an `int` literal against a `std::uint8_t` mask or MSVC's enumerator constants against its
  `int` `fmtflags`, takes the built-in operator through the conversion, as the integer code it replaces did:
  `p | 1` is an `int`, and `p - 1` is a subtraction. The compound forms take any value that converts to `Mask`, so
  `p -= ios::basefield` is the set's on every implementation. A shift by an `int`, `p << 1`, is ambiguous between
  the set's shift and the integer's; `p << 1UZ` is the set's.
- **A standard function's result keeps the standard type.** `auto q = fs::status(path).permissions();` is a
  `std::filesystem::perms`, with none of the queries until it is assigned to `xfs::perms`.
- **Printing** needs a `formatter` for the mask, which a user may not specialize for `std::filesystem::perms`; a
  program-defined mask prints through its own, `{exec, read}`.

### a-name-by-storage

A `bit_sequence<C>` naming the sequence owner by the storage its blocks sit in -- `bit_sequence<std::uint64_t>`
for `basic_bit_array<std::uint64_t, 64>` -- is how itsy_bitsy's `bit_sequence<Container>` spells a sequence. As an
alias over a trait nothing deduces through it (`C` sits in a nested-name-specifier, a non-deduced context by
[temp.deduct.type]/5.1), and it gives the storage spelling to one reading of two. No public name here
spells a reading by its storage: every owner is named by its block and width, as `basic_bit_array<std::uint64_t, 64>`
is, and the storage is its base clause's business ([the-container-adaptor](#the-container-adaptor)).

### the-generated-table

Six cells over two adaptors over three storages is the shape where an inconsistency hides in one cell and
nowhere else, so what the compiler generates is a table, held by `test/src/bits/generated.cpp` rather than by
whichever cell was read last.

Every cell answers the same to all but one column: default-constructible, copyable, movable, `==`, `<=>`,
`swap` as both a member and a free function, and **nothing-throwing** in both move directions -- a move that
could throw would cost every growing container its strong guarantee.

The allocator is the exception, and it follows the **column, not the row**: the dynamic column allocates and both
of its readings answer `get_allocator`; the static column is a `std::array` and has none to show; the
bounded column holds its blocks inline and has none either. `boost::container::static_vector` does name an
`allocator_type`, one that holds the elements rather than allocating them, so a storage's allocator counts as one
only where it has `allocate`. So `bit_set` and `bit_vector` have
it and the other four do not, which is a fact about `std::vector` rather than about sets or sequences.

That is the table's use: a cell that answers differently from its column, such as a set owner without
`get_allocator` over storage that allocates, is an absence nobody decided, and reading across finds it.

`swap` is not merely `std::swappable`, which the implicit moves would satisfy on their own. It is the storage's
own exchange through `std::ranges::swap`, so the test checks that values actually move rather than only that
the expression compiles.

### the-bounded-column

The third storage point gets public names, one per reading and each a class like every other
owner: `basic_bit_bounded_set<Key, Block, N>` and `basic_bit_bounded_vector<Block, N>` over
`bounded_blocks<Block, num_blocks_v<Block, N>>`, with `bit_bounded_set<N>` and `bit_bounded_vector<N>` at the
machine word. `bounded` is one qualifier down each column rather than a second vocabulary for the same thing.

`N` is a **capacity** in bits here, where the static column's `N` is a width, and it is exact in the same way:
`basic_bit_bounded_vector<std::uint8_t, 9>` holds nine bits, as `std::inplace_vector<bool, 9>` does, in two
blocks whose last seven bits it never uses. `bit_block_container` reads its second parameter by storage -- the
width of fixed blocks, the capacity of blocks that resize under a constant one, and `dynamic_extent` for blocks that
grow without bound -- and defaults it to that, so a container named by its storage alone holds every bit of it:
`bit_block_container<bounded_blocks<std::uint8_t, 2>>` is the storage `basic_bit_bounded_vector<std::uint8_t, 16>`
wraps, one storage however it is spelled. The constant capacity is read without an object: from a `capacity()`
usable in a constant expression, as `std::inplace_vector`'s is, or from the `static_capacity` beside a static
`capacity()` callable only at run time, as `boost::container::static_vector`'s is. `boost::container::small_vector`
has a `static_capacity` too, but it is the inline part of a capacity that grows onto the heap, and its `capacity()`
needs an object, so an owner over it is unbounded by type.

The class passes `N` through to the storage rather than leaving it in the block count alone, which is what makes
distinct capacities distinct types and lets its guide name `N`: `num_blocks_v<Block, N>` is not a
deducible context, and no inverse exists, since every `N` from 9 to 16 names two `std::uint8_t` blocks. The
names carry the capacity-versus-width distinction and the parameter lists do not, which is the same hazard
`basic_bit_fixed_set<Key, Block, N>` and `basic_bit_bounded_set<Key, Block, N>` share by shape.

The column exists on every standard library, at C++23 and at C++26 alike. `detail/bounded_blocks.hpp` names its
storage `bounded_blocks<Block, K>`: `std::inplace_vector<Block, K>` where `__cpp_lib_inplace_vector` is defined, in
practice libstdc++ >= 16 at C++26, and `boost::container::static_vector<Block, K>` everywhere else -- MSVC,
libc++, and every library at C++23. Boost.Container is already a dependency, so the fallback costs a consumer
nothing new. Over `std::inplace_vector` the owners are constant-evaluable, and the header says so by defining
`XSTD_BITS_HAS_CONSTEXPR_BOUNDED` to 1; `static_vector`'s constructors are not `constexpr`, so over it the owners
are run-time values only. Code that constant-evaluates a bounded owner tests `#ifdef XSTD_BITS_HAS_CONSTEXPR_BOUNDED`,
never `__cpp_lib_inplace_vector`, which says what the standard library has rather than what the owners are built
on. Growth past the capacity throws `std::bad_alloc` over either storage, given by the container before the
storage is asked; on the set reading that is where `insert` stops being total. A left shift is not refused: it
translates every key by `n` and keeps those below `N`, the width growing to `min(width + n, N)` first, which the
blocks always have room for, so `operator<<=` is `noexcept` over this column. The width is no part of a set's
value, so a set whose width outran its keys -- an `insert(20)` undone by `erase(20)` -- shifts exactly as one at a
narrow width does, and a shift that keeps no key empties the set at the width it has. `insert` past `N` still
throws: it names one key the set cannot hold, where a shift is a set-wide operation with a rule for what it drops,
as `std::bitset`'s is. The unbounded column follows the same rule with `max_size()` for `N`
([width-is-capacity](#width-is-capacity)).

**The two storages promise different things**, and the owners pass the copies' promise on rather than paper over it.
`std::inplace_vector` of trivially copyable blocks copies and swaps without throwing; `boost::container::static_vector`
declares its copy constructor, its copy assignment and its `swap` potentially throwing, so over it the bounded
owners' copies are not `noexcept` either. Their `swap`, member and free, is `noexcept` over both, as
`[inplace.vector.modifiers]` declares it for `bool`. `bit_block_container` still calls the storage's own `swap` through
`std::ranges::swap`, and `swaps_without_throwing_v` names the one storage whose `swap` cannot throw without saying so:
`static_vector`, which moves unsigned blocks between two inline buffers of one capacity and never allocates. Every
other storage answers with its own specification. The moves are `noexcept` over both. Neither makes an owner
trivially copyable, `std::inplace_vector` included: a run-time width's move leaves its source at width zero, which a
trivial move could not. `generated.cpp` asserts each storage's answer, that the owner copies as its blocks do, and
that it swaps without throwing over either.

**A capacity of nought holds nothing.** `[inplace.vector.overview]/5` makes `inplace_vector<T, 0>` empty, trivially
copyable and trivially default constructible, and `static_vector<Block, 0>` is none of these: it keeps a size.
The bounded vector therefore holds `bounded_blocks_for<Block, num_blocks_v<Block, N>>`, which is `bounded_blocks`
itself except at nought where the library's own type is not empty, and `no_blocks<Block>` there. `bounded_blocks` stays a
plain alias, so the owners' deduction guides deduce its capacity; a `conditional_t` would make that a non-deduced
context. `no_blocks` is an empty contiguous range whose growth past nought throws `std::bad_alloc`, as
`std::inplace_vector<Block, 0>`'s does, and it goes with the fallback once every leg ships `<inplace_vector>`. A
`std::array<Block, 0>` would not do: its layout is the implementation's (one byte in libstdc++, a whole `Block` in
libc++, and an empty type in neither), and it has no growth members, which would turn the bounded owner into a fixed
one. Over a capacity of nought `bit_block_container` stores no width (`has_zero_capacity`), defaults its moves, and
takes the `bit_members` whose two members overlap and have no initializer; every adaptor holds the container
`[[no_unique_address]]` in its `adapted_bits` base, so `basic_bit_bounded_vector<Block, 0>` is an empty type, except
under the MSVC ABI, which gives a class whose members are all empty a byte of its own. There the vector's range constructor and both append tiers
refuse any element up front through one `refuse_any`, and the ordering never compares unequal widths. A loop that
cannot go round a second time is a branch no test can take, and being trivially destructible the owner compiles to
control flow no other capacity shares, so gcov counts that branch on its own; it is also code MSVC's C4702 calls
unreachable. The bounded set, bound by no such paragraph, holds `bounded_blocks<Block, 0>` at `N == 0`
rather than `no_blocks`, and reaches the same capacity of nought: no width is stored, and every growth past nought
throws `std::bad_alloc`. Over `no_blocks` the set would carry paths of its own that a width of nought never takes,
its padded equality and its bulk operators across two widths, each a branch no test can reach. The set's left shift takes an arm there, for the same reason the vector's ordering does: with no element
to hold, the rest of the body is a branch no test can take.

P0843 declined to repeat `vector<bool>`, so `std::inplace_vector<bool, N>` holds real `bool`s and is a model
only up to its reference. The `[inplace.vector]` clauses assert each declaration on it first where the standard
library has it, exactly as the `[vector]` ones do ([the-sequence-contract](#the-sequence-contract)); what the
packing adds, `hash`, is under `test/src/spec/xstd/inplace_vector/`.

**Testing both storages takes two standards.** `__cpp_lib_inplace_vector` is a C++26 macro, and the library asks
for C++23, where every library in the matrix holds the column in `boost::container::static_vector`. The two
owners' suites run the same cases at either standard; what the standard changes is the storage underneath, whether
`XSTD_CONSTEXPR_BOUNDED_CHECK_EQUAL` adds a `static_assert` to its run-time check, and whether the lines that hold
the owners to `std::inplace_vector` itself, the model, are compiled at all.

`XSTD_BITS_CXX_STANDARD` is how a build asks for more -- 23 by default, 26 to reach the `std::inplace_vector`
storage. It raises the standard for the tests and benchmarks only, deliberately: the `INTERFACE cxx_std_23` a
consumer inherits is the library's real requirement and must not move because one storage wants more. Verified at
GCC 16 with `-std=gnu++26`, and over `boost::container::static_vector` at C++23, where the column behaves as the table says -- regular, swappable by
member and free function, no allocator, and the two readings differing exactly where they should, the sequence
starting at width zero and resizing while the set's width is its capacity ([width-is-capacity](#width-is-capacity)).

What is still missing is CI. **CMake 3.28 cannot spell C++26 for GCC or Clang at all** -- not a GCC 16 gap, a
CMake one -- and 3.28 is this project's declared minimum, so the option fails on the toolchain the matrix
currently runs. It fails *legibly*: the configure step asks `CMAKE_CXX_COMPILE_FEATURES` what CMake actually
knows and says so, rather than letting a `try_compile` blame the compiler for CMake's ignorance. Reaching the
`std::inplace_vector` storage in CI needs a newer CMake on one leg, which is a change to the shared workflow rather than to this
repository.

`max_size()` is 24 on both names over `<24, std::uint8_t>`, this column being where the readings would part on
it without the rule they share ([max-size-is-the-bits](#max-size-is-the-bits)).
`capacity()` is the sequence reading's; the set has none, `std::set` having no `capacity()` and a
set growing by `insert` ([growth](#growth)), so there a capacity is felt at the throw and reported by
`max_size()`.

The column is graded over every block the other two are, `xstd::uint128` included, which it can be because the
width member carries its own alignment ([padding](#padding)). A 16-byte-aligned block after a `std::size_t` width
would pad the class, and `-Wpadded` under `-Werror` rejects that; the bounded column is the only cell that can
reach it, a static width carrying no width member at all and a `std::vector` over a stateless allocator being
aligned as a pointer whatever it holds.

### max-size-is-the-bits

`[container.reqmts]/57` asks for `distance(begin(), end())` for the largest possible container, and under every
reading of bits that counts the same thing: **the positions there are to hold**. The set reading iterates the
positions it holds, so its largest is every position set; the sequence reading iterates one `bool` per position.
There is no separate key domain -- a set over `[0, W)` holds at most `W`
elements because there are `W` positions, which is the same `W`.

What the two do **not** share is where that count stops, because their counterparts stop in different
places and each reading owes its own:

| reading | counterpart | where the count stops |
|---|---|---|
| set | `[set]`, which names no such bound | whole blocks the blocks hold and a `size_t` counts |
| sequence | `std::vector<bool>` | that, clamped to whole blocks no wider than `PTRDIFF_MAX` |

So `max_size()` is not one function with two callers. **The storage computes both** -- `max_size()` and
`addressable_max_size()` -- because both are made of `bits_per_block` and the block
container, which is knowledge only the storage has, and each reading returns the one its counterpart names. That
is what `bit_block_container` is for: the primitives are its, the contracts are the readings'.

The difference is observable, and is not small:

| over a 64-bit block | ours | the counterpart |
|---|---|---|
| `bit_vector::max_size()` | `(PTRDIFF_MAX / 64) * 64` | libstdc++'s `std::vector<bool>`: the same |
| `bit_set::max_size()` | `SIZE_MAX - 63` | -- |

The first row is the one the standard leaves open, and it is the one place a counterpart is a **specification**
rather than an implementation. `[container.reqmts]` only requires `max_size()` to bound what `resize` accepts, and
the two major implementations already disagree by sixty-three over a 64-bit word: libstdc++ answers
`(PTRDIFF_MAX / 64) * 64`, libc++ answers a bare `PTRDIFF_MAX`. There is no single number to match. Ours is the
rounded one on both, so `bit_vector` answers the same wherever it is built, which `std::vector<bool>` does not.

That is a **number** and not a behaviour. Measured at every
boundary of the sixty-three-size window, on both libraries -- `n0 = (PTRDIFF_MAX / 64) * 64`, then `n0 + 1`,
`n0 + 32`, `PTRDIFF_MAX`:

| `resize(n)` | libstdc++ `vector<bool>` | libc++ `vector<bool>` | `bit_vector`, either |
|---|---|---|---|
| `n0` | `bad_alloc` | `bad_alloc` | `bad_alloc` |
| `n0 + 1`, `n0 + 32`, `PTRDIFF_MAX` | `length_error` | `length_error` | `length_error` |

Nothing diverges. And libc++'s number is one libc++ cannot honour: it reports `PTRDIFF_MAX` and then throws
`length_error` on `resize(PTRDIFF_MAX)`, its own `max_size()`, where `[container.reqmts]` makes that the bound
`resize` accepts. Ours is the bound: `resize(max_size())` reaches the allocator. Matching libc++'s number would
**create** the divergence that is not there now -- measured with the ceiling removed, `resize(PTRDIFF_MAX)` is
`bad_alloc` where libc++ answers `length_error`.

So the test asserts the table row for row, asking **both** containers rather than claiming a value of one, and
asserting of the counterpart only what every implementation of it promises:

| row | asserted of `bit_vector` | asserted of `std::vector<bool>` |
|---|---|---|
| `max_size() <= PTRDIFF_MAX`, and ours `<=` theirs | yes | yes |
| `max_size() % 64 == 0` | yes | no -- libc++'s is not |
| `resize(max_size() + 1)` -> `length_error` | yes | yes, against its own |
| `resize(PTRDIFF_MAX + 1)` -> `length_error` | yes | yes |
| `resize(max_size())` -> `bad_alloc` | yes, off the sanitizer legs | no -- the two libraries disagree |

The "ours `<=` theirs" row is the one that orders them, and it holds of any block width rather than of a measured
pair: a bound rounded down to whole 64-bit blocks is the **smallest** such bound any block width can produce, and
`bit_vector` is the name whose block is a `size_t`, so ours is never the larger claim.

The last row asks for the memory instead of refusing, so the allocator answers -- and under AddressSanitizer that
answer is an **abort**, not an exception. Measured: `allocation-size-too-big`, and `allocator_may_return_null=1`
only renames it to `out-of-memory`, because the throwing `operator new` calls `ReportOutOfMemory` on a null
return rather than throwing. So that row is guarded on the sanitizer and on nothing else, which is as narrow as
the evidence allows: of the nine CI failures that led here, every one was a sanitized build or a discarded
temporary whose `new`/`delete` pair clang elided at `-O2`, and the msvc and mingw legs -- neither of those --
never failed on it at all. The rows use a live object for the same reason: an elidable temporary is what broke
clang, not the allocation.

The **source** of the answer still differs by what can grow:

| | `max_size()` |
|---|---|
| a width in the type | the storage's `extent` |
| an owner over growing storage | the storage's answer for that reading, in bits |
| the same, under a mapping that closes the universe | the smaller of that answer and the mapping's `size` |
| a view, a window, a static owner | its own width, which it cannot grow |

Nothing above the storage restates the arithmetic, and nothing above it should: a constant at the adaptor drifts
from the storage the moment the storage learns something: a set adaptor answering `SIZE_MAX - 1` over blocks
whose storage answers `SIZE_MAX - 63`.

The set's is `static` exactly where the type fixes it, at a static width or under a static capacity, and is then
the storage's `extent` or `static_capacity()` as a constant ([constant-sizes](#constant-sizes)). Elsewhere an owner
must ask its storage and a view must ask what it views, neither of which a static member can reach.
`std::set::max_size()` is not static at all.

### the-sum-that-wraps

A ceiling only holds if the number reaching it is the number that was meant. Every growth here computes the
width it asks for by **addition** over a `size_t` the caller names, and every one of those additions wraps:

| where | the sum |
|---|---|
| `bit_block_container::growing_insert` | `n + 1`, to admit the position |
| `set_adaptor::operator<<=` at a run-time width | `width + n`, the translation being total over `size_t`; capped at `max_size()` |
| `set_adaptor::insert_range`, consecutive tier | `lo + len - 1`, the range's last position |
| `sequence_adaptor::insert(position, n, value)` | `size() + n` |
| `sequence_adaptor::blit` | `size() + count`, the source's own width |
| `sequence_adaptor::pack` | `size() + ranges::size(rg)`, reserved ahead of the packing loop |

Not every wrapping sum is a growth. `exclusive_find_next(n)` steps to `n + 1` to scan from the position after
`n`, and at the top of `size_t` that step is zero, so a scan from there would restart at the beginning and answer
the first set position. Its callers refuse that sum before they step: the set reading's `upper_bound` asks for a
position within the width first.

A wrapped sum is **small**. It passes the ceiling it was meant to fail, and the operation then proceeds against
a width far below where it writes. A block count spelled `align_up(n, bits_per_block) / bits_per_block` makes that
concrete: it rounds a width near the top of `size_t` to **zero**, a one-block floor turns that into one block, and
the container answers `size()` with `SIZE_MAX` over sixty-four bits. `bit_set().insert(SIZE_MAX - 1)` reaches
exactly that, and not only in release -- an `assert(n < SIZE_MAX)` in `growing_insert` refuses one position of the
two that get there, `n + 1` being a width nothing can hold for every `n` above `max_size()` -- and under
`-fsanitize=address` the write that follows is a segfault at a high address.

Two rules, and the second is what keeps the first from being written six times:

**The block count cannot wrap**, and that is a spelling rather than a guard. `blocks_for` says what
`boost::dynamic_bitset::calc_num_blocks` says -- divide, then round up by the remainder -- which is total over
every `size_t`. The rounding spelling, `align_up(n, bits_per_block) / bits_per_block`, adds first, and for the
sixty-three widths above `max_width` that sum wraps to zero blocks which a floor then turns into one: a container
claiming `SIZE_MAX` positions in sixty-four bits. Only a ceiling would stand between that spelling and its own
arithmetic. This one has nothing to stand between.

`blocks_for` is **public** for that reason, and the claim is a `static_assert`:
`blocks_for(SIZE_MAX) == max_num_blocks + 1`, one block more than the widest whole number of them, where the
rounding spelling answers one. Asserting it by *growing* to such a width instead asks `std::allocator` for 2^61
bytes, and two of this tree's CI legs will not answer that question -- a sanitized build **aborts** on a request
that size (`allocation-size-too-big`) rather than reporting `bad_alloc`, and clang at `-O2` elides the
`new`/`delete` pair of a discarded temporary, so `(void)V(SIZE_MAX)` never allocates and never throws. Both were
measured on this tree, on exactly that assertion, across nine CI jobs. The compile-time form is the better one
anyway: it names the count the rounding spelling gets wrong rather than inferring it from an exception, and it holds
on every leg.

The **refusal** still wants looking at, and `std::inplace_vector` is where it can be: it is the one block
container here that refuses a width without asking anyone for memory, so the refusal arrives at a size a test can
name. `bad_alloc` and not `length_error` is what comes back from the storage, which asks no ceiling at a door of
its own.

**The ceiling is therefore a policy, and it belongs to the reading**, because the counterparts disagree about it.
Two ceilings are computed in the storage and two refusals are spelled there -- `check_width` above `max_width`,
`check_addressable_width` above `max_addressable_width`, both `std::length_error` -- and the storage asks neither
at any door of its own. The **set** reading asks `check_width` in `guard_key`: a key past the widest it could
ever grow to is the one thing `insert` on a dynamic extent can refuse. Its `operator<<=` asks neither, capping
`width + n` at `max_size()` and dropping the keys that land past it. The **sequence** reading
asks `check_addressable_width` -- at its width constructors, `resize`, `reserve`, and each of the three sums it
computes -- because `std::vector<bool>` throws `length_error` for a size it cannot represent, and what it cannot
represent is a distance, not a `size_t`.

Three growths name no width of their own -- `clear()` resizes to zero, `pop_back()` to one less, and
`grow_to_admit` to another storage's own width -- and each is reached from something that promises not to throw
(the sequence reading declares `clear()` and `pop_back()` `noexcept`; `grow_to_admit` is how every set
operation across widths widens, and the test tree's composable checks are `noexcept` over `|`, `&`, `-` and
`^`). They go through `resize_to`, the growth behind the door, which asks no ceiling, so what they are exposed to
is only `bad_alloc`, which no `noexcept` here promises against anyway.

The mirror of that rule is that a growth which **does** name a width keeps the ceiling, and whatever promises not
to throw above it is what has to give. `growing_insert` is the case: a key past `max_size()` is `length_error`,
which is the one way `insert` on a dynamic extent can refuse ([asking-is-total](#asking-is-total)). The four
composable checks in the test tree over `|`, `&`, `-` and `^` each build their expected value with `ranges::to`,
which inserts, so they carry no `noexcept`, which would be a promise about a reachable exception, and the test
factory carries none over `resize`. The inclusion check beside them, `xstd::bit_includes` asked both ways round,
constructs nothing and is `noexcept`. The narrower ceiling stays the blocks' own: `m_blocks.resize` and
`m_blocks.reserve` are handed a count and answer for it, which is why `resize(max_width)` is `bad_alloc` and
`resize(max_width + 1)` is `length_error`. `resize` takes that count **before** it writes the last block, so a
refused growth leaves the width and the bits exactly as they were.

That holds of `resize(n, true)` only because of the order of its steps, which the `inplace_vector` test above
checks. Growing with ones sets the tail above `size()` in the current last block, those bits being the first new
ones; set before the blocks grow, a refused growth would return with the width unchanged and the tail dirty, which
is the class invariant broken rather than a partial growth: measured on a storage refused at 25, the next
`resize(20)` comes back with every bit above 9 set and `count()` at 8 instead of 1. Which block and which bits is
read off the **old** width, so it is taken before the growth; the write itself comes after, where nothing can
throw between it and `m_size`.

**The addition is `width_sum`.** `base + count` where that is a width, and the top of `size_t` where it is not.
Saturating rather than throwing keeps the rule at one throw site: a saturated width is one `blocks_for` already
refuses, so the readings above hand it their sum and inherit the diagnostic without naming a ceiling of their
own -- which is the same reason `max_size()` is not restated above the storage.

Not every sum here was worth a check. `append(block_type)` raises the width by `bits_per_block` and `capacity()`
multiplies the blocks' by it; both overflow only once the blocks already hold some `2^64` bits, which the
allocation fails long before, so a branch there is one no test can reach and the coverage gate would be right to
say so.

`sequence_adaptor::pack` reserves ahead and is **not** left alone, because of what its count is bounded by.
`pack` is handed a range of `bool`, and a sized range answers `size()` for elements it never materializes: `d.append_range(views::iota(0UZ, SIZE_MAX))`
is one call, and `size() + SIZE_MAX` wraps for every `d` that is not empty. Wrapped, the reserve asks for
nothing and returns, and the packing loop then walks a range of `2^64` elements a block at a time -- so the
`length_error` the same call answers **immediately** on an empty sequence becomes, one element in, a call that
ends when the allocator gives out rather than when the range does. Saturated it is that `length_error` at
both.

**Where the counterparts stand**, measured rather than assumed -- Boost 1.83, libstdc++ 14, `-O2 -DNDEBUG
-fsanitize=address,undefined`:

| | growth at `SIZE_MAX` | a ranged write past the width |
|---|---|---|
| `boost::dynamic_bitset` | `bad_alloc` | assert in debug, **heap-buffer-overflow** under `NDEBUG` |
| `std::vector<bool>` | `length_error` from `resize`, `reserve` and `insert`; the **fill constructor corrupts the heap** | no ranged form |
| `std::bitset` | no growth | no ranged form; the single-position members throw `out_of_range` |

The split is exact, and it says which of the two rules above a packed container can get wrong.

The **ceiling** is, and the answer is not to need one.
`boost::dynamic_bitset::calc_num_blocks` is `n / bits_per_block + (n % bits_per_block != 0)` -- a division that
cannot overflow -- which is why `dynamic_bitset(SIZE_MAX)` reaches the allocator and answers `bad_alloc`, and
`blocks_for` says it the same way. The rounding spelling, `align_up(n, bits_per_block) / bits_per_block`, is
exactly libstdc++'s `_S_nword(n) = (n + word_bit - 1) / word_bit`, which its `vector<bool>` fill constructor calls
with no `max_size()` check: `std::vector<bool> v(SIZE_MAX)` there constructs, answers `size()` with `SIZE_MAX` over
a `capacity()` of **zero**, and aborts on the first write. `resize`, `reserve` and `insert` all check and throw
`length_error`; only the constructor does not. So the hazard has `vector<bool>`'s shape, not boost's, and a
constructor is the unchecked way in.

### the-invariant-stays-in-the-storage

The padding invariant could live one level up, each adaptor restoring it after the operations that dirty it,
and it does not. Two reasons, one measured and one structural.

**Measured**, the prize is one masked store. `operator<<=` at a run-time width erases where the set reading
has already grown past anything the shift can reach ([the-set-operations-across-widths](#the-set-operations-across-widths))
unless the growth stopped at `max_size()`, so the call is dead in that path short of it. Removing it, GCC 14, `-O3 -march=native`, best of twenty-five:

| width | with | without |
|---:|---:|---:|
| 64 | 4.18 ns | 3.44 ns |
| 256 | 7.75 ns | 7.67 ns |
| 4096 | 22.50 ns | 22.45 ns |
| 65536 | 311.08 ns | 311.72 ns |

0.74 ns at one block and nothing beyond it, because the erase touches one block where the shift touches all
of them.

**Structural**, and the reason that decides it: the invariant is not a reading's operation, so the ceiling of
[what-a-sequence-may-add](#what-a-sequence-may-add) does not reach it. It exists to serve the storage's own
blockwise reads -- `operator==`, `count`, `all`, `any`, `none` and `first_difference` are every one of them
storage members that read a whole block and would have to mask without it, `xstd::bit_includes` and
`xstd::bit_intersects` read whole blocks the same way, and `set_equal` and the orderings are built on them. Moving the restoration up would put the obligation in the adaptors and the
dependency in one storage, and a missed call would be silent. Three dirtying operations across two adaptors
is six places to be right instead of four, to save a masked store on a one-block set.

That is also what the three standard libraries disagree about, and the disagreement is a real choice rather
than an oversight: a tail invariant is needed only where blockwise reads are **unmasked**. libstdc++'s
`vector<bool>` reads element-wise and keeps no invariant, so `flip()` dirties the whole allocation and nothing
notices. libc++'s reads blocks and masks at every read site. This tree reads blocks unmasked and masks at the
four writes that can dirty them.

### the-kept-count

A set's `size()` is its storage's `count()`. At a static width, `bit_fixed_set<N>` and `bit_bounded_set<N>`, that is
a popcount bounded by `N`: one or two instructions at a board's width, constant in the number of elements as
`[container.reqmts]` measures it, and a kept count would charge every write a read and an add to save them. Where
the width grows without bound, `bit_set` and `bit_small_set`, the popcount runs over every block the set has ever
grown to, so a megabit set holding one key pays thousands of popcounts per `size()`, and per `empty()` with it.
Those two keep the count as a member: `set_bits_t<Blocks>` is `bit_block_container<Blocks, N, true>` exactly where
`unbounded_width<Blocks, N>` holds. `bit_vector` holds the same blocks without it, its `size()` being its width.

Only the count is kept, not where the keys are. `begin()` and every step of an iterator scan a block at a time for
the next key, so a step costs one plus the empty blocks it skips, and those skips add up over a whole traversal to the
blocks once: a loop from `begin()` to `end()` costs O(width / w + `size()`), the scan `boost::dynamic_bitset`'s
`find_next` and Java's `BitSet.nextSetBit` make too. At a static width that is bounded by `N`, and so constant as the
standard counts. At a growing width it is the departure: `[iterator.requirements.general]` asks for amortized
constant steps, which a dense set meets and a sparse one does not, and the width scanned is the largest the set has
grown to, since neither `erase` nor `clear()` shrinks it ([width-is-capacity](#width-is-capacity)). No cheap
bookkeeping closes the gap. A kept first block would make `begin()` constant for a word and a branch in every
writer, and leave every `++` scanning as before; only tracking every key, which is the representation this library
exists to avoid, would make the steps constant. `size()` and `empty()` are different: a loop condition calls them
once per iteration, and keeping them costs an add per write.

The count is the storage's for the reason the padding invariant is
([the-invariant-stays-in-the-storage](#the-invariant-stays-in-the-storage)): a `bit_set_view` over a `bit_set`
writes through the owner's storage and never through the owner, so a count the owner held would go stale at the
first insert through a view. `set_view_storage_t` maps a view over unbounded blocks onto the counted storage its
owner holds. Every write keeps the count:

- a single bit by the bit it read, which `insert` and `erase` already return;
- a range, `set(n, len, value)`, by the range's popcount taken before the write;
- the shifts by the positions they drop, and `resize` by the positions that leave and arrive, applied once the
  growth has succeeded, so a refused allocation leaves the count with the blocks it describes;
- `&=`, `|=`, `^=` and `-=` by a popcount summed in the loop that writes the blocks, so a set operation stays one
  pass, and `transform` the same way, less what `erase_unused` is about to clear, which only the last block holds;
- `set()`, `reset()` and `flip()` by arithmetic on the width, and a rotation or a reversal not at all, a
  permutation of the bits leaving their number as it was.

A writer of raw blocks, through `blocks()` or the subscript, owes `recount()` exactly as it owes `erase_unused()`;
`bit_convert` into a run-time owner is the one such writer in the tree. The count is eager. A flag set by the bulk
writes and cleared by a recount in `size()` would make a `const` member write, which `[res.on.data.races]` forbids
without synchronization, as reference-counted copy-on-write strings found. In a debug build `count()` asserts the
kept count against the popcount, so every test that asks a size also checks the invariant.

### width-is-capacity

`std::set` has no width, so `bit_set` treats its run-time width as capacity, never as value: two sets holding
the same positions are equal whatever their storages' widths, and the width neither orders, nor hashes, nor is a
precondition of the set operations.

**`operator~` is the one operation that cannot honour this, so it is not offered at a run-time width.**
Complementing needs a universe, and capacity is not one. Two `bit_set`s holding `{1, 3}` are equal whatever
their storages grew to, but complementing reads the width, so the results are not:

```
a == b       : true          a, b both { 1 3 }
~a size      : 2             a's storage never grew
~b size      : 199           b's had admitted 200 once
~a == ~b     : false
```

`~` is then not a function of the set's value, which is a stronger objection than an inconvenience: equal
inputs, unequal outputs. At a **static** width there is no such gap, because `N` is part of the type and
equal sets share it, so `operator~` is constrained on `has_static_width` and `bit_fixed_set<N>` keeps it.
`bit_set` keeps toggling one key, which needs no universe -- an enumerator's `^=`, or `contains` and then `erase` or
`insert` -- along with the four set operators. The alternative -- letting `~` read the width --
would make the width value for exactly one operation, which is the thing this section says the set reading
does not do.

`bit_block_container`'s `operator==` cannot say that: it is width first, which is what the sequence reading
means, as `std::vector<bool>` and `boost::dynamic_bitset` do. The width is part of the value there -- a
`vector<bool>` of two elements is not one of three -- and it is not part of the value for a set. So the set
reading gets an entry of its own, `set_equal`, beside the `operator==` the sequence reading keeps, for the same
reason `set_three_way` sits beside `sequence_three_way`.

**Two spellings for two meanings, and it does not come out as evenly as the orderings.** Ordering has two
meanings and no structural answer at all -- a defaulted `<=>` would order by `m_size` first, which no reading
means -- so the storage declares none and names both, and nothing is left over. Equality has two meanings
and one of them *is* the structural answer: memberwise, width then blocks, exactly what `= default` produces
and exactly what `std::regular` asks of a storage. the common vocabulary requires that
([the-common-vocabulary](#the-common-vocabulary)), and it is not an accident of the concept: `std::bitset` and
`boost::dynamic_bitset` both have `==` and both mean width first by it. So the count is one operator and one
name rather than two names, and giving the structural meaning a second name would be three spellings for two
meanings.

Both are non-members: the defaulted `operator==` is the storage's hidden friend, and `set_equal` a free
function beside the orderings, equality asking about two values with neither as its subject exactly as an
ordering does. What remains asymmetric is only that one of the two meanings gets to keep the operator.

At a static width the distinction is unobservable, every instance carrying the one width, which is why the set
adaptor can default `==` there and nowhere else.

The **storage** answers at any two widths, and the adaptor calls it or the comparisons built on it. `set_equal`,
`set_three_way`, `xstd::bit_includes` and `xstd::bit_intersects` each carry their own width-crossing arm, so the read operations
in `set_adaptor`, and `xstd::bit_includes`, `xstd::bit_intersects` and `xstd::bit_disjoint` over it, are calls with no `same_width` test
between them -- only the four compound
operators still ask, because they mutate. The logic belongs where the blocks and the invariant are, and putting
it there is also what keeps the adaptors alike, where `sequence_adaptor` had nothing at all. At two run-time widths that differ, `==`,
`bit_includes` and `bit_intersects` all ask whole **blocks** rather than walking positions. Only the blocks both
storages have can disagree; above them the answer is the invariant, capacity holding no element. So `==` wants
the shared blocks equal and the longer one's remainder clear, `bit_includes(a, b)` wants each of `b`'s shared blocks
inside `a`'s and nothing of `b`'s above `a`'s last block, and `bit_intersects` is the negation of every
shared pair being disjoint -- their blocks above ours never need a look. Positions were the obvious spelling and
the wrong one, a walk over the elements being a `find_next` per position where this is one load per sixty-four.
Measured over two thousand elements held at two different run-time widths, each in the case that denies the
element walk its early exit, with the inputs made opaque to the optimizer so the call is not hoisted out of the
timing loop:

| | positions | blocks |
| --- | --- | --- |
| `==`, equal | 11.04us | 0.07us |
| `bit_includes`, a subset | 9.39us | 0.05us |
| `bit_intersects`, disjoint | 10.35us | 0.05us |

The padding above `size()` being zero is what lets a whole block stand in for the positions it holds, which is
the same invariant the orderings already rest on. The shared prefix needs no index arithmetic: `std::views::zip` stops at the
shorter range, which is exactly the blocks both storages have, and the blocks past it are a separate question
asked of one storage alone. Only the ordering needs a block one storage may not have, and `padded_block` reads
those as zero -- not a convention but the same invariant one block further out.

The same gate is why each of the three arms sits under `if constexpr (not has_static_width)` rather than the
plain `if` that `same_width` would fold anyway. Folding is not enough: a static width still *instantiates* the
arm, and `blocks_agree` carries a lambda no other call site shares, so those instantiations are reachable from
no test and their branches are uncovered by construction -- the same hazard the coverage job's own
`XSTD_BITS_BUILD_BENCHMARKS=OFF` exists to avoid. A translation unit naming only `bit_fixed_set` instantiates
thirty such functions with a plain `if` and none with the guard.

`<=>` is blockwise for the same reason and by a different route. The set ordering is lexicographic over the
ascending positions, and lexicographic order over two sets is decided by exactly **one** position: the lowest at
which they disagree. Whoever lacks it is less -- but for two different reasons, and the second is the one worth
naming. Usually it holds a larger element there. When it holds nothing above that position at all, its positions
are a proper *prefix* of the other's, and it is less because it runs out rather than because it compares
smaller. So the comparison is a search for the lowest differing block and a single look above it:
`padded_first_difference`, then `padded_any_above` on whichever side lacks the position. Those are the
storage's own `first_difference` and `any_above` with the index bound dropped, and `set_three_way` dispatches to
them through `padded_set_three_way` when the widths differ, so the generalisation lives beside the algorithm it
generalises rather than in the adaptor calling it. Measured as above, at two
different run-time widths: 10.62us to 0.09us over equal sets, and 11.03us to 0.06us where one set is a proper
prefix of the other.

`|=` `&=` `^=` `-=` are blockwise too, and
[the-set-operations-across-widths](#the-set-operations-across-widths) is what it took to mutate rather than
merely read. The shifts translate the set: `<<=` grows the width towards `max_size()` and keeps the keys that
land below it, and `>>=` empties past the width. Hashing appends the positions and the count at a run-time width and the bits at a static one, where equal
sets share a width ([the-hashing-invariant](#the-hashing-invariant)).

**A run-time width is storage to a set.** `boost::dynamic_bitset`'s width is part of the value, so `==`
includes it, and the width changes only when the caller asks; `xstd::bit_set`
and `xstd::bit_bounded_set` are the set reading, and their width is storage: operations grow it as they need --
`insert`, `|`, and `<<=` up to `max_size()` -- invisibly, since growth changes nothing observable. Hence the
shift's rule, grow and then truncate at `max_size()`: that is the one bound fixed by the type rather than by how
the set happened to be stored, and truncating at the current width would give two equal sets two different
results. `insert` past `max_size()` still throws, since it names one key the set cannot hold, where a shift is a
set-wide operation with a rule for what it drops, as `std::bitset`'s is. At a static width the width and
`max_size()` are both `N`, so `bit_fixed_set<N>` shifts as `std::bitset<N>` does. A mapping that closes the
universe caps a growing set's `max_size()` at its `size`, so the shift keeps no position that names no key.

| | who sets the width | operations change it | part of the value | a left shift truncates at |
| --- | --- | --- | --- | --- |
| `boost::dynamic_bitset` | the caller alone | no | yes | `size()` |
| `bit_set`, `bit_bounded_set` | the operations, as they need | yes, growing | no | `max_size()` |
| `std::bitset<N>`, `bit_fixed_set<N>` | the type | no | the type's | `N`, which is both |

### the-set-operations-across-widths

The four compound operators work a block at a time across two widths as the read-only operations do, with one
question those never have to answer: they mutate, and two of them may have to **grow**.

Measured at two run-time widths, 4096 against 4000, dense, element by element and a block at a time:

| | elements | blocks |
| --- | --- | --- |
| `&=` | 12.12us | 0.22us |
| `\|=` | 9.41us | 0.22us |
| `^=` | 9.58us | 0.22us |
| `-=` | 10.27us | 0.22us |

**They keep the operator spelling**, and that is the point worth stating, because `set_equal` and
`set_three_way` do not. A name is owed where the readings genuinely *disagree*: two orderings over one
storage, and two equalities ([two-readings-disagree](#two-readings-disagree)). `&=` `|=` `^=` `-=` are not
that. Both readings mean the same bitwise thing by them, and a precondition of equal widths on the storage's
operators would be the only difference, where the set reading wants an answer. A precondition is not a second
meaning: an operator that answers where it could assert takes nothing away from the readings that never ask,
since what they pass is always equal-width. So the operators themselves are total, under no new names, and no
`same_width` test chooses between the storage's operator and an element walk.

What the storage's operator deliberately does *not* do is grow. Growth is the set reading's rule about
capacity, so it lives with the reading that has it: `set_adaptor`'s `|=` and `^=` call `grow_to_admit` and then
the operator, and its `&=` and `-=` are the operator alone. That is the adaptor adding a guard, which is all an
adaptor should be doing; the operator stays reading-neutral, padding with the zero the invariant already keeps
and never widening what it was handed.

**Intersection and difference never widen.** A position the other lacks is a position it does not hold, so the
missing blocks read as the zero they already are and the result fits where it already sat. Both only ever
*clear* bits, so the invariant that padding above `size()` is clear survives with no `erase_unused` to restore
it.

**Union and symmetric difference do widen, and the target is not the obvious one.** This is the part that is
the set reading's alone, and the reason `grow_to_admit` sits at the call rather than inside the operator.
Growing to the other operand's `size()` would be wrong. `growing_insert(n)` resizes to `n + 1`, so inserting the other's elements one
at a time arrives at one past its **largest element** -- and a storage far wider than anything it holds must not
drag this one up with it. A 301-wide operand holding nothing above 7 widens a 61-wide set not at all; the same
operand holding 280 widens it to 281, never to 301. `grow_to_admit` is that rule and nothing else, and it
returns early on an empty operand, where there is no largest element to ask for and `exclusive_find_prev` would
assert.

Each operator keeps an equal-width fast path: at a static width that is the whole function, the unrolled one-
and two-block arms included, and at a run-time width it skips a per-block bound check. `grow_to_admit` returns
at once on an empty operand and does nothing at all at a static width, where there is neither anything to widen
nor another width to meet. Measured at 4096 against 4096, the equal-width case is unchanged.

**The width is the part that needs a way to see it.** An owning set reports `max_size()` as everything it could
grow to rather than what it currently spans ([max-size-is-the-bits](#max-size-is-the-bits)), so the growth rule
above is invisible from the owner. A *view* over the same storage reports the storage's own `size()`, which is
the width, and that is what the test asserts through, over 577,600 width pairs; comparing `max_size()` against
`max_size()` would prove only the elements, and every width claim would be vacuous on both sides.


### an-opinionated-reimagining

The charter is that each container is the **packing** of a standard one and speaks that container's vocabulary,
the way `std::flat_set` speaks `std::set`'s: keeping what time has proven effective, and throwing out what is
not. The counterpart's synopsis, asserted declaration by declaration, is how the keeping is enforced
([the-sequence-contract](#the-sequence-contract)). This is the other half, and the two are not in tension: the
synopsis governs what the containers **do**, and it says nothing about where an operator is declared.

`std::bitset` is the clearest case of what has not proven effective, and is why this library has no bitset of its
own ([interop-not-a-bitset](#interop-not-a-bitset)). It is the oldest type in the library's neighbourhood and
reads like it: a member `operator==` where every container has a non-member one, no `swap` at all, and
`operator<<` and `operator>>` as members beside `&`, `|` and `^` as non-members. C++20 changed none of it. On
where operators sit the tree follows the ordinary guidance instead: the operand a mutator belongs to keeps its
member -- `flip`, `<<=`, `&=` -- and the operators that make a new value do not. `==`, `<=>` and `swap` are
hidden friends; the shifts and `~` sit at namespace scope beside `&`, `|`, `^` and `-`.

The rule is **hide where hiding is free, and never where it widens**, with `friend` doing two separable jobs.

Only `==` and `<=>` need friendship for what it says: a defaulted `==` may be a member or a friend and nothing
else ([class.compare.default]/1), and `<=>` reads the storage. `swap`, the shifts and `~` need no access at
all -- their bodies are `x.swap(y)`, `nrv <<= pos`, `nrv.flip()`, every one of them public -- and the standard's
own free `swap` is a namespace-scope template for exactly that reason. `swap` is a friend anyway.

For `swap` the hiding earns its keep. A qualified `swap` is a mistake people make *by accident*: the habit of
writing `std::swap(a, b)` transfers, `xstd::swap(a, b)` looks equally reasonable, and it silently defeats the
customization the two-step `using std::swap; swap(a, b)` exists to find. A hidden friend has no qualified name,
so the accident cannot be spelled. That is a Murphy guard, which is the kind this tree undertakes.

The keyword is doing only that. `swap`'s body is `x.swap(y)` and reaches nothing private, so it is a `friend`
that wants no friendship -- and there is no other spelling for what it wants. A function declared at namespace
scope is always reachable by ordinary lookup; C++ offers no "namespace scope, unqualified only". So a hidden
friend is the sole mechanism for ADL-only lookup, and using it here overloads a keyword that says *access* to
mean *placement*. Reading `friend` in this tree, check the body before assuming it needs one.

The shifts and `~` are **not** hidden, and that is the same rule reaching the other answer. Nobody calls an
operator qualified -- `xstd::operator<<(s, 3)` is not a thing anyone writes by accident or otherwise -- so
hiding would foreclose nothing that was going to happen, and claiming a hazard there would be a
rationalisation. Guarding against Machiavelli is not this tree's business. So they are namespace-scope
templates beside `&`, `|`, `^` and `-`, which is where `set_adaptor` has its whole set.

**Which leaves three that deviate from the standard containers**, and only one of them by choice.
`std::vector` and `std::set` spell `==`, `<=>` and `swap` as namespace-scope templates; here all three are
hidden friends.

`==` is forced: **it is defaulted** where it can be, and [class.compare.default]/1 admits a defaulted comparison
only as a non-static member or a friend. A namespace-scope template cannot be defaulted at all. The standard
containers hand-write theirs, which is what leaves them free to put it at namespace scope; wanting `= default` is
what takes that option away here, and it is a want worth having -- the storage is the one member, and a
comparison nobody writes is a comparison nobody gets wrong.

`<=>` needs the access. It reads the storage, so a namespace-scope template would have to be granted friendship
anyway, and then it is a friend that is not hidden -- the worst of both.

`swap` is the one deviation that is a choice, and the Murphy guard above is the reason.

What this costs is not much: the address of an operator, which nothing takes and which is a use worth
discouraging in any case -- an operator is meant to be found by the grammar, not by name. What it buys is one
shape to learn instead of one per type.

### interop-not-a-bitset

**The library has no bitset of its own.** `std::bitset` predates the STL, and its interface is three at once: a
set's (`&`, `|`, `^`, `count`), a sequence's (`operator[]`, `test`, `set`, `reset` and `flip` by position) and a
string's (`to_string` and the string constructors). The grid gives each of those its own reading, with one
vocabulary across every storage -- the set reading in `bit_fixed_set`, `bit_bounded_set`, `bit_set` and
`bit_small_set`, the sequence reading in `bit_array`, `bit_bounded_vector`, `bit_vector` and `bit_small_vector`,
and a string reading planned as `bit_string`. A bitset here would be a fourth interface over the same blocks, with
nothing to say that one of those does not say better, and with a contract owed to two counterparts that disagree
with each other about growth, element access and order.

It interoperates instead, through one function for every pair of widths, **`xstd::bit_convert<To>(from)`** in
`<xstd/bits/bit/bit_convert.hpp>`. Its constraint is a rule table over the two types as declared, kept in
`detail` as `std::convertible_to` keeps `is_convertible_v` beneath it. The public concepts are two:
`xstd::bit_convertible_to<From, To>` says the call is valid, taking the source first as `std::convertible_to` does
so that `bit_convertible_to<To> auto` constrains a source, and `xstd::bit_constructible_from<To, Blocks>` says that
`Blocks` *is* bit storage `To` takes as it is through the tag ([is-and-has](#is-and-has)). A public concept for the
rule table as well would accept exactly the types `bit_convertible_to` does, under a second name. Each of the two
has its own header under `<xstd/bits/bit_concepts/>`.

- **Its two ends.** A target is any owner of either reading at any width and block width, an unsigned integer, a
  `std::array` of blocks, or a `std::bitset<N>`; never a view, which would write bits it does not own. A source is
  any of those, or a view over a whole width; a window is neither, its position zero not being its storage's.
- **Positions.** Position `i` of the source is position `i` of the target, across the readings and across block
  widths. That is the whole contract, and the name says so: this is a conversion of positions, not a
  reinterpretation of an object.
- **Two fixed widths** widen and never narrow. A source `M` positions wide converts into a target `N >= M` wide,
  position `i` to position `i` and the positions `[M, N)` clear, so `bit_convert<std::uint8_t>(bit_array<5>)` is
  what `to_ulong` gives for a `std::bitset<5>` of the same positions, and a `bit_fixed_set<100>` is an
  `xstd::uint128` or a `std::array<std::uint64_t, 2>`. Into a narrower target there is no conversion at all and the
  call does not compile: both types state their widths, so a truncation is a mistake to report where it is written,
  and narrowing is a checked conversion only from a run-time width, below. Nothing truncates, so nothing can fail:
  the call is `noexcept`, and the round trip, back through a run-time width or by `from_blocks` of the low
  positions, is the identity. Widening means one thing in both readings, position `i` staying `i`: a set's fixed
  width is a capacity, so it gains positions no key names yet, and a sequence gains `false` elements.
  `std::bitset<N>` crosses this way, read on `std::bit_cast`'s terms ([the-bytes-they-agree-on](#the-bytes-they-agree-on)):
  `bit_convert<xstd::bit_fixed_set<N>>(b)` is the set reading of a `std::bitset`, and `bit_convert<std::bitset<N>>(s)`
  hands one back. The set and sequence readings cross the same way, directly, with no bitset between them.
- **A run-time width into a fixed one** goes by value, as `std::bitset::to_ulong` and Boost's `to_number` do: a
  shorter source zero-extends, and a set position at or past the target's width throws `std::overflow_error`. So
  `bit_set{3}`, one 64-bit block wide, converts to a `bit_fixed_set<10>`, and `bit_set{3, 64}` to no
  `std::uint64_t`.
- **Into a run-time width**, a sequence target takes the source's width: a sequence's `size()`, and a set's
  universe, which is its whole blocks up to what it can hold -- `bit_set{3, 64, 129}` over 64-bit blocks is a
  `bit_vector` of 192, and a `bit_fixed_set<100>` one of 100. A set target covers that width in whole blocks of its
  own, so a `bit_vector` of 70 becomes a set over 128 positions in 64-bit blocks and over 72 in 8-bit ones. The
  bits above the width are clear afterwards, as everywhere ([padding](#padding)). A bounded target too small throws
  what its own growth throws, `std::bad_alloc`, before a block is written: a sequence target whose capacity is
  short of the source's width, and a set target, capped at its capacity rather than rounded past it, for a key at
  or beyond that capacity. A set never throws over padding: a wide universe whose keys all fit converts.
- **Blocks.** From a non-const rvalue whose extracted container is `bit_constructible_from` the target -- its
  allocator with it, being part of that type -- under a target whose capacity is the whole of that container, the
  blocks move: `extract()` into the adopting constructor, O(1), the source left at width zero. The capacity clause
  leaves out a `bit_bounded_vector<100>`, whose `inplace_vector` of two 64-bit blocks holds 128 bits the owner may
  not, and those blocks are inline, so a copy is all a move would have been. Everything else copies: the object
  bytes in one call, as `memcpy` would, where both block types are padding-free built-ins and the target is little-endian
  outside a constant expression, and byte shifts everywhere else. All of it is `constexpr`.
- **`noexcept`** holds exactly where both ends are fixed widths. A run-time end can allocate, refuse a bounded
  capacity, or throw `std::overflow_error` into a fixed target.

Position `i` staying position `i` is the one thing to watch with a `std::bitset` or a `boost::dynamic_bitset` as
the source: their `to_string()` writes position 0 last, so its text reads reversed against a string of the
positions written first to last.

`boost::dynamic_bitset<Block, AllocatorOrContainer>` crosses both ways through the same name, from
`<xstd/bits/ext/boost/dynamic_bitset.hpp>`, a header of its own so that Boost.DynamicBitset is a dependency only of
code that includes it (the `dynamic-bitset` feature in `vcpkg.json`). As a source it is a sequence of its
`size()`, read through `to_block_range`; as a target it takes the source's width and is written through
`from_block_range`, from any of our owners or a `std::bitset`. Both directions copy. Boost 1.92 hands its buffer
out nowhere -- the container is a private member, and no member, friend or constructor moves it in or out -- so
there is nothing to adopt even where its container is exactly ours.

### asking-is-total

Asking is total whatever the extent: a position past the width is a key the set does not hold, which is an
answer and not a precondition violation. That is what `[set]` gives `contains` and `find` — `s.find(k)`
returns `end()` for any `k` it does not hold, never refuses the question — and it is the difference between
the set reading and the sequence reading, where `sequence_adaptor::operator[]` indexes and out of range is
out of bounds.

**Writing is not total**, and that is the whole of the asymmetry. `insert` carries no `noexcept`, for the reason
`std::set::insert` carries none: growing a dynamic extent allocates. It is the one operation a set can be unable
to satisfy — there is nowhere to put the key — and each of the three storages says so in its own way:

| | `insert(k)` past the width |
|---|---|
| a dynamic extent | grows to admit it; past `max_size()`, `std::length_error` ([the-sum-that-wraps](#the-sum-that-wraps)) |
| an inplace extent | grows within its capacity; past it the blocks say `std::bad_alloc` |
| a static extent | `std::out_of_range`, which is what `std::bitset<N>::set` says for a position past `N` |

The three differ because the reasons do — a domain, a capacity, a representable size — but none of them is silence,
which at a static extent would be a write through a block index the array does not have. Leaving it undefined for a
performance benefit does not survive measurement: on the sieve at `N = 2^16`, GCC 14 `-O3 -march=native`, best of
twenty-five, 188.0µs unchecked against 188.1µs checked, and 75.1µs against 75.1µs over 65536 inserts. The comparison
is against a compile-time constant and is never taken; it costs nothing to keep.

An enumerator's `^=`, which toggles one key, is the same write and answers the same way. A key past the width is
absent, so the toggle that admits it **is** the insert that admits it, and it grows where insert grows.

A value that is no key of a mapping closing the universe is the fourth reason, and it has one answer at every
extent. It ranks at or past the mapping's `size`, and a static width is that `size`, so a fixed set says
`out_of_range` for it through the check above. A growing set could grow to the position, and the key it would read
back there is none, so it asks the same question of `size` before its storage's own ceiling, and says
`out_of_range` too. The two refusals never meet on one owner: a bounded set over such a mapping has that `size` for
its capacity, so `bad_alloc` is left to an open universe's key past the capacity, and `length_error` to one past
what a heap can count. Every door a value comes in through asks it: `insert`, `emplace`, `emplace_hint`, the ranged
and listed forms, the constructors that insert, and an enumerator's `|=` and `^=`. A flag value of
several bits, or of none, stays `bit_flag_mapping`'s precondition, there being no one position to rank it at.

The element-wise `insert(first, last)` and `insert(ilist)` keep what they inserted before the refused key, which
is `[set]`'s own behaviour when an allocation throws midway; the consecutive `insert_range` tier guards the
range's last position before it writes anything, so that one is all or nothing.

Erasing stays total like `contains`: removing what is not there is the no-op returning zero that
`std::set::erase` is.

**The single position**, member by member, measured at `-O1 -DNDEBUG -fsanitize=address` against libstdc++ 14:

| a key past the width, or a step past an end | `std::set<size_t>` | here |
|---|---|---|
| `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range`, `erase(key)` | answers | answers |
| `insert`, `emplace`, `emplace_hint`, `insert(hint, x)`, an enumerator's `^=` | grows | `out_of_range` at a static width, grows at a dynamic one |
| the same, with a value that is no key of a closed universe | grows | `out_of_range` at every width |
| `erase(end())` | undefined | `assert(position != end())`, which it already said |
| `erase(first, last)` reversed | aborts: a free of a pointer never allocated | `assert(*first <= *last)` |
| `++end()` | undefined | `assert(m_idx < size())` |
| `--begin()` | answers the same key again | `assert(find_first() < m_idx)` |

The first two rows are the policy above, and the four below it are preconditions, on both sides. What they do
here without the asserts is worth keeping: `--begin()` at a two-block extent falls into the arm meant for the
lower block and answers the key it started from, so a reverse walk over it never ends; at four blocks and at a
run-time width it reads past the blocks, a stack- and a heap-buffer-overflow under ASan. The reversed erase range
does not end either, where `std::set` corrupts the heap and aborts.

The backward step is the one worth asserting at the iterator, because it is **stronger** than anything the
storage checks. `exclusive_find_prev` asserts `any()` and `is_valid(n - 1)`, and `--begin()` satisfies both
while there is nothing below to find — the reverse scan's real precondition is that a set position exists below
this one, which is `find_first() < m_idx`.

The forward step's assert is the storage's own `is_valid` said one level up, and the difference is which
function a failure names. That is the rule the sequence reading keeps too
([indexing-is-a-precondition](#indexing-is-a-precondition)) — and the set reading's iterator needs nothing
beyond it: dereferencing `end()` here is the width rather than a read, so `*end()` is harmless where the
sequence reading's is a load.

### indexing-is-a-precondition

The second reading answers the same question another way, and the answer is `[sequence.reqmts]`'s rather than
ours. Asking a set is total ([asking-is-total](#asking-is-total)); a **sequence indexes**, and out of range is out
of bounds.

`at(n)` is the one member that answers: `std::out_of_range` at every extent and through every handle -- the
static owner, the dynamic one, a view and a window alike, the window measuring `n` against its own size and
not the storage's. Everything else is a precondition, exactly as `std::vector`, `std::array` and `std::span`
have it, and `operator[]` beside `at()` is the pair the standard itself draws the line between.

A precondition is not licence to say nothing when it is violated, and at five members this reading would say
nothing without an assert of its own -- or say it a function away, which for a diagnostic is nearly the same
thing. Measured under `-O1 -DNDEBUG -fsanitize=address`, libstdc++ 14 without `_GLIBCXX_ASSERTIONS`:

| the precondition | `std::vector<bool>` | here, without the assert | here |
|---|---|---|---|
| `front()` on an empty sequence | segfault | reads the block a floored count leaves and answers `false` | `assert(not empty())` |
| `back()` on an empty sequence | segfault | `offset() + size() - 1UZ` wraps: segfault | `assert(not empty())` |
| `erase(cend())` | erases the last element | erases the last element | `assert(position != cend())` |
| `insert(cend() + 3, v)` | inserts anyway: size 2, iterator at 4 | the same, to the number | `assert` in `index_of` |
| `erase(cbegin() + 3, cbegin() + 1)` | size **grows**, 4 to 6 | size grows, 4 to 6 | `assert(first <= last)` |

The counterpart column is why none of this is a contract change. Every row is undefined on `std::vector<bool>`
and undefined here, and an assert can only fire where the program was already undefined -- the one direction an
extension may take. What it buys is the diagnostic, and the rule is
that the precondition is stated at the member the caller named. Without it a debug build catches four of these
five, but one call down: `front()` and `back()` on an empty sequence reach the storage's own `is_valid(n)` only
once the proxy they return is read, and the two bad iterators reach `first`'s and `subspan`'s `count <= size()`
from inside `rebuild`. A failure that names `subspan` for a bad argument to `erase` points at the wrong function,
which is worse than a blunt one; it is the same reason `operator[]` says `n < size()` where `test(n)` would say it
again. The fifth, the reversed erase range, is caught nowhere below: both of its indices are inside the sequence,
so nothing there has anything to object to, and it grows a four-element sequence to six in a debug build as
readily as in a release one.

The iterator preconditions are said **once**, in `index_of`, which is the one place a caller's iterator becomes an
index and is reached by all five members that take one. Half of that precondition is the iterator's own: `operator-`
and `operator<=>` assert `m_ptr == m_ptr`, so an iterator into another sequence never arrives here. What is left is
the range, `cbegin() <= position` and `position <= cend()`, and past either end the subtraction below it is a
`size_type` that wraps or an index the rebuild then writes through.

`erase(first, last)` adds the one thing neither iterator says on its own -- that they are in that order -- and
`erase(position)` the one `[sequence.reqmts]` asks of the single-position form, that the position is
dereferenceable and so not `cend()`. What holds them honest is the sweep:
`test/src/bits/bit_vector.cpp` runs `insert` in its value, fill, iterator-pair and initializer-list shapes,
`insert_range`, `emplace`, and `erase` in both of its own -- at positions including `cbegin()` and `cend()`,
and over empty ranges -- each against the `std::vector<bool>` that models it. Every one of those positions is a
valid one, and the asserts are on underneath them.

**The single position**, member by member, measured at `-O1 -DNDEBUG -fsanitize=address` against libstdc++ 14:

| past the width | the counterpart | here |
|---|---|---|
| `at(n)` | `out_of_range` | `out_of_range`, at every extent and through every handle |
| `operator[](n)` | heap-buffer-overflow | `assert(n < size())` |
| `front()`, `back()` on an empty one | segfault | `assert(not empty())` |
| `*end()` | reads the padding and answers with it | `assert(m_idx < size())` |
| `it[n]` past the end | heap-buffer-overflow | the same assert, `it[n]` being `*(it + n)` |
| `subspan(20, 3)` of eight | a `std::span` of **size 3**, past the end | `assert(off <= size())` |

Every row is a precondition on both sides, and in every one of them this reading says so where the counterpart
does not. `at(n)` is the only checked door, and it is the only row where the counterpart answers too.

The reading hands out two types that name a position, and the rule above -- state it at the member the caller
named -- reaches both. The sequence iterator's `operator*` says `m_idx < size()`, because this reading's
proxy reads and writes **through the storage**, so a position it hands out has to be one the storage has. The
set reading's iterator needs no such guard and has none: its proxy converts to `m_idx` itself, so there the
position *is* the value and `*end()` is the width rather than a read. Without the sequence iterator's own
assert, `*v.end()` and `v.begin()[100]` would both report `bit_block_container::test`'s `is_valid` — a private
predicate of a detail type, two levels below the expression that is wrong — where `std::vector<bool>` reports the
first as `true` and the second as a heap-buffer-overflow.

### unchecked-writes-in-views

Reads and writes inside a view go through the **subscript**, not through `test()`, `set(n)` or `reset(n)`.
The position is already in range by then, and those are the checked accessors whose throw would escape a
`noexcept` — which `bugprone-exception-escape` is right to report. Every type in the vocabulary hands out a
proxy that writes without checking.

### the-proxy-recursion-trap

The sequence proxy writes through the storage's `assign(n, value)` and never through a subscript. A view that fell
back on `c[n] = value` for a type without `set(n, value)` would be a trap: were such a type's `operator[]` to
return our own proxy, that proxy's assignment would land back in the fallback and **recurse until the stack
is gone**. A named member cannot loop back into the proxy, which is one more reason the write is a member the
storage spells rather than a probe over whatever answers — and `bit_block_container`'s `operator[]` yields a block,
never a bit ([test-not-subscript](#test-not-subscript)), so a fallback would find nothing that loops back.

### the-iterator-is-the-primitive

The set and sequence iterators are a pointer and a position, and they reach the bits through the storage alone.
Their constructors from a pointer and a position, and their references' too, are **private**: a user reaches a
proxy through a container and never builds one over storage it cannot see. Two kinds of caller may: the adaptor
that hands proxies out -- `sequence_adaptor` or `set_adaptor`, of which they are members -- and the proxy's twin,
since the iterator's `*` builds a reference and the reference's `&` builds an iterator. An enclosing class has
no special access to a nested one's private members, nor one nested class to another's, so each iterator
befriends its reference and its adaptor, and each reference its iterator and its adaptor.

The pointer is to the **storage** an owner wraps, never to the owner: `bit_fixed_set`'s iterator holds a pointer
to its `bit_block_container<std::array<B, K>, N>`, which is why an owner is never itself the thing a view is
parameterized on.

**Where they live, and what they are called.** Each pair is a member of the adaptor that hands it out --
`basic_iterator` and `basic_reference` in both, over a `bool` saying whether the storage is read as const in
`sequence_adaptor`, and over the key in `set_adaptor` -- and is reached through a container's `iterator` and
`reference` typedefs and through nothing else; [the-adl-firewall](#the-adl-firewall) says why they are members.
Every container therefore has iterators of its own, an owner and its view included, as `std::string` and
`std::string_view` do. The test tree keeps a source for each pair, `detail/random_access.cpp` and
`detail/bidirectional.cpp`, rather than taking the exception `detail/` is granted: unnameable is not
unobservable, and what these types do -- the concepts they model, the round trip, the writes -- is the
observable behaviour of every container's `iterator`. So the contract is asserted through the containers, and
these two sources assert white-box what the containers cannot say precisely. They are named after the iterator
category, which is what the two proxies differ by; the reading names the container that hands them out, and the
category names the iterator itself.

**`operator&` on the proxy answers an iterator**, which is what a proxy can offer in place of an address, and it
is what keeps a container's subscript tied to its iteration: `&a[n]` is `a.begin() + n`, so `&a[n] == &a[0] + n`
holds for `bit_array` and `bit_vector` in iterator arithmetic, exactly where a contiguous range spells it in
pointer arithmetic. `std::vector<bool>` answers this differently per implementation, and no implementation
answers it fully. libstdc++'s `_Bit_reference` has no `operator&` at all, so `&v[n]` is ill-formed there on a
mutable *and* on a const vector. libc++ does have one, but only on `__bit_const_reference`, where it returns
`__bit_iterator<_Cp, true>`; its mutable `__bit_reference` has none on current main, so `&v[n]` is ill-formed
there too and only `&cv[n]` on a const vector answers. Ours answers on both, measured rather than assumed.

Random access is nevertheless where the ladder stops, and it stops because of the proxy.
`std::contiguous_iterator` requires `iter_reference_t<I>` to be a real `iter_value_t<I>&`, which no proxy is, so
no reading here is a `contiguous_range` and no iterator here is a `contiguous_iterator`. That is asserted as a
negative, because it is the one place the bits and the blocks part company: the **blocks** are contiguous and
`owned_bit_blocks` requires precisely that
([owned-bit-storage](#owned-bit-storage)), while the **bits** are not addressable at all. The
asymmetry is the reason the vehicle keeps its blocks to itself and hands out proxies above it.

The free functions stay qualified as `bits::detail::shl<Block>(...)` inside `xstd::bits::detail` itself.
Dropping the qualification would read more naturally and reintroduce exactly the hazard the nesting exists to
close: an unqualified call with an explicit template argument performs ADL, and the associated namespace of
the type in play can be `std` or `boost` ([why-nested](#why-nested)).

### the-adl-firewall

**Why.** A proxy that is a specialization over the user's types carries every one of their namespaces into each
comparison it takes part in. `[basic.lookup.argdep]/3` makes the namespaces of a class template
specialization's template type arguments associated with it, recursively, so such a sequence proxy over
`bit_block_container<std::vector<acme::uint128>>` would search `acme`, and a set proxy under `acme::key_mapping`
would too. A namespace declaring one generic comparison is enough to take over:

```cpp
namespace acme {
struct uint128 { /* a Block */ };
template<class A, class B> constexpr auto operator==(A const&, B const&) noexcept -> bool { return false; }
}
```

`r == r` on a proxy over `acme::uint128` then answers `false`: the template is an exact match for both
operands, and the built-in `bool == bool` needs a user-defined conversion on each. Nothing about it is
exotic. Boost.Int128 declares non-template `operator==(uint128, bool)` and its mirror, so over a
`boost::int128::uint128` Block `r == true` would find that candidate beside the built-in one, each needing one
conversion, and be ambiguous.

**The rule.** The same paragraph associates with a class type the class itself, the class it is a member of,
and its bases; only when the type *itself* is a class template specialization does it add that
specialization's template arguments. A member of a class template specialization is not a specialization of
the enclosing template, so its enclosing class's arguments never reach it. A specialization of a *member*
class template is a specialization, but of that member template, and contributes its own arguments alone.
GCC 15 and Clang 22 both read it so.

**The shape.** The iterator and the proxy are members of the adaptor that hands them out, so the adaptor's
template arguments -- the storage, the derived container, the key mapping and the comparator -- never reach them:

```cpp
template<bit_block_container_type Bits, storage Store, window W, class Derived, std::size_t E>
class sequence_adaptor
{
        template<bool IsConst> class basic_reference;      // & yields basic_iterator<IsConst>
        template<bool IsConst> class basic_iterator;       // * yields basic_reference<IsConst>
        using reference       = basic_reference<std::is_const_v<Bits>>;
        using const_reference = basic_reference<true>;
        using iterator        = basic_iterator<std::is_const_v<Bits>>;
        using const_iterator  = basic_iterator<true>;
};

template<bit_block_container_type Bits, storage Store, class Derived, class Key, bit_index_mapping<Key> KeyMapping, class Compare>
class set_adaptor
{
        template<class Value = Key> class basic_reference; // & yields basic_iterator<Value>
        template<class Value = Key> class basic_iterator;  // * yields basic_reference<Value>
        using reference      = basic_reference<>;
        using iterator       = basic_iterator<>;
        using const_iterator = iterator;
};
```

The pair is closed under `*` and `&` within one adaptor. The sequence pair is a template over a `bool` rather
than two classes so that over a storage already const, `iterator` and `const_iterator` stay one type, and a
`bool` brings no namespace. The set pair is a template over the key so that the key, and only the key, is
associated; each body asserts that `Value` is `Key`, there to be associated rather than to be a second axis.
What the set iterator does need of the comparator and the key mapping -- which way a step goes, and which key a
position is -- it asks the adaptor, whose private `next_position`, `prev_position` and `key_at` say it once.

**Why not an ADL barrier.** The usual idiom, a class in a namespace of its own that the library re-exports
with a using-declaration, closes off the namespace the class is declared in. It does nothing for the template
arguments, which stay associated however the class is reached, and they are the whole problem here.

**What is associated, and what is not.** The storage -- `Bits`, and through it the blocks, the Block and any
allocator -- the derived container, the key mapping and the direction are **not** associated with a proxy or an
iterator. The value type **is**, deliberately, so that the proxy compares as its value does: `bool` for the
sequence reading, which brings no namespace, and the key for the set reading, whose namespace is searched, so
`*it == *jt` and `*it == 3` reach a class key's own comparisons, hidden friends among them, as a real
`Key const&` would. Without that, a key whose `operator==` is a hidden friend would not compare through its
proxy at all. A reading whose value is a character type keeps that type the same way. `xstd::bits::detail` is
associated as well, and so is the adaptor itself, being the class the proxy is a member of: its hidden friends
take adaptors, which a proxy is not, so they join the candidates and are never viable. The proxies' own hidden
friends -- `swap`, `iter_move`, `iter_swap` and `format_as` -- are found by ADL on the proxy itself.

**What it costs.** No deduction reaches the adaptor's arguments through a member, so the `std::formatter`
specializations cannot be written as `formatter<sequence_adaptor<...>::reference>`. Each is a partial
specialization constrained by a `detail` concept instead, `sequence_reference` or `set_reference`, which
recognizes a proxy through the adaptor it names as `adaptor_type`: true for exactly the references some
adaptor declares. And an owner's iterator is not its view's, so code that compares the two, or an iterator of a
view over a storage with one of a view over the same storage made const, names one adaptor's `iterator` and
`const_iterator` throughout.

### the-set-for-each

`xstd::bit_for_each(s, f)` on the set reading walks blocks where the iterator walks positions, and
`xstd::bit_for_each(std::views::reverse(s), f)` walks them the other way. The difference is not
block-parallelism -- the functor still sees every set position, one at a time. It is where the loop's state may
live.

`operator++` is **flat**. It has to re-derive the block from `(pointer, position)` on every step, because an
iterator stays copyable and restartable and therefore cannot keep a partially consumed block between two
increments. A loop has somewhere to put one. So `find_next` loads a block, masks off what is at or below the
cursor, tests, possibly scans forward and takes a `countr_zero`, once per position; the walk loads once per
block and then spends two instructions per position, `tzcnt` for the position and `blsr` to drop it.

Measured against the range-for at 4.08x to 5.05x, on GCC 15 and clang 22, and the same on a two-block bitboard
carrying twenty pieces as at 2^22 with 40% density. That stability is itself the point: `find_next` is
inherently serial, no vectorizer engages on either side, and the answer does not move with the compiler --
unlike the sequence reading, where the same question gives answers ranging from 1.24x ahead to 11.4x ahead
depending on which one is asked ([the-sequence-ladder](#the-sequence-ladder)). The figures were taken when the walk
was the containers' `for_each` member; the algorithm is that member's loop, moved out unchanged.

Three decisions the shape forced:

**The functor's answer is ignored, as `std::ranges::for_each` ignores it.** The member let a functor return
`bool` to mean "keep going", and that is the one thing that did not move with it: `std::ranges::for_each` is the
specification ([algorithms-not-members](#algorithms-not-members)), it calls a functor returning `bool` on every
element, and an algorithm that stopped would disagree with it on exactly the functors that look like predicates.
A walk that stops at an answer is `find_if`'s and `any_of`'s, whose set-reading forms take a predicate where this
takes a functor.

**The descending walk clears the bit it just reported.** `w & (w - 1)` drops the lowest set bit and has no
descending twin, so the walk against the order takes `digits - 1 - countl_zero(w)` for the position and clears
exactly that bit. The set reading iterates both ways and so does this, and the reverse is the overload on
`std::views::reverse` over a set of ours, held through its `ref_view` or as a view, rather than a second name:
`for_each_reverse` named a direction that `std::ranges` spells as a view.

**It returns what `std::ranges::for_each` returns**, the end as a `borrowed_iterator_t` and the functor moved
back, so a functor that counts or collects hands its state back as the standard's does.

What is *not* here is a fat iterator carrying the residual block. Measured, it runs 2.56x to 3.80x against the
walk's 4.00x to 5.27x, so it is both slower than the walk and the only one of the two that changes a
contract: an iterator that caches a block stops observing an erase that lands ahead of it, where a closed loop
caching the same block is unobservable. The walk is the faster half and the safer half at once, which is
rare enough to record.

### void-is-a-return-type

Every function names its return type after the parameter list, `void` included: `auto f() -> void`, with the
`-> void` on its own line above the body wherever the body has lines of its own. Left to habit, the two shapes
split roughly along the library/test line, which is not a reason, so every function takes the one.

Three things were weighed and are recorded because the obvious readings of each are wrong.

**clang-tidy does not ask for this and will not keep it.** `modernize-use-trailing-return-type` fires on a
named, non-`void`, *leading* return type and rewrites that one; measured on rungs 22 and 24, it says nothing
about `void f()`, nothing about `auto f()` with a deduced return, and nothing about `auto f() -> void`. So
this is a convention the tooling is indifferent to, and only review keeps it -- which is an argument for
[#70](https://github.com/rhalbersma/xstd-bits/issues/70)'s deferred `.clang-format`, not against the
convention.

**A deduced `auto f()` would have been the shorter road and is closed.** It reads as the same idea with less
typing, but a deduced return type has to instantiate the body to be known, so any
`requires { x.f(); }` that would have been answered from the declaration instead instantiates and can hard
error where it should have said "no". That is not a stylistic loss; it is the mechanism every detection in the
tree is built on -- `can_grow`, `is_writable` and `blittable` all ask exactly that question.
Declared return types, trailing or leading, answer it from the declaration.

**Lambdas keep their trailing return inline.** A lambda is an expression inside a statement, so there is no
"above the body" to put anything on, and `modernize-use-trailing-return-type` requires the `-> void` there
anyway. The convention is about named functions.

### an-owner-reads-as-its-storage

A view over an owner is a view over the **storage** the owner wraps, and that is the only spelling there is.
`bit_set_view(s)` over an `xstd::bit_fixed_set<64>` deduces `bit_set_view<std::array<std::size_t, 1>, 64>`, the
blocks and width that storage is built over, and `bit_set_view<xstd::bit_fixed_set<64>>` is not a spelling: `Blocks` is
constrained to blocks, and an owner is not blocks ([one-storage](#one-storage)).

**Naming the owner through a forwarding trait is the alternative, and three things are wrong with it.** A trait
specialization for an owner, relaying each of the storage's twenty entries behind its own `requires`, would let a
reader name the owner rather than the storage underneath it. The argument for it is readability: the deduced
spelling names an implementation detail that is not in the landscape tables.

- It makes the plain-storage deduction guide viable for an owner too, so `bit_set_view(s)` ties. Constraining
  that guide to non-owners settles the tie, and is needed either way, an owner and its storage being two viable
  bindings.
- It relays entries rather than forwarding a type, so a dropped one compiles and merely runs slower — without
  `num_blocks` and `block`, every block-parallel walk falls to one position at a time — and only a test that
  asserts the block tier explicitly turns that into a failure.
- **It lets the two readings mix.** A trait keyed on the *storage* cannot tell a set view over a set owner from
  `bit_span<xstd::bit_fixed_set<N>>`, because both wrap the same storage. That rule belongs in the view's
  constraint ([the-readings-do-not-mix](#the-readings-do-not-mix)), where the owner is still in hand.

With the rule in the constraint, a forwarder buys only the spelling, and the spelling cannot survive `Bits` being
constrained to blocks. A reader who wants to name the type writes `decltype(xstd::bit_set_view(s))`, which is what
`test/consumer/main.cpp` does.

### what-a-view-costs

Measured on the same backend `bit_block_container` in every row -- an owner, a view holding a pointer to
that storage, and a view through a forwarding wrapper around the same storage -- so the two layers price
separately. `benchmark/` builds at `-O3 -march=native`, which is what these numbers are.

| operation | bits | pointer costs | bitset-wrapper costs |
| :--- | ---: | ---: | ---: |
| set iterate | 256 | +7.1 … +9.6% | ±1% |
| set iterate | 1024 | +8.5 … +11.4% | ±1% |
| set iterate | 4096 | +11.1 … +12.8% | ±4% |
| set iterate | 16384 | +10.3% | ±2% |
| sequence count | 256 … 16384 | ±2% | ±3% |
| sequence read | 256 … 16384 | ±1% | ±2% |

Run-to-run noise was ±0.5 to ±3%, so a figure inside a couple of percent is a zero.

**A forwarding layer is free, and not merely inside the noise.** Under callgrind, at 1024 bits, the view through
the wrapper and the view over the raw storage retire **6764 instructions per pass each**, equal to the digit, with
the same 422 data reads, 1265 branches and 17 simulated mispredicts: the twenty forwarded entries inline away
completely. The library keeps no such layer ([an-owner-reads-as-its-storage](#an-owner-reads-as-its-storage)),
and the table keeps the column all the same, because it shows that the cost that remains is the pointer and not
the layer.

**The pointer costs about eight percent on set iteration, and the cause is one `lea`.** Instruction counts,
differenced between two fixed iteration counts so startup cancels, at 1024 bits under GCC 15:

| variant | Ir/pass | data reads | branches | mispredicts |
| :--- | ---: | ---: | ---: | ---: |
| owner | 6273 | 420 | 1250 | 11 |
| owner twin | 6273 | 420 | 1250 | 11 |
| view of storage | 6764 | 422 | 1265 | 17 |
| view of bitset | 6764 | 422 | 1265 | 17 |

+491 instructions is +7.8% against +7.0% of measured time, so the gap is **more work at essentially constant
IPC** -- not a stall, not a misprediction. And 491 over the 410 set positions in the pass is one extra
instruction per step, so it is in the per-step work, not in setup. The hot block confirms it: it executes
492,000 times for 1200 passes, which is exactly 410 per pass, and the owner's copy of it is **eleven**
instructions where the view's is **twelve**.

The extra one is address arithmetic. GCC reaches the owner's blocks, which sit in the frame of the function
running the loop, with the address folded into the load's own operand:

```asm
shrx  %rcx,0x10(%rsp,%rax,8),%rdx    ; owner: base+index*8+disp, one instruction
```

and reaches a view's blocks, which are behind a pointer, in two:

```asm
lea   (%r12,%r8,8),%r10              ; view under GCC 15
shrx  %rcx,(%r10),%rdx
```

**That fold is legal and GCC just does not take it.** `shrx`'s memory operand is a full ModRM+SIB address, so
`base+index*8` is encodable with a register base exactly as it is with `%rsp`; clang emits precisely that:

```asm
shrx  %rsi,(%r12,%rdx,8),%rdi        ; view under clang 22, same operands, one instruction
```

Which is why the whole gap is compiler-specific. clang 22 retires **5829 instructions per pass for all three
variants**, equal to the digit, and measures owner 1573 ns, twin 1564, view of storage 1571, view of bitset
1548 -- a spread of ±1.6%, no gap at all.

**How much of the eight percent is a view, and how much is GCC.** The benchmark views a *local*, and clang
propagates that known frame address through the view's pointer member into the addressing mode, erasing the
indirection the row means to price. A view exists for bits that live somewhere else, so the honest variant
holds its blocks on the heap behind an asm-laundered pointer the optimizer cannot trace:

| | owner | view of a local | view of opaque blocks |
| :--- | ---: | ---: | ---: |
| GCC 15 | 1565 ns | 1691 ns (+8.0%) | 1698 ns (+8.5%) |
| clang 22 | 1546 ns | 1549 ns (+0.2%) | 1592 ns (+2.9%) |

So a view over bits the compiler cannot trace costs about **three percent**, which is what the indirection is
actually worth, and GCC adds **five more points** by not folding the address. Under GCC the cost does not
depend on traceability at all -- the missed fold is charged either way.

The guidance is narrower than a property of views in general: own the bits when you
iterate them hot *under GCC*; the abstraction itself is worth about three percent, and a view is still the
right answer when the bits are someone else's, where the alternative is not a container but no reading at
all.

**A stand-in measures the wrong program.** An asm diff of small isolated functions written to stand in for the
benchmark says that the pointer *is* hoisted and only the prologue differs, and so strikes "extra work in the
loop" off the list. Those stand-ins are not the benchmark: GCC compiles them to **15,824,004 instructions for
every variant**, identical to the digit, because a subject that is a local of a tiny function has its address
propagated the way clang propagates it above. A stand-in optimizes away the thing under test, and the benchmark's
own objects say the opposite.

The same defect sinks a harness that passes the subject to one shared timing function as a `Subject const&`: that
routes the **owner** through a pointer as well, makes every variant indirect, and reports no gap. A subject has to
be a local of the function that runs the loop, as the benchmark's variants are, or the harness measures nothing.

What is left over is small and unexplained: 491 extra instructions where the per-step `lea` accounts for 410,
so about 80 sit in the block-advance path and the prologue, and `sequence count` at 4096 bits is bimodal --
+24.3%, -3.2% and +21.8% across three runs, with the bitset column swinging the opposite way each time to land
the third variant back on the owner's time. Only the middle variant moves, and between two stable values.
Code layout does not account for it, a byte-identical twin of the owner case measuring layout at about a
percent here, so it stays recorded rather than explained. One run would have made it a finding.

Two things about the measurement itself, because each is easy to get wrong. **Every subject goes through
`DoNotOptimize` before the loop, owners included.** Without that an owner is a local the compiler folds straight
through: `count()` on one block measured 0.16 ns, half a cycle, which is not a faster reading but no reading at all,
while a view's pointer blocks the same folding -- so the comparison measured the folding. And **the ladder runs four
blocks to 256, with no one-block rung**: a single `popcount` is one cycle, so a one-cycle difference between two
variants reads as +100% and means nothing.

### swap-goes-through-adl

`std::ranges::swap` reaches a type's own `swap` by **ADL on a free function**, and a member `swap` is not
found that way. When it finds none it falls back to a move-construct and two move-assignments, which is
correct and, for a storage whose moves are cheap, not obviously worse -- which is what makes a missing free
`swap` easy to miss.

`set_adaptor` and `sequence_adaptor` each ship a free `swap` beside the member, and every container swaps by
`std::ranges::swap(m_bits, other.m_bits)` where `m_bits` **is** a `bit_block_container`. With the member alone
on the storage, it would be unreachable from the containers, and a storage with an optimized `swap` would never
see it. Measured over a storage whose swap and moves are counted, once per reading:

| | member only | member and hidden friend |
| --- | --- | --- |
| `sequence_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |
| `set_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |

Every type in the tree carries the same pair: a **member** `swap` that does the exchange, and a **hidden
friend** `swap(x, y)` that forwards to it. One rule, no exceptions -- the storage included, though it is not a
container and no requirement asks it for either.

The storage's member has no caller but the friend, which is true and is no reason to fold it away: the friend
calling the member *is* the design, so the member is the primitive rather than dead weight, and deleting it would
buy one fewer function at the price of the storage reading differently from the adaptors. The adaptors keep the
member because a container requirement asks for it; the storage keeps it so the tree has one shape.

Hidden rather than at namespace scope, matching `==` and `<=>` -- `ranges::swap` finds a hidden friend by ADL
exactly as it would a namespace-scope template,
and `std::is_nothrow_swappable_v` is a trait over *unqualified* `swap`, so it finds one too. What it costs is
`xstd::swap(a, b)` spelled with the qualification, which nothing writes. Measured across the adaptors,
each entry reads 1 storage swap and 0 moves: through the member, through unqualified `swap`, and through
`ranges::swap` alike.

The storage is not asked for a `swap` by its concept: `std::regular` implies `copyable`,
which implies `movable`, which **includes** `std::swappable`, so the concept already requires as much swapping
as `ranges::swap` can need, and any better one arrives by ADL without being asked for.

The `noexcept` asks the calls the body makes, `noexcept(std::ranges::swap(...))` on the width and the blocks,
and not `std::is_nothrow_swappable_v<Blocks>`, a trait of the `std::swap` family, which is a different family with
different rules (`ranges::swap` suppresses the generic `std::swap` template when looking for an ADL candidate).
The two agree for every storage shipped here, so the trait would be a latent mismatch rather than a bug, but it
is the same shape as the
`is_invocable_r_v`-tests-a-prvalue-and-the-call-passes-an-lvalue disagreement in
[the-functor-takes-a-value](#the-functor-takes-a-value), and the same fix applies: the specification names
the calls the body makes, with the body's own arguments.

### a-requires-clause-names-its-arguments

A `requires` clause tests the call the body makes, with the body's own arguments, and not a literal standing in
for the argument, `0UZ` say, where the body then passes a different type.

```cpp
requires requires { self.storage().growing_insert(0UZ); }    // is a size_t insertable?
{ self.insert(ilist.begin(), ilist.end()); }                 // ... a value_type is inserted
```

For every storage shipped here the two coincide, so no test would catch the difference. That is the danger, not
the defence: a `value_type` not constructible from `std::size_t` would be *admitted by a satisfied constraint*
and then fail inside the body, which is the hard error the constraint exists to prevent. A constraint that
cannot say no about the call actually made is decoration.

Where the function has the argument, the clause names it -- `*position`, `x`, `*first`, `*ilist.begin()`,
`value_type(std::forward<Args>(args)...)`, `static_cast<value_type>(*std::ranges::begin(rg))`,
`b.reserve(blocks_for(n))`. Where there is no call site to borrow from -- `can_grow`,
`blit_source`, the `for_each` block guards, `std::bitset`'s `num_blocks` -- the requires-expression declares
its own parameters, which is what `std::declval` is for at namespace scope and what C++20 gave
requires-expressions of their own:

```cpp
requires (bits_type& b, std::size_t n, bool value) {
        b.resize(n, value); b.push_back(value); b.pop_back(); b.clear();
}
```

Declaring one where the enclosing function already has that name is the same mistake one level up, and the
compilers disagree about noticing: clang's `-Wshadow` rejected two such parameters that GCC accepted
silently. If the function has an `n` or an `i`, the constraint uses it.

The same rule reaches concepts, where the argument is the type: see
[owned-bit-storage](#owned-bit-storage), whose subscript requirement exists because the
class subscripts and `std::ranges::contiguous_range` does not promise that.

### the-functor-takes-a-value

`xstd::bit_for_each` hands its functor a **prvalue** -- as `auto(x)`, C++23's decay-copy in the language
([P0849R8](https://wg21.link/P0849R8)) -- and says so with `requires std::invocable<F&, range_value_t<R>>`, a `bool`
on the sequence reading and the key on the set reading. That is one fix for one defect, spelled in two places because it is
worth catching at the interface and worth being right in the body.

It is spelled through a three-line `decay_copy` helper beside the walks, because MSVC 2022 does not implement
P0849R8 and the stable rung is on the matrix ([the-views-are-the-adaptors](#the-views-are-the-adaptors)). Without
that rung `auto(x)` would do, the paper's own spelling, which clang-tidy has nothing to say about. The other way
round it -- `T{x}` -- reads to clang-tidy as a cast to the type it already has, so it is not the alternative.

The defect it prevents is a walk that passes its named parameter along. A named parameter is an lvalue, so a
functor asking for `bool&` or `size_t&` binds to it, compiles, and writes to a local that the walk throws away.
The walk reads its storage through a `const&` and never writes back, so what looks like a mutating pass is a
silent no-op -- and on the set reading it is worse than lost, since a walk *reports* positions and changes none,
so there is nothing a written-back position could mean.

`auto` does not save this. `[](auto& b) { b = true; }` is an `auto` parameter and is exactly as silent as
`[](bool&)`; the axis is the value category, not the spelling of the type. Nor is "make them write `auto`"
enforceable: a generic lambda is distinguishable from a non-generic one -- `requires { &F::operator(); }` is
false for the generic one, its `operator()` being a template -- but that test also rejects a function pointer, a
hand-written functor and `std::function`, all of them legitimate callers, and it asks about the functor's shape
rather than about what it does with its argument.

What the prvalue settles is a **disagreement within the code**: `is_invocable_r_v<bool, F&, bool>`, one line
above the call, tests with a prvalue, so a call with an lvalue could take one arm and make a call that differs
from it, as it does for the reference-taking functor.

The two halves earn their place separately:

- **The constraint** puts the rejection at the interface, where it reads "constraint not satisfied" and names
  the algorithm, instead of erupting somewhere inside the walk. It is also what makes the rejection *testable*:
  a hard error in a template body is not something `static_assert(not ...)` can see, so a probe without the
  constraint reports everything as fine.
- **The prvalue** is what the constraint cannot reach. `std::invocable<F&, bool>` is satisfied by a functor
  overloaded on the value category, so the constraint cannot say which overload runs; only the call can. The
  tests pin it with exactly that: a functor with `operator()(T&&)` and `operator()(T&)` that records which one it
  got, on both readings and in both directions. Reverting any of the four walks to an lvalue fails the suite.

Accepted, and unchanged: by value, by `auto`, by `const&`, by `auto const&`, a function pointer, and a functor
returning anything, whose answer is dropped as `std::ranges::for_each` drops it. Writing *through* the sequence is the range-for's job and remains so --
`for (auto r : v) r = true;` writes, because the proxy is what an iterator dereferences to.

### the-sequence-aggregates

The sequence reading counts and queries its bools through four algorithms, `xstd::bit_count`, `xstd::bit_all_of`,
`xstd::bit_any_of` and `xstd::bit_none_of`, and compares two sequences through `xstd::bit_mismatch`
([algorithms-not-members](#algorithms-not-members)). Each is the `std::ranges` algorithm it is named after, over
`true` or through `std::identity`, as [alg.count] and [alg.all.of] spell them: `bit_count(v)` is
`std::ranges::count(v, true)` and returns its signed `range_difference_t`, where `std::bitset::count` returns a
`size_t`, and `bit_all_of(v)` is `true` on an empty sequence, as `std::ranges::all_of` is.

They are not redundant with `bit_set_view(v).size()`, which already answers a block-parallel count over the
same storage. That view asks a *different question* -- reinterpret these bools as a set of positions and give
me its cardinality -- which happens to return the same integer as `bit_count(v)`. Two readings over one storage is
the whole design, and [two-readings-disagree](#two-readings-disagree) exists to say they are not interchangeable;
making a caller change reading to count their bools is the fault the README levels at the two containers this
library replaces.

**The questions about `false` are identities, not further algorithms**, and they hold on a window too:

```
std::ranges::count(v, false)                 == std::ranges::ssize(v) - xstd::bit_count(v)
std::ranges::all_of(v, std::logical_not())   == xstd::bit_none_of(v)
std::ranges::any_of(v, std::logical_not())   == not xstd::bit_all_of(v)
std::ranges::none_of(v, std::logical_not())  == xstd::bit_all_of(v)
```

Short-circuiting survives them: asking whether every bool is `false` really does stop at the first set bit, because
it *is* `bit_none_of`. So there are four helpers in `bit_algorithm/detail/algorithm.hpp` -- `count_true`, `any_true`, `all_true`,
`none_true` -- and the algorithms are spelled over those, each helper choosing its tier once: the storage's own
member over a whole sequence, and a masked block at a time over a window ([windows](#windows)). `none_true` is a
helper of its own rather than `not any_true`, so the storage is asked in its own blocks; `bit_block_container` spells
`count`, `all`, `any` and `none` itself, each at the block tier.

`xstd::bit_mismatch` is `bit_block_container::first_difference` plus one `countr_zero`. The storage's helper
**keeps its name**: it scans low block to high, which is the *ascending* orderings' answer, where a bit
string's order would walk the other way. Calling it
`mismatch` on the storage would repeat the mistake an unqualified `lexicographical_compare_three_way` made
([two-readings-disagree](#two-readings-disagree)). The counterpart name goes on the algorithm, which walks blocks
over whole sequences alone: a window's blocks are not its own, so two windows are compared a `bool` at a time.

Measured on GCC 15.2, `-O3 -march=native`, 20% density, best of fifteen:

| | 2^16 | 2^20 | 2^24 |
|---|---|---|---|
| `xstd::bit_count(v)` | 0.1 µs | 2.0 µs | 49.5 µs |
| `std::count` over `bit_vector` | 54.6 µs | 889 µs | 15072 µs |
| `std::count` over `std::vector<bool>` | 45.1 µs | 702 µs | 11590 µs |

234x to 541x, and the two `std::count` columns are within 1.3x of each other, which settles what #121 asserted
and this measurement contradicts: **libstdc++ does not specialize `std::count` for `_Bit_iterator`**. A
specialization would show as two orders of magnitude, not as twenty percent. Under clang 22 the ordering
inverts -- `std::count` over `bit_vector` runs 2074 µs at 2^24 against `std::vector<bool>`'s 15419 -- because
our iterator's dereference is a pure function of its index and clang vectorizes it, where `_Bit_iterator`'s
loop-carried state blocks vectorization everywhere.

**Against libc++ the advantage is gone, and the row above says so only because both its columns are
libstdc++'s.** libc++ does specialize: `std::__count_bool` walks whole words through `popcount` and masks the
partial word at each end, and `std::find`, `std::equal`, `std::fill` and `vector<bool>`'s own `__hash_code`
are written the same way. Measured on clang 18 with the library as the only variable, a second machine, same
density and best of fifteen:

| clang 18, µs | 2^16 | 2^20 | 2^24 |
|---|---|---|---|
| `std::count` over `std::vector<bool>`, **libc++** | 0.2 | 6.1 | 68.4 |
| `xstd::bit_count(v)` | 0.3 | 3.7 | 72.6 |
| `std::count` over `std::vector<bool>`, libstdc++ | 72.3 | 1181 | 19316 |
| `std::count` over `bit_vector` | 129 | 2122 | 33489 |

282x between the two `vector<bool>` rows on identical source, and a **tie** at the top: 2^24 bits is 2 MiB and
both run it at about 29 GiB/s, which is the memory and not the loop. So the honest claim is narrower than the
one the first table invites -- `xstd::bit_count` beats `std::count` over a `vector<bool>` **whose library does not
specialize it**, and ties one whose library does. What the algorithm buys against libc++ is not speed but that
the block-parallel count is a spelling a caller can name rather than a library optimization they must hope
is present, over a storage that also answers the other two readings.

It also narrows the rule elsewhere in this file. A tail invariant is needed only where blockwise reads are
**unmasked**, which is weaker than needing one wherever reads are blockwise: libstdc++ escapes it by reading
element-wise, libc++ by masking at every read site, and this tree by masking at the four writes that can dirty
the padding ([padding](#padding)).

### the-sequence-for-each

`xstd::bit_for_each(v, f)` on the sequence reading is the set reading's walk ([the-set-for-each](#the-set-for-each))
transposed: an outer loop over blocks, an inner loop over the bits of one block, so the reload that
`operator++` must perform on every step becomes the inner loop's exit test. The functor takes what this
reading's iterator dereferences to, a `bool`, where the set reading's takes a position. A window walks its own
positions, its blocks read through `block_at` at its offset.

The reason for the walk is the same and the payoff is **not**. Measured across GCC 15, GCC 16, clang
20 and clang 22, no iterator layout beats the current `(container pointer, index)` on this reading: a block
pointer plus offset ties, and a cached residual block is *worse*, because `operator++` is flat and the reload
test becomes a per-bit branch. So whatever `for_each` wins is loop structure, and loop structure is exactly
what a vectorizer needs:

| GCC 15.2, 2^24 bits | `for_each` | range-for | ratio |
|---|---|---|---|
| `n += b` | 1685 µs | 13842 µs | 8.2x |
| `h = h * 1000003 ^ b` | 18743 µs | 19154 µs | 1.02x |

**The win is the vectorizer's, not the loop's.** Where the body carries a loop-carried dependence there is
nothing to hoist and the two are parity; where it does not, the outer-loop-over-blocks shape lets GCC
vectorize what the flat `operator++` hides. And under clang 22 the range-for already vectorizes -- 2021 µs
against `for_each`'s 1900, 1.06x -- so on that compiler the walk buys almost nothing on this reading.

That is a narrower claim than a general speed-up, and it is the measured one. `for_each` is still
worth having: it is never slower, it is 8x ahead on the compiler and body shape where the range-for leaves
the most on the table, and it is the spelling that lets the library choose the loop rather than the caller.
The reverse walk is the same overload on `std::views::reverse` the set reading takes, blocks from the top and
each block's bits from its top, so the two readings spell a backward walk alike; over a random-access range the
range-for over `std::ranges::reverse_view` costs about the same, and the overload is there for the symmetry.

### read-only-set-proxy

The set reading's proxy is read-only whatever the qualification of `Bits`, because a key is nothing to write
through: assigning to a position would mean moving an element, which a set has no spelling for. It earns its
keep anyway — `operator&` round-trips to the iterator, and the one conversion, to the key, is what every
comparison goes through. A class constructible from the key is direct-initialized from the proxy, `index(*it)`,
whether its constructor is explicit or not; copy-initialization, `index i = *it;`, would take two user-defined
conversions and is not offered (see [uint128-support](#uint128-support) for why). Nor does the proxy offer an
explicit conversion of its own: MSVC cannot resolve one beside such a constructor.

### the-proxy-copies-the-handle

Both proxies declare their copy constructor, and for opposite-looking reasons that are the same reason. The
set proxy is `= default` beside a deleted `operator=`, because a reference to a key is a value: copyable,
never assignable. The sequence proxy is `= default` beside two `operator=`s that write *through* the handle
to the bit. That second pairing is exactly the case `[-Wdeprecated-copy-with-user-provided-copy]` names: a
user-provided copy assignment makes the implicit copy constructor deprecated, because the compiler can no
longer assume the two agree -- and here they genuinely do not, which is the whole point of a proxy. So the
copy constructor is said out loud rather than inherited by default.

Nothing else copy-constructs a proxy in this library, so only formatting reaches it: `std::formatter`'s dispatch
takes the element by value, and that copy is where the deprecation would land.

### the-one-adl-exception

The sequence iterator's `iter_move` and `iter_swap` are hidden friends found by ADL, and they stay under the
no-ADL rule because they are `std::ranges`' own customization protocol: `ranges::sort` and `swap_ranges`
reach a proxy only through them, and it is where `vector<bool>` historically fell down. The three `swap`
overloads on the proxy are the pre-ranges spelling of the same thing, for `std::sort` and everything else
still built on `std::iter_swap`. `format_as` is fmt's protocol in the same sense.

The sequence proxy borrows nothing else, from `[vector.bool]` or `[template.bitset.general]`: no `flip()` and no
`operator~`. One element is negated by `v[i] = not v[i]`, and a whole array by the valarray reading's `!`.

### formatting-the-proxies

`std::format` over the containers needs nothing said about the containers. Every owner and view here is a
range, so `[format.range.formatter]` would format each one already, except that it requires
`formattable<ranges::range_reference_t<R>>` and a reference of ours is a proxy. So each proxy specializes
`std::formatter` for itself, in its own header, and stops there: `bit_set`, `bit_fixed_set`, `bit_vector`,
`bit_array`, the views, the windows and the bounded column all follow from that, none of them mentioned.

This is the same shape the proxies already had for fmt, in fmt's spelling. `format_as` is fmt's generic
per-type hook: define it for one type and every range over that type formats, which is why the proxies carry
it and no container does. `std::formatter` is the standard's hook for the same job. So each library gets one
hook per proxy -- a hidden friend for fmt, a specialization for the standard -- and in both the containers
follow for free. Nothing here is a special case for formatting; it is the general mechanism used twice.

**The two hooks share one definition.** The `std::formatter` calls `format_as`, unqualified so ADL finds the
proxy's own hidden friend, rather than reaching the value a second way of its own. That direction is the one
[#20](https://github.com/rhalbersma/xstd-bits/issues/20) argued for to WG21 on
[P3070R0](https://github.com/cplusplus/papers/issues/1731): one `format_as` in the type's own namespace serves
fmt and the standard both, where a `formatter` specialization serves one library and has to be written again
for the other. The standard has not taken that hook for general use -- neither libc++ nor the MSVC STL defines
one today -- so the specialization is needed, and it does not restate the value. A formatter that cast to
`size_t` or `bool` through the conversion operator while `format_as` read the member would land on the same
value with nothing making it: a change to one would leave `fmt::format` and `std::format` disagreeing about the
same object, with no compile error and no test to catch it, since the proxy tests check `format_as` and the
format test checks `std::format`. `format_as` is the one place that says what a proxy prints as.

**fmt is not a dependency of this repository, and `format_as` stays anyway.** The library builds and tests with
nothing but the standard's `std::format`. The hook is not a build dependency: it is a hidden friend of a
header-only proxy, costing a consumer who never formats nothing at all, and it is what a consumer who *does*
format with fmt reaches. Deleting it would take fmt interop away from them to remove a line that our own
`std::formatter` calls regardless. So the two-hook shape above is the shape, and this repository simply does not
install the second library to check the second hook.

The readings then separate themselves. `[format.range.fmtkind]` picks `range_format::set` for a range with a
`key_type` and `range_format::sequence` otherwise, so the set reading prints `{1, 3, 5}` and the sequence
reading `[false, true, false, false]` -- the same split `format_as` arrives at for fmt, reached here through
the standard's own machinery rather than by our choosing
([two-readings-disagree](#two-readings-disagree)).

Each specialization derives from `std::formatter<size_t>` or `std::formatter<bool>` instead of writing a
`parse`, which is what keeps the whole spec: a width and a fill on a single proxy, and the nested spec a range
formatter forwards, so `{::#x}` over the set reading and `{::d}` over the sequence reading reach the
underlying formatter intact.

**Why this lives with the proxy and not in a header of its own.** A separate `xstd/bits/format.hpp` outside the
umbrella would keep `<format>` off the path of a consumer who does not format, and that reason does not hold:
`sequence_adaptor` includes `<format>` already, to build the `file:line:column:` messages its exceptions carry, so
that reading pays for it whatever the umbrella does. Measured, a separate header saves the sequence reading 181
preprocessed lines -- and costs a user the knowledge that the header exists. Only the set reading would save
anything real, about 11.8k lines, `set_adaptor` being the one adaptor that does not format its own errors.

Against that: hashing and formatting are both the customization points a user-defined type owes the standard
library, and `std::hash` has always been specialized inside the adaptor headers. Putting `std::formatter`
beside `format_as` in the proxy's own header makes the two protocols for one type live in one file, and makes
the two customization points consistent with each other. The standard library itself cannot do this -- its
`formatter<pair>` lives in `<format>`, because `<format>` is allowed to know about `<utility>` and `<utility>`
must not depend on `<format>` -- but the arrow only points that way for types the standard owns. For ours it
cannot: `<format>` will never know about a proxy of ours, so the proxy is the only place the specialization
can go without a separate header to remember. Nothing here waits on P3070R0: what a formatter needs is the
proxy's formattability, and that is ours to give. #20 records the other half of the
argument, which this library is the worked example of: a proxy nested inside its container cannot be named by
either hook, because the template arguments will not deduce through
`container<T, A>::proxy_reference`. A hidden `format_as` needs no deduction, being found by ADL on the proxy
itself, and the `formatter` is a partial specialization constrained by a concept that recognizes the proxy
through the adaptor it names ([the-adl-firewall](#the-adl-firewall)): that is how the proxies here stay members
of their adaptors and format all the same.

### total-lookups-on-the-container

Every `bit_fixed_set` lookup is total over `key_type`, because `std::set`'s is: a key outside `[0, N)` names no
element, so it answers *absent* rather than reaching the bit. `bit_block_container` asserts `is_valid` on
every position it accepts and offers no total spelling of any of these — that is the layering working, not a gap
in it. **The precondition is the sequence's; the guard is the container's.**

One of those guards stops a *write* rather than a read: without it an out-of-range key clears a bit in
whatever follows the blocks.

Two findings from running the suite against the unguarded header are worth keeping, because they are why it
sweeps every width rather than one convenient one. **No single width exposed all six operations and no single
key did either** — at N = 8 over `uint8_t` every one of them was clean for `key == N`, so a narrow
single-block set proves nothing on its own. And `erase` was an out-of-bounds *write*, while `upper_bound`
failed on its returned value rather than on memory at all: `find_next`'s `++n` wrapped before it could test
the bound, so it answered with a real element where `end()` was due. That second one is why this stays a gate
on the jobs that build without sanitizers.

### the-sieve

The sieve lives in `examples/include/opt/set/`, and the path is the point: it is the library's worked example
and its bench, which makes it a *subject* rather than library code. Under `include/`, the file set's
`BASE_DIRS`, CMake would put it on the build interface: `#include <opt/set/sieve.hpp>` would compile for an
`add_subdirectory` or `FetchContent` consumer and fail for a `find_package` one, the header never being installed,
and for the first kind its names would be reachable library names. So the names are in `opt::`, and the
configure-time file-set check covers all of `include/`, so a stray directory is caught rather than shipped
([the-interface-line](#the-interface-line)). It speaks **one**
vocabulary: an ordered set of integers. It runs over `std::set`, `std::flat_set`, `bit_fixed_set` or
`bit_set`; the candidates are `iota(2, n)` converted to the set, and a sift is `erase`. Nothing
bitset-shaped takes part.

There is no second sieve running the same shape over `std::bitset` or `boost::dynamic_bitset` through
`bit_set_view`. Two sieves would be two vocabularies for one algorithm, and what one would demonstrate -- that the
view reconciles `set(pos)`/`reset(pos)` with `insert`/`erase` -- is what `test/src/bits/bit_set_view.cpp` asserts
directly, without a sieve in the way. An example earns its place by showing something no test does.

The bench is dynamic containers only, so it compares like with like: `std::flat_set`, `std::set` and
`bit_set`, one of each representation -- sorted vector, node-based, dense bitmap. A `bit_fixed_set<N>`
sifting a universe it was sized for is not measuring what a growing set is, so it stays in the test and off
the bench.

Two ladders cross. The bound doubles from `2^10` to `2^20` rather than stepping decades, because `bit_set`
changes block count on exactly those boundaries: a doubling walks whole blocks, and a cache knee reads as a
knee instead of smeared across a decade. The second ladder is `Block`, ours alone -- `std::set` and
`std::flat_set` have none to choose. At a given `n` the footprint is the same count of bits whatever the
block, so that axis is not about memory: it varies the block count against the cost per block, and since the
sift is a strided write ([index-walks](#index-walks)) and near width-indifferent while the scans are not, a
block effect should show up as a divergence between the three benches rather than as one number.

The ladder is what makes the shape visible, and the shape is not a constant factor. `std::flat_set` is the
**fastest** of the three at the bottom rung -- 0.10 ms against `std::set`'s 0.17 -- and by `2^16` it is 215 ms
against 7 ms, because `erase` on a sorted vector is linear and the sieve does about `n log log n` of them.
That is quadratic, and no amount of contiguity buys it back. A single measurement anywhere on that curve
would have supported whichever conclusion the author already held.

Which is why `std::flat_set` stops at `2^16` while the other two run to `2^20`: the ceiling is a measurement
decision before it is a budget one. Past that rung each doubling costs five times the last and establishes
nothing the slope has not already shown -- carried to `2^20` it takes **107 seconds for a single iteration**,
against `std::set`'s 0.94 and `bit_set`'s 0.0056, and spends six and a half minutes of every Release `ctest`
run to re-derive a line visible four rungs earlier. Those three numbers are a one-off measurement at `2^20`;
with the ceiling in place the bench does not produce the first of them.

Under `ctest` a bench is a smoke test -- that it runs, not what it costs -- so the test invocation passes
`--benchmark_min_time=1x`. Timing the full ladder in CI would put minutes of `std::set` at `2^20` into every
run for a number a shared runner cannot make meaningful anyway. A manual run takes the defaults.

The test keeps the static types alongside the dynamic ones, `bit_fixed_set<N>` with `std::set` and
`std::flat_set`, since the sieve is an example before it is a bench, and it keeps the degenerate widths: a
two-candidate sieve runs `sift_primes1` to exhaustion, and a three-candidate one returns from `filter_twins`
before it has a triple.

### the-unbounded-sieves

The sieve above needs its bound before it sifts anything: `generate_candidates` materializes every candidate
below `n` first, which is what makes its space `O(n)` and what makes "give me the next prime" a question it
cannot answer. Two variants remove the bound, and they remove it in different directions.

The **incremental** sieve (O'Neill, *The Genuine Sieve of Eratosthenes*, JFP 19(1), 2009) has no candidate
array at all. It keeps one entry per prime found so far -- the next composite that prime will strike, and which
prime strikes it -- so its space is `O(pi(n))` and there is no `n`: `incremental_sieve::next()` generates
forever. That is the whole of its case, because it is **slower**, and measurably: 11.3 ms against
`sift_primes1`'s 0.394 ms at `2^16`, about **29 times**. A map lookup per candidate is a large constant beside
a strided write, and O'Neill's own point is that the naive "sieve" that trial-divides is not the sieve at all.
It stops at `2^16` on the bench for the same reason `std::flat_set` does: the constant is the finding, and four
more rungs would only re-derive it.

The **segmented** sieve is the one that pays. Base primes below `sqrt(n)` once, then a single window walked
over the rest, so peak memory is `O(sqrt(n) + W)` whatever `n` is. `Window` is a template parameter carrying
its own extent, which makes `bit_fixed_set<W>` the natural argument: a compile-time width that allocates
nothing in the loop, refilled as `~Window()` and struck and read once per segment.

It is **faster than the bounded sieve, not merely thriftier**: 4.05 ms against 7.01 ms at `2^20`, about
**1.7 times**, and ahead at every rung. Asymptotically the two do the same work; the difference is that a
32768-bit window sits in L1 for its whole segment while a 2^20-bit sieve is a megabit walked with a stride.
Less memory and less time is not the usual direction for that trade to run, which is why it is recorded.

Both are held to the bounded sieve by the test rather than described as equivalent -- every bound including
the degenerate ones, and two window widths, so a window shorter than its tail is exercised beside one that
swallows it. The bounds stop at `N` because one container under test is `N` bits wide, and a fixed width is a
capacity ([width-is-capacity](#width-is-capacity)).

`generate_candidates` is total in `n`. `iota(2, n)` is a precondition violation below 2, which asking the sieve for
the primes under nothing would reach -- a question with an answer, none, rather than a contract to break. The two
unbounded sieves compute their own inner bounds, so a guard there is worth more than a comment.

### the-sequence-ladder

The bit benches ask different questions and so run different ladders, which is the point rather than an
inconsistency.

Each bench is one pairing with **one variable**: `bit_vector` against `std::vector<bool>`, a set against
`std::set`. Same reading, same storage, ours against theirs -- so a row measures the implementation and nothing
else.

`benchmark/src/sequence/access.cpp` is the **endgame-database** question: a dense flat array
indexed by a ranked position, where a lookup costs a cache miss and nothing else, so its ladder runs 8 KiB to
32 MiB and reports latency per random read rather than bytes per second.

What they have found so far, on GCC 15.2, `-O3 -march=native`, x86-64:

- **A random bit read costs the same in `bit_vector` as in `std::vector<bool>`** -- 3.0 ns in cache, about
  5.8 ns at 32 MiB for both, and construction is parity too. Representation does not matter to a lookup; only
  footprint does. For a database that is the useful negative result: what buys a lookup is fewer bits per
  position, not a better container.
- **`std::count` is slow over both, and not because libstdc++ counts words.** It reads about 1.2× to 1.3×
  slower over `bit_vector` than over `std::vector<bool>` on GCC, which looks like a specialization of
  `std::count` for `std::vector<bool>::iterator` and is not one: a word-at-a-time count is two orders of magnitude
  ahead, not twenty percent, which is what `xstd::bit_count` measures at
  ([the-sequence-aggregates](#the-sequence-aggregates)). The gap is the generic path over two different proxy
  iterators, and under clang it runs the other way, ours being the vectorizable one.
The sequence ladder's fixtures are the expensive part of a `ctest` smoke run: a 32 MiB fixture is filled a bit
at a time, and the whole file costs about eleven seconds where the other three cost five between them. That is
proportionate, and it is checked rather than assumed ([the-sieve](#the-sieve) records what happens when it is
not).

## Platform and tooling, continued

### uint128-support

A Block is any `xstd::unsigned_integer`, and three families of them are 128 bits wide: the compiler's own
`unsigned __int128` on GCC and Clang; the Microsoft STL's `std::_Unsigned128`, which is what `xstd::uint128`
names on every MSVC-ABI target, clang-cl included; and the two third-party classes xstd adapts,
`absl::uint128` and `boost::int128::uint128`. Only the first is a scalar. The other three are **classes**, and
that difference is the whole of this section.

`bits::detail::intrin` forwards `countl_zero`, `countr_zero` and `popcount` to `xstd::countl_zero` and friends,
which are `<bit>`'s domain plus one overload per integer class, reading the blocks each type already holds.
Forwarding straight to `<bit>` would implement the seam against `std::unsigned_integral` — a **closed** concept
no class can join — while it is constrained on an open one, so every 128-bit integer class would satisfy the
interface and then fail inside the body. The seam is where that body is chosen, and the forwarding is what makes
the other three families usable.

Three consequences follow, none of them obvious from the forwarding change alone.

**The calls are qualified, so they bind where they are written.** `xstd::popcount(block)` is a dependent call
by a qualified name, and ADL does not apply to qualified names, so its candidates are the overloads visible at
`intrinsic.hpp` — not at the instantiation. An adapter included afterwards declares its overload too late to be
one. A translation unit reaching for an integer-class Block therefore includes that adapter first; the test
tree does it in `test/block_types.hpp`, above every container header. The same rule decides where
`test::block_basis` can be spelled, which is why it sits in that header rather than in one of its own: a
separate header could not be relied on to sort below the adapters.

**A Block being a class breaks two assumptions that a scalar hid.** A one-block predicate returning `lhs & rhs`
into a `bool` copy-initializes, and so needs an **implicit** conversion; an integer class offers only an explicit
`operator bool`. So `block_intersects` and `block_includes` compare against a zero `Block`, while `not` and `!=`
reach `bool` by a **contextual** conversion, which an explicit operator satisfies.

And the proxies. Both of them convert to their `value_type` and to nothing else — `bool` for the sequence
proxy, the key for the set proxy — which is the shape
`std::vector<bool>::reference` has on libstdc++, libc++ and the MSVC STL alike. Neither declares an
`operator==` or an `operator<=>`: every comparison is the built-in one, or the key's own, reached through that
one conversion, so it serves two proxies over **different** Blocks exactly as it serves two over the same one.

Neither proxy carries a second, templated implicit conversion, to any class type implicitly constructible from the
`value_type`, because any such class with a non-template `operator==` in a namespace associated with the proxy — its
Block's, its key's, its key mapping's — gives every comparison a second, equally good reading: convert both sides to
the `value_type`, or convert both sides to that class. Two user-defined conversions of equal rank tie, and the tie
costs `equality_comparable` and with it `std::ranges::equal`. The 128-bit integer classes are one instance, a Block
naming its own namespace; excluding `xstd::integer` and adding comparisons exact in both operands would patch that
instance and leave every other one open, a user's strong type in a key-mapping namespace among them. A set's
comparator is no way in: it reaches the proxy only as a direction. What going without it costs is copy-initializing
a class from a proxy, `C c = *it;`, which needs two user-defined conversions; direct-initialization, `C c(*it);`,
needs one and still works.

Of the namespaces that list names, only the key's is associated with a proxy
([the-adl-firewall](#the-adl-firewall)), so the Block's contributes no candidate: Boost.Int128's non-template
`operator==(uint128, bool)` and its mirror are not found for a sequence proxy over a `boost::int128::uint128`
Block, so there `r == true` and `r == 1` compile and answer as they do over every other Block.

**Two facts, two flags, because one flag would conflate them.** `TEST_HAS_UINT128` names the compiler's 128-bit
**builtin**: a scalar, and a `std::unsigned_integral`. It feeds `block_types`, which every suite grades over, and
`test/src/bits/block/type_traits.cpp`, which asserts exactly those `std` traits of each block — both right to
assume a builtin. `TEST_HAS_MSVC_INT128` names what an MSVC-ABI target has instead, `std::_Unsigned128` under
the same `xstd::uint128` spelling: a usable Block, but a class, so not `is_integral`, not `is_unsigned`, and not
something `<bit>` will take.

One flag covering both would put a class-typed Block into `block_types`, and so into every suite at once, which
breaks sixteen MSVC targets — the `std_set` comparisons among them, whose helpers assume a Block is a `std`
integral and whose per-type cost is superlinear. So the three integer classes sit together in
`wide_block_types`, feeding only the two suites that pay a `static_assert` or one linear pass per type. MSVC's is
named beside Abseil's and Boost's, which is what it behaves like, rather than beside the builtin whose spelling
it shares.

The assert beside each flag is an **implication**, that where the flag is on the basis is really there. An
equality with `std::unsigned_integral` would be the wrong question in both directions: false for
every integer class that works as a Block, and true in dialects where `<bit>` still declines the type. The
converse is not worth asserting either — a basis a flag declines to use costs coverage, not correctness.

Abseil's and Boost's are optional: `test/ext_int128.hpp` detects each by `__has_include`, so a build without
them drops it from the Block lists rather than failing. MSVC's needs no dependency at all. All three earn their
place by being the types that catch a container assuming a Block is a scalar — every defect above was invisible
to every builtin.

### exception-escape-nolints

The primitives in `test/include/test/sequence/primitives.hpp` that hold a `BOOST_CHECK_THROW` carry
`NOLINT(bugprone-exception-escape)`. Each guards on the position and calls a member the standard specifies as
throwing outside the width — `at` — checking in the other arm that it does throw.

The check reads the callee's signature and cannot read the guard, so it reports every instantiation. The claim
being made is that a primitive answering about a position inside the width never throws, and it is what would
fail the suite loudly were the guard ever wrong.

### clang-tidy-false-positives

Four findings are suppressed because the checker cannot see what makes them right:

- `bugprone-signed-bitwise` on `detail/bits::shl` and `::shr`, whose count is cast to `int`. The two checks
  that govern this leave no third option, and both were measured: `absl::uint128` declares a single shift,
  `operator<<(uint128, int)`, so an **unsigned** count reaches it by a signedness-changing conversion and
  `-Wsign-conversion` rejects it, while an **int** count is a signed operand of a bitwise operator and
  `bugprone-signed-bitwise` rejects that. The type's own operator decides which is right, and the count is a
  bit position within one block, so the signedness the check objects to cannot be reached.
- `misc-redundant-expression` on a reflexivity check, which cannot be written without naming the object twice.
- `bugprone-std-namespace-modification` around each `namespace std` block, which holds only what
  `[namespace.std]/2` allows: a specialization of a standard library template for a program-defined type.
  `hash`, `tuple_size`, `tuple_element`, `formatter` and the rest are written alike, each block bracketed by
  `NOLINTBEGIN` and `NOLINTEND` outside its braces, with the clause as the reason.
- `modernize-avoid-c-style-cast` on `sequence_adaptor`'s `is_static_width_owner`, where it points at the
  `Store` in `owns(Store)` and offers to make it a `static_cast`. There is no cast on that line. `owns` is
  `constexpr auto owns(storage) -> bool` in `ownership.hpp`, the only entity that name denotes, and `Store` is
  a non-type template parameter rather than a type. The same `owns(Store)` is written on sixteen other lines
  here and none of them is flagged;
  what is particular is the namespace-scope variable template, whose initializer stays value-dependent until
  instantiation. clang-tidy 23 alone emits it -- 22 and 24-SVN carry every other finding for the same
  translation unit and not this one -- so it is a release's bug rather than a reading of the code, and the
  suppression can go once the ladder is past 23.

An eighth is the reverse case, and the one to be careful with: **the check is right about the language and
wrong about the compilers.** `readability-redundant-typename` on clang-tidy 22 asks for the `typename` to go
from `std::same_as<typename std::remove_const_t<Bits>::block_type, Block>` in `blit_source`'s partial
specialization. P0634 made `typename` optional only in listed contexts, and a **template argument is not one of
them** — GCC 14 rejects the elision outright, *type/value mismatch at argument 1*. An alias-declaration **is**
on the list, so the block type is named through one and the template argument is a simple-template-id that
needs no `typename` and trips no check. The same pattern already names `allocator_of` in
`bit_block_container`'s test. Measured, because "clang-tidy suggested it" is not evidence that it compiles.

The same check, from the other side, is simply right. On the set reading's
checklist it asked for the `typename` to go from five **parameter-declarations** -- the
`typename C::value_type const* first` and `... last` of `set_size_t` and `set_size_t_allocator`, and the
`[](typename C::key_type)` that `erase_if` is handed -- and a parameter-declaration IS on P0634's list, which
is why the parameters beside them already omitted it. It flagged those five and left
`std::initializer_list<typename C::value_type>` and `std::same_as<typename C::size_type>` alone on the very
same lines: the rule above, drawn by the checker itself. Template argument, the keyword stays; parameter
declaration, the keyword goes. The ones it left are not an oversight and are not to be "finished".

A ninth had a fix rather than a suppression. `modernize-use-nullptr` reads the `0` in `(a <=> b) < 0` as a
null pointer constant, which is the same false positive `-Wno-zero-as-null-pointer-constant` already covers on
the compiler side. Every site in the test sources says `std::is_lt`, `std::is_gt` or `std::is_eq` instead --
the standard's own names for those three questions, which are clearer than the comparison against a literal
and leave the check on to catch a real one. Do not spell them back.

A tenth is the one to be most careful with, because the check is right about the spelling and wrong about what
the code has to do. `modernize-type-traits` asks for `std::tuple_size_v<C>` in place of
`std::tuple_size<C>::value`, in `array_tuple_element`'s guard and again in `array_bool`'s compound
requirement. A checklist is asked of types that FAIL it -- that is the whole of what a checklist is for
([concepts-are-tested-on-ready-made-types](#concepts-are-tested-on-ready-made-types)) -- and the two spellings
fail differently. `tuple_size<C>::value` is a nested name, so for a `C` with no `tuple_size` at all the
substitution fails in the immediate context and the constraint answers false. `tuple_size_v` is a variable
template whose initializer instantiates OUTSIDE it, and the same `C` is a hard error no requires-expression
can catch: `not array_tuple_element<C>` stops compiling and reads *incomplete type `std::tuple_size<C>` used
in nested name specifier*. Measured on GCC 15 and 16 both, against a type with `tuple_size` and no
`tuple_element` and a type with neither, before either spelling was kept.

The `tuple_element` half of that same check IS taken, and the asymmetry is the point. `tuple_element_t` is an
alias template and substituting into one is transparent, so its failure stays in the immediate context and the
requirement still answers false. The concept therefore reads `typename std::tuple_element_t<0, C>;`, keeping
the `typename` -- a type-requirement needs it, and an alias-template specialization is a *type-name* under
[expr.prim.req.type], which is the half of the fix-it that would otherwise have left a bare type standing
where only an expression may. Half of one check's advice was worth taking and half of it was not, and nothing
short of running both tells you which half.

### clang-crashes-on-a-foreign-bulk-source

Asking whether a bulk operator accepts a view over a foreign storage -- `ours &= bit_span(a_std_bitset)`, in
a `requires`-expression or written out -- crashes clang 18 and clang 20 alike with an internal error, and does
so without the windowed operators as well, so the trigger is the whole view's `&=` seeing a foreign
`sequence_adaptor` as its argument. gcc rejects the expression as it should. The expression is ill-formed
either way, since bulk on a view takes a source of the destination's own block type, so nothing in the library
or the tests spells it; the crash is recorded here so nobody
adds the assertion that would.

### the-coverage-gate

The Coverage job enforces 100% of lines and branches rather than reporting them, and the benchmark tree is not
built there. The second half is not about build time: `gcovr` is told to exclude `benchmark/.*`, and that
exclusion does less than it looks like it does. It drops a benchmark as a SOURCE, but a benchmark is not where
the counted code lives -- it is a translation unit that INSTANTIATES the library's templates, and those
instantiations belong to the headers under `include/`, which is exactly what the report keeps.

What makes that fatal rather than untidy is that the job configures `Debug`, and `benchmark/CMakeLists.txt`
registers a benchmark as a test only when the build type is not `Debug`. So under coverage the benchmarks
compile and never run. Every branch of an instantiation that no test translation unit also makes is therefore
uncovered by construction, and no amount of work on the benchmark can cover it, because the benchmark does not
execute.

A Block-width benchmark shows it. `benchmark/src/set/blocks.cpp` instantiates the two-block arm of the scan for
three 128-bit carriers in one translation unit, a combination no single test unit makes; compiled under coverage,
it raises the per-line branch totals of that arm from 2, 6 and 2 to 4, 11 and 4, nine branches covered by nothing.
A benchmark shape that reaches the missing arm changes the numbers by exactly zero -- which is the proof that the
benchmark never runs, and the reason the gate is answered at the CMake level.

So `XSTD_BITS_BUILD_BENCHMARKS` gates the tree, the Coverage workflow passes it `OFF` through cpp-ci's
`cmake_args`, and the `benchmark/.*` exclusion stays as a second line that costs nothing. A benchmark measures
the library rather than being part of it, which is the stated reason for excluding it; not compiling it
into the measurement is that reason carried through.

#### a hundred per cent measures the source's shape too

Two kinds of gap the gate reports are not about tests at all, and neither can be closed by writing one.

A message whose three code units are spelled as three `static_cast`s on three lines gets a counter of its own for
each of the first two from gcov, which attaches the `std::format` call to the third, so two lines that every test
of that message runs read as never executed -- `#####` against a line whose neighbours both ran, in every
instantiation, every time. Writing the three casts on one line closes it: nothing excused, no test changed, and
the same message printed.

The other is the defaulted comparison's unreachable base branch, which is
[above](#the-comparison-is-a-hidden-friend).

What the two have in common is that the number moves without the program changing. Below a hundred, coverage
measures the tests. At exactly a hundred it also measures the source's shape -- where an argument list wraps,
where a declaration wraps, and whether the line an exclusion pattern is looking for happens to be the line
gcov anchored the counter to. That is a real cost of the gate and it is worth paying, because a gate that
reports instead of enforcing gets read as noise. But it has to be read for what it is: in both, "uncovered" means
"gcov counted this differently than you would have", and going looking for the missing test is going looking for
a test that cannot exist.

## Measurements and citations behind the code

Each of these stands behind a one-line comment in a source file, and holds what that line has no room for, here,
where length costs nothing.

### `std::count` over a bit container measures the standard library, not the container

`benchmark/src/sequence/access.cpp`'s `bm_sequential_count` row is a sweep asked through a generic
algorithm. At 65536 bits, 40% set, medians of seven runs, in nanoseconds:

| compiler and library | `vector<bool>` | `bit_vector` | ratio |
| :--- | ---: | ---: | ---: |
| g++-14, libstdc++ | 43690 | 52930 | 1.21x |
| clang++-20, libstdc++ | 83680 | 19232 | 0.23x |
| clang++-20, libc++ | 73 | 19146 | 262.27x |

Same two containers, same bits, and the answer runs from four times faster to two hundred and sixty
times slower. libc++ **specializes** `std::count` for its `vector<bool>` iterator — `__count_bool` in
`<__algorithm/count.h>`, one popcount per word — and sweeps the lot in 73ns. libstdc++ specializes
`fill` for that iterator and not `count`, so there both sides walk bit by bit through a proxy and the
remaining difference is codegen: clang turns our indexed read into something four times quicker than
it manages for theirs, gcc does not.

So the row is not a comparison of containers and not a gap to close. There is no portable way to make
`std::count` count words for a container defined outside the standard library: libc++ reaches its own
iterator by overloading inside its own namespace, and neither `std::count` nor `std::ranges::count`
offers a customization point a user-defined bit container could hook. What the row says is how much a
sweep costs when it is asked through a generic algorithm.

`bm_sequential_bit_count` asks the same question through `xstd::bit_count` instead: one popcount per
block, 91ns, 73ns and 74ns in those same three configurations. It does not care which library or which
compiler, because the loop is ours either way. Level with libc++'s specialized count at 1.01x, and
some five hundred times quicker than what libstdc++ offers for the same question. `std::vector<bool>`
has no `bit_count` to put beside it, which is why that rung is ours alone.

### Why the bidirectional steps guard on a zero width

`set_adaptor`'s `next_position` and `prev_position`, the set iterator's two steps, guard on `is_zero_width`
rather than asking the storage. The
exclusive scans take a position as a precondition and a zero width has none to give, so they assert
there.

Each step also states its own precondition beside the storage's. Forward, that this is not `end()`,
which is what the scan's `is_valid` comes to. Backward is the one worth having, because it is
**stronger** than anything below it: `exclusive_find_prev` asserts `any()` and `is_valid(n - 1)`, and
`--begin()` passes both while there is nothing below to find. Measured under `NDEBUG`, without the
guard: at a two-block extent it falls into the arm meant for the lower block and answers the highest
position there, which is the key it started from, so a reverse walk never ends; at four blocks and at
a run-time width it reads past the blocks.

### What `[set]`'s synopsis leaves out for a packed set

The `[set]` and `[container.requirements]` clauses under `test/src/spec/` assert `[set]`'s synopsis
declaration by declaration, with `std::set<std::size_t>` as the model. Three families are left out, each for a reason the packing
gives:

- `node_type`, `extract`, `insert(node_type&&)` and `merge`: there is no node. A position is a bit in
  a block, so there is nothing to unlink and hand over, and nothing to relink.
- the `template<class K>` heterogeneous overloads: they participate only where
  `Compare::is_transparent` is valid, and `key_compare` is `std::less<key_type>` or `std::greater<key_type>` here as
  it is on the `std::set` models. Neither side has them, so asking would hold the model to a line the model
  does not answer either. The owners declared with `std::less<>` or `std::greater<>` do have them, and are checked
  against `std::set` under the same transparent comparator outside the clauses.
- `insert_range` and the from_range constructors: `[set.cons]`'s C++23 lines, kept apart so the model
  can be held to them where its standard library has them (`__cpp_lib_containers_ranges`).

`pointer` and `const_pointer` are dropped from the `[associative.reqmts]` typedefs for the same
reason as the nodes: packed bits have no address.

### AddressSanitizer aborts where the allocator throws

`test/include/test/sanitizer.hpp` guards the rows that assert a width past what the blocks can hold
reaches the allocator. AddressSanitizer answers an allocation it will not serve by **aborting**, where
the C++ allocator answers with `std::bad_alloc`, so on a sanitized build the process is gone before
the catch. Measured on this tree: the report is `allocation-size-too-big`, and
`allocator_may_return_null=1` only renames it to `out-of-memory`, because the throwing `operator new`
calls `ReportOutOfMemory` on a null return rather than throwing.

Every other leg answers those rows, which is measured too: the nine failures that led to the guard
were all sanitized builds or a discarded temporary an optimizer elided, and the msvc and mingw legs,
which are neither, never failed on them at all.

Declare anything a guarded block needs **inside** it. The clang legs compile with `-Weverything
-Werror`, so a variable named outside a block that is the only thing using it is an unused-variable
error on exactly the legs the guard is for.

### Why the byte-exchange question is a template

`test/include/test/bit_exchange.hpp` asks whether a type constructs from `(xstd::from_blocks, b)`, and converts
to and from `B` through `xstd::bit_convert`. There is no trait for those, so they are concepts.

Being a **template** is not incidental. A bare requires-expression over concrete types puts a
non-dependent requirement in the immediate context, where GCC reports an unsatisfied constraint as a
hard error rather than as false — which is precisely what an assertion of the form "this is NOT
admitted" must not be.

### `[array.tuple]` does not divert `std::format`

A `std::tuple_size` specialization is what makes a type tuple-like, and it does not divert
`std::format`. `[format.tuple]/1` provides the tuple formatter "for each of pair and tuple", naming
the two class templates rather than admitting tuple-like types, and `[format.range.fmtkind]` never
asks `tuple_size_v<R>` — it asks `R::key_type`, and `tuple_size_v` of the reference type for the map
case alone. `std::array` is the proof by example: tuple-like, a range, and it prints as a range. The
bracket assertions in `test/src/bits/detail/format.cpp` are what pins this.

### The try-doors return what the draft spells

P3981R0 changed `try_push_back` and `try_emplace_back` to return `optional<reference>` once P2988R12's
`optional<T&>` was adopted; libstdc++ 16 still returns the pointer P0843R14 gave them. The checklist
asks the model for the name, and `TheTryDoorsReturnTheOptionalReferenceTheDraftSpells` asks the
packing for the signature the draft spells.

**There is no `try_append_range`, on purpose.** P0843R14 gave `inplace_vector` a third try-door that
appended as many elements as fit and returned an iterator to the first one left over. P3981R0 proposed
to return a `borrowed_subrange_t` instead, and the LEWG discussion of that change led to P4022R0
(Revzin, Wakely, Kamiński, February 2026), which removes the member from C++26 altogether. Its two
reasons are the ones any packing would inherit. A partial insertion is neither a success nor a failure,
so the `try_` name promises a contract the function does not keep. And a returned `subrange` converts
to `bool` the opposite way round from the `optional<reference>` of the other two doors: truthy means
something was left over, not that the call succeeded. The paper defers the design to C++29.

The current draft has no `try_append_range`, so the bounded owners have none either. libstdc++ 16
still ships P0843R14's version, and the suite asks it of no implementation, the model included.
When C++29 settles what the member means, it is added here and tested in the same change.

### Why a checklist spells `tuple_size<C>::value`

`modernize-type-traits` asks for `tuple_size_v<C>`, and a concept asked of types without `tuple_size`
cannot take it. A checklist is asked of types that *fail* it — that is the whole of what it is for — and
the two spellings fail differently.

`tuple_size<C>::value` is a nested name, so for a `C` with no `tuple_size` at all the substitution
fails in the immediate context and the constraint answers false. `tuple_size_v` is a variable
template whose initializer instantiates *outside* the immediate context, and the same `C` is a hard
error no requires-expression can catch. Measured rather than assumed: the `_v` spelling turns
`not array_tuple_element<C>` into "incomplete type `std::tuple_size<C>` used in nested name
specifier".

The `tuple_element` half has no such problem and the rewrite is taken — an alias template substitutes
transparently, so its failure stays in the immediate context. The `[array.tuple]` clause puts `tuple_size`
only to types that have it, so it takes `std::tuple_size<T>` whole, as the base it must derive from, and
needs neither spelling.

`[array.tuple]`'s element half also only exists for a non-empty array: `tuple_element<I, array<T, N>>`
Mandates `I < N`, so element zero is a question that cannot be put to a width of nought — on the
packing or on `std::array` itself.

### Why the byte exchange is named rather than spelled as a conversion

`from_blocks` and `xstd::bit_convert` take a field of bits in and hand one out, at the one extent where the
question has a single answer: a static width is a capacity under the set reading and the other side's
own width both, so position `n` here is bit `n` there. Nothing truncates, nothing grows, nothing
throws, and the round trip is the identity in both directions.

They are **named** rather than spelled as a conversion, which is the one thing `explicit` could not
buy. A contiguous range of unsigned integers already means something at this reading: `from_range`
reads it as a range of *keys*. So the same argument had two meanings a tag apart, and both compiled:

```cpp
bit_fixed_set<256>(std::from_range, blocks)   // {0, 5} -- the values are keys
bit_fixed_set<256>(blocks)                    // {0, 2} -- the values are blocks
```

`explicit` guards against a conversion nobody asked for. It does nothing about a reader misreading
one that *was* asked for, and that is the failure available here. A name does: `from_blocks` says which
reading of the argument is meant, at the call site, where the reader is. `std::bitset` spells its own
exit `to_ullong` for the same reason, Boost spells this pair `from_block_range` and `to_block_range`,
and the way out is `xstd::bit_convert`, one free function for every pair of widths.

They are named **by a concept** rather than by a type. `std::bitset` appears nowhere in them, which is
the point: what these two admit is an unsigned integer or a sequence of them, whose layout the
language and the sequence state between them, or a field of bits read on `std::bit_cast`'s terms —
trivially copyable, on a little-endian target, with room for N bits. So `std::bitset<N>` rides in on
the same rule as `unsigned long long`, and a big-endian target or a type too small for N is not
admitted: a call that fails to compile rather than one quietly wrong.

The integer family is the one a set reader can still misread, and the name is what answers it:
`bit_fixed_set<32>(xstd::from_blocks, 5u)` is the set of positions the value five has, `{0, 2}`, and not the
set `{5}`.

### What each storage says for a key it cannot hold

Every other member of the set reading is total over `key_type`: `contains`, `count`, `find`,
`lower_bound`, `upper_bound`, `equal_range` and `erase(key)` all answer for a key past the width
rather than refuse the question. The two that write cannot — there is nowhere to put it — and the
three storages differ because the reasons differ:

| storage | for a key past the width |
| :--- | :--- |
| dynamic extent | grows to admit it, and past `max_size()` says `std::length_error` |
| inplace extent | grows within its capacity, and past it the blocks say `std::bad_alloc` |
| static extent | says `std::out_of_range`, as `std::bitset<N>::set` does for a position past N |

A domain, a capacity and a representable size — three different limits, and none of them silent. The
dynamic width's ceiling is asked at this reading rather than at the storage, which has none of its
own, so the set reading keeps `length_error` where `boost::dynamic_bitset` answers `bad_alloc`.

### The static owner's equality is written out

A static owner's equality is its one member's: every instance carries the same width, so the arms
have nothing to choose between. It is written out rather than defaulted because a defaulted
comparison compares the bases first, and the base is `allocator_base_type`, an empty class whose own
defaulted `operator==` can only answer true. That call is a branch no input can send the other way,
and an unreachable branch is a hole in a coverage gate that admits no test. Saying the member outright
is the same comparison with nothing dead in it.

### The sequence reading's byte exchange, and why it declines a window

Byte `j` holds the positions `[8j, 8j + 8)` least significant bit first, so a fixed width over the
same positions agrees byte for byte with any other and the exchange is a copy rather than a walk.

It is named rather than spelled as a conversion for the reason the set reading gives: a sequence of
unsigned integers is one argument with two readings. This reading cannot hit the `from_range`
collision itself — its `from_range` wants `can_grow`, and anything that can grow has a dynamic extent,
which turns the exchange off — but one door with two spellings across two readings would be worse
than either spelling alone.

The vocabulary is the thing to read twice at this reading: `bit_array<32>(xstd::from_blocks, 5u)` is a packed
array of bool — true, false, true, then twenty-nine more false — and not the set `{0, 2}` that the
same bits spell one reading over.

**Not through the storage's bytes on a window**, which is the whole of why `is_window` is asked. A window
is a bit offset and a size of its own into storage it does not span: its position zero is not the
storage's, so its bytes are not the storage's bytes and reading them would hand back the wrong ones. A width
test does not catch it, since a window over a static container reports the *container's* extent rather than
its own size, so `bit_convert` asks `is_windowed` and reads a window from its offset, a block at a time,
as a run-time width. A view that is not a window spans the whole container, so its bytes are that
container's and it converts through them.

### Two places where the coverage gate decides the layout

`front()` and `back()` are both preconditions in [sequence.reqmts], and `back()`'s is the one that
subtracts: on an empty sequence `offset() + size() - 1UZ` wraps, and the reference handed back names a
position no storage has. Each assert is spelled over four lines rather than one, because gcovr
excludes an assert by a pattern anchored at the start of a line.

The static owner's defaulted `operator==` is kept on a single line for the mirror-image reason:
gcovr's `--exclude-unreachable-branches` matches the line carrying `= default;`, and gcov anchors a
defaulted comparison's branches at the declaration's first line. Split across lines — which
clang-format will do to any such declaration long enough to wrap — the exclusion stops matching and
the dead base comparison fails the 100% branch gate.

### `modernize-avoid-c-style-cast` on `owns(Store)`

clang-tidy 23 points at the `Store` in `owns(Store)` and offers to rewrite it as a `static_cast`, having
read the call as a C-style cast of a parenthesized type. There is no cast on that line.

`owns` is a function — `[[nodiscard]] constexpr auto owns(storage) -> bool`, in `ownership.hpp` — and it is
the only entity that name denotes, so `owns(Store)` can only be a call, `Store` being a non-type template
parameter rather than a type. The same `owns(Store)` is written at sixteen other
sites in this library and none of them is flagged; what is particular about this one is the
namespace-scope variable template, whose initializer is value-dependent until instantiation.

It is suppressed rather than respelled: writing `Store == storage::owned` instead would inline the one
function that exists so nobody has to.

## The storage's own measurements

### `blocks_for` is total over every `size_t`

How many blocks a run-time width needs, none at width zero. It is said as boost's `calc_num_blocks` says
it — divide, then round up by the remainder — because that *cannot* overflow, where
`align_up(n, bits_per_block)` adds first and wraps for the 63 widths above `max_width`, rounding them
to zero blocks that a one-block floor would then turn into one. A guard against that wrap is a guard against a
spelling; this spelling has nothing to guard. It is public because that totality is the claim, and a
`static_assert` is the only way to make it without asking an allocator for two exabytes.

### Equality over the shared prefix

`ranges::equal` over the shared prefix, not over the two block ranges: on two sized ranges it compares
`size()` first and answers false without looking at an element, which is the one case this asks about.
Taking the prefix as an **iterator pair** keeps the answer and gets the algorithm, which lowers to a
`memcmp` on trivially comparable contiguous blocks where `all_of` over a zip stays an element loop —
2.15us to 1.29us over 4700 blocks. The other two block walks cannot follow: `bit_includes` and
`bit_intersects` do bitwise work per block and have no such algorithm.

### The saturating sum, and whose ceiling it is

Base positions and count more, saturated at the top of `size_t` rather than wrapped: the one addition
every growth is spelled through. A wrapped sum is small, so it passes the ceiling it was meant to fail
and then sizes the blocks for far fewer positions than the operation goes on to write; a saturated one
fails that ceiling, which is what an unrepresentable width should do.

The ceiling itself is not the storage's, because the counterparts disagree about it. `std::vector`
throws `length_error` for a size it cannot represent, and the sequence reading says so. The set
reading refuses a key past the widest it could grow to. `boost::dynamic_bitset` has no ceiling at all
— a width it cannot hold reaches the allocator and answers `bad_alloc`.

Two `max_size` answers sit over the same blocks: the storage's, clamped to the whole blocks a `size_t`
counts, and `std::vector<bool>`'s, clamped further still, a random access range's positions being counted
by a `difference_type`.

### The block span, and what an assertion per block costs

`operator[]` as a range rather than one block at a time. The write side carries the subscript's write-side
contract once for the range; the read side carries nothing and exists one build short of the write
side. `operator[]` asserts its index, and an assertion per block is a loop the vectoriser leaves alone.
A Release build never sees it — a loop over the blocks reaches the memcpy floor there by itself —
but an assert-on build pays **3.4x** for a bounds check on an index the caller just produced in order.

### `erase_unused` asks the value what the other arm asks the compiler

At a static width the `if constexpr` settles it. At a run-time width, whether the last block has a
tail to erase is a property of a width the type is not given until it runs, so the arm has to ask the
value. Without the test the mask runs on every call at every width, and where the width is an exact
multiple of the block it is a read-modify-write that changes nothing: flip at 128 bits measures
**7.8ns without it against 2.3ns with**, and boost, which has the same `if`, measures
2.3ns.

### Why the byte exchange is shifts, and where it is a copy

Byte `j` holds the positions `[8j, 8j + 8)`, least significant bit first, which is what every
contiguous bit container lays them out as whatever its block width. Said in **shifts** and not a
`memcpy`, for three reasons at once: the answer does not depend on the order a block stores its own
bytes in, both directions stay `constexpr` where a `memcpy` is not, and the arithmetic is the same on
every block width, so two widths over the same positions agree byte for byte.

On a **little-endian** target the byte a position lands in does not depend on the block width, so
there the shifts and a straight copy of those bytes are the same answer and only one of them is a byte
at a time. Over the eight kilobytes of 2^16 positions, measured in one process so the ratio is the
machine's own: **9.70us by shifts against 0.07us by memcpy**, a factor of about a hundred and
forty-five. The shifts answer the two cases a copy cannot — a constant expression, where `memcpy` does
not exist, and a big-endian target, where those bytes are not these blocks.

### The one unreachable line

`is_valid`'s empty case is called only from an `assert`, and a zero-width `bit_block_container`
has no member that reaches one. It is not removable either: MSVC's `/W4` rejects a bare `n < N` as
always false (C4296). It carries a trailing `GCOVR_EXCL_LINE`, which drops that one line from the
denominator rather than counting it as reached.

### Growing with ones takes the tail before the blocks grow

The tail above `size()` in the last block is clear by the invariant, and becomes the first new bits.
Which block and which bits is read off the **old** width, so it is taken before the blocks grow; the
write itself comes after, because `m_blocks.resize` is what refuses a count these blocks cannot hold,
and a refused growth that had already dirtied the tail would leave the storage with a width it no
longer matches.

Measured on blocks that hold three: refused at a width of 25, the next `resize(20)` came back with
every bit above 9 set and `count()` at 8 where 1 was set.

### Why `blocks_for`'s totality is asserted at compile time

The claim is that `blocks_for` is total: the widths that would wrap ask for more blocks than the
blocks will ever hold. Asserting it by *growing* to such a width instead asks `std::allocator` for
2^61 bytes, which is not a question two of this tree's CI legs will answer — a sanitized build
**aborts** on a request that size rather than reporting `std::bad_alloc`, and an optimizer may drop
the `new`/`delete` pair of an unused temporary altogether, so the request is never made and nothing
is thrown. Both were measured on exactly these assertions.

A `static_assert` is stronger besides: it names the block count rather than inferring it from an
exception.

The widths in question are the sixty-three above `max_width`, where `n + bits_per_block - 1` overflows
to a sum below `bits_per_block`, the division rounds it to **zero** blocks, and the floor turns that
into **one** — a container claiming `SIZE_MAX` positions in eight bits. Dividing first, each of them
asks for one block more than the widest whole number of them, which is a count no allocator will
serve. That is also where every saturated sum arrives, the two composing.

### `sizeof` is always a multiple of `alignof`

The width slot is a `size_t`, or the blocks' alignment where that is wider, and the whole is then
rounded up to the class's own alignment. That last step is not slack in the layout test: a `sizeof`
is always a multiple of an `alignof`, so the sum alone names sizes no class can have. Blocks of four
bytes under a `size_t` width sum to twelve, and twelve is not a size a type aligned to eight can be;
sixteen is, and sixteen is what the class already was. Written without the round-up, the assertion
asks the bounded column for the impossible, which only a leg compiling that column over such blocks reports.

## Design choices for a `bitset` data structure

> "A `bitset` can be seen as either an array of bits or a set of integers. [...]
> Common usage suggests that dynamic-length `bitsets` are seldom needed."
>
> Chuck Allison, [ISO/WG21/N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf), November 25, 1991

The above quote is from the first C++ Standard Committee proposal on what would eventually become `std::bitset<N>`. The quote highlights two design choices to be made for a `bitset` data structure:

1. a sequence of `bool` versus an ordered set of `int`;
2. fixed-size versus variable-size storage.

Thirty years of use have added a value to each axis. The first choice has a third answer that the quote itself takes for granted — a `bitset` that offers **both** readings on purpose, which is what `std::bitset` and `boost::dynamic_bitset` actually are, and which this library declines ([interop-not-a-bitset](#interop-not-a-bitset)). And the second is not one choice but two, because **size** and **capacity** need not move together: fixing both gives `std::bitset`, letting both vary gives `std::vector<bool>`, and the pairing the quote had no word for is a **dynamic size over static capacity**, which allocates nothing and yet resizes, and which C++26's `std::inplace_vector` finally makes expressible. Both tables below are laid out on that one axis, so they read cell for cell.

A `bitset` should also optimize for both space (using contiguous storage) and time (using CPU-intrinsics for data-parallelism) wherever possible.

## The current `bit` landscape

The landscape is three readings crossed with three storages, so it is a three-by-three, and the C++ Standard Library and Boost between them fill six of its nine cells:

|                                  | static size and capacity     | dynamic size, static capacity        | dynamic size and capacity |
| :------------------------------- | :--------------------------- | :----------------------------------- | :------------------------ |
| **ordered set of `std::size_t`** | —                            | —                                    | `std::set<std::size_t>` <br> `std::flat_set<std::size_t>` |
| **sequence of `bool`**           | `std::array<bool, N>`        | `std::inplace_vector<bool, N>`       | `std::vector<bool>` |
| **`bitset`**                     | `std::bitset<N>`             | —                                    | `boost::dynamic_bitset<>` |

Three cells are empty and three more are filled by something of the **wrong representation**, which are two different complaints:

- `std::set` and `std::flat_set` are **sparse**: a whole element per *actual* element, where a dense set spends a single bit per *potential* one.
- `std::array<bool, N>` and `std::inplace_vector<bool, N>` are **unpacked**: a whole byte per `bool`, eight times what the bits need. `std::vector<bool>` is the one standard sequence that packs, and it is the one everybody regrets.

Notes:

1. Both `std::bitset` and `boost::dynamic_bitset` are not clear about the interface they provide. E.g. both offer a hybrid of sequence-like random element access (through `operator[]`) as well as primitives for set-like bidirectional iteration (using non-Standard GCC extensions `_Find_first` and `_Find_next` in the case of `std::bitset`). That ambiguity is why they have a row of their own here rather than appearing in one of the two above it: a `bitset` is a width of bits that has not said how it is to be read.
2. It [has been known](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2130.html#96) for over two decades that providing a variable-size sequence of `bool` through specializing `std::vector<bool>` was an unfortunate design choice.
3. For ordered sets, there is a further design choice whether to optimize for **dense** sets or for **sparse** sets. Dense sets require a single bit per **potential** element, whereas sparse sets require a whole element per **actual** element. The break-even is therefore the element's width: at a 64-bit `std::size_t`, if less (more) than 1 in 64 elements (1.5625%) are actually present, a dense representation will be less (more) compact than a sparse one — 1 in 32 (3.125%) if the sparse set stores 32-bit integers instead.
4. `std::inplace_vector` is C++26 and its `bool` is not a specialization: [P0843](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0843r7.html) declined to repeat `vector<bool>`, so that cell is occupied but unpacked.
5. Only `boost::dynamic_bitset` allows storage configuration through its `Block` template parameter (defaulted to `unsigned long`).
6. The `Block`, `Allocator` and `Compare` parameters are omitted from both tables. They configure a storage; they do not place a container in the landscape, and spelling them out obscured the one axis the columns are measuring.

## A reimagined `bit` landscape

The aforementioned issues can be resolved by implementing a single-purpose container for each cell of the design space. **This library fills every cell of the first two rows**, every one of them dense and packed, leaves the `bitset` row to the standard and Boost, and adds a row that the other table has no answer to at all. Every name below is in namespace `xstd`, so only the foreign ones carry a qualifier:

|                                  | static size and capacity     | dynamic size, static capacity     | dynamic size and capacity |
| :------------------------------- | :--------------------------- | :-------------------------------- | :------------------------ |
| **ordered set of `std::size_t`** | `bit_fixed_set<N>`          | `bit_bounded_set<N>`              | `bit_set` |
| **sequence of `bool`**           | `bit_array<N>`               | `bit_bounded_vector<N>`           | `bit_vector` |
| **a reading of blocks**          | `bit_set_view` / `bit_span` over <br> `std::array<Block, K>` | over <br> `std::inplace_vector<Block, K>` | over <br> `std::vector<Block>` |

The columns are the three storages the one underlying vehicle is parameterized on — `std::array` fixes size and capacity, `std::inplace_vector` varies size within a fixed capacity, `std::vector` varies both — so a cell is a reading crossed with a storage, and nothing else. The outer two columns are the ones the current landscape already has; the middle is the pairing it never named.

The first two rows are **containers**, one per cell, and the third is not a container at all. Blocks are a width of bits that has not said how it is to be read, and this row is how you say which reading you meant, which is the whole of what [retrofitting](#choosing-a-reading-over-blocks) means. You do not have to adopt a container to get a reading; you point a view at the bits you already have. There is no `bitset` row: `std::bitset<N>` and `boost::dynamic_bitset<>` keep their cells of the table above, and a bitset crosses into either reading by copying its blocks ([interop-not-a-bitset](#interop-not-a-bitset)).

Notes:

1. Each container is clear about the interface it provides: sequences are random access containers and ordered sets are bidirectional containers.
2. The `bitset` row is gone rather than reproduced. `std::bitset` and `boost::dynamic_bitset` are faulted above for being unclear about which interface they offer; the answer here is to make choosing a reading **explicit at the call site** and to keep no hybrid of our own. `xstd::bit_convert<xstd::bit_fixed_set<N>>(bs)` and `xstd::bit_convert<xstd::bit_array<N>>(bs)` read a `std::bitset<N>` as a set and as a sequence, and a run-time width is planned to cross through `xstd::bit_convert`.
3. A view over an owner is a view over the storage that owner wraps: `xstd::bit_set_view(s)` over a `bit_fixed_set<N>` deduces `xstd::bit_set_view<std::array<std::size_t, K>, N>`, the blocks and width the set is stored in, and `decltype` is how you name the result. The deduction guide for a plain storage is constrained to non-owners, so an owner and the storage inside it do not tie.
4. The variable-size sequence of `bool` is named `xstd::bit_vector` and decoupled from the general `std::vector` class template.
5. All containers use a dense (single bit per element) representation. Variable-size sparse sets can be provided by `flat_set`, either in [Boost](https://www.boost.org/doc/libs/1_80_0/doc/html/boost/container/flat_set.html) or in [C++ 23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1222r4.pdf).
6. The names above are the short ones, which fix `Block` to `std::size_t` and so take only the width, or nothing at all in the dynamic column where there is no width to give. Each has a `basic_` form that leaves the block open, and for a set the key before it: `xstd::basic_bit_fixed_set<Key, Block, N>`, `xstd::basic_bit_array<Block, N>` and their inplace siblings, and `xstd::basic_bit_set<Key, Block>` and `xstd::basic_bit_vector<Block, Allocator>` down the dynamic column. So `xstd::bit_set` is an alias, not a template, and `xstd::basic_bit_set<std::size_t, std::uint8_t>` is how a block is chosen.
7. Every static-width name takes `xstd::bit_align`, a type transformation rounding its width up to whole blocks so that no block carries an unused tail: `xstd::bit_align<xstd::bit_array<120>>` is `xstd::bit_array<128>`. The inplace names take it too, rounding their capacity. That costs nothing in storage at a width already spanning whole blocks, and removes the tail-restoring mask from `fill`, `flip` and the left shift.
8. Every static-width name also takes `xstd::bit_least` and `xstd::bit_fast`, which keep the width and store it in the smallest or the fastest block that holds it, as `std::uint_least8_t` and `std::uint_fast8_t` do: `xstd::bit_least<xstd::bit_fixed_set<9>>` is two bytes, a `std::uint16_t`, and past 64 bits it is several `std::uint64_t`. `xstd::bit_enum_set<E>` is `bit_least` over the enumeration's fixed set. Least then align, `bit_align<bit_least<X>>`, is the compact form with no unused tail ([block-and-width-transformations](#block-and-width-transformations)).

The **middle column** is what allocates nothing and yet carries a run-time width. Its blocks are a `std::inplace_vector` where the standard library provides one (`__cpp_lib_inplace_vector`) and a `boost::container::static_vector` elsewhere, so those two names exist everywhere; only over `std::inplace_vector` are they usable in a constant expression, which `XSTD_BITS_HAS_CONSTEXPR_BOUNDED` says.

Ownership is deliberately **not** a fourth **column**. A view is not a fourth storage: it takes the shape of whatever it views, which is why the third row spans the same three columns as the two above it rather than standing beside them. `xstd::bit_subspan` is the one that stays out of the table, because it narrows a sequence to a window rather than choosing a reading. All of them are described under [retrofitting](#choosing-a-reading-over-blocks) below.

## Requirements for `set`-like behaviour

Looking at the above code, the following four ingredients are necessary to implement the Sieve of Eratosthenes:

1. **Bidirectional iterators** `begin` and `end` in the `set`'s own namespace (to work with range-`for` and the `<ranges>` library);
2. **Constructors** taking a pair of iterators or a range (in order for `std::ranges::to` to construct a `set`);
3. A **nested type** `key_type` (in order for `std::format` to use `{}` delimiters: [format.range.fmtkind] picks `range_format::set` for a range that has one);
4. A **member function** `erase` to remove elements (for other applications: the rest of a `set`'s interface).

`xstd::bit_fixed_set<N>` implements all four of the above requirements. Note that Visual C++ support is finicky at the moment because its `<ranges>` implementation cannot (yet) handle the `xstd::bit_fixed_set<N>` proxy iterators and proxy references correctly.

## Choosing a reading over blocks

Blocks are neither a set nor a sequence: they have no `begin` of their own that reads bits, because `begin` is one
name and there are two readings. `xstd::bit_set_view` and `xstd::bit_span` are how you say which you meant.

```cpp
auto blocks = std::array<std::uint64_t, 2>();
auto const s = xstd::bit_set_view(blocks);   // the set reading of those bits

s.insert(42);                                // sets bit 42 of blocks[0]
assert(s.contains(42));
assert(std::format("{}", s) == "{42}");      // formats as a set
```

The view supplies what the blocks lack: bidirectional iterators, a nested `key_type`,
`insert`/`erase`/`contains`, and the set predicates. `xstd::bit_span` does the same for the sequence reading,
and `xstd::bit_subspan` for a window into one. A view reads the blocks themselves, so it reaches the
block-parallel paths rather than reading a position at a time. A `std::bitset<N>` hands out no blocks, so it is
read by copying instead: `xstd::bit_convert` into an owner of the reading you mean.

This is why ownership is not a fourth column of the table above: a view is not a fourth kind of container, it is
the same readings pointed at storage someone else owns.

### Printing

The snippets above use `std::format`, and `std::print` works the same way, with nothing to include beyond `<format>` and nothing to switch on:

```cpp
std::print("{}\n", primes);   // {2, 3, 5, 7, 11, ...}   the set reading, in braces
std::print("{}\n", flags);    // [false, true, ...]      the sequence reading, in brackets
std::print("{::#x}\n", primes);
```

Each proxy reference carries its own `std::formatter`, so a container that hands the proxy out brings the formatter with it. Nothing is specialized for a container: every one of them is already a range, so [`[format.range.formatter]`](https://eel.is/c++draft/format.range.formatter) formats it once its reference is formattable. The braces-versus-brackets split is the standard's, not ours — `[format.range.fmtkind]` picks `range_format::set` for a range with a `key_type` — so each reading prints in its own vocabulary without being told to. Each proxy also keeps a hidden-friend `format_as`, which is fmt's own per-type hook, so a consumer who formats with fmt gets the same output without this library depending on fmt to build or to test. The two hooks cannot drift: the `std::formatter` reads the value by calling `format_as`, rather than reaching for it a second way of its own.

## Data-parallelism

The `filter_twins` above walks the primes one at a time, because that is all `std::set` can do. A dense container can answer the same question a **block at a time**, without iterating at all. A prime is a twin exactly when it has a neighbour two away, so shifting the whole set by two in each direction and intersecting gives every twin in a handful of instructions per block:

```cpp
template<class X>
auto filter_twins_parallel(X const& primes)
{
    return primes & (primes << 2 | primes >> 2);
}
```

That is the same set the loop produces, and on a `bit_fixed_set<128>` it is four shifts, two ors and an and over two blocks — no iterator, no branch per element, no comparison. The elementwise form remains the one in `examples/include/opt/set/sieve.hpp`, because it is the form `std::set` and `std::flat_set` can also run and the benchmark needs all three on the same algorithm.

The shifts read the way they do because of the bit layout: element `0` is the least significant bit of the first block, so `<<` moves toward larger elements. What it does **not** do is make the set order the bitstring order — under this layout those are two different walks over the same blocks, and the FAQ below draws both.

which has as output:
<pre>
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}
{3, 5, 11, 17, 29, 41, 59, 71}
</pre>

### Sequence of bits

What blocks lack for the set reading to be built on top of them is iterators. `xstd::bit_set_view` supplies them: bidirectional `begin`/`end` (and `cbegin`/`cend`/`rbegin`/`rend`/`crbegin`/`crend`) plus a nested `key_type`, so blocks format like a set out of the box — as the snippet under [choosing a reading](#choosing-a-reading-over-blocks) shows. It reads the storage a block at a time rather than a position at a time, which is what makes the scan block-parallel.

## Documentation

The interface for the class template `xstd::bit_fixed_set<N>` is the coherent union of the following building blocks:

1. An almost **drop-in** implementation of the full interface of `std::set<int>`.
2. An almost complete **translation** of the [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) member functions to the [`std::set<int>`](http://en.cppreference.com/w/cpp/container/set) naming convention.
3. The single-pass and short-circuiting **set predicates** from [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html), as the free algorithms `xstd::bit_includes`, `xstd::bit_intersects` and `xstd::bit_disjoint`.
4. The bitwise operators from [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) and [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html) reimagined as composable and data-parallel **set algorithms**.

The **full** interface of `xstd::bit_fixed_set` is `constexpr`.

### 1 An almost drop-in replacement for `std::set<int>`

`xstd::bit_fixed_set<N>` is an ordered set of integers over a static width, providing conceptually the same functionality as `std::set<int, std::less<int>, Allocator>`, where `Allocator` statically allocates memory to store `N` integers. In particular, `xstd::bit_fixed_set<N>` has:

- **No customized key comparison**: `xstd::bit_fixed_set` uses `std::less<int>` as its fixed comparator (accessible through its nested types `key_compare` and `value_compare`). In particular, the `xstd::bit_fixed_set` constructors do not take a comparator argument.
- **No allocators**: `xstd::bit_fixed_set` is a set of non-negative integers over a static width and does not dynamically allocate memory. In particular, `xstd::bit_fixed_set` does **not provide** a `get_allocator()` member function and its constructors do not take an allocator argument. Its allocating counterpart `xstd::bit_set` does provide both — the allocator follows the storage column, not the set reading.
- **No splicing**: `xstd::bit_fixed_set` is **not a node-based container**, and does not provide the splicing operations as defined in [p0083r3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0083r3.pdf). In particular, `xstd::bit_fixed_set` does **not provide** the nested types `node_type` and `insert_return_type`, the `extract()` or `merge()` member functions, or the `insert()` overloads taking a node handle.

- **No container exchange**: `std::flat_set` hands its underlying container out with `extract() &&` and takes one back with `replace(container_type&&)`, which is how you build one cheaply and how you get the sorted vector back out. `xstd::bit_fixed_set` has neither name, and has the capability twice over — see the `from_blocks`/`bit_convert` bullet below, and [the comparison in design.md](#the-bytes-they-agree-on).

Minor **semantic differences** between common functionality in `xstd::bit_fixed_set<N>` and `std::set<int>` are:

- the `xstd::bit_fixed_set` member function `max_size` is `constexpr`, and at a static width its value is a constant expression usable wherever `N` is. It is a **member** rather than a `static` member function because `std::set`'s is a member: each reading takes the shape its own counterpart spells, which is why the sequence reading's middle column has a `static` one instead — `[inplace.vector.capacity]` spells all four of `capacity`, `max_size`, `reserve` and `shrink_to_fit` static there, and `xstd::bit_bounded_vector<N>::capacity()` answers without an object accordingly ([design.md#max-size-is-the-bits](#max-size-is-the-bits)). So for the set reading `s.max_size()` is a constant expression and `decltype(s)::max_size()` does not compile.
- the `xstd::bit_fixed_set` iterators are **proxy iterators**, and taking their address yields **proxy references**. The difference should be undetectable. See the FAQ at the end of this document.
- every key at once is `~xstd::bit_fixed_set<N>()`, the complement of the empty set, where `std::bitset` has `set()`.
- `xstd::bit_fixed_set<N>` exchanges bits with any field of `N` bits through a **named pair**, the tagged constructor `bit_fixed_set<N>(xstd::from_blocks, b)` and `xstd::bit_convert<B>(s)`, which also crosses with anything else that has bit storage, not through an untagged constructor or a conversion operator. What they admit is named by a concept rather than by a type: an unsigned integer or a sequence of them, whose layout the language and the sequence state between them, or a field of bits that `bits::detail::bit_layout` reads on `std::bit_cast`'s terms, trivially copyable on a little-endian target with room for `N` bits — so `std::bitset<N>` rides in on the same rule as `unsigned long long`, and a big-endian target or a type too small for `N` fails to compile rather than converting quietly. The widths are the same `N` and a static width is a capacity under this reading, so position `n` here is bit `n` there: nothing truncates, nothing grows, nothing throws, and the round trip is the identity. It is `constexpr` at every width, and a copy rather than a walk over positions.

  A name rather than a conversion, because the integer family is the one a set reader can still misread, and only a name answers it at the call site, where the reader is: `bit_fixed_set<32>(xstd::from_blocks, 5u)` is the set of positions the **value** five has, `{0, 2}`, not the set `{5}` ([design.md#the-bytes-they-agree-on](#the-bytes-they-agree-on)). `from_blocks` is a constructor on an owner; `bit_convert` also reads a set view, which spans a whole container and so has that container's bytes. The run-time-width `xstd::bit_set` has no `from_blocks`, a `std::bitset` naming one `N` that a growing set has no single value for; it crosses through `bit_convert` alone, which carries the width along.

With these caveats in mind, all static-width, defaulted comparing, non-allocating, non-splicing `std::set<int>` code in the wild should continue to work out-of-the-box with `xstd::bit_fixed_set<N>`.

### 2 An almost complete translation of `std::bitset<N>`

Almost all existing `std::bitset<N>` code has **a direct translation** (i.e. achievable through search-and-replace) to an equivalent `xstd::bit_fixed_set<N>` expression, with the same and familiar semantics as `std::set<int>` or `boost::flat_set<int>`.

| `std::bitset<N>`                | `xstd::bit_fixed_set<N>`              | Notes                                           |
| :---------------                | :-----------------              | :----                                           |
| `bs.set()`                      | `bs = ~decltype(bs)()`          | the complement of the empty set                 |
| `bs.set(n)`                     | `bs.add(n)` <br> `bs.insert(n)` | `out_of_range` past `N`, where `std::bitset` throws it too |
| `bs.set(n, v)` <br> `bs[n] = v` | `v ? bs.add(n) : bs.pop(n)`     | `out_of_range` past `N` on the insert; the erase is total |
| `bs.reset()`                    | `bs.clear()`                    | returns `void` as `std::set<int>`, not `*this` as `std::bitset<N>`  |
| `bs.reset(n)`                   | `bs.pop(n)` <br> `bs.erase(n)`  | total over the key: erasing what is not there is the no-op returning zero |
| `bs.flip()`                     | `bs = ~bs`                      | not an operator of `std::set<int>`              |
| `bs.flip(n)`                    | `bs.erase(n)` if `bs.contains(n)`, <br> else `bs.insert(n)` | `out_of_range` past `N`, as the insert it is |
| `bs.count()`                    | `bs.size()`                     | |
| `bs.size()`                     | `bs.max_size()`                 | `constexpr`; a constant expression at a static width |
| `bs.test(n)` <br> `bs[n]`       | `bs.contains(n)`                | total over the key: a position past `N` is one the set does not hold |
| `bs.all()`                      | `bs.size() == bs.max_size()`    | |
| `bs.any()`                      | `not bs.empty()`                | |
| `bs.none()`                     | `bs.empty()`                    | |

The semantic differences between `xstd::bit_fixed_set<N>` and `std::bitset<N>` are:

- `xstd::bit_fixed_set<N>` answers `max_size()` as a `constexpr` member, where `std::bitset<N>` answers the same question with `size()`;
- `xstd::bit_fixed_set<N>` splits its members by what `[set]` can promise. **Asking is total**: `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range` and `erase(key)` all answer for a key outside `[0, N)` — it is a key the set does not hold, which is an answer and not a precondition violation, exactly as `std::set::find` returns `end()` for any key it does not hold. **Writing is not**: `insert` has nowhere to put such a key, and throws `out_of_range` as `std::bitset<N>` does for a position past `N`. Leaving it undefined for a performance benefit does not survive measurement: on the sieve at `N = 2^16`, best of twenty-five, the guard costs nothing — 188.0µs against 188.1µs, and 75.1µs against 75.1µs over 65536 inserts — because the comparison is against a compile-time constant and never taken.

Functionality from `std::bitset<N>` that is not in `xstd::bit_fixed_set<N>`:

- **No string constructors and no `to_string`**: a bit string is a string reading's vocabulary, planned as `bit_string`, and `std::bitset`'s until then. The set reading declines it as it declines the rest of that vocabulary, and crossing costs one call either way: `xstd::bit_convert<std::bitset<N>>(s).to_string()`, and `xstd::bit_convert<xstd::bit_fixed_set<N>>(std::bitset<N>(str))` back. What a set prints *as itself* is `{2, 3, 5}`, which is [printing](#printing) above.
- **No integer constructor and no integer conversion operator**: here what is missing is the language's *unnamed* doors and not the capability. The byte exchange does both under a name, `xstd::bit_fixed_set<32>(xstd::from_blocks, 5u)` and `xstd::bit_convert<unsigned>(s)` — and it is named for exactly the reason a constructor would be the wrong spelling: `bit_fixed_set<32>(xstd::from_blocks, 5u)` is the set of positions the **value** five has, `{0, 2}`, and not the set `{5}`. A constructor cannot say which of those it meant; a name can.
- **No I/O streaming operators**: `operator<<` and `operator>>` are `[bitset.operators]`'s, and go with `to_string`. A set formats instead, in its own vocabulary.

Formatting a set needs nothing beyond the standard library: `std::format` and `std::print` take it in its own vocabulary, as [printing](#printing) above shows. A consumer who formats with [{fmt}](https://fmt.dev/latest/) instead gets the same output through the proxy's hidden-friend `format_as`, and pays for that hook without this library depending on fmt either to build or to test. Hashing is **not** on that list: `std::hash<xstd::bit_fixed_set<N>>` is specialized, over [Boost.Hash2](https://github.com/boostorg/hash2), and every value this library compares it also hashes, so `a == b` implies `hash(a) == hash(b)` under every reading ([design.md#the-hashing-invariant](#the-hashing-invariant)). The `hash_append` hook is there beside it, for a caller wanting an algorithm other than the defaulted `fnv1a_64`.

### 3 Set predicates from `boost::dynamic_bitset`

The set predicates `is_subset_of`, `is_proper_subset_of` and `intersects` from `boost::dynamic_bitset` have **identical semantics** in `xstd::bit_fixed_set`, spelled through the free algorithms `xstd::bit_includes` and `xstd::bit_intersects`, named after `std::ranges::includes` and a nonempty `std::ranges::set_intersection` and taking their arguments in that order. Note that these set predicates are not present in `std::bitset`. Efficient emulation of these set predicates for `std::bitset` is not possible using **single-pass** and **short-circuiting** semantics.

| `xstd::bit_fixed_set<N>`                                    | `boost::dynamic_bitset<>`  | `std::bitset<N>`             |
| :-----------------------                                    | :------------------------  | :---------------             |
| `xstd::bit_includes(b, a)`                                  | `a.is_subset_of(b)`        | `(a & ~b).none()`            |
| `xstd::bit_includes(b, a) and not xstd::bit_includes(a, b)` | `a.is_proper_subset_of(b)` | `(a & ~b).none() and a != b` |
| `xstd::bit_intersects(a, b)`                                | `a.intersects(b)`          | `(a & b).any()`              |

### 4 The bitwise operators from `std::bitset` and `boost::dynamic_bitset` reimagined as set algorithms

The bitwise operators (`&=`, `|=`, `^=`, `-=`, `~`, `&`, `|`, `^`, `-`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_fixed_set` with **identical syntax** and **identical semantics**. Note that the bitwise difference operators (`-=` and `-`) from `boost::dynamic_bitset` are not present in `std::bitset`. The `operator-` can be emulated for `std::bitset` using the identity `a - b == a & ~b`.

The bitwise-shift operators (`<<=`, `>>=`, `<<`, `>>`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_fixed_set` with **identical syntax** and **identical semantics**: a shift by `N` or more empties the set, as it resets a `std::bitset<N>`, where a native unsigned integer has undefined behaviour. The guard costs nothing at a constant distance, which is how move generators shift a board: the compiler proves `n < N` and the code is the unguarded shift. At a run-time distance it is one compare and a conditional move on a single block, and on two or more blocks the known bound lets the compiler simplify the splice, so the guarded shift is the shorter code.

With the exception of `operator~`, the non-member bitwise operators can be reimagined as **composable** and **data-parallel** versions of the set algorithms on sorted ranges. In C++23, the set algorithms are not (yet) composable, but the [range-v3](https://ericniebler.github.io/range-v3/) library contains lazy views for them.


| `xstd::bit_fixed_set<N>`      | `std::set<int>` with the range-v3 set algorithm views                                                |
| :----------------       | :----------------------------------------------------------------------------------------------------|
| `xstd::bit_includes(b, a)` | `std::ranges::includes(b, a)`                                                                     |
| <code>a &vert; b</code>, `xstd::bit_set_union(a, b)` | <code>ranges::views::set_union(a, b)                &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a & b`, `xstd::bit_set_intersection(a, b)` | <code>ranges::views::set_intersection(a, b)         &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a - b`, `xstd::bit_set_difference(a, b)` | <code>ranges::views::set_difference(a, b)           &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a ^ b`, `xstd::bit_set_symmetric_difference(a, b)` | <code>ranges::views::set_symmetric_difference(a, b) &vert; std::ranges::to&lt;std::set&gt;() </code> |

The bitwise shift operators of `xstd::bit_fixed_set<N>` can be reimagined as set **transformations** that add or subtract a non-negative constant to all set elements, followed by **filtering** out elements that would fall outside the range `[0, N)`. This can also be formulated in a composable way for `std::set<int>`, albeit without the data-parallelism that `xstd::bit_fixed_set<N>` provides.

<table>
<tr>
    <th>
        xstd::bit_fixed_set&ltN&gt
    </th>
    <th>
        std::set&ltint&gt
    </th>
</tr>
<tr>
    <td>
        <pre lang="cpp">auto b = a << n;</pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | std::views::transform([=](auto x) { return x + n; })
    | std::views::filter   ([=](auto x) { return x < N; })
    | std::ranges::to&ltstd::set&gt;()
;</pre>
    </td>
</tr>
<tr>
    <td>
        <pre lang="cpp">auto b = a >> n;</pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | std::views::transform([=](auto x) { return x - n;  })
    | std::views::filter   ([=](auto x) { return 0 <= x; })
    | std::ranges::to&ltstd::set&gt;()
;</pre>
    </td>
</tr>
</table>

## Frequently Asked Questions

### Iterators

**Q**: How can you iterate over individual bits? I thought a byte was the unit of addressing?  
**A**: Using proxy iterators, which hold a pointer and an offset.

**Q**: What happens if you dereference a proxy iterator?  
**A**: You get a proxy reference: `ref == *it`.

**Q**: What happens if you take the address of a proxy reference?  
**A**: You get a proxy iterator: `it == &ref`.

**Q**: How do you get any value out of a proxy reference?  
**A**: They implicitly convert to `int`.

**Q**: How can proxy references work if C++ does not allow overloading of `operator.`?  
**A**: Indeed, proxy references break the equivalence between functions calls like `ref.mem_fn()` and `it->mem_fn()`.

**Q**: How do you work around this?  
**A**: `int` is not a class-type and does not have member functions, so this situation never occurs.

**Q**: Aren't there too many implicit conversions when assigning a proxy reference to an implicitly `int`-constructible class?  
**A**: Yes, copy-initialization `C c = *it;` takes two, which is one too many; direct-initialization `C c(*it);` takes one and works. A proxy reference converts only to its value type, as `std::vector<bool>::reference` does, because a conversion to every class constructible from that type made comparisons ambiguous wherever such a class declares its own `operator==`.

**Q**: So iterating over an `xstd::bit_fixed_set` is really fool-proof?  
**A**: Yes, `xstd::bit_fixed_set` iterators are [easy to use correctly and hard to use incorrectly](http://www.aristeia.com/Papers/IEEE_Software_JulAug_2004_revised.htm).

### Bit-layout

**Q**: How is `xstd::bit_fixed_set` implemented?  
**A**: `bit_fixed_set` uses a `std::array` of unsigned integers, so its storage goes wherever the object does. That is true of the static-width column only: the bounded column holds a `std::inplace_vector` or a `boost::container::static_vector` inline, and the dynamic column a `std::vector`. All three are the same storage vehicle over a different container, which is an implementation detail rather than a name you reach for.

**Q**: How is a set value mapped onto the array's bit layout?  
**A**: Position `n` is bit `n % W` of block `n / W`, for a block of `W` bits. So the **least** significant bit of the first array block maps onto set value `0`, and the most significant bit of the last array block onto set value `N - 1`. That is the conventional layout: `boost::dynamic_bitset` and the mainstream `std::bitset` and `std::vector<bool>` implementations all lay their bits out the same way.

**Q**: I'm visually oriented, can you draw a diagram?  
**A**: Sure, it looks like this for `basic_bit_fixed_set<std::size_t, std::uint8_t, 16>`, each block drawn most significant bit first:

|block |       0|       1|
|:---- |-------:|-------:|
|offset|76543210|76543210|
|value |76543210|FEDCBA98|

**Q**: Why that layout, and not the mirrored one?  
**A**: Because it is very nearly forced. It is not a convention this library follows for the sake of following one, and there are four reasons, of which the first is not really negotiable at all.

**A one-block container must *be* its integer.** `std::bitset` mandates the round trip `bitset<N>(v).to_ullong() == v`, and the language fixes the value side of it: bit `n` of an unsigned integer is `2^n`. So position `i` of an integer read through `from_blocks` is `(val >> i) & 1`, `bit_convert` back to the integer is its inverse, and at one block the stored block is the integer, bit for bit, with no work at all. Mirror the layout and the standard's own round trip becomes a bit reversal in each direction — which is why `design.md` admits the integer family with **nothing to prove**, where a foreign field of bits is trusted on `std::bit_cast`'s terms, and only on a little-endian target ([design.md#the-bytes-they-agree-on](#the-bytes-they-agree-on)).

**The scan primitive is the layout.** A forward step is `bits_per_block * i + countr_zero(m_blocks[i])` — `ctz` and nothing else, because position `n` is bit `n`. A mask is `unit << offset`, one instruction. Mirror it and each grows a correction: `digits - 1 - clz` for the step, `highbit >> offset` for the mask, and a `<<` that lowers to `shr`. That cost is visible here rather than hypothetical — the **reverse** scan pays exactly that subtraction, `last_bit() - countl_zero(...)`, and mirroring would move it onto `find_first`/`find_next`, which is the set reading's common path.

**Growth appends, so the numbering must append too.** The dynamic column is a `std::vector` of blocks and `push_back` is `resize(size() + 1, value)`, so a new position lands at bit `size() % digits` of the last block: the used region of that block grows upward from its low bits, the unused tail stays above it, and nothing already stored moves. That tail-above-the-used-bits invariant is what every block-wise comparison then leans on, a block reading the same as its positions with nothing above the width.

**And the exchange is a copy only because everyone agrees.** `std::bitset`, `std::vector<bool>` and `boost::dynamic_bitset` are all LSB-first, so a field of the same width agrees byte for byte and converts by `memcpy` rather than by reversing the bits of every byte. The per-byte path is measured at 9.70µs against 0.07µs over 2^16 positions, about a hundred and forty-five times; a mirrored layout would pay at least that on every conversion, in both directions, forever.

**Q**: Does that make little-endian mandatory too?  
**A**: For this library's own containers, no. Positions are computed by shifts on block **values**, and `b[j] >> k` is the same number on either byte order, so a big-endian target is correct and merely takes the shift path where a little-endian one copies.

For **interop**, yes, and both doors close at once. The block-range family names `std::endian::native == std::endian::little` in its own constraint, and so does the field-of-bits family, because `std::bit_cast` hands back the object representation rather than the value — where bit `n` of a word lands at the far end of it. So a big-endian build keeps every container and loses every conversion from a field of bits it did not lay out itself.

**Q**: Then how does set comparison stay block-parallel, if the set order is not the blocks' integer order?  
**A**: It is two walks over the one layout, one per reading, each a block at a time rather than a position at a time:

| reading         | walk                                             | the rule it applies |
| :-------------- | :----------------------------------------------- | :------------------ |
| ordered set     | the blocks ascending, through `first_difference` | whoever holds the lowest differing position is **less** — unless the other holds nothing above it, in which case that other is a prefix, and a prefix is less |
| sequence        | the same primitive, ascending                    | whoever holds the lowest differing position is **greater**, position `0` being the sequence's first element |

**Q**: So a set's order is *not* its bitstring's order?  
**A**: No, and that is why there are two functions rather than one ([design.md#two-readings-disagree](#two-readings-disagree)). The bit string orders the blocks from the top down, as `std::bitset` and `boost::dynamic_bitset` mean `<`; the two readings here both walk up from position zero and share `first_difference`, which keeps its own name on the storage rather than being called `mismatch` there.

**Q**: Didn't the first `bitset` proposal argue for the other layout?  
**A**: It did, and this library does not follow it:

> "bit-0 is the leftmost, just like char-0 is the leftmost in character strings. [...]
> This makes converting from and to unsigned integers a little counter-intuitive,
> but the string-ness (or "array-ness") is the foundation of this abstraction.
>
> Chuck Allison, [ISO/WG21/N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf), May 26, 1992

Bit-0 leftmost buys one thing: the bitstring order and the array order agree, which is worth having when a `bitset` is one container. Here it is two readings over one storage and they disagree about order whatever the layout, so that agreement was never on offer — where the copy the conventional layout buys is.

### Storage type

**Q**: What storage type does `xstd::bit_fixed_set` use?  
**A**: By default, `xstd::bit_fixed_set` uses an array of `std::size_t` integers.

**Q**: Can I customize the storage type?  
**A**: Yes. The alias carrying the default is `template<std::size_t N> using bit_fixed_set = basic_bit_fixed_set<std::size_t, std::size_t, N>`; the underlying `template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, class Compare = std::less<Key>> basic_bit_fixed_set` requires the key and the block explicitly. Every cell of the table follows that pattern: a short name that fixes `Block`, and a set's `Key`, to `std::size_t`, and a `basic_` name that does not.

**Q**: Can the block be chosen to fit the width?  
**A**: Yes. `xstd::bit_least<xstd::bit_fixed_set<N>>` is `basic_bit_fixed_set<std::size_t, least_block_t<N>, N>`, one `std::uint8_t`, `std::uint16_t`, `std::uint32_t` or `std::uint64_t` up to 64 bits and several `std::uint64_t` past them, and `xstd::bit_least` takes any fixed-width owner, its key, mapping and direction kept. `xstd::bit_fast` picks `<cstdint>`'s fastest block instead, and `xstd::bit_align` rounds the width up to whole blocks; `bit_align<bit_least<X>>` does both, in that order.

**Q**: What other storage types can be used as template argument for `Block`?  
**A**: Any type modelling the Standard Library `unsigned_integral` concept, which includes (for GCC and Clang) `xstd::uint128`.

**Q**: Does the `xstd::bit_fixed_set` implementation optimize for the case of a small number of blocks of storage?  
**A**: Yes, there are three special cases for 0, 1 and 2 blocks of storage, as well as the general case of 3 or more blocks.

## Future directions

The containers, the views, the valarray reading and the algorithms over them are the library as it stands; the
string reading and, beyond it, regular expressions over bits are the next layer. Each entry here is designed and
not built: what it would add, what it waits on, the decisions already taken that it has to respect, and what the
analysis behind it concluded. Each names the open issue that tracks the direction, where the work and its
discussion continue; an entry leaves this section for the one describing the code when it lands.

### the-string-reading

`std::bitset` speaks three vocabularies, and the string is the one with no reading yet
([#195](https://github.com/rhalbersma/xstd-bits/issues/195)). What a string adds over a sequence is not order,
since both compare lexicographically, but **search and concatenation**: `find`, `substr`, `starts_with`, `append`
and `+`, which prefix codes, tries, CIDR prefixes and k-mers ask of a run of bits. It builds on the sequence
reading's algorithms, `xstd::bit_find` among them ([algorithms-not-members](#algorithms-not-members)), and calls
its word-parallel search as an algorithm rather than owning it. The decisions already taken:

- **Five names, one per column:** `bit_fixed_string<N>`, `bit_bounded_string<N>`, `bit_small_string<N>`,
  `bit_string` and the view `bit_string_view`, on one `string_adaptor` with no window parameter. A borrowed string
  is always windowed, because `remove_prefix` and `remove_suffix` change the view in place; an owned one never is.
  The iterator and reference are member templates of the adaptor over the character type, as the other readings
  nest theirs ([the-adl-firewall](#the-adl-firewall)), and an owner and its view have distinct iterator types.
- **Character 0 is position 0**, the least significant bit of block 0, which is N0128's orientation and the
  storage's one layout. Strings of different lengths share position 0, so `starts_with` and `<=>` stay
  block-wise, and `push_back` and `append` work at the high end as a sequence's do. The cost is at the interop
  boundary: a string converted from a `std::bitset` reads reversed against its `to_string()`, which the
  declarations of the string types and of `xstd::bit_convert` state.
- **The elements are symbols, not bits.** The class template takes the element first, as the set owners take
  their key, then the block, then the defaulted traits and allocator:
  `basic_bit_string<CharT, Block, Traits, Allocator>`, with `bit_string` as `basic_bit_string<char, std::size_t>`.
  `bit_char_traits<char>` lists `symbols = "01"` in rank order and nothing else; the library derives `size`,
  `K = std::bit_width(size - 1)` bits per character, `from_index(r) = symbols[r]` and `to_index(c)`, the protocol `sized_bit_index_mapping` names for keys
  ([the-public-names](#the-public-names)). Symbols are distinct, checked by `static_assert`, and need not ascend,
  so `".#"` makes `.` rank 0. The reference converts to the symbol, so a `bit_string` yields `'0'` and `'1'` with
  `std::string`'s pitfalls included (`if (s[0])` is always true), and `bit_vector` stays the reading whose elements
  are bits; `bit_convert` relabels one as the other in O(1). `bit_char_traits` is not `std::char_traits`:
  `[char.traits.require]` works through pointers to stored characters one at a time, which would rule out the
  block algorithms. A user's alphabet is a user's traits; the DNA alphabet, `symbols = "ACGT"`, is the K = 2 test
  case and appears only in tests and examples.
- **`<=>` is `std::string`'s order.** The first difference is found a block at a time, then the two symbols there
  are compared: the ranks directly where the symbols ascend, the sign flipped for two descending symbols, and one
  lookup in a table of at most 2^K entries otherwise. Display symbols are a run-time argument to `to_string`,
  `text`, `copy` and the formatter, `s.to_string('.', '#')` or `"{:/.#}"`; element symbols are part of the type, so
  a maze never silently equals a code word.
- **Characters are validated where they enter.** Every constructor, `assign`, `append`, `insert` and `replace`
  taking characters throws `std::invalid_argument` outside the symbols, as `std::bitset` does, and is explicit, as
  `std::bitset`'s string constructor is. Parsing packs eight characters per step, the mirror of a block-wise
  `to_string()`, and `bit_string(t).to_string() == t` for every valid `t`.
- **`bit_fixed_string<N>` follows P3094's `std::basic_fixed_string`**, still in LEWG for C++29 and audited against
  its latest revision: structural, the length-preserving half of the interface, static `integral_constant` size
  members ([constant-sizes](#constant-sizes)), `+` giving `N + M`, and `bit_convert` to and from `bit_array<N>`.
  The search members and the default constructor, all `'0'`, are recorded as departures with reasons.
  `"0110"_bs` is a `bit_string` and `"0110"_bsv` a `bit_string_view` of static extent 4 into the template
  parameter object, both validated at compile time, and a `bit_fixed_string` template parameter makes
  `xstd::bit_search<"1011">(text)` a compile-time pattern without a suffix.
- **`[string.capacity]` decides capacity in every column**, not the sequence row's bounded rules: the bounded string
  throws `std::length_error` past `N`, as `boost::static_string<N>` does, and has no `try_` or `unchecked_` doors.
  `+` on two bounded strings returns `bit_bounded_string<N + M>`, as Boost, Folly and P3094 all do, and never
  throws; `+=` and `append` keep the capacity. A smaller capacity converts implicitly and a larger one explicitly,
  checked. The small and dynamic strings keep `std::string`'s `+`.
- **Declined:** `c_str` and `data`, a bit having no address and two symbols no terminator;
  `resize_and_overwrite`; `[string.conversions]`; `[char.traits]`; and shifts and bitwise operators, which
  `[strings]` does not have. `copy(char*, n, pos)` is kept, since it writes text through the symbols into the
  caller's buffer, and it is the way out to `folly::FixedString`, `boost::static_string` and `char[]` alike.
- **Search** is an algorithm over the sequence reading, `xstd::bit_search` after `std::ranges::search`, so
  `bit_span` and `bit_vector` reach it too, and the string's `find`, `rfind` and `contains` call it. For a pattern of up to 64 characters, one mask per block asks
  the question for all 64 start positions at once, `M &= P[i] ? Ti : ~Ti` over funnel-shifted text, which is
  O(n·m/64) and `constexpr`; a longer pattern confirms the first 64 characters' candidates block by block. A
  worst-case guarantee (Two-Way) and SIMD wait for a measurement that asks for them. The oracle is
  `std::string::find` over the same text in `'0'` and `'1'`; the baselines are `std::string::find`, `std::search`
  over `std::vector<bool>`, the Boyer-Moore searchers, glibc `memmem`, StringZilla and CTRE's `ctre::search`.
- **Testing has one oracle.** Each case is written once, as characters: `"0110"s` is the `std::string` and
  `"0110"_bs` the bit string, every operation runs on both, and nothing translates symbols in between. The audits
  are `[string.view]` first and then `[basic.string]`, about 42 leaf subclauses between them, with the declined
  clauses recorded as such ([the-audit](#the-audit)). The counterpart is `std::string` rather than
  `std::basic_string`, whose `compare` goes through `traits::compare` a character at a time.

The opening section plans `string_reading_tag` as a refinement of `sequence_reading_tag`, so that every view and
algorithm taking a sequence takes a string. A sequence's elements are bools and a string's are symbols, so the
first header settles whether a sequence algorithm reads a string's bits or the refinement stands aside. A
multi-bit alphabet, DNA's `"ACGT"` at two bits, is the same template at `K > 1`, addressing its storage as a
sequence of `K`-bit elements does. The work runs in this order: `bit_char_traits`; the adaptor with
`bit_string`, `bit_string_view` and the formatters; `bit_fixed_string` and the two literals; the bounded and small
strings; search, staged as above; compile-time patterns; the audits; and the string row in the README's tables.
Prior art for the bounded and small strings is in [prior-art-by-column](#prior-art-by-column).

### bit-mdspan

A bit matrix, `std::mdspan` over packed bits, adds a dimension to the views row rather than a reading
([#196](https://github.com/rhalbersma/xstd-bits/issues/196)): bitboards, adjacency matrices, parity-check matrices,
cellular automata and binary images are its workloads. `[mdspan.accessor.reqmts]` lets an accessor's `reference`
be other than `element_type&` and its `data_handle_type` other than `element_type*`, so the accessor is the
sequence reading's proxy and iterator: `access(p, i)` is `*(p + i)` and `offset(p, i)` is `p + i`. The constraint
on `reference`, a common reference with `bool&`, is the one `std::indirectly_readable` already imposes on the
iterator, so the accessor carries no new proof obligation.

The layout is the alignment argument one dimension up. `layout_right` unpadded starts row *i* at bit `i · cols`,
so unless `cols` is a whole number of blocks every row straddles blocks and every row-wise operation takes the
funnel-shift path; `layout_right_padded` (C++26) starts each row on a block boundary for at most a block less one
bit per row, which a 64-square board does not pay at all. Padded, a row is a contiguous run of whole blocks, and
so a `bit_span` over the same storage: the sequence reading and the algorithms apply row-wise as they stand, and
the matrix algorithms worth having (Warshall's closure, Gaussian elimination over GF(2), neighbour counts by
shifted rows) are or, xor and and of two rows. A column is a strided run of single bits, one test per row, and
no packing changes that, so an interface should not present a column as a row's peer.

Open before the first header: whether `row(i)` returns a rank-1 `mdspan` through `submdspan`, a `bit_span`, or
both; which extents come first, a static `8 × 8` board or a dynamic matrix; whether `[mdspan.copy]`'s algorithms
go through the accessor or assume `element_type*`; and which ladders ship the padded layouts. There is no owning
counterpart, since the working draft has no `mdarray`: the view sits over an owner someone else holds, as the
views row already does. GF(2) linear algebra on top of it is
[#404](https://github.com/rhalbersma/xstd-bits/issues/404).

### k-bit-elements

An element wider than one bit is a parameter of a reading, not a reading of its own
([#197](https://github.com/rhalbersma/xstd-bits/issues/197)). A generic packed adaptor would commit to no
vocabulary and would bring back the layer of synthesized scans that [one-storage](#one-storage) deleted, and its two
motivating cases land on different readings anyway: an endgame table of `unknown`, `loss`, `draw` and `win` is a
sequence, and a DNA string over `ACGT` is a string. At one bit per element the sequence row is `bit_vector`; at
eight bits per byte-sized element it is `std::vector<unsigned char>`, byte for byte on a little-endian target.
`std::vector<bool>` is the one-bit point of that axis, concealed inside `vector<T>`; declared as a point on it, it
is just a container. [`sdsl::int_vector<t_width>`](https://github.com/simongog/sdsl-lite) ships the sequence row,
and [`seqan3::bitpacked_sequence`](https://docs.seqan.de/seqan3/main_user/classseqan3_1_1bitpacked__sequence.html)
derives the width from the alphabet, which is the refinement to take. Neither has a string reading or a second
reading over the same bits.

The decisions already taken: the width comes from the element's index mapping, `bit_width(size - 1)`, so it needs
no template parameter, and the letter `K` stays a count of blocks; each element is stored least significant bit
first; the proxy converts to the element type as the `bool` proxy converts to `bool`; `<=>` exists only where the
mapping's order is a total value order, so the endgame table, whose `unknown` makes it partial, has `==` alone; and
the bitwise operators drop out by constraint above one bit. The set reading takes no width, since a dense set is
one bit per potential element, and the valarray reading stays `bool`.

Open before the first header: whether the element and its mapping join the existing owners' parameter lists,
defaulted to `bool`, or name a separate family, since `basic_bit_vector<Block, Allocator>` changing touches every
alias, every `rebind` and `bit_rebind` and `bit_resize`; whether a width that does not divide the block straddles
two blocks, as `sdsl::int_vector<9>` does, or pads each block, with power-of-two widths first because they do
neither; and the one kernel that does not exist yet, a SWAR field compare for `count` and `find` over fields. The
string row waits for [the-string-reading](#the-string-reading).

### selectable-small-storage

The small column's storage is `boost::container::small_vector`, and `absl::InlinedVector` and `llvm::SmallVector`
keep up to N elements inline the same way ([#264](https://github.com/rhalbersma/xstd-bits/issues/264)). The
owners are named by what the user gets, so `bit_small_set` and `bit_small_vector` would keep their names whichever
implementation backs them. The bounded column picks its storage from what the standard library provides, so every
translation unit agrees without configuration; several small-vector libraries can be installed at once, so here
the choice is the user's, and it has to hold across the whole program.

The design: one CMake cache variable, `XSTD_BITS_SMALL_VECTOR`, takes `boost`, `absl` or `llvm`, defaults to
`boost`, and becomes a compile definition on the `xstd::bits` INTERFACE target, so every target linking the
library compiles with the same choice. Only the selected implementation's namespace is declared, `inline`, as
`xstd::with_boost`, `with_absl` or `with_llvm`, so users write `xstd::bit_small_set` and the mangled name carries
the implementation: two libraries built with different settings fail to link rather than violate the ODR. The
namespaces are not named `boost`, `absl` or `llvm`, which would hide `::boost` inside `xstd`. Boost.Container stays
required in every case, for the bounded fallback and the `dynamic_bitset` adapter. The container as a template
parameter, `std::flat_set`'s shape, is not the alternative: [owned-bit-storage](#owned-bit-storage) records why
each owner fixes its storage by its name.

Open before the first header: `llvm::SmallVector`'s growth path is defined in `LLVMSupport`, a link dependency on
a header-only library's INTERFACE target, so whether `with_llvm` is worth carrying or Boost and Abseil ship first;
`llvm::SmallVector` takes no allocator, so its owners have no `allocator_type`, as the fixed and bounded owners
have none; whether `absl::InlinedVector` and `llvm::SmallVector` satisfy the owned and resizable storage concepts
as they stand; and each storage's `swap`, moves and `==` deduce different exception specifications, which the
tests assert per implementation. CI runs one leg per implementation, with vcpkg features for the two new ones.

### a-deterministic-perf-gate

Coverage is gated at 100% of lines and branches and time at nothing: the benchmarks run under `ctest` as smoke
tests ([the-sieve](#the-sieve)), so a change that halved a container's speed would pass every check
([#185](https://github.com/rhalbersma/xstd-bits/issues/185)). Wall clock cannot be the gate. Over the full
200-row ladder, pinned to one CPU with nine interleaved repetitions on a freshly booted machine, the median
coefficient of variation is 10.7%, the 90th percentile 17.2%, and five rows of 200 come in under 5%; filtering
for ratios that clear two sigma of the rows' combined variation leaves almost nothing. Hosted runners are worse:
two to four shared vCPUs, no PMU, and a CPU model that changes between runs. Three gaps of 1.15x to 1.3x that the
ladder reported did not survive one-side-per-process isolation, and one changed sign when the same code was linked
into another binary.

The gate is **instruction counts under callgrind**. The same binary on the same input retires the same count to
the instruction, which supports a 1% threshold with no false positives. Each comparison builds the merge base and
the head in one job and compares the ratio; wall clock stays informational, reported with its variation beside
every median, as "no result" above a threshold, with `/proc/cpuinfo` recorded and no comparison across CPU models.
A count says *that* something changed and not what it costs: three instructions that stall can be 9% of a count
and 240% of the time, so a claimed win is confirmed on real hardware, and absolute timings need a self-hosted
runner with `isolcpus`, SMT and turbo off. [what-a-view-costs](#what-a-view-costs) is the method applied once.

The harness has four traps on record: valgrind 3.22 raises `SIGILL` on the AVX-512 `vpternlogq` that
`-march=native` emits, so the gated binary builds at `-march=x86-64-v3`; a harness must assert that it ran, by
valgrind's exit status and by counts that scale with `N`, and measuring at `N` and `kN` and differencing cancels
startup exactly; GCC folds a naive loop into a closed form, so the operation sits between `asm volatile` sinks on
both sides; and code layout moves wall clock between binaries, which is one more reason to compare ratios inside
one. The results would publish as a generated file naming machine, compiler, standard library, resolved `-march`
and date, for the README and this file to cite instead of figures whose provenance is a sentence. Whether the gate
lives here or in a separate repository that pins the library by SHA and builds two SHAs per job is open.

### spec-tests-over-archetypes

The spec suite checks values thoroughly and checks that ill-formed uses are rejected, each
`static_assert(not requires { … })` beside the clause that makes the use ill-formed
([#273](https://github.com/rhalbersma/xstd-bits/issues/273)). Three things the standard libraries' own suites cover are still to come:

- **Archetypes.** The range constructors, `insert(first, last)`, `assign(first, last)`, `from_range` and
  `insert_range` over single-pass input iterators, forward iterators and sized sentinels, handing in exactly what
  the clause asks and no more; the allocator-aware owners over a minimal allocator, with only the members
  `[allocator.requirements]` makes mandatory, and over a fancy-pointer allocator, beside `std::allocator` and
  `std::pmr::polymorphic_allocator`; and keys and values used for no more than `[associative.reqmts]` and
  `[sequence.reqmts]` ask. `test::minimal_blocks` is the storage's archetype already
  ([owned-bit-storage](#owned-bit-storage)).
- **A constant-evaluation driver.** The `XSTD_CONSTEXPR_CHECK` family turns one expression into a `static_assert`
  and a run-time check that cannot drift apart. It runs over the deterministic part of each input builder, edge
  cases and width-gated enumerations, while the seeded random part stays at run time; its `_IF` forms gate the
  `static_assert` on the columns usable in a constant expression, the fixed owners, the dynamic ones through a
  transient allocation, and the bounded ones where `XSTD_BITS_HAS_CONSTEXPR_BOUNDED` is defined. `std::vector<bool>`
  and `std::array` are `constexpr` and serve as models there; `std::set` and `std::flat_set` are not, so the set
  reading asserts each clause's postconditions directly. Enumerations count against `-fconstexpr-steps` and
  `-fconstexpr-ops-limit`, so this driver may need a lower width cap than the run-time one.
- **`requires (I < N)` on `bit_array`'s `tuple_element`.** The adaptor's two specializations carry it, while the two
  `basic_bit_array` forwards to them do not, so an index out of range is a hard error a requires-expression cannot
  observe, and `[array.tuple]/1` stays a declined row of the audit. With the constraint on the forwards it is
  answered.

### the-next-audit

The type-matrix table, `test/src/bits/type_matrix.cpp`, answers whether every operation compiles, or is refused on
purpose, for every owner and view over every block ([#377](https://github.com/rhalbersma/xstd-bits/issues/377)).
Three parts remain:

- **Fast-path assertions** for the dispatch predicates that have none: `plane_is_its_string` and
  `dense_planes`, which send a range of planes to the hash in one `update`, and `structural_blocks_v`, which picks
  the public base that makes an owner structural. Each gets a `static_assert` over the types meant to take the
  path and the types meant not to, as `blit_source`, `block_copies_as_bytes` and `adopts_from` have, so a path
  silently not taken fails a build rather than only a benchmark.
- **A doc-drift check**: a script, run in CI beside the include-order checks, that every backticked identifier in
  `README.md` and this file still names something under `include/` or in the standard.
- **The `doc/audit/` inventories re-run** against the current owners, the bounded ones (a capacity of nought,
  `no_blocks`) and the small ones (rebindable and resizable) first.

### a-module-interface

The library is consumed by inclusion, and `import xstd.bits;` does not exist
([#194](https://github.com/rhalbersma/xstd-bits/issues/194)). Header-only is a real choice, one `find_package`, no
build order, no module cache to invalidate, and the direction is allowed to conclude that a module is not worth
having. The cheap form is the only one to try first: a single `xstd.bits` interface unit that includes the
existing headers in its global module fragment and exports the names, leaving every header where it is,
`#include` working as before, and the change revertible. Partitions mirroring the header tree wait on a
measurement of the cheap form.

It waits on three things. Which of the seven ladders can build and consume a module interface at all, which is
the least uniform corner of the toolchains; a measured clean-build cost for a translation unit that includes the
umbrella, so that a speed-up is a number; and xstd-ints and xstd-misc, which this library includes and which
record the same direction, so the three would move together rather than one importing two that are still
included. A consumer path that imports would be a fourth leg of `consumption.yml`, and the include-what-you-use
comments, written about inclusion, do not map onto exported names mechanically.

### preconditions-as-contracts

The precondition doctrine is the storage's cheapest contract, widened by the caller that needs more
([the-cheapest-contract](#the-cheapest-contract), [total-versus-precondition](#total-versus-precondition)), and its
mechanism is about ninety `assert`s, nearly half of them in `detail/bit_block_container.hpp` and most of the rest
in the sequence and set adaptors ([#193](https://github.com/rhalbersma/xstd-bits/issues/193)). An `assert` states
that something held, not that a caller owed it, which is the one distinction the doctrine turns on, and `NDEBUG`
is one switch over a translation unit where the doctrine assigns costs per layer. A P2900 precondition says it on
the declaration, where a caller reads it, and its evaluation semantics are chosen per build.

It waits on P2900's availability across the ladders, measured with dates, and may well conclude "not yet". Three
questions are to be decided before any syntax: whether a partial migration, two vocabularies for one doctrine,
is worse than none, and so whether a mechanism that degrades to `assert` is wanted; the C++23 floor, since the
library compiles as C++23 and contracts in the interface move it; and what the checked build is for, legibility
alone if contracts are only ever ignored in release and enforced in tests. The pilot is the smallest file,
`detail/valarray_adaptor.hpp` with four, reviewed for whether it reads better, and the coverage gate needs an
exclusion for a contract annotation as it has one for an assert at the start of a line
([an-assert-begins-its-line](#an-assert-begins-its-line)).

### the-bit-infrastructure-thesis

`std::bitset` was proposed in N0075, November 1991, three years before the STL entered the draft, so it was
designed before containers, iterators and algorithms had a vocabulary
([#192](https://github.com/rhalbersma/xstd-bits/issues/192)). It has no iterators and is no range, no
`value_type`, no allocator, no block parameter and no resize; its set reading exists only as the vendor
extensions `_Find_first` and `_Find_next`; and `test` throws where `operator[]` is undefined. The committee
patches it a paper at a time, P2417R2's `constexpr`, P2697R1's `basic_string_view` constructor, LWG 4294's `charT`
constraints, which is why the yardstick here is the working draft.

The thesis is **bit infrastructure, containers and algorithms, in the vocabulary of the current draft, without a
performance compromise**. Each clause is a kind of work: conformance, each container held to its counterpart's
clauses paragraph by paragraph ([the-audit](#the-audit)); algorithms over the containers, under their standard
names with a `bit_` prefix ([algorithms-not-members](#algorithms-not-members)); and performance a gate can resolve
([a-deterministic-perf-gate](#a-deterministic-perf-gate)), with [preconditions-as-contracts](#preconditions-as-contracts)
and [a-module-interface](#a-module-interface) as what "modern" asks besides. `std::bitset` itself is not
reproduced: its three vocabularies are the set, sequence and string readings, `std::bitset`'s bitwise operators
on a row of bools are the valarray reading's, and a bitset crosses into any of them through `xstd::bit_convert`
([interop-not-a-bitset](#interop-not-a-bitset)). Done means every owner and view held to its counterpart's full
clause, algorithms that agree with `std::ranges` on every input, and a number a reader can cite from a method that
tells a regression from noise.

### bit-regex

Matching beyond fixed substrings, `1(0|11)*0` for a codeword shape or "a run of at least five ones", has no
standard counterpart worth tracking ([#328](https://github.com/rhalbersma/xstd-bits/issues/328)). Over two
symbols a deterministic automaton can consume 8 or 16 text bits per step through a 256- or 65,536-entry table per
state, built at compile time after CTRE's model, the pattern a template argument. No maintained library offers
regular expressions over bits, and the demand that exists takes other forms: field-oriented matching (Erlang's
binary patterns, Kaitai Struct, BPF filters), SystemVerilog sequences, table-driven prefix-code decoders written
by hand per format, and biological motifs over packed nucleotides. So the likely value is narrower than a regex
engine, a `constexpr` automaton builder for prefix-code decoding or Erlang-style field matching, and which, if
either, is for a concrete user to decide. It needs the string reading's compile-time patterns and block-mask
search ([the-string-reading](#the-string-reading)), and its multi-bit alphabets for motifs over packed
nucleotides.

### the-downstream-contract

This library, with xstd-ints and xstd-misc, is the foundation [tabula](https://github.com/rhalbersma/tabula) and
[dctl](https://github.com/rhalbersma/dctl) are to be built on
([#119](https://github.com/rhalbersma/xstd-bits/issues/119)). What their code shows, read rather than specified:

- **The static row at two words.** tabula's `valid_size`, the bits a board needs after minimizing its padded
  embedding over the dihedral group and trimming it, is the `N` of a fixed set, and fifteen of the nineteen named
  boards land on exactly two 64-bit words. A position is `P` planes of `N` bits, between `⌈log2 K⌉` and `K` planes
  for `K` square values, so each two-word operation runs `P` times per node of a search. `N` and `P` are
  compile-time constants, which is why the static column carries the workload, and `constexpr` is a requirement
  rather than a convenience: board masks are tables built at compile time.
- **A migration target.** dctl's `core/board/mask.hpp` holds a board as
  `xstd::bit_set_aligned<int, Board::bits()>`, the ancestor of `bit_align<bit_fixed_set<N>>`, and whatever that
  offered (`max_size()`, `add()`, the compile-time tables) the fixed set has to offer at least.
- **The sequence row at endgame-database scale.** dctl's `egdb/build.hpp` stores results as `std::vector<int>`,
  where a win-draw-loss database needs two bits a position; a slice is 10^9 bits and more, so the costs are a
  cache miss per random read, a sequential scan and a count over gigabytes, which
  [the-sequence-ladder](#the-sequence-ladder)'s access bench asks, and the allocator becomes the way to huge
  pages or a file-backed slice. A sequence of two-bit elements is the representation it points to.

Bit planes and a per-square array are transposes of one another: the move generator reads a plane, and a board
renderer, a FEN parser or a database index reads a square across the planes, which is a gather, the read side
of the valarray reading's `mask_array` ([the-valarray-reading](#the-valarray-reading)). All of this is
reconstructed from the two repositories; an architecture document covering the layer stack, the design axes and
the invariants governs, and this entry is reconciled against it when it is published. The `valid_size` count and the two file references are observations
of code and stand either way.

### prior-art-by-column

The directions above draw on one inventory of third-party containers, classified by behaviour rather than by
name: **fixed**, an exact size at compile time; **bounded**, a size up to a compile-time capacity with no heap;
**small**, an inline buffer of `N` and then the heap; **run-time-fixed**, a size chosen once at construction; and
**dynamic**, a heap that grows ([#324](https://github.com/rhalbersma/xstd-bits/issues/324)). The block choice is
compared in [block-and-width-transformations](#block-and-width-transformations) and the enum and flag sets in
[flag-types](#flag-types). Every signature was read from the header at the linked commit; these are references for
design comparison, not test oracles.

Bit containers with a selectable or surprising column: `llvm::BitVector` is small despite its name, its words in a
`SmallVector` of six inline words on a 64-bit target, and `llvm::SmallBitVector` keeps at most 57 bits inline;
`llvm::PackedVector<T, BitNum, BitVectorTy>` packs `BitNum` bits per element over a selectable bit vector;
`eastl::bitvector<Allocator, Element, Container>` takes its container and `eastl::fixed_bitvector` is small;
`boost::dynamic_bitset<Block, AllocatorOrContainer>` takes a container since 1.90; `roaring::Roaring` and
`bm::bvector` are compressed dynamic sets with `rank` and `select`; and `sdsl::bit_vector` is `int_vector<1>`.

| fixed | signature | notes | source |
|---|---|---|---|
| `base::fixed_flat_set` (Chromium) | `template <class Key, size_t N, class Compare = std::less<>> using fixed_flat_set = base::flat_set<Key, Compare, std::array<const Key, N>>` | immutable and sorted, exactly `N`, built by `consteval MakeFixedFlatSet` | [fixed_flat_set.h](https://github.com/chromium/chromium/blob/ebdf288ea139cbf98e749f5a552083a7e74fde41/base/containers/fixed_flat_set.h) |
| `frozen::set` | `template <class Key, std::size_t N, class Compare = std::less<Key>> class set` | immutable `constexpr` sorted set | [set.h](https://github.com/serge-sans-paille/frozen/blob/1a6065fc39263b24e024317044d5833d690c474a/include/frozen/set.h) |
| `ctll::fixed_string` (CTRE) | `template <size_t N> struct fixed_string { char32_t content[N]; size_t real_size; }` | a template-argument string; `real_size`, at most `N`, is set at construction | [fixed_string.hpp](https://github.com/hanickadot/compile-time-regular-expressions/blob/62b060684d571df4dfab0fafeeddcf524db7c99e/include/ctll/fixed_string.hpp) |
| `Poco::Array` | `template <class T, std::size_t N> class Array` | a `std::array` look-alike | [Array.h](https://github.com/pocoproject/poco/blob/9876c10177a29b987a7d8e9c3e15bce25a6e07ec/Foundation/include/Poco/Array.h) |

| bounded | signature | past the capacity | source |
|---|---|---|---|
| `boost::container::static_vector` | `template <typename T, std::size_t Capacity, class Options = void> class static_vector` | `std::bad_alloc` by default; with `throw_on_overflow<false>`, an assert and otherwise undefined | [static_vector.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/static_vector.hpp), [options.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/options.hpp) |
| `boost::static_strings::basic_static_string` | `template <std::size_t N, typename CharT, typename Traits = std::char_traits<CharT>> class basic_static_string` | `std::length_error`, and `std::out_of_range` for a bad position | [static_string.hpp](https://github.com/boostorg/static_string/blob/2ff9d3535e853f4913e85e7fea28f84b04ea5d81/include/boost/static_string/static_string.hpp) |
| `folly::BasicFixedString` | `template <class Char, std::size_t N> class BasicFixedString` | `std::out_of_range`, a compile error in constant evaluation | [FixedString.h](https://github.com/facebook/folly/blob/282636437de660ac3603eb61c10b83247199d421/folly/FixedString.h) |
| `folly::small_vector` with `policy_in_situ_only<true>` | `template <class Value, std::size_t RequestedMaxInline = 1, class Policy = void> class small_vector` | `std::length_error` | [small_vector.h](https://github.com/facebook/folly/blob/282636437de660ac3603eb61c10b83247199d421/folly/container/small_vector.h) |
| `eastl::fixed_vector`, `fixed_string`, `fixed_set` with `bEnableOverflow = false` | as in the small table | `EASTL_ASSERT` in debug builds, otherwise undefined or an allocation exception | as below |
| `etl::vector` | `template <typename T, const size_t MAX_SIZE_> class vector` | `etl::vector_full` with ETL exceptions on, otherwise the error handler | [vector.h](https://github.com/ETLCPP/etl/blob/4707b6b15bff21d3b4268d27ebb7a7bd2ab941c7/include/etl/vector.h), [error_handler.h](https://github.com/ETLCPP/etl/blob/4707b6b15bff21d3b4268d27ebb7a7bd2ab941c7/include/etl/error_handler.h) |
| `etl::string` | `template <size_t MAX_SIZE_> class string` | **truncates**; optionally flags it or raises `etl::string_truncation` | [string.h](https://github.com/ETLCPP/etl/blob/4707b6b15bff21d3b4268d27ebb7a7bd2ab941c7/include/etl/string.h), [basic_string.h](https://github.com/ETLCPP/etl/blob/4707b6b15bff21d3b4268d27ebb7a7bd2ab941c7/include/etl/basic_string.h) |
| `etl::flat_set` | `template <typename T, const size_t MAX_SIZE_, typename TCompare = etl::less<T>> class flat_set` | ETL's error policy | [flat_set.h](https://github.com/ETLCPP/etl/blob/4707b6b15bff21d3b4268d27ebb7a7bd2ab941c7/include/etl/flat_set.h) |
| `itlib::static_vector` | `template <typename T, size_t Capacity> class static_vector` | `std::out_of_range` by default, configurable | [static_vector.hpp](https://github.com/iboB/itlib/blob/b7eedccee7590f39f08bf9e414960a7fe854f1ad/include/itlib/static_vector.hpp) |
| `FixedVector` (Godot) | `template <class T, uint32_t CAPACITY> class FixedVector` | logs an error and leaves the vector unchanged | [fixed_vector.h](https://github.com/godotengine/godot/blob/e7cfa294a0b81bed7986be04a848cc1832a3f083/core/templates/fixed_vector.h) |
| `seastar::circular_buffer_fixed_capacity` | `template <typename T, size_t Capacity> class circular_buffer_fixed_capacity` | a ring buffer; unverified | [circular_buffer_fixed_capacity.hh](https://github.com/scylladb/seastar/blob/e417c0c0ebeb1f0578b1dd5b3dd4f65decff0a4b/include/seastar/core/circular_buffer_fixed_capacity.hh) |

| small | signature | notes | source |
|---|---|---|---|
| `llvm::SmallVector` | `template <typename T, unsigned N = CalculateSmallVectorDefaultInlinedElements<T>::value> class SmallVector` | the default `N` aims for `sizeof <= 64` | [SmallVector.h](https://github.com/llvm/llvm-project/blob/1e60b852a9d7f73ffed3540e9ea84aefde1b80ab/llvm/include/llvm/ADT/SmallVector.h) |
| `llvm::SmallString` | `template <unsigned InternalLen> class SmallString : public SmallVector<char, InternalLen>` | | [SmallString.h](https://github.com/llvm/llvm-project/blob/1e60b852a9d7f73ffed3540e9ea84aefde1b80ab/llvm/include/llvm/ADT/SmallString.h) |
| `llvm::SmallSet` | `template <typename T, unsigned N, typename C = std::less<T>> class SmallSet` | a linear `SmallVector`, then **switches structure** to `std::set` | [SmallSet.h](https://github.com/llvm/llvm-project/blob/1e60b852a9d7f73ffed3540e9ea84aefde1b80ab/llvm/include/llvm/ADT/SmallSet.h) |
| `llvm::SmallPtrSet`, `llvm::SmallDenseSet` | `template <class PtrType, unsigned SmallSize>`, `template <typename ValueT, unsigned InlineBuckets = 4, typename ValueInfoT>` | a linear array or inline buckets, then a heap hash table | [SmallPtrSet.h](https://github.com/llvm/llvm-project/blob/1e60b852a9d7f73ffed3540e9ea84aefde1b80ab/llvm/include/llvm/ADT/SmallPtrSet.h), [DenseSet.h](https://github.com/llvm/llvm-project/blob/1e60b852a9d7f73ffed3540e9ea84aefde1b80ab/llvm/include/llvm/ADT/DenseSet.h) |
| `absl::InlinedVector` | `template <typename T, size_t N, typename A = std::allocator<T>> class InlinedVector` | `N > 0` enforced | [inlined_vector.h](https://github.com/abseil/abseil-cpp/blob/33a35cf45087dffab22c0c9ae817021a45e93a47/absl/container/inlined_vector.h) |
| `folly::small_vector` | `template <class Value, std::size_t RequestedMaxInline = 1, class Policy = void> class small_vector` | `std::length_error` past `max_size()`; bounded under a policy, above | [small_vector.h](https://github.com/facebook/folly/blob/282636437de660ac3603eb61c10b83247199d421/folly/container/small_vector.h) |
| `folly::basic_fbstring` | `template <typename E, class T = std::char_traits<E>, class A = std::allocator<E>, class Storage = fbstring_core<E>> class basic_fbstring` | 23 `char` in place on a 64-bit target, then a medium and a reference-counted large form | [FBString.h](https://github.com/facebook/folly/blob/282636437de660ac3603eb61c10b83247199d421/folly/FBString.h) |
| `boost::container::small_vector` | `template <class T, std::size_t N, class Allocator = void, class Options = void> class small_vector` | | [small_vector.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/small_vector.hpp) |
| `boost::container::basic_string` | `template <class CharT, class Traits = std::char_traits<CharT>, class Allocator = void, class Options = void> class basic_string` | inline capacity set by `string_options<inline_chars<K>>`, clamped to 127; the first release shipping it unverified | [string.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/string.hpp) |
| `eastl::fixed_vector`, `fixed_string`, `fixed_set` | `template <typename T, size_t nodeCount, bool bEnableOverflow = true, typename OverflowAllocator> class fixed_vector`, and alike | `bEnableOverflow` switches small and bounded with one flag; `fixed_set` is a red-black tree over a node pool | [fixed_vector.h](https://github.com/electronicarts/EASTL/blob/e323f67e5d93fdadcd44f7998069d804697b1945/include/EASTL/fixed_vector.h), [fixed_string.h](https://github.com/electronicarts/EASTL/blob/e323f67e5d93fdadcd44f7998069d804697b1945/include/EASTL/fixed_string.h), [fixed_set.h](https://github.com/electronicarts/EASTL/blob/e323f67e5d93fdadcd44f7998069d804697b1945/include/EASTL/fixed_set.h) |
| `QVarLengthArray` | `template <class T, qsizetype Prealloc> class QVarLengthArray` | `Prealloc` defaults to 256 | [qvarlengtharray.h](https://github.com/qt/qtbase/blob/8901c04fd61ad269c3730beb5ea502b0587bbce2/src/corelib/tools/qvarlengtharray.h) |
| `mozilla::Vector` | `template <typename T, size_t MinInlineCapacity = 0, class AllocPolicy = MallocAllocPolicy> class Vector` | out of memory is `append` returning `false`, not an exception | [Vector.h](https://github.com/mozilla-firefox/firefox/blob/d0f18859d99ecb16f4c5d641b35a5f3bdb7b4f36/mfbt/Vector.h) |
| `gch::small_vector` | `template <typename T, unsigned InlineCapacity, typename Allocator = std::allocator<T>> class small_vector` | `std::length_error` past the maximum size | [small_vector.hpp](https://github.com/gharveymn/small_vector/blob/efcab60e5360b699f45d2a44dcb9a79bba40c356/source/include/gch/small_vector.hpp) |
| `itlib::small_vector` | `template <typename T, size_t StaticCapacity = 16, size_t RevertToStaticBelow = 0, class Alloc = std::allocator<T>> class small_vector` | returns to the inline buffer below `RevertToStaticBelow` | [small_vector.hpp](https://github.com/iboB/itlib/blob/b7eedccee7590f39f08bf9e414960a7fe854f1ad/include/itlib/small_vector.hpp) |
| `fmt::basic_memory_buffer` | `template <typename T, size_t SIZE = inline_buffer_size, typename Allocator> class basic_memory_buffer` | `inline_buffer_size = 500` | [format.h](https://github.com/fmtlib/fmt/blob/9197f51593dab5f637454a8889fb9638e737711a/include/fmt/format.h) |
| `hpx::detail::small_vector` | `template <typename T, std::size_t MinInlineCapacity, typename Allocator = std::allocator<T>> class small_vector` | `MinInlineCapacity <= 127`, the allocator unused | [small_vector.hpp](https://github.com/STEllAR-GROUP/hpx/blob/d4cd4bd5be7a0b123efc9e25f67bdb369db9672f/libs/core/datastructures/include/hpx/datastructures/detail/small_vector.hpp) |
| `TInlineAllocator`, `TFixedAllocator` (Unreal) | `TArray<T, TInlineAllocator<N>>` | from the public documentation only; the declarations unverified, `TFixedAllocator` believed bounded | [TInlineAllocator](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Core/Containers/TInlineAllocator/ForElementType/InlineData/) |

| run-time-fixed, and set adaptors over a container | signature | column | source |
|---|---|---|---|
| `absl::FixedArray` | `template <typename T, size_t N = kFixedArrayUseDefault, typename A = std::allocator<T>> class FixedArray` | run-time-fixed: inline up to `N`, or `256 / sizeof(T)` by default, else the heap | [fixed_array.h](https://github.com/abseil/abseil-cpp/blob/33a35cf45087dffab22c0c9ae817021a45e93a47/absl/container/fixed_array.h) |
| `folly::sorted_vector_set`, `small_sorted_vector_set` | `template <class T, class Compare, class Allocator, class GrowthPolicy, class Container = std::vector<T, Allocator>> class sorted_vector_set` | follows `Container`; the alias is small, or bounded under a policy | [sorted_vector_types.h](https://github.com/facebook/folly/blob/282636437de660ac3603eb61c10b83247199d421/folly/container/sorted_vector_types.h) |
| `boost::container::flat_set`, `small_flat_set` | `template <class Key, class Compare, class AllocatorOrContainer> class flat_set` | follows the container; no `static_flat_set` alias, so the bounded one is spelled out | [flat_set.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/flat_set.hpp), [container_fwd.hpp](https://github.com/boostorg/container/blob/2d2f12280290f4d60096d1bfb521bb689adbe91d/include/boost/container/container_fwd.hpp) |
| `base::flat_set` (Chromium) | `template <class Key, class Compare = std::less<>, class Container = std::vector<Key>> using flat_set` | dynamic by default; `fixed_flat_set` is its `std::array` form | [flat_set.h](https://github.com/chromium/chromium/blob/ebdf288ea139cbf98e749f5a552083a7e74fde41/base/containers/flat_set.h) |
| `itlib::flat_set` | `template <typename Key, typename Compare, typename Container = std::vector<Key>> class flat_set` | follows `Container` | [flat_set.hpp](https://github.com/iboB/itlib/blob/b7eedccee7590f39f08bf9e414960a7fe854f1ad/include/itlib/flat_set.hpp) |

Not containers: `frozen::basic_string` is a `constexpr` view, comparable to `std::string_view`; POCO's Foundation
has no small, bounded or bit container; Bitsery serializes and defines none; Williams' `enable_bitmask_operators`
declares operator templates in the global namespace, which ADL does not search for an enumeration in any other
namespace, so an operator in the calling namespace hides them; Godot's `BitField<T>` is a typed flag word.

**Names that collide.** "Fixed" means bounded or small as often as it means an exact size: `eastl::fixed_vector`,
`fixed_string` and `fixed_set` are small by default and bounded without overflow, and `eastl::fixed_bitvector` is
small; `absl::FixedArray` is run-time-fixed; `folly::FixedString` and Godot's `FixedVector` are bounded;
`ctll::fixed_string<N>` stores at most `N`; only Chromium's `fixed_flat_set` is fixed in this library's sense.
"Static" means bounded in `boost::container::static_vector`, `boost::static_string` and `itlib::static_vector`.
Boost.Container uses "inplace" only for the `inplace_alignment` option. `llvm::BitVector` has no "small" in its
name and is small all the same, through its `SmallVector`; `llvm::SmallBitVector` is small at a fixed 57 bits;
`llvm::SmallSet` is small by structure rather than storage; and `folly::small_vector` turns bounded under a policy.
Hence names here say what the user gets, `fixed` and `bounded`, rather than where the blocks live
([the-public-names](#the-public-names)).

**SDSL, the encoding axis.** SDSL ([xxsds/sdsl-lite](https://github.com/xxsds/sdsl-lite), the maintained fork of
Simon Gog's original) is a family of types over bit vectors that varies a different axis: this library varies the
interface and the storage over mutable containers held to the standard's clauses, and SDSL varies the encoding
and the indexes attached to it, over sequences mostly built once and then queried.

| layer | types |
|---|---|
| packed integers | `int_vector<t_width>`, `t_width == 0` choosing the width at run time; `bit_vector` is `int_vector<1>` ([int_vector.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/int_vector.hpp)) |
| compressed bit vectors | `rrr_vector` (H0), `sd_vector` (Elias-Fano, for sparse sets), `hyb_vector`, `bit_vector_il` ([rrr_vector.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/rrr_vector.hpp), [sd_vector.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/sd_vector.hpp)) |
| rank and select | `rank_support_v`, `rank_support_v5`, `select_support_mcl`, separate objects over a constant bit vector ([rank_support_v.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/rank_support_v.hpp), [select_support_mcl.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/select_support_mcl.hpp)) |
| variable-length integers | `enc_vector` (Elias γ and δ), `dac_vector`, `vlc_vector` ([enc_vector.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/enc_vector.hpp)) |
| built on top | wavelet trees, compressed suffix arrays and trees, range-minimum queries ([wt_int.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/wt_int.hpp), [csa_wt.hpp](https://github.com/xxsds/sdsl-lite/blob/e6c417391f55476c6946c6fcf76c7315354e1af9/include/sdsl/csa_wt.hpp)) |

`int_vector<1>` has this library's sequence layout and `int_vector<K>` that of a sequence of `K`-bit elements,
least significant bit first, and `seqan3::bitpacked_sequence` stores one, so exchanging blocks with either is a
matter of reaching their storage, not of converting. A `rank_support` refers to a constant bit vector and is
invalidated by any change, which is the model for a rank directory over a set's blocks: a view valid while the
set is unmodified, as an iterator is, rather than a member. Compressed
encodings, wavelet trees, compressed suffix structures and FM-indexes are SDSL's territory, and users who need
them are pointed there.
