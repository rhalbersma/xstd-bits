# `bit_hash_append`

- *Constraints:* `Hash` models `hash_algorithm`, and:
  - for the first form, `T` is an owner of ours, of either reading, or a set view. A sequence view, `bit_span` or a
    window, is turned away, as `span` has no hash, though `bit_equal` compares it with its owner;
  - for the second, `const R` models `ranges::contiguous_range` and `ranges::sized_range`, and its value type is a
    type the first form takes whose width the type fixes, or a built-in array or an `array` of such, nested to any
    depth: the planes of a board game's position, as `bit_fixed_set<64>[2][6]` or a `span<const bit_fixed_set<64>>`.
- The *bit string* of `x` is ⌈`N`/8⌉ bytes for its width `N`, byte `j` holding the positions `[8j, 8j + 8)` least
  significant bit first and the bits past `N` zero. For a set of run-time width it stops after the byte that holds
  the last key, so it is empty for an empty set: equal sets need not share a width.
- *Effects:* For the first form:
  - where the type of `x` fixes a width of zero, `boost::hash2::hash_append(h, f, '\x00')`, the one byte Hash2
    writes for an empty `array`;
  - otherwise, updates `h` with the bit string of `x`; then, where the width is a run-time value, an inplace
    capacity included, calls `boost::hash2::hash_append_size(h, f, n)`, where `n` is a sequence's size in bits and a
    set's bit string's length in bytes.

  For the second form:
  - where the type of `r` fixes a count of zero, `boost::hash2::hash_append(h, f, '\x00')`;
  - otherwise appends each element in order, by the first form or, for a nested array, by this one; then, where the
    type of `r` fixes no count, calls `boost::hash2::hash_append_size(h, f, ranges::size(r))`. A built-in array, an
    `array` and a `span` of static extent fix one.
- The block type is no part of the message, and `f` orders the bytes of the appended size alone. Under
  `little_endian_flavor`:

  | `x` | message |
  | :-- | :-- |
  | `basic_bit_fixed_set<size_t, uint64_t, 10>{1, 3, 5, 9}` | bytes `2A 02` |
  | `bit_fixed_set<0>()`, `bit_array<0>()` | byte `00` |
  | `basic_bit_vector<uint64_t>(10)`, its even positions `true` | bytes `55 01`, then the size 10 |
  | `basic_bit_set<size_t, uint64_t>{1, 3, 5}` | byte `2A`, then the size 1 |
  | `basic_bit_set<size_t, uint64_t>{1, 3, 64}` | bytes `0A 00 00 00 00 00 00 00 01`, then the size 9 |
  | an empty `basic_bit_vector<uint64_t>` or `basic_bit_set<size_t, uint64_t>` | the size 0 |
  | `array<bit_fixed_set<64>, 0>()` | byte `00` |

- *Remarks:* Wherever the storage holds the bit string, a little-endian target and a block of a built-in type whose
  value bits fill its object, the string goes in as one `update` over the storage. Elsewhere, a class-type block
  and a constant evaluation included, the same bytes are assembled, up to 256 an `update`. A Hash2 digest does not
  depend on how its message is split into updates, so the digest is the same either way.
- *Remarks:* Where each element of `r` is an owner whose object is its bit string, as `bit_fixed_set<64>` in
  `uint64_t` blocks is, and each nested `array` measures as wide as its elements, the whole range goes in as one
  `update`. A `bit_fixed_set<50>`, eight bytes for a seven-byte string, goes in an element at a time, to the same
  digest.
- *Remarks:* It is usable in a constant expression.
- Where `xstd::ranges::bit_equal(x, y)` is `true`, `x` and `y` append the same message: an owner and its view, two
  owners over different blocks, and a run-time set at any two widths. So what compares equal hashes equal.
