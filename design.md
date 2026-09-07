# Design notes

Why the code is shaped the way it is. The headers carry one line each; the reasoning lives here, and a
one-line comment ending in `[design.md#anchor]` points at the section that explains it.

Decisions still in flight are recorded on [#80](https://github.com/rhalbersma/xstd-bits/issues/80); this
file holds what has landed.

## Storage and containers

### block-storage

`block_storage` asks whether a range **is** blocks: a regular, contiguous, sized range of unsigned integers.
Regular is what lets `block_sequence` default its `==` over the width and the blocks, in that member order, so
two run-time widths part on the width before a block is read.
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

`block_inplace_vector<Block, N>` is the third storage: a run-time width under a compile-time capacity of `N`
bits, behind `__cpp_lib_inplace_vector` until every library in the matrix has it. It needs nothing of its
own, `std::inplace_vector` satisfying `block_storage` as it is; `resize`, `reserve` and `push_back` past the
capacity throw `std::bad_alloc`, as that library specifies.

The adaptors take growth by detection on the storage, never through the trait: growth is a container's
business and no view's, so it exists on an owner and on nothing else. `basic_bit_sequence` is
`std::vector<bool>` where its storage grows -- the count and count-value constructors, the range and
`initializer_list` constructors and assignments, `assign`, `resize`, `clear`, `push_back`, `pop_back`,
`emplace_back`, with `reserve`, `capacity` and `shrink_to_fit` where the blocks have them -- and
`std::array<bool, N>` where it does not, each member requiring the storage member it forwards to.
`basic_bitset` at a run-time width takes `boost::dynamic_bitset`'s growth the same way. `basic_bit_set`
takes none by name: a set grows by `insert`, and the trait's `insert` grows a run-time width to hold the
key ([asking-is-total](#asking-is-total)).

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

### the-cheapest-contract

The door's contract is the most efficient form, so `bit_traits<block_sequence<...>>` keeps the contracts
`block_sequence` gives it rather than widening to the total ones the synthesised walks happen to provide.

**A fallback synthesised for a foreign type may be more generous than the contract; it may not be less.**

So `find_next` requires `is_valid(n)`, and `find_prev` requires a set position strictly below `n` — which
`any()` does not establish, a container whose set positions all lie above `n` having none below it.

## The trait

The prose below calls `bit_traits<Bits>` *the door*: the one thing the readings ask, and the one thing a
storage answers. The three entries every specialization must have — `extent`, `size`, `at` — are *the floor*.
The headers say `bit_traits`, `Traits` and "the required entries" and point here for the reasoning.

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

### the-trait-is-a-parameter

`bit_storage` and `static_bit_extent` take the trait first and the storage second, as `block_readable`
already did, so that a type-constraint can name the trait: `bit_storage<Bits> Traits` expands to
`bit_storage<Traits, Bits>`, a type-constraint binding its own parameter first. That is what lets every
consumer of the door carry `Traits = bit_traits<Bits>` as an explicit parameter, `basic_string`-style, and
what turns the tier into a knob over identical storage — one `block_array`, two traits, one variable.

### what-the-trait-reconciles

Almost everything the two readings ask of a `Bits` is already an entry, or is the same operation under
another name:

| what a reading calls | door entry | |
|---|---|---|
| set `size()` (cardinality) | `count` | |
| set `max_size()` (width) | `size` | |
| `contains(n)` | `at(c, n)` | the same operation |
| set `erase(n)` | `unchecked_assign(c, n, false)` | the same operation |
| set `clear()` | `fill(c, false)` | |
| sequence `size()` / `operator[]` | `size` / `unchecked_assign` | |
| a view's static extent | `extent` | |

Two entries are left over, and they are the two the readings cannot synthesize:

- **`insert`** is the only operation that can *grow*, and it is exactly what the adapted types disagree
  about: `boost::dynamic_bitset` resizes, `std::bitset` cannot, `block_sequence` asserts. Reconciling that
  is what the door is for. It is not `unchecked_assign(c, n, true)`, which has no answer for a position past the
  width.
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
fallback can only be more generous, never less ([the cheapest contract](#the-cheapest-contract)), and the
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

The four use-site dispatchers in `detail::bits` (`find_first`, `find_next`, `find_prev`, `count`) carry the
width-zero arm too, ahead of the choice between the door's entry and the walk: at width zero every answer is
zero -- the total answer, `size()` -- and no entry or walk is instantiated for it. The set iterator's
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

### the-hashing-invariant

Every value the library compares, it hashes, and

```cpp
a == b  implies  hash(a) == hash(b)
```

under every reading. The counterpart rule ([the-idempotent-wrapper](#the-idempotent-wrapper)) governs a
wrapper's member surface, not the cross-cutting protocols -- equality, ordering, formatting, ranges, hashing
-- which follow the reading: the standard's own coverage, `std::bitset`, `std::vector<bool>` and
`std::string` hashing while `std::array`, `std::set` and `std::pair` do not, is history rather than design.

The engine is Boost.Hash2: each adaptor carries a `tag_invoke` hook for `hash_append`, and `std::hash` is
one detail helper over it, `fnv1a_64` folded by `get_integral_result`, so the algorithm is chosen in exactly
one place and a caller wanting another brings it through `hash_append`. What a hook appends is the value
**through the door**, never a storage's own hook: the blocks and the width where the trait reads by block,
every position and the width otherwise. So equal values hash equal whatever holds them, and a wrapper over
`std::bitset` hashes on every library whether or not `_Getword` is reachable. The set reading at a run-time
width appends the positions held and their count instead, since equal sets need not share a width
([width-is-capacity](#width-is-capacity)).

Who hashes follows [views-follow-their-precedent](#views-follow-their-precedent): the set adaptor owned or
viewed, as `std::string_view` hashes; the sequence adaptor as an owner alone, as `std::span` does not, so its
hook is constrained on ownership and a `sequence_view` hashes no more than it compares; `basic_bitset` as
`std::bitset` does. Both range adaptors tell ContainerHash they are not ranges: Hash2 chooses between its
range overload and a hook by `enable_if`, a range with a hook is ambiguous, and the range overload could not
hash the proxy the iterators return anyway. The harness checks the invariant beside `==`, wherever a
`std::hash` exists.

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

### the-three-adaptors

Three class templates carry the three readings: `basic_bit_set`, `basic_bit_sequence`, `basic_bitset`. Each
is written against the door and never against a storage, so one adaptor serves `block_array`, `block_vector`,
`std::bitset` and `boost::dynamic_bitset` alike, and no owning type ever needs a `bit_traits` of its own. The
public names are aliases: `bit_static_set<N, B>` is `basic_bit_set<block_array<B, N>, owns>`,
`bit_array<N, B>` is `basic_bit_sequence<block_array<B, N>, owns, false>`, and `bitset<N, B>` is
`basic_bitset<block_array<B, N>>`.

### ownership-is-not-an-axis

Owning versus viewing is storage lifetime, not a third axis of the model, and it collapses to one template
parameter: `ownership::owns` stores `Bits`, `ownership::refers` stores `Bits*`. Always present and only its
type changes, so a plain `conditional_t` rather than `conditional_data_member_t`. One accessor, via deducing
`this`, gives deep const to the owner — `self.m_bits` propagates `self`'s const — and shallow const to the
view — `*self.m_bits` does not — for free.

Every mutator is then gated on the door and nothing else: `requires requires { Traits::op(self.storage(), …) }`
reads "the door lets *this handle* write". A const owner's accessor hands the door a `Bits const&`, which no
`unchecked_assign` accepts; a const view's hands it `Bits&`, which is what a view is for; a view over
`Bits const` hands it `Bits const&` again. Const, ownership and a floor-only trait are all the same question, asked once.
The exceptions are the constructors and, once storage grows, the growth members, which need an explicit
`requires (owns(Own))`: the requires-expression tests what the storage can do, not what this handle may do
to it, and a view over a `block_vector` must not be able to resize what it does not own.

### views-follow-their-precedent

`bit_set_view` follows `std::string_view`: a value that happens not to own its bytes, so it has `==` and
`<=>`, and its ordering is exactly `std::set`'s. `bit_span` follows `std::span`, which P1085 stripped of both
because "same referent" and "same contents" are both defensible readings of a handle. So `basic_bit_set`
compares and hashes whatever it owns or views, and `basic_bit_sequence` compares and hashes only as an owner. The non-member copies
— `~`, `&`, `|`, `^`, `-`, `<<`, `>>` — are the owner's alone in both readings: a copied view would write
through to what it views.

Both referring adaptors opt into `std::ranges::enable_view` and `enable_borrowed_range`, the two
specializations [range.view] and [range.range] invite for a program-defined type. The first makes
`set_view(x) | views::take_while(…)` take the view as it is rather than wrapping it in an `owning_view`;
the second says what `span` says, that the iterators point at the storage and outlive the handle that made
them, which is what lets `ext/xstd/bitset.hpp` return `basic_bit_set(c).begin()` from a temporary.

### the-views-are-the-adaptors

`set_view<Bits, Traits>` is `basic_bit_set<Bits, ownership::refers, Traits>` and `sequence_view<Bits, Traits>`
is `basic_bit_sequence<Bits, ownership::refers, false, Traits>`, each a two-line derived class inheriting the
adaptor's constructors and restating its two deduction guides: not a second implementation of either reading.
The earlier views, with their own iterators, proxies and four customization points — `set_find`,
`sequence_find`, `block_access`, `bit_extent` — were the door before there was a door, and once the adaptors
read through `bit_traits` alone there was nothing left for them to do. An alias would have been the natural
spelling, and deduction through one is class template argument deduction for alias templates (P1814), which
Clang 19 and GCC 10 have and MSVC does not: `set_view(x)` on MSVC is "too few template arguments". The derived
class is the escape #80 named, and it costs a restated constructor, guide and `enable_view` per view. The
constructors are spelled out rather than inherited: inheriting them inherits the primary's guides as well
(P2582, which GCC implements), and those tie with the restated ones.

The sequence view pays the `span` half of [views-follow-their-precedent](#views-follow-their-precedent) by
becoming the adaptor: it no longer has `==` or `<=>`, and the harness checks the sequence reading through the
iterators instead.

### views-over-owners

An owner has no door of its own — `bit_static_set`, `bit_array` and `bitset` are thin wrappers over a
`block_array` that already has one — so a view over an owner is a view over the storage it wraps:
`set_view(xstd::bitset<64>&)` is `basic_bit_set<block_array<size_t, 64>, refers>`, and the pointer in the
iterator is to the `block_array`, never to the `bitset`. The owner hands its storage over through
`owned_storage<Owner>`, declared beside it as `bit_traits` is beside a storage and never defined for anything
else, so `owner_of<Owner, Bits, Traits>` reads "this owner wraps exactly the storage and door this view
refers through". Const flows one way: a const owner gives a view over `Bits const`, a mutable owner either.

The view's converting constructor takes the owner's private member directly, which is why each owner
befriends the two referring adaptors — the one friendship in the tree that runs upward, from a container to
the views over it, and it grants access to a member and to nothing that member's type does not already expose.
Storage stays private; nothing on an owner's surface says `block_array`.

### the-public-names

Twelve aliases, three primaries. The unmarked name goes to the flagship — `bit_set` is the dynamic set
benchmarked against `std::set` and `std::flat_set` — and the qualifier marks the special case, `bit_static_set`.
The sequence row is named after the `std` container it packs, `bit_array` for `std::array<bool, N>`. The rows
therefore mark different columns, and that is correct by each row's own analogy rather than an inconsistency
to fix.

One header per public name, each an alias over one storage: `bit_set`, `bit_vector` and `dynamic_bitset`
over `block_vector<Block, Allocator>`, beside `bit_static_set`, `bit_array` and `bitset` over `block_array`.
The header is the name's home and the only place it is spelled; `bits.hpp` includes them all.

### width-is-capacity

`std::set` has no width, so `bit_set` treats its run-time width as capacity, never as value: two sets holding
the same positions are equal whatever their storages' widths, and the width neither orders, nor hashes, nor
is a precondition of the set operations. The storage cannot say that -- `block_sequence`'s `==` is width
first, which is what the sequence reading and `dynamic_bitset` mean, and its bulk operators and predicates
assert equal widths -- so the set adaptor says it. Every comparison, predicate and compound operator asks
`same_width` first and takes the storage's own answer at equal widths, which is every answer at a static
width, where `same_width` is constantly true and the arm folds away. At two run-time widths that differ,
`==` is `std::ranges::equal` over the elements, `<=>` is the invariant's own algorithm, `is_subset_of` is
`std::ranges::includes`, `intersects` walks one set asking the other, and `|=` `&=` `^=` `-=` insert and
erase element by element, `insert` growing the narrower left operand as it grows for any key. The shifts
translate the set, so `<<=` grows the width to hold the result and `>>=` empties past it. Hashing appends
the positions and the count at a run-time width and the bits at a static one, where equal sets share a
width ([the-hashing-invariant](#the-hashing-invariant)).

The element walks are a fallback and priced as one: an operation at mismatched widths costs the elements
rather than the blocks. A block-wise answer over the common prefix is an optimization the storage could
offer later; nothing in the adaptor's contract would change.


### the-idempotent-wrapper

`basic_bitset<Bits>` is `[template.bitset]` over any `Bits` that speaks the vocabulary, and its requirement is
the ceiling principle at the container layer: it adds only what `Bits` lacks and forwards everything `Bits`
has. The vocabulary is the concept `has_bitops` -- the compound operators, the shifts, `set` `reset` `flip`
`all` `any` `none` `count` `size` and regularity -- and a member the concept demanded is forwarded blind, one
line each, provably native. The shifts stay in the concept although the door carries their contracts: without
them a shiftless backend would fail inside an instantiation instead of at the class.

The wrapper is idempotent **per counterpart**: `basic_bitset<std::bitset<N>>` answers as `std::bitset<N>`
does, throw for throw, and the same wrapper over `block_array` is `xstd::bitset`, which answers the same way
because `std::bitset` is the counterpart of both. So at a static width there is no `-=`, no `is_subset_of`,
`is_proper_subset_of` or `intersects`: `std::bitset` has none, they are set vocabulary that had leaked into
`xstd::bitset`, and `set_view` keeps every one of them. The test harness guards those four the way it already
guarded the three predicates. A run-time width, whose counterpart is `boost::dynamic_bitset`, has them
natively and has them here: `xstd::dynamic_bitset` is the same wrapper over `block_vector`, and it answers as
`basic_bitset<boost::dynamic_bitset<>>` does. That is boost's surface with two omissions. `operator<` is a
third reading ([the-ordering-invariant](#the-ordering-invariant)) and stays out with `<=>`; the block-range
constructor and `to_block_range`/`from_block_range` are a block interface the wrapper does not expose, since
a `Bits` need not have blocks. Everything else is there and detected on `Bits`: the width-and-word and
width-only constructors, `-=` and the three set predicates, `find_first` and `find_next` answering `npos`,
`empty`, and growth ([growth](#growth)). The word conversions are the counterparts' own: `to_ulong` and
`to_ullong` throw `overflow_error` when a set position lies past the word, asked of the door's `find_next`
from the last position the word holds, and the word constructors take an `unsigned long long` at both widths.

Not a range and no `<=>`, as its counterpart has neither; `set_view` and `sequence_view` refer into its storage
([views-over-owners](#views-over-owners)), and the set ordering is theirs. `block_type` is gone from the
surface: nothing used it, and under idempotence a wrapper over a libc++ `std::bitset` would have none.

Extraction is `[bitset.operators]/6` at both widths: the characters read become `x = basic_bitset(str)`,
so a short read lands in the low positions, and a run-time width becomes the count of characters read,
as boost's does. The static width reads at most `size()` characters; the run-time width reads to the first
character that is neither `0` nor `1`.

### checked-and-unchecked

Two members share a spelling with different contracts across the counterparts, and the door carries both
families so the wrapper forwards a native guard rather than adding one on top of it: **the branch is
relocated, never added**.

**Shift.** `block_sequence`'s `<<=` is unchecked, with `n < size()` as its precondition; `std::bitset`'s and
`boost::dynamic_bitset`'s are total and saturate to none. The trait of a total counterpart declares
`checked_shift_left`/`checked_shift_right`, which the wrapper forwards as they are; where the door declares
none, the storage's own `<<=` -- the one `has_bitops` demanded -- is the unchecked form, and the wrapper
guards it: `n < size()`, else `reset()`. That hoists the guard `xstd::bitset` used to write by hand and skips
it where the counterpart already has one. At width zero even `<<= 0` trips the storage's assert, so the
wrapper's guard is what makes that instantiation well-formed.

**Element access.** `set(pos)`, `reset(pos)`, `flip(pos)` and `test(pos)` throw on `std::bitset`, whose trait
declares `checked_set`/`checked_reset`/`checked_flip`/`checked_test`, forwarded. The unchecked family is the
door's `unchecked_assign` and `at`, with `flip` synthesised as `unchecked_assign(not at)` the way
`basic_bit_set::complement` is;
a `flip` entry of its own is an open call. Where no checked entry exists the wrapper guards and throws
`out_of_range` at a static width, matching `std::bitset`, and asserts at a run-time one, matching
`boost::dynamic_bitset` -- a deliberate inconsistency between `xstd::bitset` and `xstd::dynamic_bitset`,
because it is exactly the one between their counterparts. The const subscript is
unchecked on every counterpart, so it is the door's `at` unconditionally, and the proxy from the mutable one
writes through `unchecked_assign` alone.

### asking-is-total

Asking is total whatever the extent: a position past the width is a key the set does not hold, which is an
answer and not a precondition violation. That is what `[set]` gives `contains` and `find` — `s.find(k)`
returns `end()` for any `k` it does not hold, never refuses the question — and it is the difference between
the set reading and the sequence reading, where `basic_bit_sequence::operator[]` indexes and out of range is
out of bounds.

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

The sequence proxy writes through the door's `unchecked_assign` and never through a subscript. The earlier
view fell back on `c[n] = value` for a type without `set(n, value)`, and were such a type's `operator[]` to
return our own proxy, that proxy's assignment would land back in the fallback and **recurse until the stack
is gone**. An entry the specialization spells cannot loop back into the proxy, which is one more reason the
write is a door entry rather than a probe; where the counterpart's subscript is the unchecked way in, as
`std::bitset`'s and `boost::dynamic_bitset`'s are, the specialization says so.

### the-iterator-is-the-primitive

`bit_set_iterator` and `bit_sequence_iterator` are a pointer and a position, and they reach the bits through
the door alone. Their constructors are public, so an owner or a view builds one without being a friend: the
dependency runs one way, from the container to the iterator, and the mutual friendship and forward
declarations the earlier views needed (*"Clang requires it, GCC does not"*) have nothing left to declare.

The pointer is to the **storage** an owner wraps, never to the owner: `bit_static_set` hands out
`bit_set_iterator<block_array<B, N>>`, which is why no owning type ever needs a `bit_traits` of its own.

### read-only-set-proxy

The set reading's proxy is read-only whatever the qualification of `Bits`, because a key is nothing to write
through: assigning to a position would mean moving an element, which a set has no spelling for. It earns its
keep anyway — `operator&` round-trips to the iterator, and the conversion to any class a `size_t` converts to
lets `*it` initialize a strong index type in one step, where the two user-defined conversions of going through
`size_t` would be one too many. A type with an explicit constructor takes the `size_t` route, `index(*it)`,
and the proxy offers no explicit conversion of its own: MSVC cannot resolve one beside that constructor.

### the-one-adl-exception

The sequence iterator's `iter_move` and `iter_swap` are hidden friends found by ADL, and they stay under the
no-ADL rule because they are `std::ranges`' own customization protocol: `ranges::sort` and `swap_ranges`
reach a proxy only through them, and it is where `vector<bool>` historically fell down. The three `swap`
overloads on the proxy are the pre-ranges spelling of the same thing, for `std::sort` and everything else
still built on `std::iter_swap`. `format_as` is fmt's protocol in the same sense.

The sequence proxy borrows nothing else from `[bitset.refs]`: no `flip()` and no `operator~`. Those belong to
the bitset reading, whose `reference` is its own class.

### total-lookups-on-the-container

Every `bit_static_set` lookup is total over `key_type`, because `std::set`'s is: a key outside `[0, N)` names
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
