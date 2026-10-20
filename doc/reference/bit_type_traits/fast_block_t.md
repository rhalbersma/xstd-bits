# `fast_block_t`, `bit_fast`

- `fast_block_t<N>` is the fastest fixed-width block of at least `N` bits in one, by the thresholds of
  `least_block_t`: `uint_fast8_t` where `N <= 8`, `uint_fast16_t` where `N <= 16`, `uint_fast32_t` where `N <= 32`,
  and `uint_fast64_t` otherwise.
- `bit_fast<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a resizable owner.
  - *Result:* `bit_rebind<fast_block_t<N>, Owner>`, where `N` is `Owner`'s width.
  - `bit_fast<basic_bit_array<uint64_t, 9>>` is `basic_bit_array<uint_fast16_t, 9>`. `bit_set` is turned away.
- *Remarks:* The platform decides how wide each `uint_fastN_t` is, so `bit_fast` names a block and promises no size.
