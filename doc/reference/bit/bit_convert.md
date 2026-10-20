# `<xstd/bits/bit/bit_convert.hpp>`

## `bit_convert`

The counterpart of `bit_cast` in [[bit.cast]](https://eel.is/c++draft/bit.cast) across bit storages: where `bit_cast`
reinterprets an object, `bit_convert` maps positions, and position `i` of the source is position `i` of the result.
Let `F` be `remove_cvref_t<From>` and `T` be `remove_cv_t<To>`. The constraints use these phrases:

- A *fixed width* is a type that fixes in its type how many positions it has, its *width* `W`: an owner of ours
  whose storage fixes its width, as `basic_bit_array`, `basic_bit_fixed_set` and `basic_bit_fixed_valarray` do; a
  whole view whose storage fixes its width, `W` being the storage's, as for `bit_set_view<uint64_t>` and
  `bit_span<array<uint8_t, 3>>`; a block, an `array` of blocks or a built-in array of blocks, `W` being their
  `bit_blocks_extent_v`; and, on a little-endian target, `bitset<N>`, whose `W` is `N`.
- A *fixed target* is a fixed width that is neither a view nor a built-in array.
- A *run-time target* is an owner of ours that stores its width, a capacity included: the vectors, the run-time and
  inplace sets and valarrays, and the Boost extension's small owners.
- A *source* is a fixed width, an owner of ours, a whole view of ours of run-time width, or a window.

The rules:

| `T` | `F` | result |
| :-- | :-- | :-- |
| a fixed target | a fixed width of `W(F) <= W(T)` | the `W(F)` positions, the positions `[W(F), W(T))` clear |
| a fixed target | a source that is no fixed width | its positions below `W(T)`, zero-extended; a position at or past `W(T)` throws |
| a run-time target | a source | the source's positions at the source's width, as below |
| a type an extension makes a target | a source the extension admits | as the extension specifies |

- *Constraints:* `F` and `T` meet one of the rules, and `remove_reference_t<From>` is not `volatile` unless it is a
  scalar type.
- *Effects:* Where `remove_reference_t<From>` is a `volatile` scalar, reads it once and returns
  `bit_convert<T>(static_cast<F>(from))`. Otherwise, into a run-time target:
  - A sequence target takes the source's *width*: a sequence's size, a window's size, a fixed width's `W`, and a
    set's universe, its whole blocks up to its `max_size()`. A set target covers that width in whole blocks of its
    own, capped at its capacity where it has one. The positions past the source's width are clear.
  - Where `From` is a non-`const` rvalue whose `extract()` yields a container `C`, `bit_constructible_from<T, C>`
    holds, and `T`'s capacity, where it has one, is every bit of `C`, the blocks move: `C` is extracted into the
    `from_blocks` constructor without copying a block, a sequence target then takes the source's width, and the
    source is left at width zero. The extracted container's type carries its allocator, so another allocator is
    another `C`.
  - Otherwise the positions are copied, and the source is unchanged.
- *Returns:* The object of type `T` holding those positions.
- *Throws:*
  - `overflow_error` where `T` is a fixed target, `F` is no fixed width, and a position at or past `W(T)` is set, as
    `bitset::to_ulong` throws in [[bitset.members]](https://eel.is/c++draft/bitset.members);
  - `bad_alloc` where `T` is an inplace sequence whose capacity is short of the source's width, or a set target
    capped at its capacity would lose a key, which is what the target's own growth throws, before a block is
    written;
  - any exception the target's allocator throws.
- *Remarks:* The exception specification is `fixed-width<T> and fixed-width<F>`: nothing throws between two fixed
  widths, and a run-time end can allocate, refuse a capacity or throw `overflow_error`.
- *Remarks:* It is usable in a constant expression. There the copy is made by shifts, and outside one by a single
  copy of the bytes where both block types are built-ins without padding on a little-endian target.
- *Remarks:* `To`'s cv is dropped, as a cast's scalar prvalue drops it. `To` is never a view, which would write bits
  it does not own, nor a built-in array, which no function returns. A `span` of blocks lends its width rather than
  having it, so it is neither end; a `bit_span` over it converts.
- *Remarks:* A `bitset` is read and written through its object on the terms `bit_cast` reads one, so it converts on
  a little-endian target only. Its `to_string()` writes position 0 last, so its text reads reversed against the
  positions written first to last.
- `bit_convert<uint64_t>(bit_fixed_set<64>{0, 5, 63})` is `(1ULL << 63) | (1ULL << 5) | 1`, and
  `bit_convert<uint8_t>(bit_array<5>)` is what `to_ulong` gives for a `bitset<5>` of the same positions.
- `bit_convert<array<uint64_t, 2>>(bit_fixed_set<100>{0, 63, 64, 99})` is `{(1ULL << 63) | 1, (1ULL << 35) | 1}`,
  which `bit_cast` reads from the set's object too.
- `bit_convert<bit_fixed_set<10>>(bit_set{3, 9})` holds `{3, 9}`, and `bit_convert<bit_fixed_set<10>>(bit_set{3,
  10})` throws `overflow_error`. `bit_convert<bitset<0>>(bit_set())` converts, and `bit_set{0}` throws.
- `bit_convert<bit_vector>(bit_set{3, 64, 129})` has the set's whole blocks as its size, 192 over 64-bit blocks, and
  `bit_convert<bit_vector>(bit_fixed_set<100>{99})` has 100. A `bit_vector` of 70 converts to a set over 72 positions
  in `uint8_t` blocks.
- `bit_convert<bit_vector>(std::move(s))` for a `bit_set s` takes `s`'s blocks, and `s` is then empty; from a
  `basic_bit_set<size_t, uint8_t>` it copies, and the source keeps its bits.
