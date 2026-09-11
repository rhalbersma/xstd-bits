# Design notes

Why the code is shaped the way it is. The headers carry one line each; the reasoning lives here, and a
one-line comment ending in `[design.md#anchor]` points at the section that explains it.

Decisions still in flight live on the [open issues](https://github.com/rhalbersma/xstd-bits/issues), and
[#80](https://github.com/rhalbersma/xstd-bits/issues/80) is the closed design plan the current shape came
out of. This file holds what has landed.

## Storage and containers

### contiguous-block-container

`contiguous_block_container` asks whether a range **is** blocks: a regular, contiguous, sized, subscriptable
range of unsigned integers. Regular is what lets `block_sequence` default its `==` over the width and the
blocks, in that member order, so two run-time widths part on the width before a block is read.
`std::array` and `std::vector` both qualify, and so does `std::inplace_vector` — a runtime width over
static capacity, for free.

Subscript is spelled out rather than left to `std::ranges::contiguous_range`, which does not imply it.
`contiguous_range` gives `data()` and a `contiguous_iterator`, and a `contiguous_iterator` is a
`random_access_iterator`, so `i[n]` **is** required — of the *iterator*. The range itself is under no such
obligation, and a plain buffer wrapper proves the gap: it satisfies the other four requirements, its
iterator subscripts happily, and `r[n]` does not compile. `block_sequence` reaches for the range's
subscript in 140 places, `block_mask` among them, so without this the concept admits storages the class
cannot be instantiated over — the shortfall surfacing as a hard error inside the template rather than as an
unsatisfied constraint, which is the failure mode
[a-requires-clause-names-its-arguments](#a-requires-clause-names-its-arguments) exists to prevent.

The requirement is written against the iterator's own reference type, `range_reference_t<R>`, rather than
against `range_value_t<R>&`, because the point is not that subscript yields *a* reference but that it yields
*the same* one iteration does. The rest of that is semantic and no concept can check it, so it is stated
here as the standard states it for `random_access_iterator`'s `i[n]`:

> `r[i]` is `*(std::ranges::begin(r) + i)`

and asserted in the tests, by address rather than by value, for every storage shipped. An unchecked
semantic requirement that no test pins is a comment.

Naming it `container` follows from the same fact. A contiguous container generalizes the C array, and `a[i]`
is the C array's defining operation; a concept claiming a range *is* blocks while unable to index one would
be describing something else. The word is deliberately close to the standard's *contiguous container*
([container.reqmts]/68) without claiming it: that term drags in the whole *Container* table — `empty()`,
`max_size()`, `cbegin`/`cend`, member `swap`, seven nested typedefs — and `block_sequence` needs almost none
of it. At a static width it needs none; at a run-time width it needs `max_size`, `resize`, `push_back` and
`clear`, which are *sequence* container operations, not `Container` ones. Neither path draws the line where
[container.reqmts]/68 draws it, so the concept states its own five requirements and borrows nothing.

`block_readable` is the other side of the same word, and asks whether a bit container will
**hand its blocks over**. Nothing models both, and no scope sees both unqualified.
The name says *container*, not *storage*, because that is the whole of what it asks: `bit_storage` and the
`Storage` template parameters are about what an adaptor sits on, which is a different question and now a
different word.

### the-one-vehicle

`block_sequence` and its three aliases live under `detail/`, one header each: `detail/block_sequence.hpp` holds
the concept, `num_blocks_v`, the class and its `bit_traits`, and `detail/block_array.hpp`,
`detail/block_vector.hpp` and `detail/block_inplace_vector.hpp` hold one vehicle apiece. The names are in
`xstd::detail::bits` with the rest of `detail/`, so a container spells `detail::bits::block_array<Block, N>`
and nothing outside the library can name the vehicle at all. The one exception is the `bit_traits`
specialization, which has to be in `xstd` because that is where the primary is declared: the header closes
`xstd::detail::bits` and reopens `xstd` for it. It is the device that
turns three readings over three storages into three plus three, and a factoring device is machinery rather
than vocabulary: a user reaches every width through `bit_static_set<N>` or `basic_bit_array<N, Block>` and
never spells the pair themselves. The split is what lets each of the nine containers include only the vehicle
it uses -- `bit_array` names `block_array` and no longer sees `std::vector`, and the
`#ifdef __cpp_lib_inplace_vector` guard sits in the one header that concerns it rather than in the common one.

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

The width member takes the blocks' alignment where they out-align a `std::size_t`:

```cpp
using width_type = std::conditional_t<(alignof(std::size_t) >= alignof(Blocks)), std::size_t, block_type>;
```

Only a storage holding its blocks inline out-aligns a `size_t`, and it does so by the blocks' own alignment, so
`block_type` is both wide enough to hold any width and exactly the size of the gap it fills -- `block_sequence`
is then its two members and nothing else, at the same size the padding cost. `std::array` reaches none of this,
a static width carrying no member at all, and neither does `std::vector`, whose alignment is a pointer's whatever
it holds; `block_inplace_vector<xstd::uint128, N>` is the one cell that does. The `static_assert` beside the
alias holds the two facts that make `block_type` the right carrier, so a storage over-aligned for some other
reason fails loudly rather than truncating a width. Every reader goes through `size()`, which converts once, so
the arithmetic stays a `size_t`'s.

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
own, `std::inplace_vector` satisfying `contiguous_block_container` as it is; `resize`, `reserve` and
`push_back` past the capacity throw `std::bad_alloc`, as that library specifies.

The adaptors take growth by detection on the storage, never through the trait: growth is a container's
business and no view's, so it exists on an owner and on nothing else. `sequence_adaptor` is
`std::vector<bool>` where its storage grows -- the count and count-value constructors, the range and
`initializer_list` constructors and assignments, `assign`, `resize`, `clear`, `push_back`, `pop_back`,
`emplace_back`, with `reserve`, `capacity` and `shrink_to_fit` where the blocks have them -- and
`std::array<bool, N>` where it does not, each member requiring the storage member it forwards to.
`bitset_adaptor` at a run-time width takes `boost::dynamic_bitset`'s growth the same way. `set_adaptor`
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

### the-blit

`word_at<Traits>(c, pos)` is the block-wide word at any position of any storage the trait reads by block:
the bits `[pos, pos + digits)`, assembled from `block(pos / digits) >> r` and the next block `<< (digits - r)`
where `r = pos % digits`. Two things make it total where it is called. A shift by `digits` is undefined, so
an aligned read is the block itself with no second term; and the last block has nothing above it, so the
read stops there rather than asking for a block past the end. What the word reaches beyond the width is the
clear tail, and it is the caller's to trim. It is the one primitive under every unaligned read: the sequence
adaptor's `append_range` from a sequence read by block ([the-range-members](#the-range-members)), the bulk
operations on a window ([windows](#windows)), and the bitset reading's order at unequal widths
([the-ordering-invariant](#the-ordering-invariant)), where each operand's top window is read as words at its
own alignment and both windows end at their own width, so the tail needs no mask.

`block_sequence::set_word(pos, value, mask)` is its write side, on our storage alone as `set_block` is: the
bits of `value` under `mask` land at `[pos, pos + digits)`, split over two blocks where `pos` is not aligned,
the tail kept clear. Every masked write goes through it: boost's ranged `set`, `reset` and `flip`, a window's
`fill`, and a window's bulk operators, each walking the words a range spans with the mask of what each holds,
whole words and a partial one at the end.

### the-funnel-shift

Under `word_at` and both shift operators is one operation: two adjacent blocks spliced into a double-width
word and shifted down. `block_sequence::straddled_block(index, L_shift, R_shift)` is that splice, and the three
sites now read as three uses of it rather than three spellings.

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

The trait's contract is the most efficient form, so `bit_traits<block_sequence<...>>` keeps the contracts
`block_sequence` gives it rather than widening to the total ones the synthesised walks happen to provide.

**A fallback synthesised for a foreign type may be more generous than the contract; it may not be less.**

So `find_next` requires `is_valid(n)`, and `find_prev` requires a set position strictly below `n` — which
`any()` does not establish, a container whose set positions all lie above `n` having none below it.

## The trait

The prose below calls `bit_traits<Bits>` *the trait*: the one thing the readings ask, and the one thing a
storage answers. The three entries every specialization must have — `extent`, `size`, `at` — are *the floor*.
The headers say `bit_traits`, `Traits` and "the required entries" and point here for the reasoning.

### opt-in

`bit_traits` is declared and never defined. Adaptation is opt-in rather than guessed, so a type nobody has
adapted is a compile error naming an incomplete type, rather than a silent fallback onto whatever members
happened to answer. That is the failure per-operation member probing walks into, and the trait's reason for
existing.

`bit_storage` gates the adaptors on the floor rather than on `bit_traits<Bits>` being complete, which turns
*incomplete type* into *constraint not satisfied* — the error that names the real problem.

### detection-by-absence

`block_readable` is meaningful **only because nothing supplies a default**: a specialization that does not
spell `block()` genuinely has no block access, so absence is an answer rather than an oversight.

That is also why the walks are free functions and not a base class to inherit from. A base would satisfy
the concept for every type, and the tier choice would collapse silently — and the same applies to the
`requires` probes for `find_first`, `find_next` and `find_prev`, which read a missing entry as "this type has
no native search".

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
consumer of the trait carry `Traits = bit_traits<Bits>` as an explicit parameter, `basic_string`-style, and
what turns the tier into a knob over identical storage — one `block_array`, two traits, one variable.

### what-the-trait-reconciles

Almost everything the two readings ask of a `Bits` is already an entry, or is the same operation under
another name:

| what a reading calls | trait entry | |
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
  is what the trait is for. It is not `unchecked_assign(c, n, true)`, which has no answer for a position past the
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
reachable the forward scans are native. Both return `N` when nothing is set, which is already the trait's
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

"Lexicographic" is underspecified below the reading layer. All three readings are lexicographic; they order
over different sequences, and **they disagree**, pairwise, as two pairs show:

| pair | set reading, ascending positions | sequence reading, bools from index 0 | bitset reading, bools from the top |
|---|---|---|---|
| `{0}` against `{1}` | `[0]` vs `[1]`: less | `[1,0]` vs `[0,1]`: greater | `"01"` vs `"10"`: less |
| `{0,1}` against `{1}` | `[0,1]` vs `[1]`: less | `[1,1]` vs `[0,1]`: greater | `"11"` vs `"10"`: greater |

A trait serving three readings cannot hold one of their orderings without choosing for its callers, so it
holds none under that name. It holds **all three, separately named**: `bit_traits` has a `set_three_way`
entry, a `sequence_three_way` entry and a `bitset_three_way` entry, never one `lexicographical_three_way`, so a
caller says which reading it means rather than being handed whichever the trait happened to pick.

### the-ordering-primitive

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

**The bitset reading needs neither piece.** The bit string, most significant position first, is the blocks
from the top block down, with the unused tail kept clear, so `bitset_three_way` is the plain `<=>` of the
blocks from the top: one comparison at a static width within a word, a loop from the last block otherwise,
equal only when every block is. It is the one reading whose order is plain lexicographic over words.

**The prefix clause is not removable.** Set order is not plain lexicographic over words under *any*
comparator. At `digits = 4`, `A = {1}` and `B = {5}` differ in word 0, where `A₀ = {1}` and `B₀ = {}`; a
prefix rule over words says `B < A`, but the sets truly compare `A < B`. `any_above` is exactly the repair,
and is the whole of what separates the two readings.

**The fallback is the specification.** A `Bits` that will not show its words -- `std::bitset` under libc++,
`boost::dynamic_bitset` -- falls back to that standard algorithm over the reading's own iterators, which is
[the invariant](#the-ordering-invariant) itself. So the trait's contract stays the efficient form and the
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
width-zero arm too, ahead of the choice between the trait's entry and the walk: at width zero every answer is
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

`bitset_adaptor` does not iterate, so its left-hand side is the sequence reading's traversal **reversed**:
`std::views::reverse` over the bools, the order `to_string()` writes them in. That is `boost::dynamic_bitset::operator<`
exactly, a **third** reading. Boost pairs *a*'s highest bit with *b*'s highest, second with second, then
breaks a tie on width -- the standard algorithm over reverse iterators. Verified over 1,046,529 pairs
spanning every width 0-9 against every other: zero mismatches against reverse-lex, 223,893 against numeric
order. It is *not* magnitude ordering, though it coincides with one at equal width: `"1"` and `"01"` are both
the number 1, and boost orders them strictly, the shorter first. The harness pins it three ways: against
boost's own `<` pair for pair over those widths, against `to_string()` compared as strings, and within a word
against `to_ullong()`.

`bitset_adaptor::operator<=>` is `bitset_three_way` at equal widths, and at unequal ones boost's own walk,
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
**through the trait**, never a storage's own hook: the blocks and the width where the trait reads by block,
every position and the width otherwise. So equal values hash equal whatever holds them, and a set view over
a `std::bitset` hashes on every library whether or not `_Getword` is reachable. The set reading at a run-time
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
`std::bitset::test` throws where this asserts, a difference the containers state as a guard rather than
one this name should try to carry.

### qualifier-prefixes

Contract qualifiers are prefixes, not suffixes — `inclusive_find_next`, `exclusive_find_prev`,
`unchecked_assign` — so that the contract reads before the operation and the cheaper form cannot be called
by mistake.

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

Three class templates carry the three readings: `set_adaptor`, `sequence_adaptor`, `bitset_adaptor`. Each
is written against the trait and never against a storage, so one adaptor serves `block_array` and
`block_vector` alike as an owner, and `std::bitset` and `boost::dynamic_bitset` as a view
([owning-is-ours](#owning-is-ours)), and no owning type ever needs a `bit_traits` of its own. The
public names are aliases in two layers over them: `basic_bit_static_set<N, B>` is
`set_adaptor<block_array<B, N>, owns>`, `basic_bit_array<N, B>` is `sequence_adaptor<block_array<B, N>, owns, false>`,
and `basic_bitset<N, B>` is `bitset_adaptor<block_array<B, N>>`; `bit_static_set<N>`, `bit_array<N>` and
`bitset<N>` are those at `std::size_t`.

### owning-is-ours

Owning is ours and viewing is interop. An owning adaptor sits over a storage of this library, `block_array` or
`block_vector`, and nothing else is supported or tested: `bitset_adaptor` requires the vocabulary
([a-strict-extension](#a-strict-extension)) and block access, which neither counterpart satisfies, and the
owning `set_adaptor` and `sequence_adaptor` take their ordering from the trait's entries alone, with no
synthesized fallback for a storage without them. A view sits over any type with a trait, which is what the
two `ext/` specializations are for.

The split is what a foreign owner cost against what it bought. Every owner-only member -- construction
through the storage's constructors, growth, the saturating shifts, the checked element access, the ordering
-- needed an entry in each foreign trait and harness arms over each foreign storage, and the blit and the
range insertions of #80's 7d would have added more. It bought one object where a view over a member gives the
same reading: `bit_set_view(my_std_bitset)` is the set reading over a `std::bitset` someone else owns, and
`bit_static_set` is the same reading over storage of ours. What is given up is the wrapped-equals-raw proof
that the wrapper adds and drops nothing; the views keep it for reads, writes, iteration, searches and hashing,
and construction, growth and the ownership protocol are proven over our storages alone.

So the `ext/` traits are the view's contract: `extent`, `size`, `at`, `count`, `unchecked_assign`, `insert`,
`fill`, and the searches and block reads where the type has them. The `checked_*` family and the checked
shifts, which only a wrapper asked, are gone, and so are the `operator-=` and `operator-` on `std::bitset`
that lived in `namespace std` against `[namespace.std]`: `bit_set_view` has `-=`, and `std::bitset` has no
set difference of its own.

### ownership-is-not-an-axis

Owning versus viewing is storage lifetime, not a third axis of the model, and it collapses to one template
parameter: `ownership::owns` stores `Bits`, `ownership::refers` stores `Bits*`. Always present and only its
type changes, so a plain `conditional_t` rather than `conditional_data_member_t`. One accessor, via deducing
`this`, gives deep const to the owner — `self.m_bits` propagates `self`'s const — and shallow const to the
view — `*self.m_bits` does not — for free.

Every mutator is then gated on the trait and nothing else: `requires requires { Traits::op(self.storage(), …) }`
reads "the trait lets *this handle* write". A const owner's accessor hands the trait a `Bits const&`, which no
`unchecked_assign` accepts; a const view's hands it `Bits&`, which is what a view is for; a view over
`Bits const` hands it `Bits const&` again. Const, ownership and a floor-only trait are all the same question, asked once.
The exceptions are the constructors and, once storage grows, the growth members, which need an explicit
`requires (owns(Own))`: the requires-expression tests what the storage can do, not what this handle may do
to it, and a view over a `block_vector` must not be able to resize what it does not own.

### views-follow-their-precedent

`bit_set_view` follows `std::string_view`: a value that happens not to own its bytes, so it has `==` and
`<=>`, and its ordering is exactly `std::set`'s. `bit_span` follows `std::span`, which P1085 stripped of both
because "same referent" and "same contents" are both defensible readings of a handle. So `set_adaptor`
compares and hashes whatever it owns or views, and `sequence_adaptor` compares and hashes only as an owner. The non-member copies
— `~`, `&`, `|`, `^`, `-`, `<<`, `>>` — are the owner's alone in both readings: a copied view would write
through to what it views.

Both referring adaptors opt into `std::ranges::enable_view` and `enable_borrowed_range`, the two
specializations [range.view] and [range.range] invite for a program-defined type. The first makes
`bit_set_view(x) | views::take_while(…)` take the view as it is rather than wrapping it in an `owning_view`;
the second says what `span` says, that the iterators point at the storage and outlive the handle that made
them, which is what lets `ext/xstd/bitset.hpp` return `set_adaptor(c).begin()` from a temporary.

### the-views-are-the-adaptors

`bit_set_view<Bits, Traits>` **is** `set_adaptor<Bits, ownership::refers, Traits>` and `bit_span<Bits, Traits>`
**is** `sequence_adaptor<Bits, ownership::refers, false, Traits>` — alias templates, the way `bit_subspan`
always was, and not a second implementation of either reading. They carry the names of
[the-public-names](#the-public-names), one header each beside the owners; the `set_view` and `sequence_view` of
the rewire were the same classes before the viewing column was filled. The earlier views, with their own
iterators, proxies and four customization points — `set_find`, `sequence_find`, `block_access`, `bit_extent` —
were the trait before there was one, and once the adaptors read through `bit_traits` alone there was nothing
left for them to do.

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
not this shape of it — one pinned non-type argument plus a defaulted, constrained trait argument depending on
the first — and on that shape MSVC 17 fails while claiming support. A four-day-old note quoting a specific
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

**The diagnostics belong on that same list.** An alias is transparent, so a storage that fails `bit_storage`
is diagnosed where the alias is *written*: `void f(my_set<int>)` is an error at that declaration, quoting the
unsatisfied `requires` and naming `bit_traits<int>` as the undefined template. A derived class that leaves the
constraint to its base is not: naming one in a declaration does not require a complete type, so the same line
**compiles**, the base is never instantiated, and the diagnosis waits for whoever first completes the type.
It then arrives twice, because a dependent base is named twice and cannot be named once -- in the
base-specifier, and again in the using-declaration that inherits the constructors. Measured on `set_adaptor`
over a storage with no trait: 13 lines and one error through the alias, 22 lines and two errors through such a
derived class.

Restating the constraint on the derived class's own parameter recovers all of that, and then some: it fails at
the declaration, once, in **fewer** lines than the alias, having no indirection to explain. So this is not a
second reason standing beside the restatements -- it is one more entry on the same list, and the failure mode
is the derived class that skips it. A four-line reduction holding a constrained class template, an alias of
it, and both derived forms reproduces the shape exactly, GCC and Clang agreeing to the line, so it is the
language rather than a diagnostic quirk.

Where it would bite is the views, and only the views. The nine owners choose their own storage, so a storage
with no trait cannot arise through them at all; the one parameter a user supplies is the `Block`, constrained
at every layer. A view takes the storage -- that is what a view is for ([owning-is-ours](#owning-is-ours)) --
so a view is exactly the place where a constraint left to the base would go undiagnosed until use.

The alias pays for this on the other side, and the trade is worth stating whole. Being transparent, it is not
what a compiler prints: a diagnostic about `bit_array<100>` names `sequence_adaptor<block_sequence<array<
unsigned long, 2>, 100>, ...>`, a spelling the user did not write and cannot write back. `std::string` makes
the same trade and the world lives with `basic_string<char, char_traits<char>, allocator<char>>` -- though
`std::string` aliases a *class*, where both layers here are aliases, which is why the printed name falls
through to the adaptor rather than stopping at `basic_bit_array`.

One constraint moved rather than vanished. The guide for a plain storage is viable for an owner too, now that
a bitset has a `bit_traits` of its own, and would tie with the owner guide — so it is constrained to
non-owners, as [a-bitset-reads-as-its-storage](#a-bitset-reads-as-its-storage) describes. That constraint used
to sit on each view's restated guide; it now sits on `set_adaptor`'s and `sequence_adaptor`'s own, which is
where the aliases deduce through.

The sequence view pays the `span` half of [views-follow-their-precedent](#views-follow-their-precedent) by
becoming the adaptor: it no longer has `==` or `<=>`, and the harness checks the sequence reading through the
iterators instead.

### windows

`bit_subspan<Bits, Traits>` is `sequence_adaptor<Bits, refers, true, Traits>`: the referring adaptor
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

A window's end blocks are shared with what lies outside it, so its bulk operations are masked words: `fill`
over a window of ours is `block_sequence::set(pos, len, value)`, a word at a time through `set_word`, and one
position at a time over a window of anything else; `&=`, `|=`, `^=` and `-=` on a window of ours take a source
of any shape that reads blocks of the same block type, a window at any other alignment included, reading both
sides through `word_at` and writing through `set_word` masked to the window ([the-blit](#the-blit)). The two
must be of one size, and must not overlap short of coinciding, `w ^= w` being fine. A window has no shifts:
shifting within a window is nobody's counterpart.

### the-range-members

`append_range` has two tiers. Where the source is a sequence adaptor of any shape, owner, view or window,
whose trait reads blocks of the destination's block type, the source's bits are read as words at the source's
own alignment through `word_at` and appended a word at a time through `block_sequence::append(block)`, which
splits each word over the destination's own alignment; then the width is trimmed to the count, `resize`
clearing whatever the last word carried past it. Everything else, a `std::vector<bool>`, an `iota` under a
`transform`, a sequence over another block type, is packed a word at a time, which is boost's private
`bit_appender`, and trimmed the same way. A source inside the very storage being appended to is safe: the
blit reads positions below the old width alone, and no append writes one.

`insert_range`, `insert` in its four shapes, `emplace` and both `erase`s rebuild rather than shift: the head
through `first(pos)`, the middle, the tail through `subspan(pos)`, into a fresh sequence that is then swapped
in. Every step runs at the blit's tier, insertion into a packed sequence is linear however it is done, and
the strong exception guarantee comes free, which is what `std::vector::insert_range` gives on reallocation.
`subspan` pays for itself here, the head and the tail being exactly windows. In-place block-wise shifting of
the suffix stays available as a later optimization behind profiling.

`flip()` is `[vector.bool]`'s, a bulk operation like the six operators beside it, so an owner and a whole
view have it and a window does not; the static `swap(reference, reference)` is the proxies' own swap under
the name the standard gives it.

### the-sequence-contract

`bit_vector` answers every line of `[vector.bool]`'s synopsis and `bit_array<N>` every line of `[array]`'s,
and the test says so as a checklist rather than a claim: `test/sequence/concepts.hpp` spells each synopsis
as one requires-expression, `vector_bool` and `array_bool`, and `std::vector<bool>` and `std::array<bool, N>`
are asserted against it first. A line the model itself fails is a wrong line, so the checklist is known to
be honest before ours is held to it; the C++23 range members are a second concept, `vector_bool_ranges`, so
the model is held to them only where its standard library has them.

The sweep found what the range members had not needed. The allocator: `allocator_type` through the same
empty base `bitset_adaptor` has, `get_allocator`, and the allocator-extended constructors,
`[container.alloc.reqmts]`'s copy and move included, which `block_sequence` gains beneath them, deduced and
matched to the storage's own so a static owner has none. `[vector.erasure]`'s `erase` and `erase_if` as
non-members over the owner's `erase(first, last)`, `std::ranges::remove_if` running unchanged over the
proxies, which move and swap. And `std::array`'s aggregate initialization as an `initializer_list`
constructor on the static owner, the listed values leading and the rest false, a longer list being the
error it is on `std::array`.

Three things are not offered, each because packed bits have no address. `data()`, and the `pointer` and
`const_pointer` typedefs, name what a proxy cannot give; the checklist leaves them out of
`[container.reqmts]`'s typedefs rather than inventing a pointer to a bit. `std::array`'s tuple interface,
`get<I>`, `tuple_size` and `tuple_element`, is left out with them: it is `std::array`'s claim to be a
product of `N` objects, and a packed sequence is one object. `std::hash` is asked of the vector alone,
`std::array` having none, while `bit_array` hashes as every owner does
([the-hashing-invariant](#the-hashing-invariant)).

### views-over-owners

An owner has no trait of its own — `bit_static_set`, `bit_array` and `bitset` are thin wrappers over a
`block_array` that already has one — so a view over an owner is a view over the storage it wraps:
`bit_set_view(xstd::bitset<64>&)` is `set_adaptor<block_array<size_t, 64>, refers>`, and the pointer in the
iterator is to the `block_array`, never to the `bitset`. The owner hands its storage over through
`owned_storage<Owner>`, declared beside it as `bit_traits` is beside a storage and never defined for anything
else, so `owner_of<Owner, Bits, Traits>` reads "this owner wraps exactly the storage and trait this view
refers through". Const flows one way: a const owner gives a view over `Bits const`, a mutable owner either.

The view's converting constructor takes the owner's private member directly, which is why each owner
befriends the two referring adaptors — the one friendship in the tree that runs upward, from a container to
the views over it, and it grants access to a member and to nothing that member's type does not already expose.
Storage stays private; nothing on an owner's surface says `block_array`.

### the-public-names

Three layers of names. The primaries carry the reading and take the storage: `set_adaptor<Bits, Own, Traits>`,
`sequence_adaptor<Bits, Own, Windowed, Traits>`, `bitset_adaptor<Bits, Traits>`, the parameters the trait's consumers
need and no more. The `basic_` layer chooses the storage and leaves the block open, `basic_string`-style:
`basic_bit_static_set<N, Block>`, `basic_bit_set<Block, Allocator>` and their four siblings. The restricted layer
fixes `std::size_t` and `std::allocator`: `bit_static_set<N>`, `bit_array<N>` and `bitset<N>` keep one parameter,
and `bit_set`, `bit_vector` and `dynamic_bitset` keep none, so the flagship is `xstd::bit_set` and the counterpart
of `boost::dynamic_bitset<>` is `xstd::dynamic_bitset`, without the `<>`.

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
`bit_vector` and `dynamic_bitset` over `block_vector<Block, Allocator>`, beside `bit_static_set`, `bit_array` and
`bitset` over `block_array<Block, N>`, and `bit_inplace_set`, `bit_inplace_vector` and `inplace_bitset` over
`block_inplace_vector<Block, N>` ([the-inplace-column](#the-inplace-column)). The header is the name's home and
the only place it is spelled; `bits.hpp` includes them all. Each static name has an `aligned` form in the
namespace of that name, in both layers, its width rounded up to whole blocks so that no block carries an unused
tail: `aligned::bitset<9>` is `bitset<64>` and `aligned::basic_bitset<9, std::uint8_t>` is
`basic_bitset<16, std::uint8_t>`. The inplace column has no `aligned` form, its `N` being a capacity the storage
already rounds up rather than a width to round.

### the-generated-table

Nine cells over three adaptors over three storages is the shape where an inconsistency hides in one cell and
nowhere else, so what the compiler generates is a table, held by `test/src/bits/generated.cpp` rather than by
whichever cell was read last.

Every cell answers the same to all but one column: default-constructible, copyable, movable, `==`, `<=>`,
`swap` as both a member and a free function, and **nothing-throwing** in both move directions -- a move that
could throw would cost every growing container its strong guarantee.

The allocator is the exception, and it follows the **column, not the row**: the dynamic column allocates and
all three of its readings answer `get_allocator`; the static column is a `std::array` and has none to show;
the inplace column holds its blocks inline and has none either. So `bit_set`, `bit_vector` and `dynamic_bitset`
have it and the other six do not, which is a fact about `block_vector` rather than about sets, sequences or
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
adaptors: `basic_bit_inplace_set<N, Block>`, `basic_bit_inplace_vector<N, Block>` and
`basic_inplace_bitset<N, Block>` over `block_inplace_vector<Block, N>`, with `bit_inplace_set<N>`,
`bit_inplace_vector<N>` and `inplace_bitset<N>` at the machine word. `inplace_bitset` takes no `bit_` prefix
because `bitset` already carries the word, and `inplace` is one storage word down each column rather than a
second vocabulary for the same thing.

`N` is a **capacity** in bits here, where the static column's `N` is a width. The names carry that and the
parameter lists do not, which is the same hazard `basic_bit_static_set<N, Block>` and `basic_bit_inplace_set<N, Block>`
share by shape. The capacity is rounded up to whole blocks by `block_inplace_vector` itself, so
`basic_bit_inplace_vector<9, std::uint8_t>` holds sixteen bits; the width under it is a run-time one and carries
an unused tail like any other.

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
reading of bits that is one number: **the positions there are to hold**. The set reading iterates the positions
it holds, so its largest is every position set; the sequence reading iterates one `bool` per position; the
bitset reading owes boost the same answer. There is no per-reading meaning of `max_size()` and no separate key
domain -- a set over `[0, W)` holds at most `W` elements because there are `W` positions, which is the same `W`.

So all three ask the same question of the same place, and only the answer's source differs by what can grow:

| | `max_size()` |
|---|---|
| a width in the type | `Traits::extent` |
| an owner over growing storage | the storage's `max_size()`, in bits |
| a view, a window, a static owner | its own width, which it cannot grow |

`block_sequence::max_size()` is where the real limit lives, and it is not `SIZE_MAX`: a width rounds up to whole
blocks, so the largest addressable one is `min(blocks.max_size(), SIZE_MAX / bits_per_block) * bits_per_block`
-- `SIZE_MAX - 63` at a `size_t` block, and `N` rounded up at an inplace one. Nothing above it needs to restate
that arithmetic, and nothing above it should: a constant at the adaptor drifts from the storage the moment the
storage learns something, which is how `set_adaptor` came to answer `SIZE_MAX - 1` while `dynamic_bitset`
answered `SIZE_MAX - 63` over the same blocks.

That the set's is not `static` follows: an owner must ask its storage and a view must ask what it views, neither
of which a static member can reach. `std::set::max_size()` is not static either.

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

The vocabulary is the concept `has_bitops` -- the compound operators including `-=`, the shifts, `set` `reset`
`flip` `all` `any` `none` `count` `size`, the three set predicates and regularity -- and a member the concept
demanded is forwarded blind, one line each. The shifts stay in the concept although the guard is the
wrapper's ([the-one-guard](#the-one-guard)): without them a shiftless storage would fail inside an
instantiation instead of at the class.

The two widths share one surface. Boost's set vocabulary, `-=`, `-`, `is_subset_of`, `is_proper_subset_of`
and `intersects`, and its two searches, `find_first` and `find_next` answering `npos`, are there at a static
width as well: the storage spells them alike, and an extension may add. Only growth is gated on a run-time
width ([growth](#growth)): `empty`, `resize`, `clear`, `push_back`, `pop_back`, `append`, `reserve`,
`capacity` and `shrink_to_fit`, detected on the storage. The word conversions are the counterparts' own:
`to_ulong` and `to_ullong` throw `overflow_error` when a set position lies past the word, asked of the
trait's `find_next` from the last position the word holds, and the word constructors take an
`unsigned long long` at both widths, boost's taking the width first.

Three additions are ours, with no counterpart on either side. `find_last()` and `find_prev(pos)` mirror
boost's forward pair: the highest set position below `pos`, `npos` where none, a `pos` past the width meaning
from the end, so `find_prev(npos)` is `find_last()` the way boost's `find_next(npos)` wraps to `find_first()`,
and the two loops are each other's reverse. They are total, so they take the generic walk rather than the
trait's `find_prev`, whose contract is the iterator's cheaper one ([total-versus-precondition](#total-versus-precondition)).
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

Two members share a spelling with different contracts between the storage and the counterparts, and the
wrapper carries the one guard between them.

**Shift.** `block_sequence`'s `<<=` is unchecked, with `n < size()` as its precondition; `std::bitset`'s and
`boost::dynamic_bitset`'s are total and saturate to none. The wrapper guards the storage's shift, `n < size()`
else `reset()`, which is the guard `xstd::bitset` used to write by hand. At width zero even `<<= 0` trips the
storage's assert, so the guard is what makes that instantiation well-formed.

**Element access.** `set(pos)`, `reset(pos)`, `flip(pos)` and `test(pos)` are the trait's `unchecked_assign`
and `at` behind a guard, with `flip` synthesised as `unchecked_assign(not at)` the way
`set_adaptor::complement` is; a `flip` entry of its own is an open call. The guard throws `out_of_range` at
a static width, matching `std::bitset`, and asserts at a run-time one, matching `boost::dynamic_bitset` -- a
deliberate inconsistency between `xstd::bitset` and `xstd::dynamic_bitset`, because it is exactly the one
between their counterparts. The const subscript is unchecked on every counterpart, so it is the trait's `at`
unconditionally, and the proxy from the mutable one writes through `unchecked_assign` alone.

The `checked_*` family the traits once carried, so that a wrapper over `std::bitset` could forward its native
throw, went with the foreign owners ([owning-is-ours](#owning-is-ours)): the branch is the wrapper's, and
there is one of it.

### asking-is-total

Asking is total whatever the extent: a position past the width is a key the set does not hold, which is an
answer and not a precondition violation. That is what `[set]` gives `contains` and `find` — `s.find(k)`
returns `end()` for any `k` it does not hold, never refuses the question — and it is the difference between
the set reading and the sequence reading, where `sequence_adaptor::operator[]` indexes and out of range is
out of bounds.

`insert` carries no `noexcept`, for the reason `std::set::insert` carries none: growing a dynamic extent
allocates. It is the one operation a set can be unable to satisfy, and only a **static** extent ever is — a
fixed capacity cannot come to hold a position outside it, so that is the precondition violation. A dynamic
extent grows to hold it, `[set]` giving `insert` no way to fail. Growing has a limit of its own, and it is the
storage's rather than the address space's: `max_size()` ([max-size-is-the-bits](#max-size-is-the-bits)).

Erasing stays total like `contains`: removing what is not there is the no-op returning zero that
`std::set::erase` is.

### unchecked-writes-in-views

Reads and writes inside a view go through the **subscript**, not through `test()`, `set(n)` or `reset(n)`.
The position is already in range by then, and those are the checked accessors whose throw would escape a
`noexcept` — which `bugprone-exception-escape` is right to report. Every type in the vocabulary hands out a
proxy that writes without checking.

### the-proxy-recursion-trap

The sequence proxy writes through the trait's `unchecked_assign` and never through a subscript. The earlier
view fell back on `c[n] = value` for a type without `set(n, value)`, and were such a type's `operator[]` to
return our own proxy, that proxy's assignment would land back in the fallback and **recurse until the stack
is gone**. An entry the specialization spells cannot loop back into the proxy, which is one more reason the
write is a trait entry rather than a probe; where the counterpart's subscript is the unchecked way in, as
`std::bitset`'s and `boost::dynamic_bitset`'s are, the specialization says so.

### the-iterator-is-the-primitive

`bidirectional_bit_iterator` and `random_access_bit_iterator` are a pointer and a position, and they reach the bits
through the trait alone. Their constructors are public, so an owner or a view builds one without being a
friend: the dependency runs one way, from the container to the iterator, and the mutual friendship and forward
declarations the earlier views needed (*"Clang requires it, GCC does not"*) have nothing left to declare.

The pointer is to the **storage** an owner wraps, never to the owner: `bit_static_set` hands out
`detail::bits::bidirectional_bit_iterator<block_array<B, N>>`, which is why no owning type ever needs a
`bit_traits` of its own.

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

The walks stay qualified as `detail::bits::find_next<Traits>(...)` inside `xstd::detail::bits` itself. Dropping
the qualification would read more naturally and reintroduce exactly the hazard the nesting exists to close: an
unqualified call with an explicit template argument performs ADL, and the associated namespace of the storage
being walked is `std` or `boost` ([why-nested](#why-nested)).

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
error where it should have said "no". That is not a stylistic loss; it is the mechanism
[detection-by-absence](#detection-by-absence) is built on -- `can_grow`, `word_writable`, `blittable` and
every trait tier ask exactly that question. Declared return types, trailing or leading, answer it from the
declaration.

**Lambdas keep their trailing return inline.** A lambda is an expression inside a statement, so there is no
"above the body" to put anything on, and `modernize-use-trailing-return-type` requires the `-> void` there
anyway. The convention is about named functions.

### a-bitset-reads-as-its-storage

A `bitset` has a `bit_traits` of its own, so a view can name it: `bit_set_view<xstd::bitset<N>>` and
`bit_span<xstd::bitset<N>>` are spellings, not errors. **One** specialization does it, on `bitset_adaptor`,
because `xstd::bitset<N>`, `xstd::inplace_bitset<N>` and `xstd::dynamic_bitset` are all aliases of that one
template over a different `block_sequence` -- so all three, and every `basic_` form, arrive together.

This supersedes the reasoning recorded when `ext/xstd` was deleted, which concluded that a bitset needs no
trait because it already joins through `owned_storage`. That is still how a view *deduces*: over an owner,
`bit_set_view(bs)` binds the storage the owner wraps, and the deduced type is
`bit_set_view<block_array<std::size_t, N>>`. What was missing is that the deduced spelling is the only one a
reader can write down, and it names an implementation detail -- `block_array` is not in the landscape tables
and should not have to be. Naming the bitset is what a reader means.

The two coexist rather than compete, and one line keeps them from tying. The guide for a plain storage was
unconstrained, viable for anything; it stayed out of the way for an owner only because
`bit_traits<Owner>` was incomplete, which is precisely what this change undoes. Completing it makes both
guides viable and `bit_set_view(bs)` **ambiguous**, so the storage guide is now constrained to non-owners:

```cpp
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
bit_set_view(Bits&) -> bit_set_view<Bits>;
```

The forwarding relays all twenty of the storage trait's entries, each behind its own `requires`, because
absence is the mechanism the tiers select on ([detection-by-absence](#detection-by-absence)). A forwarder
that relayed only the three required entries would compile and be **slower**: without `num_blocks` and
`block` every word-parallel walk falls back to one position at a time, and nothing would have said so. The
test asserts `block_readable` through the trait, not merely `bit_storage`, so a dropped entry fails rather
than degrades.

Scope stops at `bitset_adaptor`. A generic trait over every owner was tried first and rejected: it would
complete `bit_traits` for `set_adaptor` and `sequence_adaptor` too, which is where the tier probes and the
view constraints do their work, and it buys nothing -- a set view of a set is not a spelling anyone wants.
The narrow specialization is also the one that matches the convention `ownership.hpp` already states, that a
trait sits beside the thing it adapts.

`xstd::bitset` still has no iterators and is still not a range; this changes how a view is *named*, not what
the bitset offers. [a-strict-extension](#a-strict-extension)

### what-a-view-costs

Measured on the same backend `block_sequence` in every row -- an owner, a view holding a pointer to that
storage, and a view over the `bitset_adaptor` wrapping it -- so the two layers price separately. `benchmark/`
builds at `-O3 -march=native`, which is what these numbers are; an earlier version of this section said `-O2`,
which was never true of any build in the tree.

| operation | bits | pointer costs | trait costs |
| :--- | ---: | ---: | ---: |
| set iterate | 256 | +7.1 … +9.6% | ±1% |
| set iterate | 1024 | +8.5 … +11.4% | ±1% |
| set iterate | 4096 | +11.1 … +12.8% | ±4% |
| set iterate | 16384 | +10.3% | ±2% |
| sequence count | 256 … 16384 | ±2% | ±3% |
| sequence read | 256 … 16384 | ±1% | ±2% |

Run-to-run noise was ±0.5 to ±3%, so a figure inside a couple of percent is a zero.

**The trait layer is free, and not merely inside the noise.** Under callgrind, at 1024 bits, the view over a
`bitset_adaptor` and the view over the raw `block_array` retire **6764 instructions per pass each**, equal to
the digit, with the same 422 data reads, 1265 branches and 17 simulated mispredicts. So
[a-bitset-reads-as-its-storage](#a-bitset-reads-as-its-storage) costs nothing at run time: the twenty
forwarded entries inline away completely.

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
+24.3%, -3.2% and +21.8% across three runs, with the trait column swinging the opposite way each time to land
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
`block_sequence` had only the member, and every one of the nine containers swaps by
`std::ranges::swap(m_bits, other.m_bits)` where `m_bits` **is** a `block_sequence`. So the member was
unreachable from the containers, and a storage with an optimized `swap` never saw it. Measured over a
storage whose swap and moves are counted, once per reading:

| | before | after |
| --- | --- | --- |
| `sequence_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |
| `set_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |
| `bitset_adaptor` | 0 storage swaps, 3 moves | 1 swap, 0 moves |

`block_sequence` now has the free `swap` too, as a hidden friend delegating to the member. Nothing else
about swapping changed, and the storage is not asked for one: `std::regular` implies `copyable`, which
implies `movable`, which **includes** `std::swappable`, so the concept already requires as much swapping as
`ranges::swap` can need, and any better one arrives by ADL without being asked for.

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
requires requires { Traits::insert(self.storage(), 0UZ); }   // is a size_t insertable?
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
[contiguous-block-container](#contiguous-block-container), whose subscript requirement exists because the
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
public members are spelled over those, each helper choosing its tier once: the trait's door over the whole, a
masked word at a time over a window of ours, one position at a time over a window of anything else, which is
the same three tiers `fill` uses ([windows](#windows)). `none_true` is a helper of its own rather than `not
any_true`, so a storage that spells `none()` itself is asked in its own words; all three adapted here do.

`bit_traits` grows `all`, `any` and `none` doors beside `count`, taken from an entry where the storage has one
-- `block_sequence`, `std::bitset` and `boost::dynamic_bitset` all spell all three themselves -- and
synthesized where it does not. Neither synthesis walks a bit at a time that it could avoid: `any` is
`find_first` compared against the width, and `all` is `count` compared against it, which is the block tier
through `count`'s own door.

`mismatch` is `block_sequence::first_difference` plus one `countr_zero`. That helper existed already, private
and used only by `sequence_three_way`; it is now public, and **keeps its name**: it scans low block to high,
which is the *ascending* orderings' answer, where `bitset_three_way` deliberately walks the other way and does
not use it. Calling it `mismatch` on the storage would repeat the mistake `lexicographical_three_way` made
([two-readings-disagree](#two-readings-disagree)). The counterpart name goes on the public member, which is
the owner's alone: a window's blocks are not its own.

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
loop-carried state blocks vectorization everywhere. `count()` is still 47x ahead of the faster of the two.

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
`formattable<ranges::range_reference_t<R>>` and a reference of ours is a proxy. So `xstd/bits/format.hpp`
specializes `std::formatter` for the two proxies and stops there: `bit_set`, `bit_static_set`, `bit_vector`,
`bit_array`, the views, the windows and the inplace column all follow from that, none of them mentioned.

This is the same shape the proxies already had for fmt, in fmt's spelling. `format_as` is fmt's generic
per-type hook: define it for one type and every range over that type formats, which is why the proxies carry
it and no container does. `std::formatter` is the standard's hook for the same job. So each library gets one
hook per proxy -- a hidden friend for fmt, a specialization for the standard -- and in both the containers
follow for free. Nothing here is a special case for formatting; it is the general mechanism used twice.

The readings then separate themselves. `[format.range.fmtkind]` picks `range_format::set` for a range with a
`key_type` and `range_format::sequence` otherwise, so the set reading prints `{1, 3, 5}` and the sequence
reading `[false, true, false, false]` -- the same split `format_as` arrives at for fmt, reached here through
the standard's own machinery rather than by our choosing
([two-readings-disagree](#two-readings-disagree)).

Each specialization derives from `std::formatter<size_t>` or `std::formatter<bool>` instead of writing a
`parse`, which is what keeps the whole spec: a width and a fill on a single proxy, and the nested spec a range
formatter forwards, so `{::#x}` over the set reading and `{::d}` over the sequence reading reach the
underlying formatter intact.

The header is not in `xstd/bits.hpp`. The umbrella keeps `<format>` off every consumer path for the reason it
keeps the `ext/` adaptors and Boost off it; a consumer who formats says so by including the header. Issue #20
had this waiting on P3070R0, which is not what blocked it: the proxy's formattability was, and that is ours to
fix.

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

### the-sieve

The sieve under `include/opt/set/` is the library's worked example and its bench, and it speaks **one**
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

Five findings are suppressed because the checker cannot see what makes them right:

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

A sixth had a fix rather than a suppression. `modernize-use-nullptr` reads the `0` in `(a <=> b) < 0` as a
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
