# Design notes

Why the code is shaped the way it is. The headers carry one line each; the reasoning lives here, and a
one-line comment ending in `[design.md#anchor]` points at the section that explains it.

Decisions still in flight live on the [open issues](https://github.com/rhalbersma/xstd-bits/issues), and
[#80](https://github.com/rhalbersma/xstd-bits/issues/80) is the closed design plan the current shape came
out of. This file holds what has landed.

## Storage and containers

### contiguous-block-range

`contiguous_block_range` asks whether a range **is** blocks: a regular, sized, contiguous, subscriptable
range of unsigned integers. Regular is what lets `contiguous_bit_container` default its `==` over the width and
the blocks, in that member order, so two run-time widths part on the width before a block is read. `std::array`
and `std::vector` both qualify, and so does `std::inplace_vector` — a runtime width over static capacity, for
free.

The element clause is `unsigned_integer` and **not** the wider `bitwise_operators`, which would be the concept
if the operators were all a block is asked for. They are not. Beyond them the body wants the `<bit>` intrinsics
— `popcount`, `countr_zero` and `countl_zero`, each constrained on `xstd::unsigned_integer` in
`detail/intrin.hpp` and reached at some thirty sites — a `numeric_limits<block_type>::digits` for
`bits_per_block`, and block arithmetic: `shl(unit, count) - unit`, and the `block & (block - 1)` step the set
reading's block walk takes, where `bitwise_operators` omits `-` deliberately, subtraction and set difference
being indistinguishable to a concept.

The two agree on every block the library ships, and both refuse `bool`, the character types and every signed
type, so `std::vector<int>` stays out either way. They part on the class types that are fields of bits without
being numbers, `std::bitset` among them: `bitwise_operators` admits those and this concept does not, which is
the point. `std::array<std::bitset<64>, 4>` is asserted refused, a ready-made negative case
([concepts-are-tested-on-ready-made-types](#concepts-are-tested-on-ready-made-types)) for exactly the gap
between the two spellings. Widening the clause would move that refusal from an unsatisfied constraint to a hard
error inside the template — the failure mode the subscript clause below exists to prevent, and there is no
reason to accept it on the element clause when the narrower concept states the truth.

The asymmetry it records is the layering, not an accident. `unsigned_integer` goes **in** and the container
gives `contiguous_bit_sequence` — and, once the non-assigning operators land, `bitwise_operators` — **out**: it
asks more of a block than it offers its own user, consuming numbers and yielding a field of bits, shedding the
arithmetic on the way up. That is also why nesting cannot work: a `contiguous_bit_container` will have every
operator and still no `popcount`, no `digits` and no `- 1`.

The concept has a header of its own at `detail/contiguous_block_range.hpp`: the concept says what a `Blocks` **is**,
the container is the vehicle built over it, and a reader asking the first question need not open the 1100 lines
answering the second. Its includes are `<concepts>`, `<ranges>`, the one xstd-ints concept and the alias of
[the-const-reference](#the-const-reference) — a leaf. `num_blocks_v` stays behind, being a block-count computation rather than a statement about what a
`Blocks` is, and both alias headers that use it already take the container whole.

Subscript is spelled out rather than left to `std::ranges::contiguous_range`, which does not imply it.
`contiguous_range` gives `data()` and a `contiguous_iterator`, and a `contiguous_iterator` is a
`random_access_iterator`, so `i[n]` **is** required — of the *iterator*. The range itself is under no such
obligation: a plain buffer wrapper can satisfy the other four requirements and have an iterator that subscripts
happily while `c[n]` does not compile. No *ready-made* type isolates that gap — every std candidate that fails
this concept fails for some other clause first (`span` and `subrange` are not `regular`, `string_view`'s and
`vector<int>`'s elements are not unsigned integers, `vector<bool>` is not contiguous) — and the gap is not worth
a class written only to be
asked about ([concepts-are-tested-on-ready-made-types](#concepts-are-tested-on-ready-made-types)), so it is
stated here from [range.refinements] rather than pinned by a test. `contiguous_bit_container` reaches for the
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

Naming it `container` follows from the same fact. A contiguous container generalizes the C array, and `a[i]` is
the C array's defining operation; a concept claiming a range *is* blocks while unable to index one would be
describing something else. The word is deliberately close to the standard's *contiguous container*
([container.reqmts]/68) without claiming it: that term drags in the whole *Container* table — `empty()`,
`max_size()`, `cbegin`/`cend`, member `swap`, seven nested typedefs — and `contiguous_bit_container` needs
almost none of it. At a static width it needs none; at a run-time width it needs `max_size`, `resize`,
`push_back` and `clear`, which are *sequence* container operations, not `Container` ones. Neither path draws the
line where [container.reqmts]/68 draws it, so the concept states its own five requirements and borrows nothing.

The name says *container*, not *storage*: what an adaptor sits on is a different question, asked by
`specialization_of_contiguous_bit_container` and answered by name rather than by members
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
first, and this is not a wait-for-the-next-rung situation: an earlier attempt to name the standard alias
directly was reverted off Xcode 16.4.

**A conditional is only as good as its other arm, and that is the whole argument for the transcription.** The
first attempt made the arms `std::ranges::range_const_reference_t` and `range_reference_t<C const>`, and those
two are not the same question: P2278 asks what `C`'s own iterator yields once const-ified, the second what a
`C const` iterates. Measured, they part on exactly one thing:

| storage | `range_const_reference_t<C>` | `range_reference_t<C const>` |
| --- | --- | --- |
| `vector<Block>`, `array<Block, N>`, `inplace_vector<Block, N>` | `Block const&` | `Block const&` |
| a regular, span-like handle | `Block const&` | `Block&` |

Within `contiguous_range` that makes P2278 strictly the tighter arm, and the one thing it catches is a
**shallow-const** blocks type: one whose `c[n]` hands back a writable `Block&` through a `C const&`, which would
let a `contiguous_bit_container const` be written through. That conditional would therefore have admitted such a
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

`contiguous_bit_container` and its three aliases live under `detail/`, one header each:
`detail/contiguous_bit_container.hpp` holds the concept, `num_blocks_v`, the class and the detector that names
it, and
`detail/contiguous_bit_array.hpp`, `detail/contiguous_bit_vector.hpp` and
`detail/contiguous_bit_inplace_vector.hpp` hold one vehicle apiece. The names are in `xstd::detail::bits` with
the rest of `detail/`, so a container spells `detail::bits::contiguous_bit_array<Block, N>` and nothing outside
the library can name the vehicle at all. It is the device that turns three readings over three storages into three plus three, and a
factoring device is machinery rather than vocabulary: a user reaches every width through `bit_static_set<N>` or
`basic_bit_array<Block, N>` and never spells the pair themselves. The split is what lets each of the nine
containers include only the vehicle it uses -- `bit_array` names `contiguous_bit_array` and no longer sees
`std::vector`, and the `#ifdef __cpp_lib_inplace_vector` guard sits in the one header that concerns it rather
than in the common one.

**The name says what it does to its argument.** It takes a `contiguous_block_range` — a range that *is*
blocks ([contiguous-block-range](#contiguous-block-range)) — and adds the bit interface. In goes
storage that answers about blocks, out comes something that answers about bits. Concept and class differ by one
word, and it is the word that changes. The three aliases follow the same rule: `contiguous_bit_array`,
`contiguous_bit_vector` and `contiguous_bit_inplace_vector` each name the bit container over one block
container. Their former names pointed at the argument instead of the result — `block_array` read as *an array of
blocks*, which is what it is instantiated over, `std::array<Block, n>`, and not what the alias is.

That is also what earns it the name the adaptors are constrained on. It is the one storage they admit
([one-storage](#one-storage)), the only one there is: the `ext/` specializations for `std::bitset` and
`boost::dynamic_bitset` are gone, and those two are comparison targets rather than adapted storages
([owning-is-ours](#owning-is-ours)). It lives under `detail/` because nobody outside spells it
([the-interface-line](#the-interface-line)). What the three have in common as *members* is
[the-common-vocabulary](#the-common-vocabulary). The former name, `block_sequence`, hid that: a *sequence of
blocks* reads as storage that happens to have been adapted, where a *bit container* reads as the thing the
readings are built on.

`contiguous_bit_container<Blocks, N>` is the single storage vehicle. `N` is the width when that is a constant
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
`unsigned_integer` from `bitwise_operators` ([contiguous-block-range](#contiguous-block-range)). Only the
range-subscript clause has no ready-made counterexample, and it is argued in prose there instead.

`counting_blocks` in `test/src/bits/detail/contiguous_bit_container.cpp` is not an exception to this. It is a
swap-and-move fixture with instrumented operations ([swap-goes-through-adl](#swap-goes-through-adl)), which no
std container can be, and it carries no concept assertion of its own: it is instantiated as a
`contiguous_bit_container`'s `Blocks`, and that instantiation is the only check it needs to satisfy.

### the-common-vocabulary

`contiguous_bit_sequence` is what the three bit containers answer **in their own names**, with no trait in
between: the intersection of `std::bitset`'s vocabulary, `boost::dynamic_bitset`'s and
`contiguous_bit_container`'s. Measured against all three rather than guessed, and larger than it first looks —
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
| `contiguous_bit_container` | `operator[]` ([test-not-subscript](#test-not-subscript)), unary `~`, `set(n, value)` |
| `std::bitset` | `-=`, `is_subset_of`, `find_first`, member `swap` |
| `boost::dynamic_bitset` | `to_string` |

The asymmetry is the point, and it runs in two directions at once. `contiguous_bit_container` provides the
**union** of what the three readings ask of it — a set reading needs `find_first` and `find_next`, a sequence
reading needs positional writes, a bitset reading needs [template.bitset] and boost's set vocabulary both
([a-strict-extension](#a-strict-extension)) — while the concept asks only what the three *containers* have in
common. Union below, intersection across.

**Nothing is constrained on it, and that is deliberate.** The adaptors admit their storage *nominally*, by
`specialization_of_contiguous_bit_container` ([one-storage](#one-storage)), and the structural question is a
different question: `std::bitset` and `boost::dynamic_bitset` both model `contiguous_bit_sequence` and neither
is a storage this library wraps. Constraining an adaptor on the concept would turn *this is not one of ours*
into *your type lacks `count()`*, which is the wrong diagnosis about the wrong type.

The concept is therefore a **description, pinned by the tests, not a gate**. It says what the three have in
common and fails loudly if one of them drifts.

### the-primitive-basis

Two layers now, and the lower one is the whole design. `contiguous_bit_container` provides the **primitives** —
the operations a reading needs to be efficient rather than merely correct — and each adaptor **assembles** them
into the standard-like API its reading presents. There was a third layer between them, and
[one-storage](#one-storage) is why there no longer is.

The primitives are the point, and the reason is measurable: they are exactly what the counterparts do not have.
What each provides natively, on libstdc++:

| primitive | `contiguous_bit_container` | `std::bitset<N>` | `boost::dynamic_bitset` |
|---|---|---|---|
| block read | yes | only `N <= 64`, through `to_ullong` | **no** |
| `find_first` | yes | `_Find_first`, where the library has it | yes |
| `find_next` | yes | `_Find_next`, where the library has it | yes |
| `find_prev` | yes | **no** | **no** |
| `count`, `all`, `any`, `none` | yes | yes | yes |

`std::bitset` and `boost::dynamic_bitset` are an **incomplete basis**: enough to implement the full views API
correctly, never enough to implement it efficiently. Neither walks backwards, and boost hands over no blocks at
all, so a view over one would have had to walk in reverse a position at a time — quadratic over a full
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

Only the bitset reading shares that purpose — the bitwise operators over a fixed width
([a-strict-extension](#a-strict-extension)). The set and sequence readings ask a wide integer questions it was
never meant to answer, and answering them efficiently is what the storage below is for.

So the conclusion the table argues for is not a shim over three spellings of one thing. It is **one storage
that is a complete basis**, and the `ext/` specializations that once worked around the shortfall are gone: they
hand-wrote `find_first` and `find_next` over whatever the storage offered, added `num_blocks` and `block`
behind a `requires (N <= ullong_digits)` guard, and wrote no `find_prev` at all, because neither counterpart
had one to forward to. The counterparts are comparison targets now ([owning-is-ours](#owning-is-ours)), and
with no second basis to reconcile, the layer that reconciled them went too ([one-storage](#one-storage)).

### the-bytes-they-agree-on

A comparison target can become a **value** the set reading converts to and from, and at a static width it is an
exact one. A field of `N` bits has the same `N`, and under this reading that width is a capacity, so position
`n` here is bit `n` there with nothing left to choose: nothing truncates, nothing grows, nothing throws, and the
round trip is the identity both ways. `bit_static_set<N>` therefore carries an explicit constructor and an
explicit conversion operator back. Explicit in both directions, and not because either could fail — a set of
positions and a field of bits are two readings of the same bits, and this library makes a reader pick one rather
than letting a conversion pick for them.

Neither is spelled with a type. `std::bitset` appears nowhere in the set adaptor: the two are templates
constrained on `bit_castable<B, N>`, which admits anything whose `N` bits this library can prove it reads
correctly. That is two families, and only one of them has anything to prove.

| family | admitted because | what it costs |
|---|---|---|
| unsigned integer | bit `n` of the value is 2^n, *by the language* | nothing: no probe, no assumption |
| field of bits | a layout proved below | five fixed probes |

`unsigned long long` is therefore not a special case in the header but an instance of the first family, which is
what `to_ullong` and the constructor taking one reduce to. `std::bitset<N>` is an instance of the second, and so
is this library's own bitset reading — `xstd::bitset<N>` crosses to `bit_static_set<N>` on the same rule, with
neither side named in the constraint. An implementation that ever laid its bits out otherwise is simply not
admitted: a call that fails to compile rather than one quietly wrong.

One spelling in the integer family repays reading twice at *this* reading. `bit_static_set<32>(5u)` is the set of
positions the **value** five has, `{0, 2}`, not the set `{5}` — the same bits `std::bitset<32>(5u)` would hold.
It is explicit, so it is asked for rather than arrived at, but it is the one place here a reader coming from the
set vocabulary can misread.

The currency is **bytes**, not words. Byte `j` holds the positions `[8j, 8j + 8)` least significant bit first,
which is what every contiguous bit container lays them out as whatever its block width, so two widths over the
same positions agree byte for byte. That is what makes this a copy rather than a walk over positions, and the
storage's own `assign_bytes` and `to_bytes` are the primitives.

Both are said **twice**, and the reason is measured rather than tasteful. Said in **shifts**, they name where a
position goes instead of assuming a byte order, so they are right on either endianness and they are a constant
expression — neither of which a `memcpy` is. They are also a byte at a time, and over the eight kilobytes of
2^16 positions that costs **9.70µs against 0.07µs**, a factor of about a hundred and forty-five. So the shifts
keep the two cases a copy cannot take, a constant expression and a big-endian target, and the copy takes the
case that is neither, which is every rung this ladder runs. `if consteval` picks the first,
`if constexpr (endian::native == endian::little)` the second; neither is a branch a coverage slot can miss,
because neither is a branch at run time.

That factor is what decides a question this design keeps inviting: whether a foreign bitset should be **read**
block-wise in place rather than converted. In place is not portably possible — `bit_cast` yields a copy, so
reading someone else's words needs a pointer into them, which is `_M_p`, `__seg_` or `_Myptr` by turns. It also
turns out not to be worth wanting. Iterating the set positions of a `std::bitset<2^16>` at 2.7% density costs
**41.1µs** asking every position, against **libstdc++'s own in-place `_Find_first`/`_Find_next`** and
**converting once and then iterating**, which come out level: five runs put their ratio at 1.06, 0.94, 0.95,
1.06 and 1.01, so on par to within about six percent. The absolute microseconds are not quotable — this box is
bimodal, and both of those two move between roughly 7.5µs and 18.5µs *together*, which is what makes the ratio
the only honest figure and a single run of either a trap. So there is no `bit_readable` to add: converting
costs what reading in place would, the conversion is the door, and the readings' own block-wise algorithms are
what waits behind it.


The second family's layout is **proved, not believed**. `bit_layout_holds<B, N>` checks that a default-built `B`
is all clear, then lights one position at a time and checks that `count()` is one and that the expected byte
holds exactly its bit. That last pair is what makes it a proof rather than a spot check: with one position set
and one byte holding it, no *other* byte can hold anything, so it refuses a reordered word, a reversed bit order,
a trailing word the implementation does not keep clean, and — `bit_cast` handing back the object representation
rather than the value — a big-endian target, where bit `n` of a word lands at the far end of it. The endianness
guard **is** this probe, with no second one to fall out of step with it. Four deliberately wrong layouts in the
tests take each refusal, so what it rejects is exercised rather than asserted.

**Five** positions, and the count is a budget rather than a taste. Each probe materialises the whole byte array
through `bit_cast`, so it costs O(`sizeof(B)`) however few bytes it then reads; clang's default
`-fconstexpr-steps` admits six at a width of 2^20 and refuses seven, where GCC's limit is higher. Five leaves
margin and puts the ceiling at 2^20 — a 128 KiB object — past which a caller raises the flag.

Two constraints before it are load-bearing, and atomic constraints being checked in order is what makes them
work. `B().size() == N` pins the source's **own** width: without it the size window admits a neighbour, since a
`std::bitset<9>` is eight bytes and clears every bound a width of eight sets, and eight of its nine positions
would convert while the ninth vanished. It also keeps the probe from asking a narrower source for a position it
does not have, where `std::bitset<8>::set(8)` throws and a throw is no constant expression — an unsatisfied
concept where that would be a hard error. And `bit_cast_is_constant<B>` rules out the shapes `bit_cast` refuses
to be `constexpr` for: a pointer member, a reference member, a union.

What is **not** asked is `has_unique_object_representations_v`, and that is measured rather than preferred. GCC
13 through 16 answer false for any class with an empty non-static data member, even one `[[no_unique_address]]`
makes free, where clang answers true at identical layout — `sizeof` 8 and `offsetof` 0 on both. Every container
this library defines has such a member, so that trait would make this concept false on every GCC rung and true
on every clang rung. An empty *base* keeps the trait on GCC where an empty member does not, which is the remedy
if it is ever wanted; the probe needs none of it, because interleaved padding breaks the byte alignment it
already checks.

A run-time width has neither conversion, and that is the policy and not an omission: a field of `N` bits names
one `N` at compile time and a growing set has no single one to mean.

### padding

`static_used_bits` is the mask of the last block that is not padding. `num_bits` is `align_up(N)`, so
`num_bits - N` lies in `[0, bits_per_block)` and the shift is always in range.

Width zero is the one case that form cannot express — there is nothing to align up, so it reports no
padding where in truth the sole block is all of it — and it gets a selection instead.

**Naming zero rather than computing it matters on MSVC**, which constant-folds both arms of a `?:` and
answers C4293, *shift count too big*, on the arm it discards. `used_bits()` is the same two cases at a
run-time width.

The width member takes the blocks' alignment where they out-align a `std::size_t`:

```cpp
using width_type = std::conditional_t<(alignof(std::size_t) >= alignof(Blocks)), std::size_t, block_type>;
```

Only a storage holding its blocks inline out-aligns a `size_t`, and it does so by the blocks' own alignment, so
`block_type` is both wide enough to hold any width and exactly the size of the gap it fills --
`contiguous_bit_container` is then its two members and nothing else, at the same size the padding cost.
`std::array` reaches none of this, a static width carrying no member at all, and neither does `std::vector`,
whose alignment is a pointer's whatever it holds; `contiguous_bit_inplace_vector<xstd::uint128, N>` is the one
cell that does. The `static_assert` beside the alias holds the two facts that make `block_type` the right
carrier, so a storage over-aligned for some other reason fails loudly rather than truncating a width. Every
reader goes through `size()`, which converts once, so the arithmetic stays a `size_t`'s.

### default-construction

A defaulted default constructor plus an NSDMI, rather than two constructors constrained on the extent:
`std::vector` default-constructs empty, and the at-least-one-block invariant has to hold from the start.

### growth

Growth is the run-time width's alone, and every member of it leaves the unused tail clear. `resize(n, value)`
resizes the blocks to what `n` needs, filled with `value`, moves the width, and masks the new last block;
growing with ones first sets the old last block's tail, clear by the invariant, since those are the first new
positions. `push_back` and `pop_back` are `resize` by one, `clear` is `resize(0)` -- the object a default
constructor makes -- and `append(block)` is boost's: the block's bits become the next `bits_per_block`
positions, split across two blocks where the width is not aligned, and the floor block takes the first one
at width zero. `reserve`, `capacity` and `shrink_to_fit` are in bits and exist where the blocks have them:
`std::vector` and `std::inplace_vector`, not `std::array`.

`clear()` here is the sequence reading's, width to zero, which is what `std::vector<bool>` and
`boost::dynamic_bitset` mean by it; the set reading's `clear()` is `fill(false)` and never reaches this
member, so the landmine #80 recorded -- probing `clear()` on boost and emptying the width -- cannot recur.

`contiguous_bit_inplace_vector<Block, N>` is the third storage: a run-time width under a compile-time capacity
of `N` bits, behind `__cpp_lib_inplace_vector` until every library in the matrix has it. It needs nothing of its
own, `std::inplace_vector` satisfying `contiguous_block_range` as it is; `resize`, `reserve` and `push_back`
past the capacity throw `std::bad_alloc`, as that library specifies.

The adaptors take growth by detection on the storage: growth is a container's business and no view's, so it exists on an owner and on nothing else. `sequence_adaptor` is
`std::vector<bool>` where its storage grows -- the count and count-value constructors, the range and
`initializer_list` constructors and assignments, `assign`, `resize`, `clear`, `push_back`, `pop_back`,
`emplace_back`, with `reserve`, `capacity` and `shrink_to_fit` where the blocks have them -- and
`std::array<bool, N>` where it does not, each member requiring the storage member it forwards to.
`bitset_adaptor` at a run-time width takes `boost::dynamic_bitset`'s growth the same way. `set_adaptor`
takes none by name: a set grows by `insert`, and the storage's `growing_insert` grows a run-time width to hold
the key ([asking-is-total](#asking-is-total)).

## Scans

### inclusive-is-the-primitive

Inclusive and exclusive name the only thing separating the two forward scans: whether `n` itself is a
candidate. That is a property of the scan rather than of a reading, which is why this is not called
`lower_bound` — that is the set reading's word, and this layer serves the sequence reading equally.
`bit_static_set::lower_bound` is where the set name belongs, and it maps here one for one.

The inclusive form is the primitive and the exclusive one is derived, because the reverse does not close:

```
exclusive_find_next(n) == inclusive_find_next(n + 1)     for every n
find_first()           == inclusive_find_next(0)
```

Exclusive-first would need `find_first() == exclusive_find_next(-1)`, and `size_t` has no such value —
which is exactly why the container used to branch on `x == 0`. **Both derivations are `+ 1` and never
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
`find_if`. The iterator forms had to recover the block's position with `distance()`; the index *is* that
position, so it never needed recovering. The reverse form composed two adaptors only to have `distance()`
undo them.

### the-blit

`contiguous_bit_container::word_at(pos)` is the block-wide word at any position:
the bits `[pos, pos + digits)`, assembled from `block(pos / digits) >> r` and the next block `<< (digits - r)`
where `r = pos % digits`. Two things make it total where it is called. A shift by `digits` is undefined, so
an aligned read is the block itself with no second term; and the last block has nothing above it, so the
read stops there rather than asking for a block past the end. What the word reaches beyond the width is the
clear tail, and it is the caller's to trim. It is the one primitive under every unaligned read: the sequence
adaptor's `append_range` from a sequence read by block ([the-range-members](#the-range-members)), the bulk
operations on a window ([windows](#windows)), and the bitset reading's order at unequal widths
([the-ordering-invariant](#the-ordering-invariant)), where each operand's top window is read as words at its
own alignment and both windows end at their own width, so the tail needs no mask.

`contiguous_bit_container::set_word(pos, value, mask)` is its write side, on our storage alone as `set_block`
is: the bits of `value` under `mask` land at `[pos, pos + digits)`, split over two blocks where `pos` is not
aligned, the tail kept clear. Every masked write goes through it: boost's ranged `set`, `reset` and `flip`, a
window's `fill`, and a window's bulk operators, each walking the words a range spans with the mask of what each
holds, whole words and a partial one at the end.

### the-funnel-shift

Under `word_at` and both shift operators is one operation: two adjacent blocks spliced into a double-width word
and shifted down. `contiguous_bit_container::straddled_block(index, L_shift, R_shift)` is that splice, and the
three sites now read as three uses of it rather than three spellings.

It takes the shift **and** its complement, named as both operators already name them, because both already hold
the pair as loop invariants: passing only one would have the other derived back inside from what it was derived
from outside. That round trip is one hoisted instruction, but it is also the thing that made the codegen differ
at all -- with the pair passed, GCC emits the same instruction mix as the hand-written original (954 lines, 60
subtractions, six `$64` immediates against 1016, 67 and twelve), differing only in register assignment. The two
adding to the block width is the whole contract, and the assert says so. There is no one-shift overload:
`word_at` holds only the offset and spells the complement at its single call site.

They did not look alike, which is why it went unnoticed. `word_at(n)` takes `(index, offset)` from `n`.
`operator>>=` reads `(i + n_blocks, R_shift)` — literally `word_at(i * digits + n)`, reached without
recomputing the division. `operator<<=` reads *one block below* its destination, `(i - n_blocks - 1,
R_shift)`, because it walks down while writing up; its loop names `L_shift` and the complement is the
offset, which is what hid the third one.

**It asserts rather than guards, and that is the design content.** Every caller already handles its own edge,
and they are different edges: `word_at`'s is a full-width shift or a missing block above, each operator's is
the destination block with nothing beyond it, and — the one that matters — both operators branch on the
*aligned* case **outside** their loop, into `std::shift_left` or `std::shift_right`. A guarded primitive
called once per block would drag that test into the loop and cost the `memmove` fast path, which is the one
thing those operators get for free. Keeping the check where it is preserves it, and the splice still exists
once instead of three times.

## Contracts

### total-versus-precondition

`contiguous_bit_container::exclusive_find_prev` is deliberately **not** total. Where `inclusive_find_next`
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

`contiguous_bit_container` states each operation in its cheapest form and lets the caller that needs more pay
for more. `exclusive_find_next` requires `is_valid(n)`; `exclusive_find_prev` requires a set position strictly
below `n` — which `any()` does not establish, a container whose set positions all lie above `n` having none
below it.

**A caller that needs a total answer restores totality where the width is already in hand.** Reverse iteration
does it with `rend()` ([total-versus-precondition](#total-versus-precondition)). `bitset_adaptor::find_prev` has
no iterator to lean on, so it does it in two comparisons it was making anyway: clamp a position past the width
down to the width, and answer `npos` when the first set position is not below the clamped one — which covers
both the empty storage and position zero. Neither guard is in the storage, because every other caller of the
reverse step already knows it does not need them.

This is the direction that matters. A caller may widen a contract; the storage may not narrow one. Putting the
guard in the storage would cost the three instructions at every call site to serve the one that asked.

## One storage

### one-storage

There used to be a trait. `bit_traits<Bits>` was declared and never defined, specialized beside each storage,
and the adaptors carried it as a fourth template parameter — `set_adaptor<Bits, Own, Traits>` — so that a
question about a storage could be answered by something other than the storage. Around it sat a layer of
generic scans that synthesized whatever a specialization had not declared, choosing a block tier or an
element tier by whether the entry was there.

All of that existed for a plurality that no longer exists. The `ext/` specializations for `std::bitset` and
`boost::dynamic_bitset` became comparison targets rather than adapted storages
([owning-is-ours](#owning-is-ours)), and the three adaptors were constrained to
`specialization_of_contiguous_bit_container` — nominally, so a storage is ours because the detector says so
and not because its members answer. From there the trait named exactly one thing, the adaptors named it twice,
and `bit_traits<contiguous_bit_container<...>>` was twenty entries forwarding to members its only caller
already had. So the parameter is gone, the trait is gone, and the scans are gone with it: the storage answers
in its own name.

**What that is worth is not the line count.** The element tier was the half of the layer that only a foreign
storage could ever reach, and it was reachable code carrying reachable branches — a reverse walk one position
at a time, quadratic over a traversal, sitting behind a probe that our storage always fails. The scans also
had to be total, because a synthesized fallback cannot know what its caller checked; the members they now
resolve to state preconditions instead ([the-cheapest-contract](#the-cheapest-contract)). And every question
now has one answer at one address, so a reader chasing `find_first` reaches the loop rather than a dispatcher
over a probe.

**What it costs is the extension point.** Specializing `bit_traits<MyStorage>` was how an outside storage
joined, and nothing replaces it: a storage joins by being a `contiguous_bit_container`, which lives under
`detail/` and is spelled by the library alone. That is a real narrowing and it is deliberate — the trait was
paying for a generality with no second instance ([the-interface-line](#the-interface-line)).

**There is no local detector.** `specialization_of_contiguous_bit_container` was a variable template
specialized on `contiguous_bit_container<Blocks, N>` plus a concept stripping the const, because the obvious
`is_specialization_of` names a template through a `template<class...>` parameter and this storage is a type
then a value. xstd-misc now carries one concept per parameter shape, so the adaptors spell the constraint
where they take the parameter:

```cpp
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, ownership Own>
class set_adaptor;
```

Not an alias for it. A local name for a library concept applied to a local template is a second thing to
learn that says nothing the spelling does not, and it hid which part was general: `TN` is the shape, the
storage is the argument. `TN` is spelt `<class U, U...>` rather than `<class, auto...>`, which would also
take a value typed by the type, and it strips the const a view over a const owner names -- the reason the
local concept existed beside the local trait at all. A constrained parameter is no obstacle:
`contiguous_block_range Blocks` binds it as a bare `class` would.

### what-the-readings-share

Almost everything the three readings ask of a storage is the same operation under another name, which is why
the vocabulary is smaller than the three APIs suggest:

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
  make `contiguous_bit_sequence` the intersection of the three vocabularies rather than the union
  ([the-common-vocabulary](#the-common-vocabulary)).
- **`fill(value)`** is bulk, and `clear` is `fill(false)`. Not an overload of `set` for the same reason.

### why-nested

The free functions live in `xstd::detail::bits` rather than in `xstd`, and the nesting is load-bearing.

Since C++20 ([temp.names]/3, P0846) an unqualified call with explicit template arguments — `shl<Block>(b, n)`,
or the `scan_first<Traits>(c)` this rule was written for — parses its `<` as a template argument list and then
performs ADL **with those arguments included**, so `std` and `boost`, the associated namespaces of the types
in play, would join the overload set. `[namespace.std]` bars *users* from adding to `std` but not
implementations, and boost is under no such constraint.

Down here nothing is visible unqualified from `xstd`, so the qualification is enforced by **scoping** rather
than by remembering a prefix at every call site — which is what a class was previously substituting for.

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
block writes are only ever needed on storage we control, and the unused tail is `contiguous_bit_container`'s
to keep.

## Orderings

### two-readings-disagree

"Lexicographic" is underspecified below the reading layer. All three readings are lexicographic; they order
over different sequences, and **they disagree**, pairwise, as two pairs show:

| pair | set reading, ascending positions | sequence reading, bools from index 0 | bitset reading, bools from the top |
|---|---|---|---|
| `{0}` against `{1}` | `[0]` vs `[1]`: less | `[1,0]` vs `[0,1]`: greater | `"01"` vs `"10"`: less |
| `{0,1}` against `{1}` | `[0,1]` vs `[1]`: less | `[1,1]` vs `[0,1]`: greater | `"11"` vs `"10"`: greater |

A storage serving three readings cannot hold one of their orderings under a neutral name without choosing for
its callers, so it holds none under that name. It holds **all three, separately named**:
`contiguous_bit_container` has a `set_lexicographical_compare_three_way`, a
`sequence_lexicographical_compare_three_way` and a `string_lexicographical_compare_three_way`, never one
`lexicographical_compare_three_way`, so a caller says which reading it means rather than being handed whichever
the storage happened to pick. Each is the reading's own blockwise answer to the standard algorithm it is named
for, which is also what pins it: whatever it answers has to agree with `std::lexicographical_compare_three_way`
over that reading's own iterators.

**Each is named for what it orders over, not for who asks.** Positions, bools from index 0, and the bit
string — which is why the third is `string_lexicographical_compare_three_way` and not
`bitset_lexicographical_compare_three_way`. The storage does not know what a bitset is; it knows the order
`to_string()` would put its bits in, most significant first, and that order is the one `xstd::bitset` and
`boost::dynamic_bitset` both mean by `<`. Naming two of the three after readings and the third after a
container would have put a caller's word on the storage's member, which is the same mistake an unqualified
`lexicographical_compare_three_way` makes one step further along.

### the-ordering-primitive

All three orderings are **hidden friends** of the storage rather than members:
`set_lexicographical_compare_three_way(x, y)` and not `x.set_lexicographical_compare_three_way(y)`. An ordering
is a question about two values with neither as its subject, and the member
spelling put one of them in a place the operation does not have -- the same asymmetry a member `operator<=>`
would carry. As friends they are reached by ADL, which is how the three adaptors call them.

`any_above` stays a member because it IS asked of one value: whether *this* storage holds anything above a
position. `first_difference` is symmetric -- its answer is an `xor`, which commutes -- and stays a member only
because it is a private step of the orderings rather than a vocabulary anyone spells; the same is true of
`padded_first_difference` and `padded_set_three_way`. `set_equal` **has** taken the same form, and for the same
reason: equality is as much a question about two values with neither as its subject as an ordering is, and
`x.set_equal(y)` spelled a symmetry the operation has and the call did not. It now sits as a hidden friend
beside the defaulted `operator==`, which was already a non-member.

`intersects` followed, and the standard library says why: **`intersects` is to `set_intersection` what
`contains` is to `find`** — the predicate form of an algorithm. `find` is a member of `std::set`, asked of one
set with a key, and so `contains` is a member too. `set_intersection` is a free algorithm over *two* ranges,
so `intersects` is free.

**Which reading keeps a member is then decided by the counterpart, not by taste.** `boost::dynamic_bitset` has
`a.intersects(b)`, so `bitset_adaptor` keeps that member by [a-strict-extension](#a-strict-extension). `std::set`
has nothing of the kind, so `set_adaptor` keeps no member and offers the friend alone — which is also the
spelling the analogy above asks for.

Where a member and a friend both exist, **the friend forwards to the member and never the other way**, and that
is a language rule rather than a preference. A member of the name ends unqualified lookup before ADL begins
([basic.lookup.argdep]/1), so from inside `bitset_adaptor::intersects` the call `intersects(m_bits, rhs.m_bits)`
finds the enclosing class's own member, fails to match it, and never reaches the storage's friend. No spelling
recovers it, a hidden friend having no qualified name either, and the same wall stands between a member and its
*own* class's friend. Both measured, not assumed. So the storage carries the pair `swap` already carries — a
member that does the work and a hidden friend that forwards — and `bitset_adaptor` carries it too, its member
reaching the storage's member and its friend reaching its own member. `set_adaptor`, having no member in the
way, reaches the storage's friend directly.

`is_subset_of` and `is_proper_subset_of` take neither form, since `a ⊆ b` is not `b ⊆ a` and the member
spelling states that correctly.

The first two orderings are answered a word at a time, from two pieces:

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

**Both are total across two widths, and the sequence reading was not.** `sequence_lexicographical_compare_three_way` opened with
`assert(x.size() == y.size())` while `sequence_adaptor::operator<=>` called it unconditionally, so every
`bit_vector` comparison of two lengths aborted in a debug build -- and, with the assert compiled out, answered
*wrongly* rather than not at all. Measured against `lexicographical_compare_three_way` over `std::vector<bool>`
across 148,225 pairs (every length 0 to 70 plus the block boundaries 127/128/129/191/192/193, five patterns
each): **27,312 wrong answers** before, zero after.

The repair is the set reading's, one clause lighter. `padded_sequence_three_way` pads the shorter operand's
missing blocks with zero exactly as `padded_set_three_way` does -- the invariant keeps everything above
`size()` clear, so a block that is not there reads the same as a block that is
([width-is-capacity](#width-is-capacity)) -- and where the set version asks `padded_any_above` at the deciding
position, the sequence version asks nothing: position 0 is the sequence's *first* element, so holding the
lowest differing position settles it outright. What the sequence reading needs instead is the case the set
reading cannot have: agreeing at every position the two share leaves only length, and the shorter is then a
proper prefix of the longer and so less, which is `size() <=> size()`.

That an assert stood where an answer belonged is the pattern in [the cheapest
contract](#the-cheapest-contract) read the wrong way round. A precondition is free to be narrower than the
naive form only where the caller can honour it; this caller could not, having two lengths whenever its user
did.

**Why this beats iterating.** The `lexicographical_compare_three_way` form walks *set bits*; this walks
*words*, and a word step is an `xor` and a test rather than a load, a shift, a `countr_zero` and a branch.
Equal values are the clearest case: iteration confirms every element, so a full 1024-bit set costs 1024
bit-scans against 16 word `xor`s. Near-equal and dense values are the same story. Only an early difference
makes the two comparable, both exiting at once.

**It is one sweep, not two passes.** `first_difference` covers blocks `[0, i]`; `any_above` then covers
`[i, n)` on **one** operand, the one that lacked the bit. Block `i` is the only one touched twice, so the
pair costs `n + 1` block reads. And when the values are equal `any_above` is never reached at all, since
`first_difference` already settles it.

**The bitset reading needs neither piece, and its width-crossing arm is a third shape again.** The bit string,
most significant position first, **is** the blocks from the top block down, with the unused tail kept clear, so
it is the one reading whose order is plain lexicographic over words — and plain lexicographic over words is
`std::lexicographical_compare_three_way` over the blocks reversed. `string_lexicographical_compare_three_way`
is that call and nothing else: no loop of its own, and no arm for either degenerate width, since a zero width
still holds its one all-padding block, clear in both, and a one-block width is the algorithm's first step.

Handing it to the standard algorithm costs nothing at the widths the hand-rolled version had arms for. GCC 15
at `-O2`: `xstd::bitset<64>` is one `cmpq`, and `xstd::bitset<128>` is the same two comparisons fully unrolled,
top block first.

Two widths it answers by `top_aligned_three_way`, which is boost's order and **cannot pad**. The other two
readings run from position 0 upward, so a block the narrower storage does not have sits *above* its positions
and reads as the zero the invariant already keeps there. The bit string runs from the top down, so what the
wider one holds outside the shared window sits *below* the comparison rather than above it, and is never
consulted at all: the top `min(size())` positions of each are paired from the top, read as words at either
one's own alignment through `word_at`, and only if that window ties does the shorter one lose for being
shorter. That walk lived in `bitset_adaptor`, which meant the one reading whose adaptor could not simply call
its storage; it now sits beside `string_lexicographical_compare_three_way` and is reached from it, so all three orderings are total and
none of the three adaptors branches on width. A pure relocation, and measured as one: identical answers over
20,172 unequal-width comparisons.

**The prefix clause is not removable.** Set order is not plain lexicographic over words under *any*
comparator. At `digits = 4`, `A = {1}` and `B = {5}` differ in word 0, where `A₀ = {1}` and `B₀ = {}`; a
prefix rule over words says `B < A`, but the sets truly compare `A < B`. `any_above` is exactly the repair,
and is the whole of what separates the two readings.

**The standard algorithm is the specification.** Whatever the word-at-a-time form answers has to agree with
`lexicographical_compare_three_way` over the reading's own iterators, which is
[the invariant](#the-ordering-invariant) itself, and the test is that the two paths agree. The efficient form
is then free to state preconditions the naive one does not ([the cheapest contract](#the-cheapest-contract));
what it may not do is answer differently.

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
each spell out the one- and two-block cases the way `find_front` and `intersects` do.

Each caller of a scan carries the width-zero arm ahead of the call, and `zero_width<Bits>` is the one
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

which is stated on the **reading**, never on the storage: `contiguous_bit_container` has no `begin()`/`end()`,
and `boost::dynamic_bitset` has no public iterators, so it cannot be written against a backend at all.

`bitset_adaptor` does not iterate, so its left-hand side is the sequence reading's traversal **reversed**:
`std::views::reverse` over the bools, the order `to_string()` writes them in. That is `boost::dynamic_bitset::operator<`
exactly, a **third** reading. Boost pairs *a*'s highest bit with *b*'s highest, second with second, then
breaks a tie on width -- the standard algorithm over reverse iterators. Verified over 1,046,529 pairs
spanning every width 0-9 against every other: zero mismatches against reverse-lex, 223,893 against numeric
order. It is *not* magnitude ordering, though it coincides with one at equal width: `"1"` and `"01"` are both
the number 1, and boost orders them strictly, the shorter first. The harness pins it three ways: against
boost's own `<` pair for pair over those widths, against `to_string()` compared as strings, and within a word
against `to_ullong()`.

`bitset_adaptor::operator<=>` is `string_lexicographical_compare_three_way` at equal widths, and at unequal ones boost's own walk,
the top `min(size())` positions paired from the top and then the shorter first, a word at a time through
`word_at` ([the-blit](#the-blit)). `==` stays width-first, and `<=>` never answers equal at unequal widths,
so the two agree ([the-hashing-invariant](#the-hashing-invariant)).

Where a specialization offers nothing faster, the default is that standard algorithm over the reading's own
iterators — so the default cannot disagree with the specification, and only an optimization can.

### the-hashing-invariant

Every value the library compares, it hashes, and

```cpp
a == b  implies  hash(a) == hash(b)
```

under every reading. The counterpart rule ([a-strict-extension](#a-strict-extension)) governs a
wrapper's member surface, not the cross-cutting protocols -- equality, ordering, formatting, ranges, hashing
-- which follow the reading: the standard's own coverage, `std::bitset`, `std::vector<bool>` and
`std::string` hashing while `std::array`, `std::set` and `std::pair` do not, is history rather than design.

The engine is Boost.Hash2: each adaptor carries a `tag_invoke` hook for `hash_append`, and `std::hash` is
one detail helper over it, a hash folded by `get_integral_result`. The algorithm is chosen in exactly one
place, and it is chosen as a **default rather than a fact**: `std_hash` takes `Hash h = {}` over a defaulted
`fnv1a_64`, so overriding it is an argument from outside rather than an edit here. Defaulted on the template
parameter as well as the function parameter, because a default function argument is not a deduced context and
`std_hash(v)` would otherwise fail to deduce `Hash`; and taken by value rather than by type alone, so a
seeded instance substitutes and not only a default-constructed one.

What `std::hash` itself gets is always that default. Its `operator()` takes one argument and has no second to
forward, so all three specializations take `fnv1a_64` and the parameter is unreachable through them — which
is why it is asserted directly, in `test/src/bits/detail/hash.cpp`, rather than through a specialization. A
caller wanting another algorithm has the better door anyway: the adaptors' `hash_append` hooks, reached with
a hash of their own. What a hook appends is the value
**itself**, never a storage's own hook: the blocks and the width. So equal values hash equal whatever holds
them, and no storage's `std::hash` is consulted. The set reading at a run-time
width appends the positions held and their count instead, since equal sets need not share a width
([width-is-capacity](#width-is-capacity)).

Who hashes follows [views-follow-their-precedent](#views-follow-their-precedent): the set adaptor owned or
viewed, as `std::string_view` hashes; the sequence adaptor as an owner alone, as `std::span` does not, so its
hook is constrained on ownership and a `bit_span` hashes no more than it compares; `bitset_adaptor` as
`std::bitset` does. Both range adaptors tell ContainerHash they are not ranges: Hash2 chooses between its
range overload and a hook by `enable_if`, a range with a hook is ambiguous, and the range overload could not
hash the proxy the iterators return anyway. The harness checks the invariant beside `==`, wherever a
`std::hash` exists.

## Coverage

### per-instantiation-slots

gcovr counts branch slots **per template instantiation and never merges them**. A loop that a one-block
instantiation can never enter is a branch never taken, whatever every other instantiation does.

So a degenerate width does not get an unreachable loop; it gets **different code**, via `if constexpr` on
`static_num_blocks == 1` and `== 2` in `contiguous_bit_container`, and on `static_block_count` and `extent == 0`
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

`front()` and `back()` on the sequence reading were one-liners and are four lines apiece for this, and no other
reason. Adding the two preconditions to them cost seven branches across the deducing-this instantiations and
took the gate from 100% to 99.2%, where the same asserts in `erase` and `index_of` -- each on a line of its own
-- cost nothing at all. The rule is worth stating because the failure is silent at the point of writing: the
code is right, the assert fires as intended, and only the gate says otherwise.

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

### msvc-reports-the-unreachable-tail

A zero width holds no position, so `bitset_adaptor`'s guard throws for **every** `pos`:

```c++
if constexpr (has_static_width) {
        if (pos >= size()) { throw out_of_range(pos); }   // size() is 0, so always
}
```

Which makes everything after `guard(pos)` in the five guarded element accessors unreachable at that width, and
MSVC 2026 Release says so — **C4702**, fatal under `/WX`. The trait had been hiding it: `Traits::size(c)` and
`Traits::unchecked_assign(c, pos, val)` were calls MSVC's `/O2` flow analysis did not see through, so it never
folded `size()` to zero and never proved the guard unconditional. Reading the storage directly is what let it.
That is the same trade the rest of this refactor makes — one fewer layer, and the compiler sees what the layer
was covering — and here what it sees is true.

Guarding only the *write* moves the warning rather than removing it: the tail is then `return *this;`, which is
no more reachable. So the zero width gets its own arm, and at that width the whole body is the throw:

```c++
if constexpr (detail::bits::zero_width<Bits>) {
        throw out_of_range(pos);
} else {
        guard(pos);
        m_bits.assign(pos, val);
        return *this;
}
```

Nothing changes for a caller — the guard threw the same exception for the same positions — and the
instantiation now carries no statement that no control flow reaches. It is the same remedy
[degenerate-widths](#degenerate-widths) reaches for elsewhere: a degenerate width gets **different code**, not
unreachable code.

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
requires is_owner and requires (bits_type const& b) { sequence_lexicographical_compare_three_way(b, b); }
```

`bits_type` is complete and already in hand, and for an owner it is exactly what `storage()` returns, so
satisfaction is unchanged on every compiler. The call is spelled as a free function because the three orderings
are hidden friends of the storage rather than its members ([the-ordering-primitive](#the-ordering-primitive));
when this was diagnosed it read `b.sequence_lexicographical_compare_three_way(b)`, and the failing form above read
`x.storage().sequence_lexicographical_compare_three_way(y.storage())`. What made MSVC complete the class was naming a **member** of
what the accessor returns, which is the shape the record below is about; whether the call spelling would have
tripped it too was never measured, because the constraint had already moved off the accessor.

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

### uint128-printing

Never `BOOST_CHECK_EQUAL` on a block-typed value. It prints its operands on failure, and no standard library
defines `operator<<` for `__int128`, so a `graded_extents` sweep that includes `xstd::uint128` fails to
compile on libc++ where it happens to compile on libstdc++. `BOOST_CHECK` compares without printing.

## Naming

### test-not-subscript

`contiguous_bit_container::test` rather than `operator[]`: this reads and cannot be written through.
`std::bitset`'s `operator[]` returns an assignable proxy and this returns `bool`, so the subscript spelling
would promise an assignment that does not compile.

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

## Views and containers

### the-three-adaptors

Three class templates carry the three readings: `set_adaptor`, `sequence_adaptor`, `bitset_adaptor`. Each is
written against `contiguous_bit_container` and against nothing else, so one adaptor serves
`contiguous_bit_array`, `contiguous_bit_vector` and `contiguous_bit_inplace_vector` alike, at both widths and
in both ownerships ([one-storage](#one-storage)). The public names
are aliases in two layers over them: `basic_bit_static_set<B, N>` is `set_adaptor<contiguous_bit_array<B, N>,
owns>`, `basic_bit_array<B, N>` is `sequence_adaptor<contiguous_bit_array<B, N>, owns, false>`, and
`basic_bitset<B, N>` is `bitset_adaptor<contiguous_bit_array<B, N>>`; `bit_static_set<N>`, `bit_array<N>` and
`bitset<N>` are those at `std::size_t`.

### owning-is-ours

Owning is ours and viewing is interop. An owning adaptor sits over a storage of this library,
`contiguous_bit_array` or `contiguous_bit_vector`, and nothing else is supported or tested: `bitset_adaptor`
requires the vocabulary ([a-strict-extension](#a-strict-extension)) and block access, which neither counterpart
satisfies, and the owning `set_adaptor` and `sequence_adaptor` take their ordering from the storage's own
three-way members, with no synthesized fallback for a storage without them. A view sat over any type with a
trait, which is what the two `ext/` specializations were for.

The split is what a foreign owner cost against what it bought. Every owner-only member -- construction
through the storage's constructors, growth, the saturating shifts, the checked element access, the ordering
-- needed an entry in each foreign trait and harness arms over each foreign storage, and the blit and the
range insertions of #80's 7d would have added more. It bought one object where a view over a member gives the
same reading: `bit_set_view(my_std_bitset)` is the set reading over a `std::bitset` someone else owns, and
`bit_static_set` is the same reading over storage of ours. What is given up is the wrapped-equals-raw proof
that the wrapper adds and drops nothing; the views keep it for reads, writes, iteration, searches and hashing,
and construction, growth and the ownership protocol are proven over our storages alone.

So the `ext/` traits were the view's contract: `extent`, `size`, `at`, `count`, `unchecked_assign`, `insert`,
`fill`, and the searches and block reads where the type had them. They went with the viewing column, along with
the `checked_*` family and the checked shifts, which only a wrapper asked, and the `operator-=` and `operator-`
on `std::bitset` that lived in `namespace std` against `[namespace.std]`: `bit_set_view` has `-=`, and
`std::bitset` has no set difference of its own. With no foreign storage left to adapt, the trait that adapted
them had nothing to abstract over either ([one-storage](#one-storage)).

### ownership-is-not-an-axis

Owning versus viewing is storage lifetime, not a third axis of the model, and it collapses to one template
parameter: `ownership::owns` stores `Bits`, `ownership::refers` stores `Bits*`. Always present and only its
type changes, so a plain `conditional_t` rather than `conditional_data_member_t`. One accessor, via deducing
`this`, gives deep const to the owner — `self.m_bits` propagates `self`'s const — and shallow const to the
view — `*self.m_bits` does not — for free.

Every mutator is then gated on the storage and nothing else: `requires requires { self.storage().op(…) }`
reads "the storage lets *this handle* write". A const owner's accessor hands back a `Bits const&`, which has no
`assign` to reach; a const view's hands back `Bits&`, which is what a view is for; a view over `Bits const`
hands back `Bits const&` again. Const and ownership are the same question, asked once, and answered by the type
the accessor returns. The exceptions are the constructors and, once storage grows, the growth members, which need an explicit
`requires (owns(Own))`: the requires-expression tests what the storage can do, not what this handle may do to
it, and a view over a `contiguous_bit_vector` must not be able to resize what it does not own.

### views-follow-their-precedent

`bit_set_view` follows `std::string_view`: a value that happens not to own its bytes, so it has `==` and
`<=>`, and its ordering is exactly `std::set`'s. `bit_span` follows `std::span`, which P1085 stripped of both
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

This branch had it the other way first, and the history shows the detour: an `incomparable_base` deriving from
the one empty type and deleting the two comparisons it brought. That worked — the deleted pair took
`sequence_adaptor` exactly where the base's took the empty type by a derived-to-base conversion, so it won
overload resolution by [over.ics.rank]/4.4 and the answer was ill-formed rather than wrong — but it needed
BOTH deletions, and neither implied the other: `<=>` rewrites the four relationals and never `==`; `!=`
rewrites from `==` and never from `<=>`; and a defaulted `<=>` implicitly declares a defaulted `==` beside it
([class.compare.default]). Deleting `==` alone left `<`, `>`, `<=` and `>=`; deleting `<=>` alone left `==` and
`!=`. It also could not be written where it belonged: MSVC rejects a trailing requires clause on a *deleted*
friend (C7599) where it accepts one on a defaulted friend, so the pair could not be constrained on
`not is_owner` in the adaptor and had to move into a base of its own. Splitting the empty type upstream
deletes all of that.

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
them, which is what lets `ext/xstd/bitset.hpp` return `set_adaptor(c).begin()` from a temporary.

### the-comparison-is-a-hidden-friend

All three adaptors spell `operator==` as a defaulted hidden friend. `bitset_adaptor` was the exception until
measured, carrying the member that `std::bitset` specifies, on the reading that a defaulted comparison has to
be one. It does not: [class.compare.default]/1 admits a non-static member **or a friend**, and
`sequence_adaptor` had been defaulting a *constrained* friend all along.

The choice used to be observable, and no longer is. `std::bitset`'s converting constructor from
`unsigned long long` is not `explicit`, so a mixed comparison compiles; which spellings compile depends on
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
first one did. That form would reject what `std::bitset` accepts, which is the one way to break
[a-strict-extension](#a-strict-extension); the hidden friend only ever accepts more. It would also need the
forward-declaration and `friend operator==<>` dance for private access: three declarations in dependency order
for one operator.

The shifts went the same way, later and for the same reason. `std::bitset` spells `operator<<` and
`operator>>` as members while spelling `&`, `|` and `^` as non-members -- its own inconsistency, not a rule --
and the guidance it departs from is the ordinary one: `@=` belongs to the left operand and `@` does not.
Nothing observable turns on it: a shift's other operand is a `size_t`, so it brings no class to ADL, and the
left operand must already be a `bitset_adaptor` for any candidate to be found at all. So this is shape, as the
comparison was, and it takes the same form the rest of the tree takes.

Mirroring `std::bitset` is not itself an argument. It is the oldest type in the library and reads like it: a
member `operator==` where every container has a non-member one, no `swap` at all, and the split above. C++20
changed none of it.

What the friend costs is `&bitset<N>::operator==`, and the namespace-scope shifts cost
`&bitset<N>::operator<<`, both well-formed against `std::bitset` because the standard puts those operators in
the class. Nothing forms a pointer-to-member to a comparison, and the alternative was one
adaptor of three disagreeing with its siblings on every read of the file.

### the-views-are-the-adaptors

`bit_set_view<Bits>` **is** `set_adaptor<Bits, ownership::refers>` and `bit_span<Bits>`
**is** `sequence_adaptor<Bits, ownership::refers, false>` — alias templates, the way `bit_subspan`
always was, and not a second implementation of either reading. They carry the names of
[the-public-names](#the-public-names), one header each beside the owners; the `set_view` and `sequence_view` of
the rewire were the same classes before the viewing column was filled. The earlier views, with their own
iterators, proxies and four customization points — `set_find`, `sequence_find`, `block_access`, `bit_extent` —
were the trait before there was one, and once the adaptors read the storage alone there was nothing left for
them to do.

**They were derived classes first, and the reason was real but has expired.** This section used to say that
deduction through an alias is class template argument deduction for alias templates (P1814), "which Clang 19
and GCC 10 have and MSVC does not: `bit_set_view(x)` on MSVC is 'too few template arguments'". That
observation was **correct**, and the matrix still reproduces it word for word — `C2976: 'xstd::bit_set_view':
too few template arguments`, alongside `C2641: cannot deduce template arguments`, 151 times over both views
on the VS 2022 rung.

What changed is the generation, not the claim. **MSVC 18 (VS 2026) deduces through these aliases; MSVC 17 (VS
2022) does not**, in both `msvc` and `msvc_analyze`, Debug and Release. So the aliases cost the 2022 rung, and
that is the trade [msvc.yml](.github/workflows/msvc.yml) now takes.

Two corrections worth keeping, because both were mine and both were wrong in the same direction — trusting a
document over a compiler. Microsoft's conformance table lists `P1814R0 CTAD for alias templates` as **VS 2019
16.7**, footnoted only with the flag gate this project clears at `/std:c++23`; from that I concluded the
feature "was never what was missing" and that the note here was wrong. The table is describing the feature,
not this shape of it — one pinned non-type argument plus a defaulted, constrained argument depending on the
first — and on that shape MSVC 17 fails while claiming support. A four-day-old note quoting a specific
diagnostic was the better evidence, and it deserved to be believed over a vendor's feature matrix.

**The break is the MSVC compiler, not the VS 2022 platform.** `clang_cl` keeps all three rungs and passes on
all of them, 2022 included, because clang-cl is Clang and Clang has had P1814 since 19. So VS 2022's runner,
STL and platform stay covered; only the MSVC 17 front end is gone from the matrix.

**The rung has paid two dividends so far, and the ledger belongs here with the cost.** Both were workarounds
that existed for MSVC 17 and nothing else. The first: `decay_copy` became `auto(x)`
([the-functor-takes-a-value](#the-functor-takes-a-value)), twenty lines for two. The second: the one
`typename` still written in `test/include/test/set/primitives.hpp`, on the default argument of a constrained
type-parameter -- `std::integral T = typename X::key_type`, which MSVC 17 rejected without it as `C2061:
syntax error: identifier 'integral'` while GCC, clang, clang-cl and Apple clang all took it. It was the last
site in the tree where [P0634R3](https://wg21.link/P0634R3) permits the omission and the keyword was still
spelled; the remaining `typename X::value_type` sites are template arguments and functional casts, which
P0634R3 does not reach. The `NOLINTNEXTLINE(readability-redundant-typename)` that had to sit above it went
with it -- a live suppression, not a dead one: the check exists in clang-tidy 22 and 24.

Being the adaptor rather than deriving from it is what removes the restatements, and they were the whole cost
of the workaround: a derived class needed its own constructors, its own two deduction guides, and its own
`enable_view`, `enable_borrowed_range`, `std::hash` and `is_range` specializations, because **a derived class
is not its base to a partial specialization** — the base's opt-ins say nothing about the derived name. An
alias *is* the base, so all four apply to it already. The constructors also had to be spelled out rather than
inherited, since inheriting them inherits the primary's guides as well (P2582, which GCC implements) and those
tie with the restated ones; with no restated guides there is nothing left to tie.

**The diagnostics belong on that same list.** An alias is transparent, so a storage the library does not wrap
is diagnosed where the alias is *written*: `void f(my_set<int>)` is an error at that declaration, quoting the
unsatisfied constraint. A derived class that leaves the
constraint to its base is not: naming one in a declaration does not require a complete type, so the same line
**compiles**, the base is never instantiated, and the diagnosis waits for whoever first completes the type.
It then arrives twice, because a dependent base is named twice and cannot be named once -- in the
base-specifier, and again in the using-declaration that inherits the constructors. Measured on `set_adaptor`
over a storage the library does not wrap: 13 lines and one error through the alias, 22 lines and two errors
through such a derived class.

Restating the constraint on the derived class's own parameter recovers all of that, and then some: it fails at
the declaration, once, in **fewer** lines than the alias, having no indirection to explain. So this is not a
second reason standing beside the restatements -- it is one more entry on the same list, and the failure mode
is the derived class that skips it. A four-line reduction holding a constrained class template, an alias of
it, and both derived forms reproduces the shape exactly, GCC and Clang agreeing to the line, so it is the
language rather than a diagnostic quirk.

Where it would bite is the views, and only the views. The nine owners choose their own storage, so a storage
the library does not wrap cannot arise through them at all; the one parameter a user supplies is the `Block`,
constrained at every layer. A view takes the storage -- that is what a view is for ([owning-is-ours](#owning-is-ours)) --
so a view is exactly the place where a constraint left to the base would go undiagnosed until use.

The alias pays for this on the other side, and the trade is worth stating whole. Being transparent, it is not
what a compiler prints: a diagnostic about `bit_array<100>` names
`sequence_adaptor<contiguous_bit_container<array< unsigned long, 2>, 100>, ...>`, a spelling the user did not
write and cannot write back. `std::string` makes the same trade and the world lives with `basic_string<char,
char_traits<char>, allocator<char>>` -- though `std::string` aliases a *class*, where both layers here are
aliases, which is why the printed name falls through to the adaptor rather than stopping at `basic_bit_array`.

One constraint moved rather than vanished. The guide for a plain storage is viable for an owner too, now that
an owner is nothing but a storage under a wrapper, and would tie with the owner guide — so it is constrained to
non-owners, as [an-owner-reads-as-its-storage](#an-owner-reads-as-its-storage) describes. That constraint used
to sit on each view's restated guide; it now sits on `set_adaptor`'s and `sequence_adaptor`'s own, which is
where the aliases deduce through.

The sequence view pays the `span` half of [views-follow-their-precedent](#views-follow-their-precedent) by
becoming the adaptor: it no longer has `==` or `<=>`, and the harness checks the sequence reading through the
iterators instead.

### windows

`bit_subspan<Bits>` is `sequence_adaptor<Bits, refers, true>`: the referring adaptor
windowed, an alias rather than a derived class because nothing deduces it -- it is what `first`, `last` and
`subspan` return on a `bit_span` or on another window, and never spelled at a call site. It stores what
`std::span` stores, a pointer and a size, with the pointer's role split over a pointer and a position
because bits are not addressable: the sequence iterator's two fields and a count, 24 bytes beside the whole
view's 8. The offset is applied once, in a private `offset()` that answers zero for every other shape, so
`begin()`, `operator[]`, `at`, `front` and `back` are written the same way for all three.

The three members are [span.sub]'s, on a view and never on an owner, since `std::array` and `std::vector`
have no subviews either; they assert their preconditions rather than throw, and `std::dynamic_extent` is the
to-the-end sentinel. A window is a view in `std::ranges`' sense and borrowed like `span`, and like `span` it
neither compares nor hashes ([views-follow-their-precedent](#views-follow-their-precedent)).

A window's end blocks are shared with what lies outside it, so its bulk operations are masked words: `fill` over
a window of ours is `contiguous_bit_container::set(pos, len, value)`, a word at a time through `set_word`, and
one position at a time over a window of anything else; `&=`, `|=` and `^=` on a window of ours take a
source of any shape that reads blocks of the same block type, a window at any other alignment included, reading
both sides through `word_at` and writing through `set_word` masked to the window ([the-blit](#the-blit)). The
two must be of one size, and must not overlap short of coinciding, `w ^= w` being fine. No sequence has shifts,
window or whole ([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)).

**A window over a const storage writes nothing, and the predicate has to be asked of the right type to say so.**
`bits_type` is `Bits` with the const stripped, because a view over a const owner names `Bits const` and the
typedef wants the storage's own name. Asking `block_writable` of *that* asks whether a non-const storage takes
a masked write, which is always yes, so a const window advertised the three bulk operators it could not
perform -- and the mismatch surfaced inside `combine` as a hard error on a discarded qualifier rather than as
a constraint that simply failed, so even asking whether the operator existed would not compile. It is asked of
`Bits` instead, which carries the const, and the operators drop out of overload resolution the way `fill`'s
already did by constraining through `self.storage()`. `set_adaptor`'s const view was never affected: every one
of its mutators constrains through the storage expression.

### the-range-members

`append_range` has two tiers. Where the source is a sequence adaptor of any shape, owner, view or window, over
a storage whose blocks are the destination's block type, the source's bits are read as words at the source's own
alignment through `word_at` and appended a word at a time through `contiguous_bit_container::append(block)`,
which splits each word over the destination's own alignment; then the width is trimmed to the count, `resize`
clearing whatever the last word carried past it. Everything else, a `std::vector<bool>`, an `iota` under a
`transform`, a sequence over another block type, is packed a word at a time, which is boost's private
`bit_appender`, and trimmed the same way. A source inside the very storage being appended to is safe: the blit
reads positions below the old width alone, and no append writes one.

`insert_range`, `insert` in its four shapes, `emplace` and both `erase`s rebuild rather than shift: the head
through `first(pos)`, the middle, the tail through `subspan(pos)`, into a fresh sequence that is then swapped
in. Every step runs at the blit's tier, insertion into a packed sequence is linear however it is done, and
the strong exception guarantee comes free, which is what `std::vector::insert_range` gives on reallocation.
`subspan` pays for itself here, the head and the tail being exactly windows. In-place block-wise shifting of
the suffix stays available as a later optimization behind profiling.

`flip()` is `[vector.bool]`'s, a bulk operation like the three operators beside it, so an owner and a whole
view have it and a window does not; the static `swap(reference, reference)` is the proxies' own swap under
the name the standard gives it.

### what-a-sequence-may-add

`[a-strict-extension](#a-strict-extension)` binds the bitset adaptor in one sentence, and the sequence
adaptor had only `[the-sequence-contract](#the-sequence-contract)`: *`bit_vector` answers every line of
`[vector.bool]`'s synopsis*. That is a floor with nothing above it, and a floor is how the shifts arrived --
declared, never argued for, and spelled backwards
([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)).

The ceiling: **the sequence adaptor adds an operation only where the sequence reading is what asks for it,
and the spelling is the one that reading already uses.** Two questions, and a candidate answers both or it
does not cross:

- *Does a sequence of `bool` want this?* `flip()` is `[vector.bool]`'s own. The elementwise operators are
  `std::valarray<bool>`'s, the standard's one model for a bulk logical operation over bools
  ([the-elementwise-reading](#the-elementwise-reading)). A **difference** answers no: no standard sequence
  of bools spells `a and not b`, and `valarray<bool>`'s `operator-=` is arithmetic. A **shift** answers no
  twice over.
- *Is this the name that reading gives it?* `<<=` fails here even where the operation is wanted, because a
  sequence already spells moving elements `std::shift_left` and `std::shift_right`, in the opposite
  direction.

Being free is not an argument. Every operation the storage already has is free to forward, which is what
makes the forwarding tempting and the ceiling necessary: `-=` and the shifts were each one line over a
storage member that exists regardless, and the cost of a wrong one is not compile time but a caller who
reads `v <<= 1` as `std::shift_left`. What the storage provides is the **union** of the three readings'
demands ([the-three-adaptors](#the-three-adaptors)); each adaptor exposes its own reading's share, and the
shares are not the same set.

The four ways a reading answers an operation are all visible in the current surface:

| | |
|---|---|
| one name, a different thing per reading | `size()` -- the sequence's element count, the set's **cardinality**, the bitset's width |
| one reading alone | `complement()`, the set's; `count()` the sequence's and bitset's, where the set answers cardinality with `size()` |
| one reading spelling it otherwise | `flip()` on the sequence and bitset is `complement()` on the set |
| one reading declining it | `-=` on the set and bitset, not the sequence; the shifts on the bitset and set, not the sequence |

### no-shifts-on-a-sequence

`std::vector<bool>` has no shifts, and neither has any sequence here. The storage keeps `<<=` and `>>=`
because two of the three readings ask for them and mean different things by them: the bitset reading's is
`[bitset.members]`'s truncating bit string, and the set reading's translates, `<<=` growing a run-time width
to hold the result and `>>=` emptying past it. The sequence reading is the one with nothing to add. Worse,
it already spells moving elements, and spells it the other way round: `operator<<=` is implemented with
`std::shift_right` and `operator>>=` with `std::shift_left`, because a sequence's low index is its front
where a bit string's low bit is its right. Exposing the operators would have `v <<= 1` mean the opposite of
the algorithm whose name the sequence reading already owns. So `flip()` and the three
compound operators cross to the sequence adaptor and the shifts do not.

### the-elementwise-reading

What `&=` means on a sequence of bools is **elementwise logical**, not bitwise: `a &= b` is
`a[i] = a[i] and b[i]` over every position. `std::valarray<bool>` is the standard's one model for that, and
the only standard container that spells it -- neither `std::vector<bool>` nor `std::array<bool, N>` has the
operator at all. On packed bits the elementwise operation is the set operation's own instruction, so serving
the reading costs nothing, which is why the operators sit on the sequence adaptor rather than waiting for a
`bit_valarray` to be written.

Three, and not the storage's four. A **difference** has no elementwise reading: `valarray<bool>`'s own
`operator-=` is arithmetic, and `a and not b` is set vocabulary, so `-=` stays on the set and bitset adaptors
and comes off the sequence reading with the shifts
([no-shifts-on-a-sequence](#no-shifts-on-a-sequence)). The binary `&` `|` `^` are each their compound over a
copy and `~` is `flip()`'s value, all four on an **owner** alone: a view's copy refers to the very storage it
views, so a value returned by one would write through to it.

`bit_vector` is therefore `[vector.bool]`'s synopsis plus `flip()`'s value form, three compound operators,
three binary ones, `fill`, and the four aggregates -- and nothing that needs a bit to have an address.

### the-sequence-contract

`bit_vector` answers every line of `[vector.bool]`'s synopsis and `bit_array<N>` every line of `[array]`'s,
and the test says so as a checklist rather than a claim: `test/sequence/concepts.hpp` spells each synopsis
as one requires-expression, `vector_bool` and `array_bool`, and `std::vector<bool>` and `std::array<bool, N>`
are asserted against it first. A line the model itself fails is a wrong line, so the checklist is known to
be honest before ours is held to it; the C++23 range members are a second concept, `vector_bool_ranges`, so
the model is held to them only where its standard library has them.

The sweep found what the range members had not needed. The allocator: `allocator_type` through the same empty
base `bitset_adaptor` has, `get_allocator`, and the allocator-extended constructors,
`[container.alloc.reqmts]`'s copy and move included, which `contiguous_bit_container` gains beneath them,
deduced and matched to the storage's own so a static owner has none. `[vector.erasure]`'s `erase` and `erase_if`
as non-members over the owner's `erase(first, last)`, `std::ranges::remove_if` running unchanged over the
proxies, which move and swap. And `std::array`'s aggregate initialization as an `initializer_list` constructor
on the static owner, the listed values leading and the rest false, a longer list being the error it is on
`std::array`.

Three things are not offered, each because packed bits have no address. `data()`, and the `pointer` and
`const_pointer` typedefs, name what a proxy cannot give; the checklist leaves them out of
`[container.reqmts]`'s typedefs rather than inventing a pointer to a bit. `std::array`'s tuple interface,
`get<I>`, `tuple_size` and `tuple_element`, is left out with them: it is `std::array`'s claim to be a
product of `N` objects, and a packed sequence is one object. `std::hash` is asked of the vector alone,
`std::array` having none, while `bit_array` hashes as every owner does
([the-hashing-invariant](#the-hashing-invariant)).

### views-over-owners

An owner is not itself a storage — `bit_static_set`, `bit_array` and `bitset` are thin wrappers over a
`contiguous_bit_array` — so a view over an owner is a view over the storage it wraps:
`bit_set_view(xstd::bitset<64>&)` is `set_adaptor<contiguous_bit_array<size_t, 64>, refers>`, and the pointer in
the iterator is to the `contiguous_bit_array`, never to the `bitset`. The owner hands its storage over through
`owned_storage<Owner>`, declared beside it and never defined for anything else, so
`owner_of<Owner, Bits, R>` reads "this owner wraps exactly the storage this view refers through, and is not
already committed to another reading". Const flows one way: a const owner gives a
view over `Bits const`, a mutable owner either.

The view's converting constructor takes the owner's private member directly, which is why an owner befriends
a referring adaptor — the one friendship in the tree that runs upward, from a container to the views over it,
and it grants access to a member and to nothing that member's type does not already expose. Which adaptors it
befriends is [the-readings-do-not-mix](#the-readings-do-not-mix). Storage stays private; nothing on an owner's
surface says `contiguous_bit_array`.

### viewing-an-owner-is-implicit

`bit_set_view(my_set)` is a converting constructor on the **view**, not a conversion operator on the owner, and
the choice is forced rather than stylistic. `bit_set_view` and `bit_span` are alias templates, so every
deduction runs through constructors and guides on the adaptor; a conversion function contributes nothing to
class template argument deduction. Measured on a model of both shapes: with only the operator, `view(owner)` is
`no matching function for call to 'adaptor(...)'`. The constructor has to exist anyway, and once it does the
operator is a second mechanism for a conversion already spelled. Three lesser reasons agree. The constructor is
where `owner_of<Bits, R>` already hangs, so the readings-do-not-mix rule is stated once. Const falls out
of deducing `Owner&` rather than needing an `operator view<Bits>() &` and an `operator view<Bits const>() const&`
kept in step by hand. And `Owner&` is an lvalue reference, so a temporary owner never binds — the `string_view`
foot-gun closed by the signature instead of by a `&`-qualifier someone has to remember.

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

A view over an owner is a second reading of bits that already have one, and two of the three pairings are a
view choosing for the caller. `bit_set` is a set of positions; spanning it as bools would present the
container's capacity as its contents. `bit_vector` is a sequence of bools; viewing it as a set would present
the indices of the true ones as the elements. Neither is wrong in the way a bug is wrong — the bits do support
the reading — but neither is what the owner's own name says it holds, and the owner is the thing the caller
named. So a set owner admits only `bit_set_view`, a sequence owner only `bit_span`.

`bitset` is the exception and the reason both views exist. `[template.bitset]` is the bitwise surface, a width
of bits and the operators over it, committed to neither reading; its whole documented use is that you go on to
ask it something it does not answer itself — which positions are set, or what the bools are at each index.
That is what a view is for, so a `bitset` admits either.

The rule is one enumerator on the owner's side of the protocol, `owned_storage<Owner>::reads`, and one clause
in `owner_of`: the owner's reading is the view's, or it is `reading::bitset`. It has to live in the constraint
and not in the friendship alone. Dropping only the friendship leaves the constructor declared and viable, and
its `m_bits(&c.m_bits)` is a mem-initializer — not the immediate context — so the access check happens at
instantiation and nowhere earlier. Measured: `std::is_constructible_v<bit_set_view<Blocks>, bit_array<8>&>`
still answers **true**, and the actual construction fails with `'m_bits' is private within this context`
pointing into `set_adaptor.hpp`. A type trait that lies and an error inside a constructor the caller never
meant to reach are both worse than the constructor simply not being there, which is what the clause gives: no viable
deduction guide for `bit_span(bit_set{})`, and `is_constructible_v` false.

It is the constraint that does the work, so the friendships follow it rather than the other way round:
`set_adaptor` befriends `set_adaptor`, `sequence_adaptor` befriends `sequence_adaptor`, and `bitset_adaptor`
befriends both.

What this gives up is real and small: `bit_span(some_bit_set)` used to compile, and now does not. Nothing in
the library or the tests wanted it — the one assertion that exercised it was asserting the mechanism, not a
use — and a caller who genuinely wants the other reading of an owner's bits is asking for a conversion between
owners, which is an owner's to offer explicitly and not a view's to perform silently. None exists today; if
one ever should, it belongs beside the containers, where it can be named and its cost seen.

### the-interface-line

**If a user never spells it, it lives in `detail/`.** The name or the header, either counts. That is the whole
rule, and it is a test rather than a judgement: `bit_static_set` is spelled, `contiguous_bit_array` is not;
`ownership` is spelled by anyone naming an adaptor, `bidirectional_bit_reference` is reached only through the
`iterator` and `reference` typedefs and is spelled by nobody.

What the rule keeps on the interface side, each with the reason it is not obvious:

- **The nine containers and the three views.** Uncontested, and the reason the rest is worth stating.
- **The three adaptors.** Interface by necessity rather than by intent: the containers and the views *are*
  these types ([the-views-are-the-adaptors](#the-views-are-the-adaptors)), so every diagnostic quotes one,
  every `decltype` prints one, and a consumer pattern-matching on what it was handed writes
  `sequence_adaptor<B, O, W>` to do it.
- **`ownership`.** Dragged in by that: no adaptor can be named without writing `ownership::refers`. Said out
  loud because this is the kind of enum that gets called a detail right up until someone has to type it.
- **`contiguous_bit_sequence`.** The vocabulary the three bit containers share, which is a claim about
  `std::bitset` and `boost::dynamic_bitset` as much as about ours, so it is stated where a reader can check it
  ([the-common-vocabulary](#the-common-vocabulary)).

**One line moved from the first column to the second, and it is worth naming.** `bit_traits` used to be here,
with `ext/` as its worked example, because specializing `bit_traits<MyStorage>` was *the* extension point.
There is no such point now ([one-storage](#one-storage)): `Bits` must be a `contiguous_bit_container`, which
lives under `detail/`. So a consumer reaches the three adaptors by **deduction** — `bit_span(bs)`, or the
`decltype` of a container — and never by instantiating one over storage of their own. They are still interface,
because a name you cannot avoid reading is interface whether or not you can write it; they are no longer an
extension point.

On the other side, the two that had to be argued. The four proxy types are reached only through container
typedefs, so no user spells them. `contiguous_bit_container` and its three aliases are the device that turns
three readings times three storages into three plus three ([the-one-vehicle](#the-one-vehicle)), and the
`basic_` layer already exposes the block parameter — `basic_bit_static_set<std::uint8_t, 24>` reaches the
capability without the storage being named. Recorded against: `contiguous_bit_container` *is* instantiated by
name throughout `test/`, which is a real signal, and demoting it makes the test tree reach into `detail/`. The
counter is that a test is not a user; a test tree that mirrors the library, `detail/` included, is what testing
an implementation looks like.

**Enforced, not asserted.** `test/consumer/main.cpp` includes `<xstd/bits.hpp>` and no other header of ours,
and names every type above: the nine containers, the three views, the three adaptors and `ownership`. It
pattern-matches each container against the adaptor it is — `is_set_adaptor<bit_static_set<100>>` and so on for
all nine — which is the claim [the-views-are-the-adaptors](#the-views-are-the-adaptors) rests on, asked from
outside. The view names it reaches by deduction, since that is now the only way in. The three `consumption`
configurations build it against the installed headers, so a name that stops being reachable from the umbrella,
or an interface header that starts needing one from `detail/`, fails there rather than in a user's build.

It found two on the way in. The first was a name: `bit_traits.hpp`, interface under this rule at the time, was
not in `bits.hpp`, so it reached consumers only transitively, through the three adaptors and the three views
that all included it — the same thing the include order guards against, our headers before Boost's and the
standard's so that a transitive include is found rather than leaned on. That header is gone and
`contiguous_bit_sequence.hpp` took its place in `bits.hpp`. The second was worse, and no compiler leg could have
caught it: `CMakeLists.txt`'s `FILE_SET HEADERS` is hand-written, and the three inplace headers had reached
`include/` and `bits.hpp` without ever reaching it. `<xstd/bits.hpp>` therefore named three headers that were
never installed, so **every** installed consumer's umbrella include was broken, on every compiler, for as long
as the inplace column has existed. Every compiler leg builds from the source tree, where the files are present;
only the consumer translation unit builds against the install tree, and until it was made this gate it included
one container header and asked nothing of the umbrella. The file set is now checked against `include/xstd/`
at configure time, the way `test/` checks that every public header is mirrored — a hand-written list that
nothing verifies is a list that drifts.

The rule reaches `include/` as a whole, not only `include/xstd/`. `include/opt/set/sieve.hpp` — the sieve the
containers are benchmarked with — sat beside the library for as long as the library existed, never installed and
therefore invisible to a `find_package` consumer, yet on the build-interface include path of every
`add_subdirectory` and `FetchContent` consumer, to whom `xstd::sift_primes0` was a perfectly reachable name. It
lives in `examples/` now, in namespace `opt`, and the file-set check globs all of `include/` so a stray
directory is a configure error rather than a silent extra ([the-sieve](#the-sieve)).

`ext/` is the one interface piece the umbrella leaves out. It costs nothing here — a consumer that wants
`std::bitset` or `boost::dynamic_bitset` adapted includes the one header for it — and it keeps Boost off the
path of every consumer who does not.

### the-public-names

Three layers of names. The primaries carry the reading and take the storage: `set_adaptor<Bits, Own>`,
`sequence_adaptor<Bits, Own, Windowed>`, `bitset_adaptor<Bits>`, the parameters each reading needs and no
more. The `basic_` layer chooses the storage and leaves the block open,
`basic_string`-style: `basic_bit_static_set<Block, N>`, `basic_bit_set<Block, Allocator>` and their four
siblings. The block leads in every column, so a `basic_` name hands its vehicle the arguments in the order it
was given them -- `basic_bit_static_set<Block, N>` is `set_adaptor<contiguous_bit_array<Block, N>, owns>`,
straight through. The static and inplace columns used to take `<N, Block>` and transpose at the call, which
nothing gained: `Block` carries no default in those columns, so it is free to lead, and leading is what
`std::array<T, N>`, `std::inplace_vector<T, N>` and `std::span<T, Extent>` all do with the pair. The restricted
layer fixes `std::size_t` and `std::allocator`: `bit_static_set<N>`, `bit_array<N>` and `bitset<N>` keep one
parameter, and `bit_set`, `bit_vector` and `dynamic_bitset` keep none, so the flagship is `xstd::bit_set` and
the counterpart of `boost::dynamic_bitset<>` is `xstd::dynamic_bitset`, without the `<>`.

The unmarked name goes to the flagship — `bit_set` is the dynamic set
benchmarked against `std::set` and `std::flat_set` — and the qualifier marks the special case, `bit_static_set`.
The sequence row is named after the `std` container it packs, `bit_array` for `std::array<bool, N>`. The rows
therefore mark different columns, and that is correct by each row's own analogy rather than an inconsistency
to fix.

**Why the prefix leads.** `bit_` is a storage-strategy prefix, and the Standard already has the other one:
`std::flat_set` keeps a sorted sequence of the elements that are there, so it is sparse in the universe of
possible keys, where `bit_set` keeps one bit per position in that universe, so it is dense but packed. Same
container, same interface, different representation — and the prefix is what says which, so it has to come
first. `static_bit_set` would read as a qualified `bit_set`; `bit_static_set` is `bit_` applied to a
`static_set`, the way `flat_set` is `flat_` applied to a `set`.

**Why `static` and not `finite`.** `finite` selects nothing: `bit_set` is a finite set of
positions too, as every bit set is. What separates them is that `N` is a compile-time constant, which is
*static*, and static-versus-dynamic is one of the two axes the whole design is built on — so the name reads off
the design rather than off a true-but-non-distinguishing adjective. Recorded against it: P0843 renamed
`boost::static_vector` to `std::inplace_vector` partly because *static* is overloaded in C++. Accepted anyway,
because `inplace` names where the storage lives, which is the interesting property for a vector with static
capacity and a run-time size, where a `bit_static_set`'s extent is genuinely fixed. The inplace column takes
that name for exactly the case P0843 was naming ([the-inplace-column](#the-inplace-column)).

`bitset` and `dynamic_bitset` sit outside the rule on purpose: they are not `bit_` anything, they are the
counterparts reproduced under their own names ([a-strict-extension](#a-strict-extension)).

One header per restricted name, holding its `basic_` form beside it, each over one storage: `bit_set`,
`bit_vector` and `dynamic_bitset` over `contiguous_bit_vector<Block, Allocator>`, beside `bit_static_set`,
`bit_array` and `bitset` over `contiguous_bit_array<Block, N>`, and `bit_inplace_set`, `bit_inplace_vector` and
`inplace_bitset` over `contiguous_bit_inplace_vector<Block, N>` ([the-inplace-column](#the-inplace-column)). The
header is the name's home and the only place it is spelled; `bits.hpp` includes them all. Each static name has
an `aligned` form in the namespace of that name, in both layers, its width rounded up to whole blocks so that no
block carries an unused tail: `aligned::bitset<9>` is `bitset<64>` and `aligned::basic_bitset<std::uint8_t, 9>`
is `basic_bitset<std::uint8_t, 16>`. The inplace column has no `aligned` form, its `N` being a capacity the
storage already rounds up rather than a width to round.

### the-generated-table

Nine cells over three adaptors over three storages is the shape where an inconsistency hides in one cell and
nowhere else, so what the compiler generates is a table, held by `test/src/bits/generated.cpp` rather than by
whichever cell was read last.

Every cell answers the same to all but one column: default-constructible, copyable, movable, `==`, `<=>`,
`swap` as both a member and a free function, and **nothing-throwing** in both move directions -- a move that
could throw would cost every growing container its strong guarantee.

The allocator is the exception, and it follows the **column, not the row**: the dynamic column allocates and all
three of its readings answer `get_allocator`; the static column is a `std::array` and has none to show; the
inplace column holds its blocks inline and has none either. So `bit_set`, `bit_vector` and `dynamic_bitset` have
it and the other six do not, which is a fact about `contiguous_bit_vector` rather than about sets, sequences or
bitsets.

Two of those answers were the same fact arriving late. `set_adaptor` was the one owning adaptor without
`get_allocator`, so `bit_set` alone could not be asked for an allocator it demonstrably had; and
`bitset_adaptor` was the one without `swap`, which `std::bitset` also lacks but
`boost::dynamic_bitset` has -- and the two widths share one surface, an extension may add
([a-strict-extension](#a-strict-extension)), so it is there at both. Neither absence was a decision; both were
a cell nobody had read across.

`swap` is not merely `std::swappable`, which the implicit moves would satisfy on their own. It is the storage's
own exchange through `std::ranges::swap`, so the test checks that values actually move rather than only that
the expression compiles.

### the-inplace-column

The third storage point gets public names, one per reading and each an alias like every other name below the
adaptors: `basic_bit_inplace_set<Block, N>`, `basic_bit_inplace_vector<Block, N>` and
`basic_inplace_bitset<Block, N>` over `contiguous_bit_inplace_vector<Block, N>`, with `bit_inplace_set<N>`,
`bit_inplace_vector<N>` and `inplace_bitset<N>` at the machine word. `inplace_bitset` takes no `bit_` prefix
because `bitset` already carries the word, and `inplace` is one storage word down each column rather than a
second vocabulary for the same thing.

`N` is a **capacity** in bits here, where the static column's `N` is a width. The names carry that and the
parameter lists do not, which is the same hazard `basic_bit_static_set<Block, N>` and
`basic_bit_inplace_set<Block, N>` share by shape. The capacity is rounded up to whole blocks by
`contiguous_bit_inplace_vector` itself, so `basic_bit_inplace_vector<std::uint8_t, 9>` holds sixteen bits; the
width under it is a run-time one and carries an unused tail like any other.

The whole column sits behind `__cpp_lib_inplace_vector`, in practice libstdc++ >= 16, which the matrix carries on
gcc 16 and 17-SVN. An alias adds no capability, so the guard withholds a name rather than a feature, and each
header's includes sit inside the guard too: on a library without the storage the header is its include guard and
nothing else. Growth past the capacity throws `std::bad_alloc`, which is `std::inplace_vector`'s own answer
reaching the caller unchanged; on the set reading that is where `insert` stops being total.

P0843 declined to repeat `vector<bool>`, so there is no `std::inplace_vector<bool>` to check a packed sequence
against. `test::sequence::inplace_vector_bool` is `[vector.bool]`'s checklist minus the lines the allocator
reaches, and `std::vector<bool>` answers every line of it, so it is asserted on the model first exactly as
`vector_bool` is ([the-sequence-contract](#the-sequence-contract)).

**Building it at all is a separate problem from writing it.** `__cpp_lib_inplace_vector` is a C++26 macro, and
the library asks for C++23; at that standard the guard closes and the three cells compile to nothing, so a suite
that passes has never seen a third of the matrix. Measured: `bit_inplace_set`, `bit_inplace_vector` and
`inplace_bitset` run **six** test cases between them at C++23 and **sixteen** at C++26, and
`generated.cpp`'s inplace rows are inert at the lower standard as well.

`XSTD_BITS_CXX_STANDARD` is how a build asks for more -- 23 by default, 26 to reach the column. It raises the
standard for the tests and benchmarks only, deliberately: the `INTERFACE cxx_std_23` a consumer inherits is the
library's real requirement and must not move because one column wants more. Verified at GCC 16 with
`-std=gnu++26`, where all sixteen cases pass and the column behaves as the table says -- regular, swappable by
member and free function, no allocator, and the two readings differing exactly where they should, the bitset
starting at width zero and resizing while the set's width is its capacity ([width-is-capacity](#width-is-capacity)).

What is still missing is CI. **CMake 3.28 cannot spell C++26 for GCC or Clang at all** -- not a GCC 16 gap, a
CMake one -- and 3.28 is this project's declared minimum, so the option fails on the toolchain the matrix
currently runs. It fails *legibly*: the configure step asks `CMAKE_CXX_COMPILE_FEATURES` what CMake actually
knows and says so, rather than letting a `try_compile` blame the compiler for CMake's ignorance. Reaching the
column in CI needs a newer CMake on one leg, which is a change to the shared workflow rather than to this
repository.

`max_size()` is 24 on all three names over `<24, std::uint8_t>`, this column being where the three readings
first disagreed about it and the reason they no longer do ([max-size-is-the-bits](#max-size-is-the-bits)).
`capacity()` is the sequence and bitset readings'; the set has neither, `std::set` having no `capacity()` and a
set growing by `insert` ([growth](#growth)), so there a capacity is felt at the throw and reported by
`max_size()`.

The column is graded over every block the other two are, `xstd::uint128` included, which it can be because the
width member now carries its own alignment ([padding](#padding)). Before that, a 16-byte-aligned block after a
`std::size_t` width padded the class, and `-Wpadded` under `-Werror` rejected it; the inplace column was the only
cell that could reach it, a static width carrying no width member at all and `std::vector`'s alignment being a
pointer's whatever it holds.

Every leg without the storage compiles each of the three tests' `#else` arm, one case asserting the absence,
because a Boost.Test module whose test tree is empty is a setup error rather than a pass.

### max-size-is-the-bits

`[container.reqmts]/56` asks for `distance(begin(), end())` for the largest possible container, and under every
reading of bits that counts the same thing: **the positions there are to hold**. The set reading iterates the
positions it holds, so its largest is every position set; the sequence reading iterates one `bool` per position;
the bitset reading counts positions too. There is no separate key domain -- a set over `[0, W)` holds at most `W`
elements because there are `W` positions, which is the same `W`.

What the three do **not** share is where that count stops, because their counterparts stop in three different
places and each reading owes its own:

| reading | counterpart | where the count stops |
|---|---|---|
| set | `[set]`, which names no such bound | whole blocks the blocks hold and a `size_t` counts |
| bitset | `boost::dynamic_bitset` | that, saturated to `SIZE_MAX` where the product overflows |
| sequence | `std::vector<bool>` | that, clamped to whole blocks no wider than `PTRDIFF_MAX` |

So `max_size()` is not one function with three callers. **The storage computes all three** -- `max_size()`,
`saturating_max_size()`, `addressable_max_size()` -- because all three are made of `bits_per_block` and the block
container, which is knowledge only the storage has, and each reading returns the one its counterpart names. That
is what `contiguous_bit_container` is for: the primitives are its, the contracts are the readings'.

Both differences are observable, and neither is small:

| over a 64-bit block | ours | the counterpart |
|---|---|---|
| `dynamic_bitset::max_size()` | `SIZE_MAX` | `boost::dynamic_bitset`: `SIZE_MAX` |
| `bit_vector::max_size()` | `(PTRDIFF_MAX / 64) * 64` | libstdc++'s `std::vector<bool>`: the same |
| `bit_set::max_size()` | `SIZE_MAX - 63` | -- |

The middle row is the one the standard leaves open, and it is the one place a counterpart is a **specification**
rather than an implementation. `[container.reqmts]` only requires `max_size()` to bound what `resize` accepts, and
the two major implementations already disagree by sixty-three over a 64-bit word: libstdc++ answers
`(PTRDIFF_MAX / 64) * 64`, libc++ answers a bare `PTRDIFF_MAX`. There is no single number to match. Ours is the
rounded one on both, so `bit_vector` answers the same wherever it is built, which `std::vector<bool>` does not.

That is a **number** and not a behaviour, which is what makes it unlike the boost row above. Measured at every
boundary of the sixty-three-size window, on both libraries -- `n0 = (PTRDIFF_MAX / 64) * 64`, then `n0 + 1`,
`n0 + 32`, `PTRDIFF_MAX`:

| `resize(n)` | libstdc++ `vector<bool>` | libc++ `vector<bool>` | `bit_vector`, either |
|---|---|---|---|
| `n0` | `bad_alloc` | `bad_alloc` | `bad_alloc` |
| `n0 + 1`, `n0 + 32`, `PTRDIFF_MAX` | `length_error` | `length_error` | `length_error` |

Nothing diverges. And libc++'s number is one libc++ cannot honour: it reports `PTRDIFF_MAX` and then throws
`length_error` on `resize(PTRDIFF_MAX)`, its own `max_size()`, where `[container.reqmts]` makes that the bound
`resize` accepts. Ours is the bound: `resize(max_size())` reaches the allocator. Matching libc++'s number would
**create** the divergence that is not there now -- measured with the ceiling removed, which is what the bitset
reading over the same storage is, `resize(PTRDIFF_MAX)` is `bad_alloc` where libc++ answers `length_error`.

So the test asserts the table row for row, asking **both** containers rather than claiming a value of one, and
asserting of the counterpart only what every implementation of it promises:

| row | asserted of `bit_vector` | asserted of `std::vector<bool>` |
|---|---|---|
| `max_size() <= PTRDIFF_MAX`, and ours `<=` theirs | yes | yes |
| `max_size() % 64 == 0` | yes | no -- libc++'s is not |
| `resize(max_size() + 1)` -> `length_error` | yes | yes, against its own |
| `resize(PTRDIFF_MAX + 1)` -> `length_error` | yes | yes |
| `resize(max_size())` -> `bad_alloc` | yes, off the sanitizer legs | no -- the two libraries disagree |

The "ours `<=` theirs" row is the one that orders them, and it holds of any word width rather than of a measured
pair: a bound rounded down to whole 64-bit words is the **smallest** such bound any word width can produce, and
`bit_vector` is the name whose word is a `size_t`, so ours is never the larger claim.

The last row asks for the memory instead of refusing, so the allocator answers -- and under AddressSanitizer that
answer is an **abort**, not an exception. Measured: `allocation-size-too-big`, and `allocator_may_return_null=1`
only renames it to `out-of-memory`, because the throwing `operator new` calls `ReportOutOfMemory` on a null
return rather than throwing. So that row is guarded on the sanitizer and on nothing else, which is as narrow as
the evidence allows: of the nine CI failures that led here, every one was a sanitized build or a discarded
temporary whose `new`/`delete` pair clang elided at `-O2`, and the msvc and mingw legs -- neither of those --
never failed on it at all. The rows use a live object for the same reason: an elidable temporary is what broke
clang, not the allocation.

The bitset reading's is written as a **sum** rather than as boost's choice: `bits_per_block` is a power of two, so
`SIZE_MAX` is `max_width` plus one block's bits less one, and the clamped answer needs exactly that much back
wherever the clamp bit. Boost's `?:` would be a branch whose two arms belong to different allocators --
`std::allocator`'s ceiling always saturates -- so no one instantiation could take both, and
[per-instantiation-slots](#per-instantiation-slots) makes that an unwinnable branch rather than an untested one.

The **source** of the answer still differs by what can grow:

| | `max_size()` |
|---|---|
| a width in the type | the storage's `extent` |
| an owner over growing storage | the storage's answer for that reading, in bits |
| a view, a window, a static owner | its own width, which it cannot grow |

Nothing above the storage restates the arithmetic, and nothing above it should: a constant at the adaptor drifts
from the storage the moment the storage learns something, which is how `set_adaptor` came to answer `SIZE_MAX - 1`
while `dynamic_bitset` answered `SIZE_MAX - 63` over the same blocks.

That the set's is not `static` follows: an owner must ask its storage and a view must ask what it views, neither
of which a static member can reach. `std::set::max_size()` is not static either.

### the-sum-that-wraps

A ceiling only holds if the number reaching it is the number that was meant. Every growth here computes the
width it asks for by **addition** over a `size_t` the caller names, and every one of those additions wraps:

| where | the sum |
|---|---|
| `contiguous_bit_container::growing_insert` | `n + 1`, to admit the position |
| `set_adaptor::operator<<=` at a run-time width | `width + n`, the translation being total over `size_t` |
| `set_adaptor::insert_range`, consecutive tier | `lo + len - 1`, the range's last position |
| `sequence_adaptor::insert(position, n, value)` | `size() + n` |
| `sequence_adaptor::blit` | `size() + count`, the source's own width |
| `sequence_adaptor::pack` | `size() + ranges::size(rg)`, reserved ahead of the packing loop |
| `bitset_adaptor::guard_range` | `pos + len`, against `size()` |

Not every wrapping sum is a growth. `exclusive_find_next(n)` steps to `n + 1` to scan from the position after
`n`, and at the top of `size_t` that step is zero, so `find_next(npos)` scanned from the beginning and answered
the first set position. What refuses that sum is the guard that makes the scan total
([the-one-guard](#the-one-guard)), as the ranged forms' guard refuses `pos + len`.

A wrapped sum is **small**. It passes the ceiling it was meant to fail, and the operation then proceeds against
a width far below where it writes. `blocks_for` made that concrete: `align_up(n, bits_per_block)` rounds a width
near the top of `size_t` to **zero**, the floor turns that into one block, and the container answers `size()`
with `SIZE_MAX` over sixty-four bits. `bit_set().insert(SIZE_MAX - 1)` reached exactly that, and not only in
release -- the `assert(n < SIZE_MAX)` that stood in `growing_insert` refuses one position of the two that get
there, `n + 1` being a width nothing can hold for every `n` above `max_size()`. Under `-fsanitize=address` the
write that followed was a segfault at a high address; the ranged `set(pos, len, val)` was quieter, `pos + len`
wrapping below `size()` and the walk then running zero times, so an out-of-range request was answered by doing
nothing and saying nothing.

Two rules, and the second is what keeps the first from being written six times:

**The block count cannot wrap**, and that is a spelling rather than a guard. `blocks_for` says what
`boost::dynamic_bitset::calc_num_blocks` says -- divide, then round up by the remainder -- which is total over
every `size_t`. The spelling it replaced, `align_up(n, bits_per_block) / bits_per_block`, adds first, and for
the sixty-three widths above `max_width` that sum wraps to zero blocks which the floor then turns into one: a
container claiming `SIZE_MAX` positions in sixty-four bits. A ceiling was what stood between that spelling and
its own arithmetic. This one has nothing to stand between.

`blocks_for` is **public** for that reason, and the claim is a `static_assert`:
`blocks_for(SIZE_MAX) == max_num_blocks + 1`, one block more than the widest whole number of them, where the old
spelling answered one. Asserting it by *growing* to such a width instead asks `std::allocator` for 2^61 bytes,
and two of this tree's CI legs will not answer that question -- a sanitized build **aborts** on a request that
size (`allocation-size-too-big`) rather than reporting `bad_alloc`, and clang at `-O2` elides the `new`/`delete`
pair of a discarded temporary, so `(void)V(SIZE_MAX)` never allocates and never throws. Both were measured on
this tree, on exactly that assertion, across nine CI jobs. The compile-time form is the better one anyway: it
names the count the old spelling got wrong rather than inferring it from an exception, and it holds on every leg.

The **refusal** still wants looking at, and `std::inplace_vector` is where it can be: it is the one block
container here that refuses a width without asking anyone for memory, so the refusal arrives at a size a test can
name. `bad_alloc` and not `length_error` is what comes back, at the storage and at the bitset reading over it --
which is the whole of the difference from the two readings beside that one.

**The ceiling is therefore a policy, and it belongs to the reading**, because the counterparts disagree about it.
Two ceilings are computed in the storage and two refusals are spelled there -- `check_width` above `max_width`,
`check_addressable_width` above `max_addressable_width`, both `std::length_error` -- and the storage asks neither
at any door of its own. The **set** reading asks `check_width` in `guard_key` and in `operator<<=`: a key past the
widest it could ever grow to is the one thing `insert` on a dynamic extent can refuse. The **sequence** reading
asks `check_addressable_width` -- at its width constructors, `resize`, `reserve`, and each of the three sums it
computes -- because `std::vector<bool>` throws `length_error` for a size it cannot represent, and what it cannot
represent is a distance, not a `size_t`. The **bitset** reading asks neither, because `boost::dynamic_bitset` has
no such ceiling: the width reaches the allocator and the allocator answers `bad_alloc`. That was the one row where
`xstd::dynamic_bitset` differed from boost on an expression boost **defines**
([a-strict-extension](#a-strict-extension)), and moving the ceiling up is what closed it.

Three growths name no width of their own -- `clear()` resizes to zero, `pop_back()` to one less, and
`grow_to_admit` to another storage's own width -- and each is reached from something that promises not to throw
(the sequence reading declares `clear()` and `pop_back()` `noexcept`; `grow_to_admit` is how every set
operation across widths widens, and the test tree's composable checks are `noexcept` over `|`, `&`, `-` and
`^`). They kept `resize_to`, the growth behind the door, from when the door itself could throw
`length_error`; what they are safe from now is only `bad_alloc`, which no `noexcept` here promises against
anyway.

The mirror of that rule is that a growth which **does** name a width keeps the ceiling, and whatever promises not
to throw above it is what has to give. `growing_insert` is the case: a key past `max_size()` is `length_error`,
which is the one way `insert` on a dynamic extent can refuse ([asking-is-total](#asking-is-total)). The four
composable checks in the test tree were `noexcept` over `|`, `&`, `-` and `^`, and each builds its expected value
with `ranges::to`, which inserts -- so the `noexcept` was a promise about a reachable exception, and it went, as
`test/bitset/factory.hpp`'s did over `resize`. `includes()` beside them constructs nothing and keeps its. The narrower ceiling stays the blocks' own: `m_blocks.resize` and
`m_blocks.reserve` are handed a count and answer for it, which is why `resize(max_width)` is `bad_alloc` and
`resize(max_width + 1)` is `length_error`. `resize` takes that count **before** it writes the last block, so a
refused growth leaves the width and the bits exactly as they were.

That last sentence was not true of `resize(n, true)`, and the `inplace_vector` test above is what found it.
Growing with ones sets the tail above `size()` in the current last block, those bits being the first new ones --
and `resize_to` set them, and *then* asked the blocks to grow. A refused growth therefore returned with the width
unchanged and the tail dirty, which is the class invariant broken rather than a partial growth: measured on a
storage refused at 25, the next `resize(20)` came back with every bit above 9 set and `count()` at 8 instead of 1.
Which block and which bits is read off the **old** width, so it is taken before the growth; the write itself now
comes after, where nothing can throw between it and `m_size`.

**The addition is `width_sum`.** `base + count` where that is a width, and the top of `size_t` where it is not.
Saturating rather than throwing keeps the rule at one throw site: a saturated width is one `blocks_for` already
refuses, so the readings above hand it their sum and inherit the diagnostic without naming a ceiling of their
own -- which is the same reason `max_size()` is not restated above the storage.

Not every sum here was worth a check. `append(block_type)` raises the width by `bits_per_block` and `capacity()`
multiplies the blocks' by it; both overflow only once the blocks already hold some `2^64` bits, which the
allocation fails long before, so a branch there is one no test can reach and the coverage gate would be right to
say so. The reserve inside `append(first, last)` is left alone for a different reason: it is an optimization, and
a wrapped count there under-reserves, where the appends that follow still raise the width one checked step at a
time.

`sequence_adaptor::pack` reserves the same way and is **not** left alone, and the difference is what the count
is bounded by. `append(first, last)` is handed iterators over blocks that exist, so its distance is bounded by
what the caller already allocated and cannot reach the top of `size_t`. `pack` is handed a range of `bool`, and
a sized range answers `size()` for elements it never materializes: `d.append_range(views::iota(0UZ, SIZE_MAX))`
is one call, and `size() + SIZE_MAX` wraps for every `d` that is not empty. Wrapped, the reserve asks for
nothing and returns, and the packing loop then walks a range of `2^64` elements a word at a time -- so the
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

The split is exact, and it says which of the two rules above was ours to get wrong.

The **ceiling** was, and the fix in the end was to stop needing one.
`boost::dynamic_bitset::calc_num_blocks` is `n / bits_per_block + (n % bits_per_block != 0)` -- a division that
cannot overflow -- which is why `dynamic_bitset(SIZE_MAX)` reaches the allocator and answers `bad_alloc`, and
which `blocks_for` now says the same way. It rounded first, and `align_up(n, bits_per_block) / bits_per_block`
is exactly
libstdc++'s `_S_nword(n) = (n + word_bit - 1) / word_bit`, which its `vector<bool>` fill constructor calls with
no `max_size()` check: `std::vector<bool> v(SIZE_MAX)` there constructs, answers `size()` with `SIZE_MAX` over a
`capacity()` of **zero**, and aborts on the first write. `resize`, `reserve` and `insert` all check and throw
`length_error`; only the constructor does not. So the shape of our bug was `vector<bool>`'s, not boost's, and
the constructor was the unchecked way in on both sides.

The **guard** was not. `range_operation` carries two asserts, `pos + len <= m_num_bits` and, with a comment
naming the case, `pos + len >= len` for the overflow the first one cannot see. Ours carried only the first, so
boost caught in debug what we passed. Under `NDEBUG` neither library checks at all and both write out of bounds
for a plain `set(1000, 2, true)` at a width of 64. That is the contract, and it stays the contract
([the-one-guard](#the-one-guard)) -- but the subtraction now says both of boost's asserts in one expression,
where the addition said neither.

What the guards do **not** change is which contract each reading carries. `guard_range` still throws at a static
width and asserts at a run-time one, for the reason the single-position guard does
([the-one-guard](#the-one-guard)): the inconsistency is `std::bitset`'s and boost's, not ours. It is the
arithmetic inside the guard that was wrong, not the choice of guard -- said as a subtraction now, `pos <= size()`
and `len <= size() - pos`, which also gives the empty range at `pos == size()` the answer it should have.

### the-invariant-stays-in-the-storage

The padding invariant could live one level up, each adaptor restoring it after the operations that dirty it,
and it does not. Two reasons, one measured and one structural.

**Measured**, the prize is one masked store. `operator<<=` at a run-time width erases where the set reading
has already grown past anything the shift can reach ([the-set-operations-across-widths](#the-set-operations-across-widths)),
so the call is dead in that path. Removing it, GCC 14, `-O3 -march=native`, best of twenty-five:

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
blockwise reads -- `operator==`, `count`, `all`, `any`, `none`, `is_subset_of`, `intersects`,
`first_difference` and the three orderings are every one of them storage members that read a whole block and
would have to mask without it. Moving the restoration up would put the obligation in three adaptors and the
dependency in one storage, and a missed call would be silent. Three dirtying operations across three adaptors
is nine places to be right instead of four, to save a masked store on a one-block set.

That is also what the three standard libraries disagree about, and the disagreement is a real choice rather
than an oversight: a tail invariant is needed only where blockwise reads are **unmasked**. libstdc++'s
`vector<bool>` reads element-wise and keeps no invariant, so `flip()` dirties the whole allocation and nothing
notices. libc++'s reads blocks and masks at every read site. This tree reads blocks unmasked and masks at the
four writes that can dirty them.

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
equal sets share it, so `complement()` and `operator~` are constrained on `has_static_width` and
`bit_static_set<N>` keeps both. `bit_set` keeps `complement(x)`, which toggles one position and needs no
universe, along with `fill()` and the four set operators. The alternative -- letting `~` read the width --
would make the width value for exactly one operation, which is the thing this section says the set reading
does not do.

`contiguous_bit_container`'s `operator==` cannot say that: it is width first, which is what the sequence reading
and `dynamic_bitset` mean, and those two need it. The width is part of the value for both of them -- a
`vector<bool>` of two elements is not one of three, and a `dynamic_bitset` is equal only at equal size -- and it
is not part of the value for a set. So the set reading gets an entry of its own, `set_equal`, beside the
`operator==` the other two keep, for the same reason `set_lexicographical_compare_three_way` sits beside
`sequence_lexicographical_compare_three_way` and `string_lexicographical_compare_three_way`.

**Two spellings for two meanings, and it does not come out as evenly as the orderings.** Ordering has three
meanings and no structural answer at all -- a defaulted `<=>` would order by `m_size` first, which no reading
means -- so the storage declares none and names all three, and nothing is left over. Equality has two meanings
and one of them *is* the structural answer: memberwise, width then blocks, exactly what `= default` produces
and exactly what `std::regular` asks of a storage. `contiguous_bit_sequence` requires that
([the-common-vocabulary](#the-common-vocabulary)), and it is not an accident of the concept: `std::bitset` and
`boost::dynamic_bitset` both have `==` and both mean width first by it. So the count is one operator and one
name rather than three names, and giving the structural meaning a second name would be three spellings for two
meanings.

Both are non-members, and since the orderings became hidden friends `set_equal` is one too: a defaulted
`operator==` already was one, and equality asks about two values with neither as its subject exactly as an
ordering does. What remains asymmetric is only that one of the two meanings gets to keep the operator.

At a static width the distinction is unobservable, every instance carrying the one width, which is why the set
adaptor can default `==` there and nowhere else.

The **storage** answers at any two widths, and the adaptor calls it. `set_equal`, `set_lexicographical_compare_three_way`,
`is_subset_of`, `is_proper_subset_of` and `intersects` each carry their own width-crossing arm, so the four
read operations in `set_adaptor` are calls with no `same_width` test between them -- only the four compound
operators still ask, because they mutate. The logic belongs where the blocks and the invariant are, and putting
it there is also what keeps the three adaptors alike: `bitset_adaptor` had its own `top_aligned_three_way` and
`sequence_adaptor` had nothing at all. At two run-time widths that differ, `==`,
`is_subset_of` and `intersects` all ask whole **blocks** rather than walking positions. Only the blocks both
storages have can disagree; above them the answer is the invariant, capacity holding no element. So `==` wants
the shared blocks equal and the longer one's remainder clear, `is_subset_of` wants each of our shared blocks
inside the matching one and nothing of ours above their last block, and `intersects` is the negation of every
shared pair being disjoint -- their blocks above ours never need a look. Positions were the obvious spelling and
the wrong one, a walk over the elements being a `find_next` per position where this is one load per sixty-four.
Measured over two thousand elements held at two different run-time widths, each in the case that denies the
element walk its early exit, with the inputs made opaque to the optimizer so the call is not hoisted out of the
timing loop:

| | positions | blocks |
| --- | --- | --- |
| `==`, equal | 11.04us | 0.07us |
| `is_subset_of`, a subset | 9.39us | 0.05us |
| `intersects`, disjoint | 10.35us | 0.05us |

The padding above `size()` being zero is what lets a whole block stand in for the positions it holds, which is
the same invariant the orderings already rest on. The shared prefix needs no index arithmetic: `std::views::zip` stops at the
shorter range, which is exactly the blocks both storages have, and the blocks past it are a separate question
asked of one storage alone. Only the ordering needs a block one storage may not have, and `padded_block` reads
those as zero -- not a convention but the same invariant one block further out.

The same gate is why each of the three arms sits under `if constexpr (not has_static_width)` rather than the
plain `if` that `same_width` would fold anyway. Folding is not enough: a static width still *instantiates* the
arm, and `blocks_agree` carries a lambda no other call site shares, so those instantiations are reachable from
no test and their branches are uncovered by construction -- the same hazard the coverage job's own
`XSTD_BITS_BUILD_BENCHMARKS=OFF` exists to avoid. A translation unit naming only `bit_static_set` instantiated
thirty such functions before the guard and none after.

`<=>` is blockwise for the same reason and by a different route. The set ordering is lexicographic over the
ascending positions, and lexicographic order over two sets is decided by exactly **one** position: the lowest at
which they disagree. Whoever lacks it is less -- but for two different reasons, and the second is the one worth
naming. Usually it holds a larger element there. When it holds nothing above that position at all, its positions
are a proper *prefix* of the other's, and it is less because it runs out rather than because it compares
smaller. So the comparison is a search for the lowest differing block and a single look above it:
`padded_first_difference`, then `padded_any_above` on whichever side lacks the position. Those are the
storage's own `first_difference` and `any_above` with the index bound dropped, and `set_lexicographical_compare_three_way` dispatches to
them when the widths differ, so the generalisation lives beside the algorithm it generalises rather than in the
adaptor calling it. Measured as above, at two
different run-time widths: 10.62us to 0.09us over equal sets, and 11.03us to 0.06us where one set is a proper
prefix of the other.

`|=` `&=` `^=` `-=` are blockwise too, and
[the-set-operations-across-widths](#the-set-operations-across-widths) is what it took to mutate rather than
merely read. The shifts translate the set, so `<<=` grows the width to hold the result and `>>=` empties past
it. Hashing appends the positions and the count at a run-time width and the bits at a static one, where equal
sets share a width ([the-hashing-invariant](#the-hashing-invariant)).

### the-set-operations-across-widths

The four compound operators were the last element walks, and they were left for last because they mutate and
two of them may have to **grow** -- which is a question the four read-only operations never had to answer.

Measured at two run-time widths, 4096 against 4000, dense:

| | before | after |
| --- | --- | --- |
| `&=` | 12.12us | 0.22us |
| `\|=` | 9.41us | 0.22us |
| `^=` | 9.58us | 0.22us |
| `-=` | 10.27us | 0.22us |

**They keep the operator spelling**, and that is the point worth stating, because `set_equal` and
`set_lexicographical_compare_three_way` do not. A name is owed where the readings genuinely *disagree*: three orderings over one
storage, and two equalities ([two-readings-disagree](#two-readings-disagree)). `&=` `|=` `^=` `-=` are not
that. All three readings mean the same bitwise thing by them, and the only difference was that the storage's
operators stated a precondition of equal widths where the set reading wanted an answer. A precondition is not
a second meaning: widening the operator to answer where it used to assert takes nothing away from the readings
that never asked, since what they passed was always equal-width. So the operators themselves became total,
no new names, and `same_width` -- which existed only to choose between the storage's operator and an element
walk -- is gone.

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

**The width is the part that needed a way to see it.** An owning set reports `max_size()` as everything it could
grow to rather than what it currently spans ([max-size-is-the-bits](#max-size-is-the-bits)), so the growth rule
above is invisible from the owner. A *view* over the same storage reports the storage's own `size()`, which is
the width, and that is what the test asserts through. The first differential run over 577,600 width pairs
compared `max_size()` against `max_size()` and so proved only the elements; the width claims it appeared to
check were vacuous on both sides.


### an-opinionated-reimagining

The README states the charter: *a modern and opinionated reimagining of `std::bitset<N>`, keeping what time has
proven to be effective, and throwing out what is not.* [a-strict-extension](#a-strict-extension) is how the
keeping is enforced -- every expression of the counterpart's, with the same answer. This is the other half, and
the two are not in tension: the extension rule governs what the containers **do**, and it says nothing about
where an operator is declared.

`std::bitset` is the clearest case of what has not proven effective. It is the oldest type in the library and reads like it: a member
`operator==` where every container has a non-member one, no `swap` at all, and `operator<<` and `operator>>` as
members beside `&`, `|` and `^` as non-members. C++20 changed none of it. Mirroring that faithfully would
import an accident and call it conformance, so on where operators sit the tree follows the ordinary guidance
instead: the operand a mutator belongs to keeps its member -- `flip`, `<<=`, `&=` -- and the operators that
make a new value do not. `==`, `<=>` and `swap` are hidden friends; the shifts and `~` joined `&`, `|`, `^` and
`-` at namespace scope.

The rule is **hide where hiding is free, and never where it widens**, with `friend` doing two separable jobs.

Only `==` and `<=>` need friendship for what it says: `<=>` reads `m_bits` and calls the private
`top_aligned_three_way`, and a defaulted `==` may be a member or a friend and nothing else
([class.compare.default]/1). `swap`, the shifts and `~` need no access at all -- their bodies are
`x.swap(y)`, `nrv <<= pos`, `nrv.flip()`, every one of them public -- and the standard's own free `swap` is a
namespace-scope template for exactly that reason. They are friends anyway, and the reason differs between them.

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
operator qualified -- `xstd::operator<<(bs, 3)` is not a thing anyone writes by accident or otherwise -- so
hiding would foreclose nothing that was going to happen, and claiming a hazard there would be a
rationalisation. Guarding against Machiavelli is not this tree's business. So they are namespace-scope
templates beside `&`, `|`, `^` and `-`, which is also where `set_adaptor` has had its whole set all along.

**Which leaves three that deviate from the standard containers**, and only one of them by choice.
`std::vector` and `std::set` spell `==`, `<=>` and `swap` as namespace-scope templates; here all three are
hidden friends.

`==` is forced twice over, and the plainer reason comes first: **it is defaulted**, and
[class.compare.default]/1 admits a defaulted comparison only as a non-static member or a friend. A
namespace-scope template cannot be defaulted at all. The standard containers hand-write theirs, which is what
leaves them free to put it at namespace scope; wanting `= default` is what takes that option away here, and it
is a want worth having -- the storage is the one member, and a comparison nobody writes is a comparison nobody
gets wrong.

The second reason would force it even if the first did not. `std::bitset`'s converting constructor from
`unsigned long long` is not `explicit`, so `bs == 42ULL` and `42ULL == bs` both compile against it. A
namespace-scope template rejects both -- deduction does not consider user-defined conversions -- and rejecting
what the counterpart accepts is the one thing [a-strict-extension](#a-strict-extension) forbids. The standard
containers escape this too, none of them converting implicitly from anything. Measured on a stand-in with an
implicit `unsigned long long` constructor, both orders:

| | `x == 42ULL` | `42ULL == x` |
| --- | --- | --- |
| defaulted member | yes | yes |
| defaulted hidden friend | yes | yes |
| namespace-scope template | no | no |

So the strict-extension argument separates the **template** from the other two and not member from friend. The
member converts on the right directly and on the left through C++20's reversed candidate, exactly as the friend
does -- which is why `std::bitset`'s own member `==` became symmetric in C++20 without anyone touching it
([the-comparison-is-a-hidden-friend](#the-comparison-is-a-hidden-friend)). Between member and friend the choice
rests on the first reason and on the two paragraphs below: `<=>` needs the access regardless, and the two sibling
adaptors already spell both as friends.

`<=>` needs the access. It reads `m_bits` and calls the private `top_aligned_three_way`, so a namespace-scope
template would have to be granted friendship anyway, and then it is a friend that is not hidden -- the worst
of both.

`swap` is the one deviation that is a choice, and the Murphy guard above is the reason.

`&`, `|`, `^` and `-` stay namespace-scope templates, because there hiding is **not** free. Both their operands
are `bitset_adaptor`, so a hidden friend is reachable by ADL from either side and the other side could then
take the implicit `unsigned long long` conversion: `42ULL & bs` would start compiling, where `std::bitset`
rejects it and deduction on a namespace-scope template rejects it too. That is the Murphy case: a silent
conversion nobody asked for, in an expression that looks like integer arithmetic. A shift cannot do it, its
other operand being a `size_t` that brings no class to ADL. Widening what an operator accepts is a change to
what the containers **do**, and that is the extension rule's business, not this section's.

What this costs is named in [a-strict-extension](#a-strict-extension) and is not much: the address of an
operator, which nothing takes and which is a use worth discouraging in any case -- an operator is meant to be
found by the grammar, not by name. What it buys is one shape to learn instead of one per type.

### a-strict-extension

`bitset_adaptor<Bits>` is `[template.bitset]` over a storage of ours ([owning-is-ours](#owning-is-ours)), and
its rule is one sentence: **`xstd::bitset<N>` is a strict extension of `std::bitset<N>`, and
`xstd::dynamic_bitset` of `boost::dynamic_bitset<>`.** Every expression valid on the counterpart is valid on
ours with the same result, the same exceptions and the same `constexpr`-ness -- the proxy reference from
`operator[]`, the `to_string` signature, `to_ulong`'s `overflow_error`, the stream operators, `std::hash` --
and ours adds on top. Extension means nothing of the counterpart's is dropped, renamed or re-typed; strict
means it goes one way, code written against `std::bitset` compiling unchanged on `xstd::bitset` and not the
reverse. The harness's raw `std::bitset` and raw boost arms are the oracle for the inclusion, and the
elementwise readings are the oracle for what is added.

The surface is **not** identical to boost's, and it is worth having the difference written down rather than
implied. Measured one call per process, so that an assert's abort is observable, against Boost 1.83:

| `xstd::dynamic_bitset` against `boost::dynamic_bitset<>` | boost | here |
|---|---|---|
| `set(pos, len, val)` past the width | assert | `out_of_range` |
| `set(pos, val)`, `test(pos)` past the width | assert | assert |
| `at(pos)` past the width | `out_of_range` | `out_of_range` |
| `&=`, `\|=`, `is_subset_of` across unequal widths | assert | answers |
| `a < b` across unequal widths | answers | answers |
| the string constructor, `pos` past the string | assert | `out_of_range` |
| the string constructor, a character that is neither | assert | `invalid_argument` |
| `to_ulong()` with a position past the word | `overflow_error` | `overflow_error` |
| a width above `max_width` | `bad_alloc` | `bad_alloc` |
| `max_size()` | `SIZE_MAX` | `SIZE_MAX` |

The first of those two rows is measured rather than asserted in CI: at `std::allocator` the only way to reach it
is a 2^61-byte request, which a sanitized build aborts on and an optimizer may elide
([the-sum-that-wraps](#the-sum-that-wraps)). What CI asserts instead is the arithmetic it rests on, at compile
time, and the refusal itself over `std::inplace_vector`, whose blocks answer without allocating.

Every row where the two differ is a row where **boost asserts**, which is to say the expression is not one boost
defines -- and defining it, or throwing for it, is what an extension may do. That is now true of every row.

The last two were not, and are the reason this table exists. A width above `max_width` is valid on boost: it
reaches the allocator and answers `bad_alloc`, where this reading answered `std::length_error` -- a different
exception for an expression boost defines, which a program catching `bad_alloc` alone would not catch. The
window was sixty-three widths wide, exactly the ones `align_up` used to wrap on. It closed by taking the
ceiling out of the storage and giving it to the readings whose counterparts want it
([the-sum-that-wraps](#the-sum-that-wraps)): the sequence and set readings keep `length_error`, and this one
has no ceiling, as boost has none.

`max_size()` was the same gap said as a value rather than as a throw. Boost multiplies the blocks' limit by the
bits in one and answers `SIZE_MAX` where that product does not fit; this reading answered the storage's clamped
number, sixty-three positions lower, for an expression boost **defines** and a program may well compare against.
It closed by having the storage compute boost's answer too, and this reading return that one
([max-size-is-the-bits](#max-size-is-the-bits)).

Both rows are asserted against boost itself, row for row, rather than measured and written down -- boost being a
single implementation, every row has one answer and the test simply compares the two. The `max_size()` values,
and `resize(max_size() + 1)`, which is one past the top of a `size_t` and so wraps to zero and resizes both to
empty rather than refusing. The two rows that reach the allocator -- `resize(max_size())` and
`resize(PTRDIFF_MAX + 1)`, the latter being where the sequence reading beside this one answers `length_error` and
this one must not -- carry the same sanitizer guard as the sequence reading's, for the same measured reason.

The static column needed nothing. Measured the same way against `std::bitset<N>` -- the four position members,
the string constructor's three outcomes, both word conversions' `overflow_error`, the zero-width edge cases and
the saturating shifts -- `xstd::bitset<N>` answers identically in every case.

The rule governs **expressions**, and one thing it deliberately does not govern is **where an operator sits**.
`operator==` and the shifts are hidden friends where `std::bitset` makes all three members
([the-comparison-is-a-hidden-friend](#the-comparison-is-a-hidden-friend)). Every call is unchanged -- `a == b`,
`a << n` -- and what is not carried over is `&bitset<N>::operator==` and `&bitset<N>::operator<<`. Taking the
address of an operator is not a use to preserve; it names a function whose whole point is to be found by the
grammar rather than by name, and no code that should exist forms one. This is where the tree stops
transcribing and starts choosing ([an-opinionated-reimagining](#an-opinionated-reimagining)).

The vocabulary used to be a concept, `has_bitops` -- the compound operators including `-=`, the shifts, `set`
`reset` `flip` `all` `any` `none` `count` `size`, the three set predicates and regularity -- because a foreign
storage might arrive not speaking it. Only `contiguous_bit_container` can arrive now, and it speaks all of it by
construction, so the question was answering itself; the gate is nominal like the other two
([one-storage](#one-storage)) and each member is forwarded one line each.

The two widths share one surface. Boost's set vocabulary, `-=`, `-`, `is_subset_of`, `is_proper_subset_of`
and `intersects`, and its two searches, `find_first` and `find_next` answering `npos`, are there at a static
width as well: the storage spells them alike, and an extension may add. Only growth is gated on a run-time
width ([growth](#growth)): `empty`, `resize`, `clear`, `push_back`, `pop_back`, `append`, `reserve`,
`capacity` and `shrink_to_fit`, detected on the storage. The word conversions are the counterparts' own:
`to_ulong` and `to_ullong` throw `overflow_error` when a set position lies past the word, asked of the
storage's own forward step from the last position the word holds, and the word constructors take an
`unsigned long long` at both widths, boost's taking the width first.

Three additions are ours, with no counterpart on either side. `find_last()` and `find_prev(pos)` mirror
boost's forward pair: the highest set position below `pos`, `npos` where none, a `pos` past the width meaning
from the end, so `find_prev(npos)` is `find_last()`, and the two loops are each other's reverse. The forward
pair asks the same question in the other direction and therefore answers `npos` for such a `pos`: nothing is
set above a position past the width. An earlier draft of this paragraph had boost's `find_next(npos)` wrapping
round to `find_first()` instead, and that is not what boost does -- its body opens
`if (pos >= (sz-1) || sz == 0) return npos`, measured as well as read. Ours wrapped, which is what that
sentence was describing rather than boost, until the guard in [the-one-guard](#the-one-guard) made the forward
scan total. They are total, and the storage's reverse step is not, so
`find_prev` restores totality itself, in the two comparisons the width already affords
([the-cheapest-contract](#the-cheapest-contract)).
`operator<=>` is the bit string's order, boost's, at both widths ([the-ordering-invariant](#the-ordering-invariant)),
so `std::set<xstd::bitset<N>>` works and boost's `<` is no longer an omission. And boost's block interface is
in at both widths, every storage under the wrapper having blocks: `block_type`, `bits_per_block`, `num_blocks()`,
`to_block_range` writing every block including the clear tail, `from_block_range` reading at most every block
with the tail masked rather than trusted, and the block-range constructor at a run-time width, a whole number
of blocks wide. The rest of boost's surface is there too, at both widths where a width does not preclude it:
the throwing `at(pos)`, `test_set`, the ranged `set`/`reset`/`flip(pos, len)` behind the one guard on the whole
range and then a masked word at a time ([the-blit](#the-blit)); `max_size()` in bits, the width at a static one and
the widest whole number of blocks the blocks and the address space admit at a run-time one; and where the
storage takes an allocator, `allocator_type`, `get_allocator()` and the allocator arguments on the
constructors. The name `allocator_type` is an empty base a class either has or has not, a class having no
conditional typedef. One boost constructor is left out on purpose: `dynamic_bitset(str, pos, n, num_bits,
alloc)` puts a width where `std::bitset`'s `(str, pos, n, zero, one)` puts a character, and one signature
cannot extend both; `std::bitset`'s wins, the width being the characters read.

What ours does not add is a range, and there is no opt-in that adds one either. Becoming a range would change
what generic code does with it, from `fmt` to `std::ranges::to`, which is the one addition a strict extension
cannot make -- and `std::bitset` is already schizophrenic enough about its interface without our making it
worse. `bit_set_view` and `bit_span` refer into its storage ([views-over-owners](#views-over-owners)) and
carry the readings: bidirectional over positions, random access over bools, each said out loud at the call
site. That is the whole iteration story.

`ext/xstd/bitset.hpp` used to be a third answer -- twelve ADL `begin`/`end`/`rbegin`/`rend` overloads routing
to the set reading -- justified in its own comment as keeping iteration "reachable by name and only by name,
which is what a free function found by ADL is and what a member could never be". That reasoning was wrong:
`std::ranges::begin` is *defined* to find ADL `begin`, so a free function is not a weaker form of a member but
the primary one. Measured, including the header took `std::ranges::range<xstd::bitset<100>>` from `false` to
`true` and let `std::ranges::to<std::vector<size_t>>` swallow a bitset -- the named prohibition, by the named
mechanism. It is deleted. The asymmetry was the tell: `begin` is one name and there are two readings, so the
set reading held it by fiat and the sequence reading could never have had it, which is not a design.

Nor a `const_reference` proxy
from the const subscript, libc++'s way: `auto x = cb[i]` would change type and could dangle, and a strict
extension re-types nothing.

Extraction is `[bitset.operators]/6` at both widths: the characters read become `x = bitset_adaptor(str)`,
so a short read lands in the low positions, and a run-time width becomes the count of characters read,
as boost's does. The static width reads at most `size()` characters; the run-time width reads to the first
character that is neither `0` nor `1`.

### the-one-guard

Several members share a spelling with different contracts between the storage and the counterparts, and the
wrapper carries the one guard between them.

**Shift.** `contiguous_bit_container`'s `<<=` is unchecked, with `n < size()` as its precondition;
`std::bitset`'s and `boost::dynamic_bitset`'s are total and saturate to none. The wrapper guards the storage's
shift, `n < size()` else `reset()`, which is the guard `xstd::bitset` used to write by hand. At width zero even
`<<= 0` trips the storage's assert, so the guard is what makes that instantiation well-formed.

**Element access.** `set(pos)`, `reset(pos)`, `flip(pos)` and `test(pos)` are the storage's `assign` and
`test` behind a guard. The guard throws `out_of_range` at a static width, matching `std::bitset`, and asserts
at a run-time one, matching `boost::dynamic_bitset` -- a deliberate inconsistency between `xstd::bitset` and
`xstd::dynamic_bitset`, because it is exactly the one between their counterparts. The const subscript is
unchecked on every counterpart, so it is `test` unconditionally, and the proxy from the mutable one writes
through `assign` alone.

**The checked door at a run-time width is `at(pos)`**, and it is what makes that split a whole policy rather
than half of one. boost carries both halves: `set(pos)`, `reset(pos)`, `flip(pos)`, `test(pos)` and
`test_set(pos)` assert, and `at(pos)` throws `std::out_of_range`, in a mutable and a const overload. Ours has
both, and `at` throws at both widths — an extension over `std::bitset`, which has no `at` at all. Measured at
`-O1 -DNDEBUG -fsanitize=address`, Boost 1.83 and libstdc++ 14:

| `pos` past the width | `std::bitset<64>` | `boost::dynamic_bitset(64)` | `xstd::bitset<64>` | `xstd::dynamic_bitset(64)` |
|---|---|---|---|---|
| `set`, `reset`, `flip`, `test` | `out_of_range` | heap-buffer-overflow | `out_of_range` | heap-buffer-overflow |
| `test_set` | no such member | heap-buffer-overflow | `out_of_range` | heap-buffer-overflow |
| `at` | no such member | `out_of_range` | `out_of_range` | `out_of_range` |
| `operator[]` | undefined | undefined | undefined | undefined |

Every column answers as its own counterpart does, and where one counterpart has no such member the extension
answers as the other's does. That is the difference from the ranged family below: there the split left one
width unanswered and no counterpart supplied the missing half, where here both halves are boost's own.

**The scans are total**, a third answer again and boost's too. `find_next(pos)` returns `npos` for every `pos`
at or past the width — boost's own body opens `if (pos >= (sz-1) || sz == 0) return npos`. Ours did not, and
that was a defect rather than a policy. `exclusive_find_next` asserts `is_valid(n)` and steps to `n + 1`, so
`xstd::dynamic_bitset(64).find_next(1000)` was a clean heap-buffer-overflow under ASan, and `find_next(npos)`
was the worse half: `n + 1` wraps to zero, the scan restarts at the beginning, and the answer is the **first**
set position rather than none ([the-sum-that-wraps](#the-sum-that-wraps)). The guard belongs to the reading,
which is where the set reading's `upper_bound` already keeps the same one over the same primitive.
`find_prev(pos)` was total from the start, clamping a `pos` past the width to the width — and its tests had
asked for exactly the positions the forward pair's had never been asked for.

**The ranged forms** `set(pos, len, val)`, `reset(pos, len)` and `flip(pos, len)` carry the same guard over a
range, said as a subtraction rather than as `pos + len` ([the-sum-that-wraps](#the-sum-that-wraps)) -- and they
depart from the split above: they **throw at both widths**.

The split is right for element access because there are two counterparts to mirror, and each of ours answers as
its own does. The ranged family has only one: `std::bitset` has no `set(pos, len, val)` at all. So a static
width had nothing to follow here, and `xstd::bitset`'s throw was already ours to choose rather than
`std::bitset`'s to dictate -- which left one family checked at one width and not the other for no reason either
counterpart supplies. Half a policy is not one, and the half worth keeping is the one that answers.

It costs no compatibility, because the rule governs expressions *valid* on the counterpart
([a-strict-extension](#a-strict-extension)) and a range past the width is not one. Boost says so itself: its
`range_operation` opens with `BOOST_ASSERT(pos + len <= m_num_bits)`, and a second assert beside it,
`pos + len >= len`, for the overflow the first cannot see. Under `NDEBUG` both vanish and what is left is a
masked write through a block index the blocks never allocated -- `dynamic_bitset<>(64).set(1000, 2, true)` is a
clean heap-buffer-overflow under ASan on boost as it was here. Defining what a counterpart leaves undefined is
what an extension may add; it is the one direction that cannot break a program that was already correct.

What stays unchecked is what is unchecked on every counterpart and inside this tree: `operator[]`, and the
storage's own `set(n, len, value)` and `flip(n, len)`, whose precondition the assert states and whose one other
caller -- the set reading's consecutive `insert_range` tier -- establishes it by growing first
([width-is-capacity](#width-is-capacity)). The wrapper is the checked door; the primitive behind it is not, and
that is the division the rest of the tree already keeps.

The `checked_*` family the traits once carried, so that a wrapper over `std::bitset` could forward its native
throw, went with the foreign owners ([owning-is-ours](#owning-is-ours)): the branch is the wrapper's, and
there is one of it.

### asking-is-total

Asking is total whatever the extent: a position past the width is a key the set does not hold, which is an
answer and not a precondition violation. That is what `[set]` gives `contains` and `find` — `s.find(k)`
returns `end()` for any `k` it does not hold, never refuses the question — and it is the difference between
the set reading and the sequence reading, where `sequence_adaptor::operator[]` indexes and out of range is
out of bounds.

**Writing is not total**, and that is the whole of the asymmetry. `insert` carries no `noexcept`, for the reason
`std::set::insert` carries none: growing a dynamic extent allocates. It is the one operation a set can be unable
to satisfy — there is nowhere to put the key — and what the three storages say about that used to be three
different things, one of them nothing:

| | `insert(k)` past the width |
|---|---|
| a dynamic extent | grows to admit it; past `max_size()`, `std::length_error` ([the-sum-that-wraps](#the-sum-that-wraps)) |
| an inplace extent | grows within its capacity; past it the blocks say `std::bad_alloc` |
| a static extent | **had nothing to say**, and said it by writing through a block index the array does not have |

So the static one says `out_of_range` now, which is what its own neighbour `xstd::bitset<N>` says for a position
past `N`. The three differ because the reasons do — a domain, a capacity, a representable size — but none of
them is silence. It was undefined on the grounds of a performance benefit, and that grounds does not survive
measurement: on the sieve at `N = 2^16`, GCC 14 `-O3 -march=native`, best of twenty-five, 188.0µs unchecked
against 188.1µs checked, and 75.1µs against 75.1µs over 65536 inserts. The comparison is against a compile-time
constant and is never taken; it costs nothing to keep.

`complement(x)` is the same write and now answers the same way, having been the worse of the two: it asserted at
*every* extent, so a dynamic set — which grows for `insert(x)` — wrote past its blocks for `complement(x)` on a
key it would happily have admitted. A key past the width is absent, so the toggle that admits it **is** the
insert that admits it, and it grows where insert grows.

The element-wise `insert(first, last)` and `insert(ilist)` keep what they inserted before the refused key, which
is `[set]`'s own behaviour when an allocation throws midway; the consecutive `insert_range` tier guards the
range's last position before it writes anything, so that one is all or nothing.

Erasing stays total like `contains`: removing what is not there is the no-op returning zero that
`std::set::erase` is.

**The single position**, member by member, measured at `-O1 -DNDEBUG -fsanitize=address` against libstdc++ 14:

| a key past the width, or a step past an end | `std::set<size_t>` | here |
|---|---|---|
| `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range`, `erase(key)` | answers | answers |
| `insert`, `emplace`, `emplace_hint`, `insert(hint, x)`, `complement` | grows | `out_of_range` at a static width, grows at a dynamic one |
| `erase(end())` | undefined | `assert(position != end())`, which it already said |
| `erase(first, last)` reversed | aborts: a free of a pointer never allocated | `assert(*first <= *last)` |
| `++end()` | undefined | `assert(m_idx < size())` |
| `--begin()` | answers the same key again | `assert(find_first() < m_idx)` |

The first two rows are the policy above, and the four below it are preconditions, on both sides. What they were
here is worth keeping: `--begin()` at a two-block extent fell into the arm meant for the lower block and
answered the key it started from, so a reverse walk over it never ends; at four blocks and at a run-time width
it read past the blocks, a stack- and a heap-buffer-overflow under ASan. The reversed erase range did not end
either, where `std::set` corrupts the heap and aborts.

The backward step is the one worth asserting at the iterator, because it is **stronger** than anything the
storage checks. `exclusive_find_prev` asserts `any()` and `is_valid(n - 1)`, and `--begin()` satisfies both
while there is nothing below to find — the reverse scan's real precondition is that a set position exists below
this one, which is `find_first() < m_idx`. It is the same guard the bitset reading keeps over the same
primitive in `find_prev`, where the answer is `npos` rather than an assert, that reading's scans being total
([the-one-guard](#the-one-guard)).

The forward step's assert is the storage's own `is_valid` said one level up, and the difference is which
function a failure names. That is the rule the sequence reading keeps too
([indexing-is-a-precondition](#indexing-is-a-precondition)) — and the set reading's iterator needs nothing
beyond it: dereferencing `end()` here is the width rather than a read, so `*end()` is harmless where the
sequence reading's is a load.

### indexing-is-a-precondition

The third reading answers the same question a third way, and the answer is `[sequence.reqmts]`'s rather than
ours. Asking a set is total ([asking-is-total](#asking-is-total)); a bitset's element access throws or asserts
by extent ([the-one-guard](#the-one-guard)); a **sequence indexes**, and out of range is out of bounds.

`at(n)` is the one member that answers: `std::out_of_range` at every extent and through every handle -- the
static owner, the dynamic one, a view and a window alike, the window measuring `n` against its own size and
not the storage's. Everything else is a precondition, exactly as `std::vector`, `std::array` and `std::span`
have it, and `operator[]` beside `at()` is the pair the standard itself draws the line between.

A precondition is not licence to say nothing when it is violated, and at five members this reading said
nothing -- or said it a function away, which for a diagnostic is nearly the same thing. Measured under
`-O1 -DNDEBUG -fsanitize=address`, libstdc++ 14 without `_GLIBCXX_ASSERTIONS`:

| the precondition | `std::vector<bool>` | here, before | here, now |
|---|---|---|---|
| `front()` on an empty sequence | segfault | reads the block a floored count leaves and answers `false` | `assert(not empty())` |
| `back()` on an empty sequence | segfault | `offset() + size() - 1UZ` wraps: segfault | `assert(not empty())` |
| `erase(cend())` | erases the last element | erases the last element | `assert(position != cend())` |
| `insert(cend() + 3, v)` | inserts anyway: size 2, iterator at 4 | the same, to the number | `assert` in `index_of` |
| `erase(cbegin() + 3, cbegin() + 1)` | size **grows**, 4 to 6 | size grows, 4 to 6 | `assert(first <= last)` |

The counterpart column is why none of this is a contract change. Every row is undefined on `std::vector<bool>`
and undefined here, and an assert can only fire where the program was already undefined -- the one direction an
extension may take ([a-strict-extension](#a-strict-extension)). What it buys is the diagnostic, and the rule is
that the precondition is stated at the member the caller named. A debug build did catch four of these five, but
one call down: `front()` and `back()` on an empty sequence were the storage's own `is_valid(n)`, reached only
once the proxy they returned was read, and the two bad iterators were `first`'s and `subspan`'s
`count <= size()` from inside `rebuild`. A failure that names `subspan` for a bad argument to `erase` points at
the wrong function, which is worse than a blunt one; it is the same reason `operator[]` says `n < size()` where
`test(n)` would have said it again. The fifth, the reversed erase range, was caught nowhere: both of its
indices are inside the sequence, so nothing below it had anything to object to, and it grew a four-element
sequence to six in a debug build as readily as in a release one.

The iterator preconditions are said **once**, in `index_of`, which is the one place a caller's iterator becomes
an index and is reached by all five members that take one. Half of that precondition was already the iterator's
own: `operator-` and `operator<=>` assert `m_ptr == m_ptr`, so an iterator into another sequence never arrives
here. What is left is the range, `cbegin() <= position` and `position <= cend()`, and past either end the
subtraction below it is a `size_type` that wraps or an index the rebuild then writes through.

`erase(first, last)` adds the one thing neither iterator says on its own -- that they are in that order -- and
`erase(position)` the one `[sequence.reqmts]` asks of the single-position form, that the position is
dereferenceable and so not `cend()`. What holds them honest is the sweep that was already there:
`test/src/bits/bit_vector.cpp` runs `insert` in its value, fill, iterator-pair and initializer-list shapes,
`insert_range`, `emplace`, and `erase` in both of its own -- at positions including `cbegin()` and `cend()`,
and over empty ranges -- each against the `std::vector<bool>` that models it. Every one of those positions is a
valid one, and the asserts are now on underneath them.

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
named -- reaches both. `random_access_bit_iterator::operator*` says `m_idx < size()`, because this reading's
proxy reads and writes **through the storage**, so a position it hands out has to be one the storage has. The
set reading's iterator needs no such guard and has none: its proxy converts to `m_idx` itself, so there the
position *is* the value and `*end()` is the width rather than a read. Before, `*v.end()` and `v.begin()[100]`
both reported `contiguous_bit_container::test`'s `is_valid` — a private predicate of a detail type, two levels
below the expression that was wrong — where `std::vector<bool>` reported the first as `true` and the second as
a heap-buffer-overflow.

### unchecked-writes-in-views

Reads and writes inside a view go through the **subscript**, not through `test()`, `set(n)` or `reset(n)`.
The position is already in range by then, and those are the checked accessors whose throw would escape a
`noexcept` — which `bugprone-exception-escape` is right to report. Every type in the vocabulary hands out a
proxy that writes without checking.

### the-proxy-recursion-trap

The sequence proxy writes through the storage's `assign(n, value)` and never through a subscript. The earlier
view fell back on `c[n] = value` for a type without `set(n, value)`, and were such a type's `operator[]` to
return our own proxy, that proxy's assignment would land back in the fallback and **recurse until the stack
is gone**. A named member cannot loop back into the proxy, which is one more reason the write is a member the
storage spells rather than a probe over whatever answers — and `contiguous_bit_container` has no `operator[]`
at all ([test-not-subscript](#test-not-subscript)), so there is nothing for a fallback to find.

### the-iterator-is-the-primitive

`bidirectional_bit_iterator` and `random_access_bit_iterator` are a pointer and a position, and they reach the bits
through the storage alone. Their constructors are public, so an owner or a view builds one without being a
friend: the dependency runs one way, from the container to the iterator, and the mutual friendship and forward
declarations the earlier views needed (*"Clang requires it, GCC does not"*) have nothing left to declare.

The pointer is to the **storage** an owner wraps, never to the owner: `bit_static_set` hands out
`detail::bits::bidirectional_bit_iterator<contiguous_bit_array<B, N>>`, which is why an owner is never itself
the thing a view or an iterator is parameterized on.

**Where they live, and what they are called.** Both pairs are in `detail/`, one header each --
`detail/bidirectional.hpp` and `detail/random_access.hpp` -- because nobody spells these names: they are
reached through a container's `iterator` and `reference` typedefs and through nothing else. The test tree
mirrors that split rather than taking the exception `detail/` is granted: unnameable is not unobservable, and
what these types do -- the concepts they model, the round trip, the writes -- is the observable behaviour of
every container's `iterator`. So the contract is asserted through the containers, and these two sources assert
white-box what the containers cannot say precisely. The header is
therefore named after the iterator category rather than after the reading, which is what the two proxies
differ by; the reading names the container that hands them out, and the category names the iterator itself.
The `bit_` infix then says what is iterated, as `bit_` says what is stored in the container names
([the-public-names](#the-public-names)) -- and it is what keeps `bidirectional_bit_iterator` clear of
`std::bidirectional_iterator`, whose spelling the category alone would have taken.

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
`contiguous_block_range` requires precisely that
([contiguous-block-range](#contiguous-block-range)), while the **bits** are not addressable at all. The
asymmetry is the reason the vehicle keeps its blocks to itself and hands out proxies above it.

The free functions stay qualified as `detail::bits::shl<Block>(...)` inside `xstd::detail::bits` itself.
Dropping the qualification would read more naturally and reintroduce exactly the hazard the nesting exists to
close: an unqualified call with an explicit template argument performs ADL, and the associated namespace of
the type in play can be `std` or `boost` ([why-nested](#why-nested)).

### the-set-for-each

`for_each(f)` and `for_each_reverse(f)` on the set reading walk blocks where the iterator walks positions, and
the difference is not word-parallelism -- the functor still sees every set position, one at a time. It is
where the loop's state may live.

`operator++` is **flat**. It has to re-derive the word from `(pointer, position)` on every step, because an
iterator stays copyable and restartable and therefore cannot keep a partially consumed block between two
increments. A loop has somewhere to put one. So `find_next` loads a block, masks off what is at or below the
cursor, tests, possibly scans forward and takes a `countr_zero`, once per position; the walk loads once per
block and then spends two instructions per position, `tzcnt` for the position and `blsr` to drop it.

Measured against the range-for at 4.08x to 5.05x, on GCC 15 and clang 22, and the same on a two-word bitboard
carrying twenty pieces as at 2^22 with 40% density. That stability is itself the point: `find_next` is
inherently serial, no vectorizer engages on either side, and the answer does not move with the compiler --
unlike the sequence reading, where the same question gives answers ranging from 1.24x ahead to 11.4x ahead
depending on which one is asked ([the-sequence-ladder](#the-sequence-ladder)).

Three decisions the shape forced:

**A functor may return `bool` to mean "keep going".** A `void` one always continues. That is what a move
generator wants once it has found its answer, and it is one `if constexpr` on `is_invocable_r_v<bool, F&,
size_t>`. Nothing else is accepted: a functor returning something else is a caller error rather than a value
to discard quietly.

**The descending walk clears the bit it just reported.** `w & (w - 1)` drops the lowest set bit and has no
descending twin, so `for_each_reverse` takes `digits - 1 - countl_zero(w)` for the position and clears exactly
that bit. The set reading iterates both ways and so does this.

**A storage with no block access takes the iterator.** `boost::dynamic_bitset` is the one, and there the walk
falls back to the range-for it was written to beat, which is still correct and still the same answer. The
same three tiers `fill` uses ([windows](#windows)).

What is *not* here is a fat iterator carrying the residual word. It was measured -- 2.56x to 3.80x, against
the walk's 4.00x to 5.27x -- so it is both slower than the member and the only one of the two that changes a
contract: an iterator that caches a block stops observing an erase that lands ahead of it, where a closed loop
caching the same block is unobservable. The member is the faster half and the safer half at once, which is
rare enough to record.

### void-is-a-return-type

Every function names its return type after the parameter list, `void` included: `auto f() -> void`, with the
`-> void` on its own line above the body wherever the body has lines of its own. 118 functions were declared
`void f()` before this and 68 were already `auto f() -> void`; the split ran roughly along the library/test
line, which is not a reason, so they are all one shape now.

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
tree is built on -- `can_grow`, `word_writable`, `is_writable` and `blittable` all ask exactly that question.
Declared return types, trailing or leading, answer it from the declaration.

**Lambdas keep their trailing return inline.** A lambda is an expression inside a statement, so there is no
"above the body" to put anything on, and `modernize-use-trailing-return-type` requires the `-> void` there
anyway. The convention is about named functions.

### an-owner-reads-as-its-storage

A view over an owner is a view over the **storage** the owner wraps, and that is the only spelling there is.
`bit_set_view(bs)` over an `xstd::bitset<64>` deduces `bit_set_view<contiguous_bit_array<std::size_t, 64>>`,
and `bit_set_view<xstd::bitset<64>>` is not a spelling: `Bits` is constrained to a
`contiguous_bit_container`, and an owner is not one ([one-storage](#one-storage)).

**It was a spelling for a while, and the record of why it stopped is the point of this section.** A
`bit_traits<bitset_adaptor<Bits, Traits>>` specialization once relayed all twenty of the storage trait's
entries, each behind its own `requires`, so that a reader could name the bitset rather than the
`contiguous_bit_array` underneath it. The argument for it was readability: the deduced spelling names an
implementation detail, and `contiguous_bit_array` is not in the landscape tables.

Three things were wrong with it, and only the third was visible at the time.

- It made the plain-storage deduction guide viable for an owner too, so `bit_set_view(bs)` tied. The fix was
  to constrain that guide to non-owners, which is a line that is still there and still needed, an owner and
  its storage being two viable bindings either way.
- It relayed entries rather than forwarding a type, so a dropped one compiled and merely ran slower — without
  `num_blocks` and `block`, every word-parallel walk falls to one position at a time. The test had to assert
  the block tier explicitly to turn that into a failure.
- **It let the two readings mix.** Naming `bit_set_view<xstd::bitset<N>>` and `bit_span<xstd::bitset<N>>` is
  exactly what a bitset should allow; naming `bit_span<xstd::bit_static_set<N>>` is not, and a trait keyed on
  the *storage* cannot tell them apart, because they wrap the same storage. That rule had to move into the
  view's constraint anyway ([the-readings-do-not-mix](#the-readings-do-not-mix)), where the owner is still in
  hand.

With the rule in the constraint, the forwarder bought only the spelling, and the spelling could not survive
`Bits` being constrained to one storage. So it went, along with the 140 lines of it. A reader who wants to
name the type writes `decltype(xstd::bit_set_view(bs))`, which is what `test/consumer/main.cpp` does.

`xstd::bitset` still has no iterators and is still not a range; this is about how a view is *named*, not what
the bitset offers. [a-strict-extension](#a-strict-extension)

### what-a-view-costs

Measured on the same backend `contiguous_bit_container` in every row -- an owner, a view holding a pointer to
that storage, and a view over the `bitset_adaptor` wrapping it -- so the two layers price separately.
`benchmark/` builds at `-O3 -march=native`, which is what these numbers are; an earlier version of this section
said `-O2`, which was never true of any build in the tree.

| operation | bits | pointer costs | bitset-wrapper costs |
| :--- | ---: | ---: | ---: |
| set iterate | 256 | +7.1 … +9.6% | ±1% |
| set iterate | 1024 | +8.5 … +11.4% | ±1% |
| set iterate | 4096 | +11.1 … +12.8% | ±4% |
| set iterate | 16384 | +10.3% | ±2% |
| sequence count | 256 … 16384 | ±2% | ±3% |
| sequence read | 256 … 16384 | ±1% | ±2% |

Run-to-run noise was ±0.5 to ±3%, so a figure inside a couple of percent is a zero.

**The forwarding layer was free, and not merely inside the noise.** Under callgrind, at 1024 bits, the view
over a `bitset_adaptor` and the view over the raw `contiguous_bit_array` retired **6764 instructions per pass
each**, equal to the digit, with the same 422 data reads, 1265 branches and 17 simulated mispredicts: the
twenty forwarded entries inlined away completely. That forwarder is gone
([an-owner-reads-as-its-storage](#an-owner-reads-as-its-storage)) and the two rows below it are now one
measurement rather than two, which is why the table keeps them: the cost that remains is the pointer, and it
was never the layer.

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

The guidance narrows accordingly, and it is no longer a property of views in general: own the bits when you
iterate them hot *under GCC*; the abstraction itself is worth about three percent, and a view is still the
right answer when the bits are someone else's, where the alternative is not a container but no reading at
all.

**Two earlier conclusions in this section were wrong, both from measuring the wrong program.** It first
asserted that "the indirect load per block is not hoisted out of the iterator's per-step work", from no
disassembly at all. It then replaced that with the opposite -- that the pointer *is* hoisted and only the
prologue differs -- from an asm diff of small isolated functions written to stand in for the benchmark. Those
stand-ins are not the benchmark: GCC compiles them to **15,824,004 instructions for every variant**, identical
to the digit, because a subject that is a local of a tiny function has its address propagated the way clang
propagates it above. The stand-in had optimized away the thing under test, and "extra work in the loop" was
struck off the list on its evidence. The benchmark's own objects say the opposite.

The same defect sank two harnesses written while chasing this: both passed the subject to one shared timing
function as a `Subject const&`, which routes the **owner** through a pointer as well, makes every variant
indirect, and reports no gap. A subject has to be a local of the function that runs the loop, as the
benchmark's variants are, or the harness measures nothing.

What is left over is small and unexplained: 491 extra instructions where the per-step `lea` accounts for 410,
so about 80 sit in the block-advance path and the prologue, and `sequence count` at 4096 bits is bimodal --
+24.3%, -3.2% and +21.8% across three runs, with the bitset column swinging the opposite way each time to land
the third variant back on the owner's time. Only the middle variant moves, and between two stable values.
That was first attributed to code layout, before a byte-identical twin of the owner case measured layout at
about a percent here, so it stays recorded rather than explained. One run would have made it a finding.

Two things about the measurement itself, because both were wrong on the first attempt. **Every subject goes
through `DoNotOptimize` before the loop, owners included.** Without that an owner is a local the compiler
folds straight through: `count()` on one word measured 0.16 ns, half a cycle, which is not a faster reading
but no reading at all, while a view's pointer blocks the same folding -- so the comparison measured the
folding. And **the one-word rung was removed from the ladder**, which now runs four words to 256: a single `popcount`
is one cycle, so a one-cycle difference between two variants reads as +100% and means nothing.

### swap-goes-through-adl

`std::ranges::swap` reaches a type's own `swap` by **ADL on a free function**, and a member `swap` is not
found that way. When it finds none it falls back to a move-construct and two move-assignments, which is
correct and, for a storage whose moves are cheap, not obviously worse -- which is how this went unnoticed.

`set_adaptor`, `sequence_adaptor` and `bitset_adaptor` each ship a free `swap` beside the member.
`contiguous_bit_container` had only the member, and every one of the nine containers swaps by
`std::ranges::swap(m_bits, other.m_bits)` where `m_bits` **is** a `contiguous_bit_container`. So the member was
unreachable from the containers, and a storage with an optimized `swap` never saw it. Measured over a
storage whose swap and moves are counted, once per reading:

| | before | after |
| --- | --- | --- |
| `sequence_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |
| `set_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |
| `bitset_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |

Every type in the tree now carries the same pair: a **member** `swap` that does the exchange, and a **hidden
friend** `swap(x, y)` that forwards to it. One rule, no exceptions -- the storage included, though it is not a
container and no requirement asks it for either.

The member was briefly folded away on the storage, on the ground that it had no caller but the friend. That
measured something true and concluded the wrong thing: the friend calling the member *is* the design, so the
member is the primitive rather than dead weight, and deleting it bought one fewer function at the price of the
storage reading differently from the three adaptors. The adaptors keep the member because a container
requirement asks for it; the storage keeps it so the tree has one shape.

Hidden rather than at namespace scope, which is where the three adaptors' free `swap`s used to live, matching
`==` and `<=>` -- `ranges::swap` finds a hidden friend by ADL exactly as it found the namespace-scope template,
and `std::is_nothrow_swappable_v` is a trait over *unqualified* `swap`, so it finds one too. What it costs is
`xstd::swap(a, b)` spelled with the qualification, which nothing writes. Measured across all three adaptors,
each entry still reads 1 storage swap and 0 moves: through the member, through unqualified `swap`, and through
`ranges::swap` alike.

Nothing else about swapping changed, and the storage is not asked for one: `std::regular` implies `copyable`,
which implies `movable`, which **includes** `std::swappable`, so the concept already requires as much swapping
as `ranges::swap` can need, and any better one arrives by ADL without being asked for.

The `noexcept` moved with it. It read `noexcept(std::is_nothrow_swappable_v<Blocks>)` -- a trait of the
`std::swap` family -- above a body calling `std::ranges::swap`, which is a different family with different
rules (it suppresses the generic `std::swap` template when looking for an ADL candidate). They agree for
every storage shipped here, so this is a latent mismatch and not a bug, but it is the same shape as the
`is_invocable_r_v`-tests-a-prvalue-and-the-call-passes-an-lvalue disagreement in
[the-functor-takes-a-value](#the-functor-takes-a-value), and the same fix applies: the specification names
the calls the body makes, with the body's own arguments.

### a-requires-clause-names-its-arguments

A `requires` clause tests the call the body makes, with the body's own arguments. Twenty-odd of them tested
something else: a literal, almost always `0UZ`, stood in for the argument and the body then passed a
different type.

```cpp
requires requires { self.storage().growing_insert(0UZ); }    // is a size_t insertable?
{ self.insert(ilist.begin(), ilist.end()); }                 // ... a value_type is inserted
```

For every storage shipped here the two coincide, so nothing was ever caught. That is the danger, not the
defence: a `value_type` not constructible from `std::size_t` would be *admitted by a satisfied constraint*
and then fail inside the body, which is the hard error the constraint exists to prevent. A constraint that
cannot say no about the call actually made is decoration.

Where the function has the argument, the clause names it -- `*position`, `x`, `*first`, `*ilist.begin()`,
`value_type(std::forward<Args>(args)...)`, `static_cast<value_type>(*std::ranges::begin(rg))`,
`b.reserve(blocks_for(n))`. Where there is no call site to borrow from -- `can_grow`, `word_writable`,
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
[contiguous-block-range](#contiguous-block-range), whose subscript requirement exists because the
class subscripts and `std::ranges::contiguous_range` does not promise that.

### the-functor-takes-a-value

Both `for_each`es hand their functor a **prvalue** -- as `auto(x)`, C++23's decay-copy in the language
([P0849R8](https://wg21.link/P0849R8)) -- and both members say so with `requires std::invocable<F&, bool>` and
`requires std::invocable<F&, size_t>`. That is one fix for one defect, spelled in two places because it is
worth catching at the interface and worth being right in the body.

It was a three-line `decay_copy` helper per adaptor until the MSVC 17 rung left the matrix
([the-views-are-the-adaptors](#the-views-are-the-adaptors)): MSVC 2022 does not implement P0849R8, and the
other way round it -- `T{x}` -- reads to clang-tidy as a cast to the type it already has. That second
objection was only ever against the workaround; `auto(x)` is the paper's own spelling and clang-tidy has
nothing to say about it. Twenty lines went with the two helpers.

The defect was that `invoke_continues` named its parameter and passed that name along. A named parameter is an
lvalue, so a functor asking for `bool&` or `size_t&` bound to it, compiled, and wrote to a local that the walk
throws away. The walk reads its storage through a `const&` and never writes back, so what looked like a
mutating pass was a silent no-op -- and on the set reading it was worse than lost, since a walk *reports*
positions and changes none, so there is nothing a written-back position could have meant.

`auto` does not save this. `[](auto& b) { b = true; }` is an `auto` parameter and was exactly as silent as
`[](bool&)`; the axis is the value category, not the spelling of the type. Nor is "make them write `auto`"
enforceable: a generic lambda is distinguishable from a non-generic one -- `requires { &F::operator(); }` is
false for the generic one, its `operator()` being a template -- but that test also rejects a function pointer, a
hand-written functor and `std::function`, all of them legitimate callers, and it asks about the functor's shape
rather than about what it does with its argument.

What the prvalue fixes is a **disagreement the code already contained**: `is_invocable_r_v<bool, F&, bool>`, one
line above the call, tests with a prvalue, and the call used an lvalue. So the arm chosen and the call made
could differ, and for the reference-taking functor they did.

The two halves earn their place separately:

- **The constraint** puts the rejection at the interface, where it reads "constraint not satisfied" and names
  the member, instead of erupting somewhere inside `walk_words`. It is also what makes the rejection *testable*:
  a hard error in a template body is not something `static_assert(not ...)` can see, which is why the first
  attempt to probe this reported everything as fine.
- **The prvalue** is what the constraint cannot reach. `std::invocable<F&, bool>` is satisfied by a functor
  overloaded on the value category, so the constraint cannot say which overload runs; only the call can. The
  tests pin it with exactly that: a functor with `operator()(bool&&)` and `operator()(bool&)` that records which
  one it got, one per arm of the `if constexpr`, on both readings. Reverting either arm of either reading to an
  lvalue fails the suite.

Accepted, and unchanged: by value, by `auto`, by `const&`, by `auto const&`, a function pointer, and a functor
returning `bool` to mean "keep going". Writing *through* the sequence is the range-for's job and remains so --
`for (auto r : v) r = true;` writes, because the proxy is what an iterator dereferences to.

### the-sequence-aggregates

The sequence reading answers `count`, `all`, `any`, `none` and `mismatch` in its own vocabulary, each taking
the `bool` that [alg.count] and [alg.all.of] give them where the bitset reading's four take none. That is the
shape this row already has: `fill` takes a `bool` where the bitset reading splits `set()` and `reset()`.

They are not redundant with `bit_set_view(v).size()`, which already answers a word-parallel count over the
same storage. That view asks a *different question* -- reinterpret these bools as a set of positions and give
me its cardinality -- which happens to return the same integer for `value == true`, and has no spelling at all
for `count(false)`, `all(false)` and `none(false)`. Three readings over one storage is the whole design, and
[two-readings-disagree](#two-readings-disagree) exists to say they are not interchangeable; making a caller
change reading to count their bools is the fault the README levels at the two containers this library replaces.

**The `false` arms are identities, not second implementations**, and they hold on a window too:

```
count(false) == size() - count(true)      all(false)  == none(true)
any(false)   == not all(true)             none(false) == all(true)
```

Short-circuiting survives them: `all(false)` really does stop at the first set bit, because it *is*
`none(true)`. So there are four private helpers -- `count_true`, `any_true`, `all_true`, `none_true` -- and the
public members are spelled over those, each helper choosing its tier once: the storage's own member over the
whole, and a masked word at a time over a window ([windows](#windows)). `none_true` is a helper of its own
rather than `not any_true`, so the storage is asked in its own words; `contiguous_bit_container` spells
`count`, `all`, `any` and `none` itself, each at the block tier.

`mismatch` is `contiguous_bit_container::first_difference` plus one `countr_zero`. That helper existed already,
private and used only by `sequence_lexicographical_compare_three_way`; it is now public, and **keeps its
name**: it scans low block to high, which is the *ascending* orderings' answer, where
`string_lexicographical_compare_three_way` deliberately walks the other way and does not use it. Calling it
`mismatch` on the storage would repeat the mistake an unqualified `lexicographical_compare_three_way` made
([two-readings-disagree](#two-readings-disagree)). The counterpart name goes on the public member, which is the
owner's alone: a window's blocks are not its own.

Measured on GCC 15.2, `-O3 -march=native`, 20% density, best of fifteen:

| | 2^16 | 2^20 | 2^24 |
|---|---|---|---|
| `bit_vector::count()` | 0.1 µs | 2.0 µs | 49.5 µs |
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
| `bit_vector::count()` | 0.3 | 3.7 | 72.6 |
| `std::count` over `std::vector<bool>`, libstdc++ | 72.3 | 1181 | 19316 |
| `std::count` over `bit_vector` | 129 | 2122 | 33489 |

282x between the two `vector<bool>` rows on identical source, and a **tie** at the top: 2^24 bits is 2 MiB and
both run it at about 29 GiB/s, which is the memory and not the loop. So the honest claim is narrower than the
one the first table invites -- `count()` beats `std::count` over a `vector<bool>` **whose library does not
specialize it**, and ties one whose library does. What the member buys against libc++ is not speed but that
the word-parallel count is the spelling a caller reaches for rather than a library optimization they must hope
is present, over a storage that also answers the other two readings.

It also narrows the rule elsewhere in this file. A tail invariant is needed only where blockwise reads are
**unmasked**, which is weaker than needing one wherever reads are blockwise: libstdc++ escapes it by reading
element-wise, libc++ by masking at every read site, and this tree by masking at the four writes that can dirty
the padding ([padding](#padding)).

### the-sequence-for-each

`for_each(f)` on the sequence reading is the set reading's member ([the-set-for-each](#the-set-for-each))
transposed: an outer loop over words, an inner loop over the bits of one word, so the reload that
`operator++` must perform on every step becomes the inner loop's exit test. The functor takes what this
reading's iterator dereferences to, a `bool`, where the set reading's takes a position, and may return `void`,
or `bool` to mean "keep going", by the same one `if constexpr`.

The reason it has to be a member is the same and the payoff is **not**. Measured across GCC 15, GCC 16, clang
20 and clang 22, no iterator layout beats the current `(container pointer, index)` on this reading: a block
pointer plus offset ties, and a cached residual word is *worse*, because `operator++` is flat and the reload
test becomes a per-bit branch. So whatever `for_each` wins is loop structure, and loop structure is exactly
what a vectorizer needs:

| GCC 15.2, 2^24 bits | `for_each` | range-for | ratio |
|---|---|---|---|
| `n += b` | 1685 µs | 13842 µs | 8.2x |
| `h = h * 1000003 ^ b` | 18743 µs | 19154 µs | 1.02x |

**The win is the vectorizer's, not the loop's.** Where the body carries a loop-carried dependence there is
nothing to hoist and the two are parity; where it does not, the outer-loop-over-words shape lets GCC
vectorize what the flat `operator++` hides. And under clang 22 the range-for already vectorizes -- 2021 µs
against `for_each`'s 1900, 1.06x -- so on that compiler the member buys almost nothing on this reading.

That is a narrower claim than the one #127 was filed with, and it is the measured one. `for_each` is still
worth having: it is never slower, it is 8x ahead on the compiler and body shape where the range-for leaves
the most on the table, and it is the spelling that lets the library choose the loop rather than the caller.
There is no `for_each_reverse` here, because unlike the set reading's, nothing about this walk is asymmetric:
a reverse sequence walk is `std::ranges::reverse_view` over a random-access range, and it costs the same.

### read-only-set-proxy

The set reading's proxy is read-only whatever the qualification of `Bits`, because a key is nothing to write
through: assigning to a position would mean moving an element, which a set has no spelling for. It earns its
keep anyway — `operator&` round-trips to the iterator, and the conversion to any class a `size_t` converts to
lets `*it` initialize a strong index type in one step, where the two user-defined conversions of going through
`size_t` would be one too many. A type with an explicit constructor takes the `size_t` route, `index(*it)`,
and the proxy offers no explicit conversion of its own: MSVC cannot resolve one beside that constructor.

### the-proxy-copies-the-handle

Both proxies declare their copy constructor, and for opposite-looking reasons that are the same reason. The
set proxy is `= default` beside a deleted `operator=`, because a reference to a key is a value: copyable,
never assignable. The sequence proxy is `= default` beside two `operator=`s that write *through* the handle
to the bit. That second pairing is exactly the case `[-Wdeprecated-copy-with-user-provided-copy]` names: a
user-provided copy assignment makes the implicit copy constructor deprecated, because the compiler can no
longer assume the two agree -- and here they genuinely do not, which is the whole point of a proxy. So the
copy constructor is said out loud rather than inherited by default.

Nothing else copy-constructs a proxy in this library, which is why nothing caught it until `<format>` arrived:
`std::formatter`'s dispatch takes the element by value, and that first copy is where the deprecation lands.

### the-one-adl-exception

The sequence iterator's `iter_move` and `iter_swap` are hidden friends found by ADL, and they stay under the
no-ADL rule because they are `std::ranges`' own customization protocol: `ranges::sort` and `swap_ranges`
reach a proxy only through them, and it is where `vector<bool>` historically fell down. The three `swap`
overloads on the proxy are the pre-ranges spelling of the same thing, for `std::sort` and everything else
still built on `std::iter_swap`. `format_as` is fmt's protocol in the same sense.

The sequence proxy borrows nothing else from `[bitset.refs]`: no `flip()` and no `operator~`. Those belong to
the bitset reading, whose `reference` is its own class.

### formatting-the-proxies

`std::format` over the containers needs nothing said about the containers. Every owner and view here is a
range, so `[format.range.formatter]` would format each one already, except that it requires
`formattable<ranges::range_reference_t<R>>` and a reference of ours is a proxy. So each proxy specializes
`std::formatter` for itself, in its own header, and stops there: `bit_set`, `bit_static_set`, `bit_vector`,
`bit_array`, the views, the windows and the inplace column all follow from that, none of them mentioned.

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
one today -- so the specialization is still needed; what it no longer does is restate the value. They used to: the formatter
cast to `size_t` or `bool` through the conversion operator while `format_as` read the member. Both landed on
the same value, and nothing made them: a change to one would have left `fmt::format` and `std::format`
disagreeing about the same object, with no compile error and no test to catch it, since the proxy tests check
`format_as` and the format test checks `std::format` but nothing checked that they agree. `format_as` is now
the one place that says what a proxy prints as.

The readings then separate themselves. `[format.range.fmtkind]` picks `range_format::set` for a range with a
`key_type` and `range_format::sequence` otherwise, so the set reading prints `{1, 3, 5}` and the sequence
reading `[false, true, false, false]` -- the same split `format_as` arrives at for fmt, reached here through
the standard's own machinery rather than by our choosing
([two-readings-disagree](#two-readings-disagree)).

Each specialization derives from `std::formatter<size_t>` or `std::formatter<bool>` instead of writing a
`parse`, which is what keeps the whole spec: a width and a fill on a single proxy, and the nested spec a range
formatter forwards, so `{::#x}` over the set reading and `{::d}` over the sequence reading reach the
underlying formatter intact.

**Why this lives with the proxy and not in a header of its own.** It used to be `xstd/bits/format.hpp`, kept
outside the umbrella so that `<format>` stayed off the path of a consumer who does not format. That reason had
quietly expired: `sequence_adaptor` and `bitset_adaptor` both include `<format>` already, to build the
`file:line:column:` messages their exceptions carry, so two of the three readings were paying for it whatever
the umbrella did. Measured, the separate header saved the sequence reading 181 preprocessed lines and the
bitset reading 379 -- and cost a user the knowledge that the header exists. Only the set reading paid
anything real, about 11.8k lines, `set_adaptor` being the one adaptor that does not format its own errors.

Against that: hashing and formatting are both the customization points a user-defined type owes the standard
library, and `std::hash` has always been specialized inside the adaptor headers. Putting `std::formatter`
beside `format_as` in the proxy's own header makes the two protocols for one type live in one file, and makes
the two customization points consistent with each other. The standard library itself cannot do this -- its
`formatter<pair>` lives in `<format>`, because `<format>` is allowed to know about `<utility>` and `<utility>`
must not depend on `<format>` -- but the arrow only points that way for types the standard owns. For ours it
cannot: `<format>` will never know about a proxy of ours, so the proxy is the only place the specialization
can go without a separate header to remember. Issue #20 had this waiting on P3070R0, which is not what blocked
it: the proxy's formattability was, and that is ours to fix. The same issue records the other half of the
argument, which this library is the worked example of: a proxy nested inside its container cannot be named by
either hook, because the template arguments will not deduce through
`container<T, A>::proxy_reference`. Factoring the reference out into a class of its own is what makes both the
`format_as` overload and the `formatter` specialization writable at all -- which is what
`detail/bidirectional.hpp` and `detail/random_access.hpp` are.

### total-lookups-on-the-container

Every `bit_static_set` lookup is total over `key_type`, because `std::set`'s is: a key outside `[0, N)` names no
element, so it answers *absent* rather than reaching the bit. `contiguous_bit_container` asserts `is_valid` on
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
and its bench, which makes it a *subject* rather than library code. It sat under `include/` until it was noticed
that `include/` is the file set's `BASE_DIRS`, so CMake puts the whole directory on the build interface —
`#include <opt/set/sieve.hpp>` therefore compiled for an `add_subdirectory` or `FetchContent` consumer and
failed for a `find_package` one, the header never having been installed, and for the first kind
`xstd::sift_primes0` was a reachable `xstd::` name. Moving it out settles both: the names are in `opt::` now,
and the configure-time file-set check widened from `include/xstd/` to all of `include/` so the next stray
directory is caught rather than shipped ([the-interface-line](#the-interface-line)). It speaks **one**
vocabulary: an ordered set of integers. It runs over `std::set`, `std::flat_set`, `bit_static_set` or
`bit_set`; the candidates are `iota(2, n)` converted to the set, and a sift is `erase`. Nothing
bitset-shaped takes part.

There was a second sieve, `opt/bitset/sieve.hpp`, running the same shape over `std::bitset`,
`boost::dynamic_bitset` or ours through `bit_set_view`. It is gone. Two sieves were two vocabularies for one
algorithm, and the thing it was there to demonstrate -- that the view reconciles `set(pos)`/`reset(pos)`
with `insert`/`erase` -- is what `test/src/bits/bit_set_view.cpp` already asserts directly, over all three
bitsets, without a sieve in the way. An example earns its place by showing something no test does.
Bitset performance is measured against `std::bitset` and boost on its own bench instead, at the widths a
bitset is actually used at, rather than through an algorithm that suits a set.

The bench is dynamic containers only, so it compares like with like: `std::flat_set`, `std::set` and
`bit_set`, one of each representation -- sorted vector, node-based, dense bitmap. A `bit_static_set<N>`
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
run to re-derive a line visible four rungs earlier. Those three numbers were measured once, at `2^20`, before
the ceiling was put in; the bench no longer produces the first of them.

Under `ctest` a bench is a smoke test -- that it runs, not what it costs -- so the test invocation passes
`--benchmark_min_time=1x`. Timing the full ladder in CI would put minutes of `std::set` at `2^20` into every
run for a number a shared runner cannot make meaningful anyway. A manual run takes the defaults.

The test keeps the static types alongside the dynamic ones, `bit_static_set<N>` with `std::set` and
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
forever. That is the whole of its case, because it is **slower**, and now measurably: 11.3 ms against
`sift_primes1`'s 0.394 ms at `2^16`, about **29 times**. A map lookup per candidate is a large constant beside
a strided write, and O'Neill's own point is that the naive "sieve" that trial-divides is not the sieve at all.
It stops at `2^16` on the bench for the same reason `std::flat_set` does: the constant is the finding, and four
more rungs would only re-derive it.

The **segmented** sieve is the one that pays. Base primes below `sqrt(n)` once, then a single window walked
over the rest, so peak memory is `O(sqrt(n) + W)` whatever `n` is. `Window` is a template parameter carrying
its own extent, which makes `bit_static_set<W>` the natural argument: a compile-time width that allocates
nothing in the loop, `fill`ed and struck and read once per segment.

It is **faster than the bounded sieve, not merely thriftier**: 4.05 ms against 7.01 ms at `2^20`, about
**1.7 times**, and ahead at every rung. Asymptotically the two do the same work; the difference is that a
32768-bit window sits in L1 for its whole segment while a 2^20-bit sieve is a megabit walked with a stride.
Less memory and less time is not the trade the issue expected to be recording, which is why it is recorded.

Both are held to the bounded sieve by the test rather than described as equivalent -- every bound including
the degenerate ones, and two window widths, so a window shorter than its tail is exercised beside one that
swallows it. The bounds stop at `N` because one container under test is `N` bits wide, and a fixed width is a
capacity ([width-is-capacity](#width-is-capacity)).

`generate_candidates` is now total in `n`. `iota(2, n)` is a precondition violation below 2, and it was reached
by asking the sieve for the primes under nothing -- a question with an answer, none, rather than a contract to
break. The two unbounded sieves compute their own inner bounds, so a guard there is worth more than a comment.

### the-sequence-ladder

The three bit benches ask different questions and so run different ladders, which is the point rather than an
inconsistency.

Each bench is one pairing with **one variable**: `xstd::bitset<N>` against `std::bitset<N>`,
`xstd::dynamic_bitset` against `boost::dynamic_bitset<>`, `bit_vector` against `std::vector<bool>`. Same
reading, same storage, ours against theirs -- so a row measures the implementation and nothing else. Putting
`boost::dynamic_bitset` on the sequence row would compare a bitset against a sequence of `bool` and confound
the two.

`benchmark/src/bitset/ops.cpp` and `dynamic.cpp` are the **bitboard** question: a handful of words, ALU-bound,
1 to 512 words. `benchmark/src/sequence/access.cpp` is the **endgame-database** question: a dense flat array
indexed by a ranked position, where a lookup costs a cache miss and nothing else, so its ladder runs 8 KiB to
32 MiB and reports latency per random read rather than bytes per second.

What the three have found so far, on GCC 15.2, `-O3 -march=native`, x86-64:

- **Against `boost::dynamic_bitset` we are ahead**, on the operations where a block representation should tell:
  `count` by 2.6× at one word and 1.4× at 512, `scan` by 1.2× to 1.6× at every rung. The bitwise operators are
  parity, as they are against `std::bitset`.
- **Against `std::bitset` the scan is behind** (see [two-block-case](#two-block-case)). Both facts hold at once
  and neither is a contradiction: libstdc++'s `_Find_first`/`_Find_next` are better than ours, boost's
  `find_first`/`find_next` are worse. It is one measurement of our scan against two different implementations.
- **A random bit read costs the same in `bit_vector` as in `std::vector<bool>`** -- 3.0 ns in cache, about
  5.8 ns at 32 MiB for both, and construction is parity too. Representation does not matter to a lookup; only
  footprint does. For a database that is the useful negative result: what buys a lookup is fewer bits per
  position, not a better container.
- **`std::count` is slow over both, and the explanation this once carried was wrong.** It reads about 1.2× to
  1.3× slower over `bit_vector` than over `std::vector<bool>` on GCC, and this file used to say libstdc++
  specializes `std::count` for `std::vector<bool>::iterator` and counts a word at a time. It does not: a word-at-
  a-time count is two orders of magnitude ahead, not twenty percent, which is what `bit_vector::count()` now
  measures at ([the-sequence-aggregates](#the-sequence-aggregates)). The gap was the generic path over two
  different proxy iterators, and under clang it runs the other way, ours being the vectorizable one.
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

`detail::bits::intrin` used to forward `countl_zero`, `countr_zero` and `popcount` straight to `<bit>`, whose
domain is `std::unsigned_integral` — a **closed** concept no class can join. So the seam was constrained on an
open concept and implemented against a closed one: every 128-bit integer class satisfied the interface and
then failed inside the body. It now forwards to `xstd::countl_zero` and friends, which are that same domain
plus one overload per integer class, reading the words each type already holds. This is the change of body the
seam was left open for, and it is what makes the other three families usable.

Three consequences follow, none of them obvious from the forwarding change alone.

**The calls are qualified, so they bind where they are written.** `xstd::popcount(block)` is a dependent call
by a qualified name, and ADL does not apply to qualified names, so its candidates are the overloads visible at
`intrin.hpp` — not at the instantiation. An adapter included afterwards declares its overload too late to be
one. A translation unit reaching for an integer-class Block therefore includes that adapter first; the test
tree does it in `test/block_types.hpp`, above every container header. The same rule decides where
`test::block_basis` can be spelled, which is why it sits in that header rather than in one of its own: a
separate header could not be relied on to sort below the adapters.

**A Block being a class breaks two assumptions that a scalar hid.** `detail::bits::pred`'s `intersects`
returned `lhs & rhs` into a `bool`, which copy-initializes and so needs an **implicit** conversion; an integer
class offers only an explicit `operator bool`. Its two neighbours never needed the cast, `not` and `!=` both
reaching `bool` by a **contextual** conversion, which an explicit operator satisfies. And the sequence proxy in
`random_access.hpp` carried a templated implicit conversion to any class type constructible from its
`value_type`. An integer class is such a class, so every operator on that proxy acquired a second, equally good
reading — convert both sides to `bool`, or convert both sides to the Block — which cost it
`equality_comparable` and with it `std::ranges::equal`. The conversion now excludes `xstd::integer`: a proxy
stands for one bit, and a bit is not an integer. That alone is not enough, because a proxy names its Block
among its template arguments, so the Block's namespace is an **associated** one and ADL contributes whatever
templated comparisons it declares — Boost.Int128 declares exactly such a set. It therefore also declares
comparisons that are exact in both operands, which win outright.

The set proxy in `bidirectional.hpp` is deliberately **untouched**, and the attempt to keep it in step was a
mistake worth recording. Nothing had failed there: a set over an integer-class Block already worked, because
that proxy stands for a position rather than a bit and its `value_type` is `size_t`. Giving it the same exact
comparisons broke a case no integer class is involved in at all — `std::ranges::equal` over **two different**
instantiations of it, which is how `bit_set_view<B>` is compared against the view deduced from `B`'s own
storage, and which those homogeneous overloads no longer serve. A fix that no failure asked for cost a working
path, at a Block as ordinary as `uint64_t`.

**Two facts, two flags, because one flag conflated them.** `TEST_HAS_UINT128` names the compiler's 128-bit
**builtin**: a scalar, and a `std::unsigned_integral`. It feeds `word_types`, which every suite grades over, and
`test/src/bits/block/type_traits.cpp`, which asserts exactly those `std` traits of each word — both right to
assume a builtin. `TEST_HAS_MSVC_INT128` names what an MSVC-ABI target has instead, `std::_Unsigned128` under
the same `xstd::uint128` spelling: a usable Block, but a class, so not `is_integral`, not `is_unsigned`, and not
something `<bit>` will take.

Widening the one flag to cover both put a class-typed Block into `word_types`, and so into every suite at once,
which broke sixteen MSVC targets — the `std_bitset` and `std_set` comparisons among them, whose helpers assume a
Block is a `std` integral and whose per-type cost is superlinear. So the three integer classes sit together in
`wide_word_types`, feeding only the two suites that pay a `static_assert` or one linear pass per type. MSVC's is
named beside Abseil's and Boost's, which is what it behaves like, rather than beside the builtin whose spelling
it shares.

The assert beside each flag is an **implication**, that where the flag is on the basis is really there. The
equality it replaces asked `std::unsigned_integral`, which is the wrong question in both directions: false for
every integer class that works as a Block, and true in dialects where `<bit>` still declines the type. The
converse is not worth asserting either — a basis a flag declines to use costs coverage, not correctness.

Abseil's and Boost's are optional: `test/ext_int128.hpp` detects each by `__has_include`, so a build without
them drops it from the Block lists rather than failing. MSVC's needs no dependency at all. All three earn their
place by being the types that catch a container assuming a Block is a scalar — every defect above was invisible
to every builtin.

### exception-escape-nolints

Nine primitives in `test/include/test/bitset/primitives.hpp` carry `NOLINT(bugprone-exception-escape)` on a
`noexcept operator()`. Each guards on `pos < self.size()` and calls a member the standard specifies as
throwing outside that width — `set`, `reset`, `flip`, `test`, `at` — checking in the other arm that it does
throw.

The check reads the callee's signature and cannot read the guard, so it reports every instantiation: 104 of
them across the two `std_bitset` units. The `noexcept` is the claim being made — that a primitive answering
about a position inside the bitset never throws — and it is what would fail the suite loudly were the guard
ever wrong.

### clang-tidy-false-positives

Six findings are suppressed because the checker cannot see what makes them right:

- `bugprone-signed-bitwise` on `detail/bits::shl` and `::shr`, whose count is cast to `int`. The two checks
  that govern this leave no third option, and both were measured: `absl::uint128` declares a single shift,
  `operator<<(uint128, int)`, so an **unsigned** count reaches it by a signedness-changing conversion and
  `-Wsign-conversion` rejects it, while an **int** count is a signed operand of a bitwise operator and
  `bugprone-signed-bitwise` rejects that. The type's own operator decides which is right, and the count is a
  bit position within one block, so the signedness the check objects to cannot be reached.
- `bugprone-unhandled-self-assignment` on the bitset proxy's `operator=`, which owns no storage: `b[i] = b[i]`
  reads the bit and writes it back.
- `bugprone-string-constructor` on `to_string`, which sees the `N == 0` instantiation where the string is
  empty — which is what `bitset<0>::to_string()` returns.
- `misc-const-correctness` on a variable assigned inside an `if constexpr (N > 0)` that the `N == 0`
  instantiation discards; it sees only that one and asks for a `const` that would stop every other
  instantiation compiling.
- `misc-redundant-expression` on a reflexivity check, which cannot be written without naming the object twice.
- `bugprone-std-namespace-modification` on the two `std::formatter` specializations, which is precisely the
  modification `[namespace.std]/2` allows: a specialization of a standard library template for a
  program-defined type. clang-tidy 22 and 23 read the qualified definition as modifying the namespace; 24 no
  longer does, and the suppression stays until the whole ladder is past 23.

An eighth is the reverse case, and the one to be careful with: **the check is right about the language and
wrong about the compilers.** `readability-redundant-typename` on clang-tidy 22 asks for the `typename` to go
from `std::same_as<typename std::remove_const_t<Bits>::block_type, Block>` in `blit_source`'s partial
specialization. P0634 made `typename` optional only in listed contexts, and a **template argument is not one of
them** — GCC 14 rejects the elision outright, *type/value mismatch at argument 1*. An alias-declaration **is**
on the list, so the block type is named through one and the template argument is a simple-template-id that
needs no `typename` and trips no check. The same pattern already names `allocator_of` in
`contiguous_bit_container`'s test. Measured before pushing, because "clang-tidy suggested it" is not evidence
that it compiles.

A ninth had a fix rather than a suppression. `modernize-use-nullptr` reads the `0` in `(a <=> b) < 0` as a
null pointer constant, which is the same false positive `-Wno-zero-as-null-pointer-constant` already covers on
the compiler side. Every site in the test sources says `std::is_lt`, `std::is_gt` or `std::is_eq` instead --
the standard's own names for those three questions, which are clearer than the comparison against a literal
and leave the check on to catch a real one. Do not spell them back.

### clang-crashes-on-a-foreign-bulk-source

Asking whether a bulk operator accepts a view over a foreign storage -- `ours &= bit_span(a_std_bitset)`, in
a `requires`-expression or written out -- crashes clang 18 and clang 20 alike with an internal error, and it
did so before the windowed operators existed, so the trigger is the whole view's `&=` seeing a foreign
`sequence_adaptor` as its argument. gcc rejects the expression as it should. The expression is ill-formed
either way, since bulk on a window takes a source of the destination's own block type and the whole view's
takes its own type, so nothing in the library or the tests spells it; the crash is recorded here so nobody
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

It was a Block-width benchmark that surfaced this. `benchmark/src/set/blocks.cpp` instantiates the two-block
arm of the scan for three 128-bit carriers in one translation unit, a combination no single test unit makes;
the per-line branch totals at `contiguous_bit_container.hpp:410`, `:413` and `:454` went from 2, 6 and 2 to 4,
11 and 4, and the nine new branches were covered by nothing. The first attempt at a fix added a benchmark shape
that reaches the missing arm, and it changed the numbers by exactly zero -- which is the proof that the
benchmark never ran, and the reason the gate is now answered at the CMake level instead.

So `XSTD_BITS_BUILD_BENCHMARKS` gates the tree, the Coverage workflow passes it `OFF` through cpp-ci's
`cmake_args`, and the `benchmark/.*` exclusion stays as a second line that costs nothing. A benchmark measures
the library rather than being part of it, which was always the stated reason for excluding it; not compiling it
into the measurement is that reason carried through.
