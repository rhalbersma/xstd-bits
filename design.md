# Design notes

Why the code is shaped the way it is. The headers carry one line each; the reasoning lives here, and a
one-line comment ending in `[design.md#anchor]` points at the section that explains it.

Decisions still in flight are recorded on [#80](https://github.com/rhalbersma/xstd-bits/issues/80); this
file holds what has landed.

## Storage and containers

### block-storage

`block_storage` asks whether a range **is** blocks: a contiguous, sized range of unsigned integers.
`std::array` and `std::vector` both qualify, and so does `std::inplace_vector` — a runtime width over
static capacity, for free.

`xstd::ranges::block_range` is the other side of the same word, and asks whether a bit container will
**hand its blocks over**. Nothing models both, and no scope sees both unqualified.

### the-one-vehicle

`block_sequence<Blocks, N>` is the single storage vehicle. `N` is the width when that is a constant and
`std::dynamic_extent` when the width is carried at run time.

It owns the **unused-tail invariant** — every bit at or above `size()` reads zero — which is what makes
whole-block comparison, popcount and the block-at-a-time scans mean anything at all.

It has no iterators and no proxies. Those are readings, and a reading here would be picking one; it would
also make `std::ranges` see a sequence of blocks rather than of bits.

The hand-unrolled one- and two-block cases are where the static performance lives, so they stay
compile-time branches. The general path they fall through to is written over `m_blocks` as a range, and
therefore serves the dynamic width unchanged.

### padding

`static_used_bits` is the mask of the last block that is not padding. `num_bits` is `align_up(N)`, so
`num_bits - N` lies in `[0, bits_per_block)` and the shift is always in range.

Width zero is the one case that form cannot express — there is nothing to align up, so it reports no
padding where in truth the sole block is all of it — and it gets a selection instead.

**Naming zero rather than computing it matters on MSVC**, which constant-folds both arms of a `?:` and
answers C4293, *shift count too big*, on the arm it discards. `used_bits()` is the same two cases at a
run-time width.

### default-construction

A defaulted default constructor plus an NSDMI, rather than two constructors constrained on the extent:
`std::vector` default-constructs empty, and the at-least-one-block invariant has to hold from the start.

## Scans

### inclusive-is-the-primitive

Inclusive and exclusive name the only thing separating the two forward scans: whether `n` itself is a
candidate. That is a property of the scan rather than of a reading, which is why this is not called
`lower_bound` — that is the set reading's word, and this layer serves the sequence reading equally.
`bit_finite_set::lower_bound` is where the set name belongs, and it maps here one for one.

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

## Contracts

### total-versus-precondition

`block_sequence::exclusive_find_prev` is deliberately **not** total. Where `inclusive_find_next` answers
`size()` for "nothing at or above", this one has a precondition instead, and that is the whole of why it is
three instructions cheaper at every width: it never materializes a not-found value.

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

### the-ceiling-principle

The door's contract is the most efficient form, so `bit_traits<block_sequence<...>>` keeps the contracts
`block_sequence` gives it rather than widening to the total ones the synthesised walks happen to provide.

**A fallback synthesised for a foreign type may be more generous than the contract; it may not be less.**

So `find_next` requires `is_valid(n)`, and `find_prev` requires a set position strictly below `n` — which
`any()` does not establish, a container whose set positions all lie above `n` having none below it.

## The door

### opt-in

`bit_traits` is declared and never defined. Adaptation is opt-in rather than guessed, so a type nobody has
adapted is a compile error naming an incomplete type, rather than a silent fallback onto whatever members
happened to answer. That is the failure per-operation member probing walks into, and the door's reason for
existing.

`bit_storage` gates the adaptors on the floor rather than on `bit_traits<Bits>` being complete, which turns
*incomplete type* into *constraint not satisfied* — the error that names the real problem.

### detection-by-absence

`block_readable` is meaningful **only because nothing supplies a default**: a specialization that does not
spell `block()` genuinely has no block access, so absence is an answer rather than an oversight.

That is also why the walks are free functions and not a base class to inherit from. A base would satisfy
the concept for every type, and the tier choice would collapse silently — and the same applies to the
`requires` probes for `checked_test` and `checked_shift_left`, which read a missing entry as "this backend
has none".

### why-nested

The walks live in `xstd::detail::bits` rather than in `xstd`, and the nesting is load-bearing.

Since C++20 ([temp.names]/3, P0846) an unqualified `scan_first<Traits>(c)` parses its `<` as a template
argument list and then performs ADL **with the explicit template arguments included** — so `std` and
`boost`, the associated namespaces of the very types being adapted, would join the overload set.
`[namespace.std]` bars *users* from adding to `std` but not implementations, and boost is under no such
constraint.

Down here nothing is visible unqualified from `xstd`, so the qualification is enforced by **scoping** rather
than by remembering a prefix at every call site — which is what a class was previously substituting for.

### what-the-door-reconciles

Almost everything the two readings ask of a `Bits` is already an entry, or is the same operation under
another name:

| what a reading calls | door entry | |
|---|---|---|
| set `size()` (cardinality) | `count` | |
| set `max_size()` (width) | `size` | |
| `contains(n)` | `at(c, n)` | the same operation |
| set `erase(n)` | `assign(c, n, false)` | the same operation |
| set `clear()` | `fill(c, false)` | |
| sequence `size()` / `assign` | `size` / `assign` | |
| `bit_extent<Bits>` | `extent` | |

Two entries are left over, and they are the two the readings cannot synthesize:

- **`insert`** is the only operation that can *grow*, and it is exactly what the adapted types disagree
  about: `boost::dynamic_bitset` resizes, `std::bitset` cannot, `block_sequence` asserts. Reconciling that
  is what the door is for. It is not `assign(c, n, true)`, which has no answer for a position past the width.
- **`fill`** is bulk, and `clear` is `fill(false)`.

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
reachable the forward scans are native. Both return `N` when nothing is set, which is already the door's
total contract, so no `npos` mapping is needed — unlike boost, whose `find_first` answers `npos`.

### proxies-compare-themselves

Neither ordering needs a comparator. A proxy reference converts implicitly to its `value_type`, and that
conversion is what satisfies `three_way_comparable` — checked for both this library's `sequence_reference`
and `std::vector<bool>::reference`, on which `lexicographical_compare_three_way` works with no comparator
at all. An explicit `[](bool a, bool b) { return int(a) <=> int(b); }` says nothing the conversion has not
already said.

### block-writes

`set_block` is the write side of `block()`, and deliberately not a trait entry: block writes are only ever
needed on storage we control, and the unused tail is `block_sequence`'s to keep.

## Orderings

### two-readings-disagree

"Lexicographic" is underspecified below the reading layer. Both readings are lexicographic; they order over
different sequences, and **they disagree**:

| `{0,1}` against `{1}` | compared as | result |
|---|---|---|
| set reading | ascending positions, `[0,1]` against `[1]` | `{0,1} < {1}` |
| sequence reading | bools from index 0, `[1,1,0…]` against `[0,1,0…]` | `{0,1} > {1}` |

A door serving three readings cannot hold one of their orderings without choosing for its callers, so it
holds neither under that name. It holds **both, separately named**: `bit_traits` has a `set_three_way` entry
and a `sequence_three_way` entry, never one `lexicographical_three_way`, so a caller says which reading it
means rather than being handed whichever the door happened to pick.

### the-ordering-primitive

Both orderings are answered a word at a time, from two pieces:

- **`first_difference`** returns the lowest block at which two values differ, together with that block's
  `xor`. `countr_zero` of that `xor` is then the lowest position at which they differ.
- **`any_above`** asks whether one value holds anything strictly above a given position. The value's bit
  *at* that position is clear -- it is the one that lacked the differing bit -- so `block >> offset` leaves
  exactly what it holds above, and the shift is always defined because `offset < digits`. No two-step shift,
  and no guard against shifting by the width.

From there the two readings differ by one clause and nothing else:

| reading | at the lowest differing position |
|---|---|
| set | whoever HOLDS it is greater, **unless** the other holds nothing above it |
| sequence | whoever HOLDS it is greater, full stop -- the widths are equal, so there is no prefix case |

**Why this beats iterating.** The `lexicographical_compare_three_way` form walks *set bits*; this walks
*words*, and a word step is an `xor` and a test rather than a load, a shift, a `countr_zero` and a branch.
Equal values are the clearest case: iteration confirms every element, so a full 1024-bit set costs 1024
bit-scans against 16 word `xor`s. Near-equal and dense values are the same story. Only an early difference
makes the two comparable, both exiting at once.

**It is one sweep, not two passes.** `first_difference` covers blocks `[0, i]`; `any_above` then covers
`[i, n)` on **one** operand, the one that lacked the bit. Block `i` is the only one touched twice, so the
pair costs `n + 1` block reads. And when the values are equal `any_above` is never reached at all, since
`first_difference` already settles it.

**The prefix clause is not removable.** Set order is not plain lexicographic over words under *any*
comparator. At `digits = 4`, `A = {1}` and `B = {5}` differ in word 0, where `A₀ = {1}` and `B₀ = {}`; a
prefix rule over words says `B < A`, but the sets truly compare `A < B`. `any_above` is exactly the repair,
and is the whole of what separates the two readings.

**The fallback is the specification.** A `Bits` that will not show its words -- `std::bitset` under libc++,
`boost::dynamic_bitset` -- falls back to that standard algorithm over the reading's own iterators, which is
[the invariant](#the-ordering-invariant) itself. So the door's contract stays the efficient form and the
fallback can only be more generous, never less ([the ceiling principle](#the-ceiling-principle)), and the
test is that the two paths agree.

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

### the-ordering-invariant

Every ordering in the library satisfies

```cpp
std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end()) == (a <=> b)
```

which is stated on the **reading**, never on the storage: `block_sequence` has no `begin()`/`end()`, and
`boost::dynamic_bitset` has no public iterators, so it cannot be written against a backend at all.

`basic_bitset` therefore has no `operator<=>`, because it does not iterate — the existing "no iteration and
no `<=>` here by design" is a consequence rather than a separate rule.

`boost::dynamic_bitset::operator<` is a **third** reading, not an unreachable one. It pairs *a*'s highest bit
with *b*'s highest, second with second, then breaks a tie on width — which is that same standard algorithm
over **reverse** iterators. Verified over 1,046,529 pairs spanning every width 0-9 against every other: zero
mismatches against reverse-lex, 223,893 against numeric order. It is *not* magnitude ordering, though it
coincides with one at equal width, which is the only case our operators admit: `"1"` and `"01"` are both the
number 1, and boost orders them strictly.

So it stays out of `operator<=>` because that is defined by *forward* iteration, not because nothing could
reach it — and should a dynamic bitset ever want boost's exact ordering, it costs no new algorithm, being
`sequence_three_way` over `rbegin`/`rend`.

Where a specialization offers nothing faster, the default is that standard algorithm over the reading's own
iterators — so the default cannot disagree with the specification, and only an optimization can.

## Coverage

### per-instantiation-slots

gcovr counts branch slots **per template instantiation and never merges them**. A loop that a one-block
instantiation can never enter is a branch never taken, whatever every other instantiation does.

So a degenerate width does not get an unreachable loop; it gets **different code**, via `if constexpr` on
`static_num_blocks == 1` and `== 2` in `block_sequence`, and on `static_block_count` and `extent == 0` in
the walks. `static_block_count` is derived from `extent` rather than declared for exactly this reason:
nothing new has to be supplied to know it, since `block_readable`'s contract already fixes the layout.

The same gate is why a cursor lives inside the walk it belongs to rather than beside the block index: at one
block the walk is discarded, and a cursor declared outside would never be written — which
`misc-const-correctness` reads, correctly, as a variable that should have been `const`.

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

`block_sequence::test` rather than `operator[]`: this reads and cannot be written through. `std::bitset`'s
`operator[]` returns an assignable proxy and this returns `bool`, so the subscript spelling would promise an
assignment that does not compile.

The writable proxy belongs to the containers above, which is also where the checked reading lives —
`std::bitset::test` throws where this asserts, a difference the door states as `unchecked_test` rather than
one this name should try to carry.

### qualifier-prefixes

Contract qualifiers are prefixes, not suffixes — `inclusive_find_next`, `exclusive_find_prev`,
`unchecked_test`, `checked_shift_left` — so that the contract reads before the operation and the cheaper
form cannot be called by mistake.

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

### asking-is-total

Asking is total whatever the extent: a position past the width is a key the set does not hold, which is an
answer and not a precondition violation. That is what `[set]` gives `contains` and `find` — `s.find(k)`
returns `end()` for any `k` it does not hold, never refuses the question — and it is the difference between
the set reading and the sequence reading, where `sequence_view::operator[]` indexes and out of range is out
of bounds.

`insert` carries no `noexcept`, for the reason `std::set::insert` carries none: growing a dynamic extent
allocates. It is the one operation a set can be unable to satisfy, and only a **static** extent ever is — a
fixed capacity cannot come to hold a position outside it, so that is the precondition violation. A dynamic
extent grows to hold it, `[set]` giving `insert` no way to fail. Growing has a limit of its own: `n + 1` must
be a width the container can address, and `dynamic_bitset::max_size()` being `SIZE_MAX`, the one position
ruled out is the one whose successor wraps to zero.

Erasing stays total like `contains`: removing what is not there is the no-op returning zero that
`std::set::erase` is.

### unchecked-writes-in-views

Reads and writes inside a view go through the **subscript**, not through `test()`, `set(n)` or `reset(n)`.
The position is already in range by then, and those are the checked accessors whose throw would escape a
`noexcept` — which `bugprone-exception-escape` is right to report. Every type in the vocabulary hands out a
proxy that writes without checking.

### the-proxy-recursion-trap

`sequence_view` refuses the `operator[]` fallback for a type without `set(n, value)`. Were such a type's
`operator[]` to return our own proxy, that proxy's assignment would land back in the fallback and **recurse
until the stack is gone**.

Where `set(n, value)` does exist the type is a concrete bitset, and its subscript is the unchecked way in —
which is the one to take, the position being a precondition asserted just below, where `std::bitset::set` and
`xstd::bitset::set` would check it again and throw out of a `noexcept`.

### total-lookups-on-the-container

Every `bit_finite_set` lookup is total over `key_type`, because `std::set`'s is: a key outside `[0, N)` names
no element, so it answers *absent* rather than reaching the bit. `block_sequence` asserts `is_valid` on every
position it accepts and offers no total spelling of any of these — that is the layering working, not a gap in
it. **The precondition is the sequence's; the guard is the container's.**

One of those guards stops a *write* rather than a read: without it an out-of-range key clears a bit in
whatever follows the blocks.

Two findings from running the suite against the unguarded header are worth keeping, because they are why it
sweeps every width rather than one convenient one. **No single width exposed all six operations and no single
key did either** — at N = 8 over `uint8_t` every one of them was clean for `key == N`, so a narrow
single-block set proves nothing on its own. And `erase` was an out-of-bounds *write*, while `upper_bound`
failed on its returned value rather than on memory at all: `find_next`'s `++n` wrapped before it could test
the bound, so it answered with a real element where `end()` was due. That second one is why this stays a gate
on the jobs that build without sanitizers.

## Platform and tooling, continued

### uint128-support

`xstd::uint128` names a type on every compiler the matrix runs, but the library can only carry it where
`<bit>` will: `detail::bits::intrin` forwards `countl_zero`, `countr_zero` and `popcount` straight through,
and those take `std::unsigned_integral` alone. That is three separate facts.

GCC and Clang have the built-in. libstdc++ and libc++ hand it the `numeric_limits` specialization that carries
it into the concept **only outside `__STRICT_ANSI__`**, which is why the matrix compiles as `gnu++23`. And the
Microsoft STL's `std::_Unsigned128` is a class type, so `<bit>` declines it whatever the mode — that block
waits on an `xstd::countl_zero`, not on anything here.

The condition worth testing is the concept, which no `#if` can spell, so the assert holds the macro to it in
both directions. The day that seam grows its own implementation, or a new pairing lands on the matrix, the
build says so there rather than at fifteen instantiation lists or, worse, nowhere.

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

Four findings are suppressed because the checker cannot see what makes them right:

- `bugprone-unhandled-self-assignment` on the bitset proxy's `operator=`, which owns no storage: `b[i] = b[i]`
  reads the bit and writes it back.
- `bugprone-string-constructor` on `to_string`, which sees the `N == 0` instantiation where the string is
  empty — which is what `bitset<0>::to_string()` returns.
- `misc-const-correctness` on a variable assigned inside an `if constexpr (N > 0)` that the `N == 0`
  instantiation discards; it sees only that one and asks for a `const` that would stop every other
  instantiation compiling.
- `misc-redundant-expression` on a reflexivity check, which cannot be written without naming the object twice.
