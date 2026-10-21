# `<xstd/bits/bit_algorithm/bit_equal.hpp>`

## `bit_equal`

As `ranges::equal` in [[alg.equal]](https://eel.is/c++draft/alg.equal), with:

- *Constraints:* `R1` and `R2` are of one reading and one width kind:
    - both sequences of ours, or both sets of ours with the same `key_type`, `key_mapping_type` and `key_compare`;
    - the width their types fix, a window's extent where it has one and otherwise its storage's, is the same, or both
      widths are run-time values.

    So `bit_array<8>` compares with `basic_bit_array<uint8_t, 8>` and `bit_vector` with `basic_bit_vector<uint8_t>`,
    and `bit_array<8>` does not compare with `bit_vector`, with `bit_array<9>` or with `bit_fixed_set<8>`, nor
    `bit_fixed_set<8>` with `bit_set` or with a set of the same keys under `greater<>`.
- *Returns:* `ranges::equal(r1, r2)` over the elements in iteration order: for two sequences, whether the sizes and
  every position agree; for two sets, whether they hold the same keys, at any two widths.
- *Complexity:* Two whole operands over one storage type compare their own blocks, in O(`N`/`D`). Any other pair
  compares 64-bit words assembled from each side's blocks at each side's offset, in O(`N1`/`D1` + `N2`/`D2`).
- *Remarks:* The constraints admit exactly the pairs `bit_hash` hashes alike where their values are equal, so what
  compares equal hashes equal. `bit_equal_to` is the transparent comparison over it.
- A capacity of zero against a run-time width that holds bools compares their sizes.
