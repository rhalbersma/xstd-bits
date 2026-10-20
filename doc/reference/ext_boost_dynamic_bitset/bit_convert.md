# `bit_convert` with `boost::dynamic_bitset`

As `bit_convert`, with `boost::dynamic_bitset<Block, AllocatorOrContainer>` as either end:

- **As a source**, a `dynamic_bitset` is a sequence of its `size()`, of run-time width, read through
  `boost::to_block_range`, each of its blocks landing on its own positions at any target block width:
  - into a run-time target, as from any run-time sequence: a sequence target takes its size, a set target whole
    blocks of its own, and an inplace target too small throws `bad_alloc`;
  - into a fixed target, by value: zero-extended, and `overflow_error` for a position at or past the target's width.
- **As a target**, `bit_convert<boost::dynamic_bitset<Block, AllocatorOrContainer>>(from)` takes a source of ours
  that is an owner, a whole view or a fixed width, a `bitset` and a block among them. The result is the source's
  width, as a sequence target takes it, written through `boost::from_block_range` in blocks of its own width.
- A window is no source for a `dynamic_bitset`, and one `dynamic_bitset` converts into no other.
- *Throws:* Into a `dynamic_bitset`, any exception its allocation throws. Neither direction is `noexcept`.
- *Remarks:* Boost hands its blocks out nowhere, so both directions copy: an rvalue `dynamic_bitset` source keeps
  its bits, and nothing is adopted even where its container is an owner's own.
- *Remarks:* Since Boost 1.90 the container parameter converts like any other, a `small_vector` of blocks included,
  and both directions are usable in a constant expression.
- *Remarks:* As with `bitset`, its `to_string()` writes position 0 last.
- A `dynamic_bitset<uint8_t>` of 13 with positions `{0, 5, 12}` converts to a `bit_vector` of 13 with them, and
  back to an equal `dynamic_bitset<uint8_t>`. `bit_convert<boost::dynamic_bitset<>>(bit_set{3, 64, 129})` has the
  set's whole blocks as its size, and `bit_convert<boost::dynamic_bitset<uint8_t>>(bit_array<20>())` 20.
