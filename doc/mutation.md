# Mutation testing

Coverage shows every line runs and the audits show every paragraph is cited; neither shows that a test would fail if
the code were wrong. [`tools/mutate.py`](../tools/mutate.py) measures that. It changes one operator, literal or call
at one site of one header, builds the tests that depend on that header against the change, and runs them, the
header's own mirrored test first. A test that fails, times out, or rejects the change at compile time kills the
mutant. A mutant every dependent test passes survived: either a test is missing, or the change cannot alter what
the code does. This page records the second kind.

## Running it

The tool reads a configured and built Ninja tree of this repository, with `BUILD_TESTING` and
`CMAKE_EXPORT_COMPILE_COMMANDS` on, and leaves the checkout untouched: each mutant is compiled into a private copy of
`include/` with the build's own compile and link commands.

```sh
cmake -S . -B build/mutate -G Ninja -DCMAKE_CXX_COMPILER=clang++-22 -DCMAKE_CXX_FLAGS=--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15 \
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG=-O0 -DXSTD_BITS_WARNINGS_AS_ERRORS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/mutate
python3 tools/mutate.py $(cd include && find xstd/bits/bit_algorithm xstd/bits/bit_iterator -name '*.hpp') --build build/mutate --output mutants.json
```

`--list` prints the mutants without building anything. `--lines`, `--operators` and `--tests` narrow a run to one
site, one kind of change or some of the tests. `--resume` takes the verdicts an earlier `--output` reached and
decides the rest, and `--limit` decides only that many, so a long run can be split. `--max-targets` stops a mutant after
that many of the targets that depend on its header, the likeliest to catch it first. A survivor records how many it
passed, and a later `--resume` run with a higher cap, or none, widens it from there rather than from the start. Warnings are off as errors
because a mutant that only trips a warning has not been caught by a test.

Only executables CTest runs with no arguments and the default pass rule are run. The hardened preconditions, a
death test handed the violation to commit, are not.

## Scope

`include/xstd/bits/bit_algorithm/` and `include/xstd/bits/bit_iterator/`, 417 mutants. Every dependent test was run
against each survivor.

The block kernels under `include/xstd/bits/detail/`, 351 mutants over 24 headers: the gathers, scatters and masks the
valarray's subscripts and the `[bit.permute]` functions build on, the block walks, shifts and intrinsics, and the
comparisons. [`tools/mutate-kernels.sh`](../tools/mutate-kernels.sh) runs them as the weekly workflow does. Two
things set them apart from the folders above:

- **They branch on the instruction set.** `intrinsic.hpp` expands a block into bools with one AVX-512BW store, a PDEP
  per byte where PDEP is fast, or a table, and `compressed.hpp` and `expanded.hpp` skip a full mask only where no
  instruction permutes bits. A build for the machine at hand compiles in one of those paths, so the kernels are built
  with `XSTD_BITS_TEST_ARCH` set to a level instead: `x86-64-v3` compiles in every path but the portable one, and
  `x86-64` compiles in that one. The AVX-512 store has no operator to mutate.
- **Each is run against the tests that call it.** A kernel header reaches some 200 tests, most of which only include
  it; run against all of them, a survivor costs hours. Each group of headers is run against the tests that killed one
  of its mutants, so a survivor there is a gap in those tests or a mutant listed below.

`bit_block_container.hpp`, the storage under every container, follows with 650 mutants.

## Equivalent mutants

Each row is a mutant that survived every dependent test because it cannot change what the code computes, and the
reason why. The tool fails on a survivor missing from this table, and on a row whose mutant a test now kills or that
the code no longer contains. A row names its mutant by the text of its line, which of the header's lines with that text it is, and its column
in that text, so an edit to the line, or a new line with the same text above it, brings the mutant back for review.

| header | line | occurrence | column | mutation | reason |
| :--- | :--- | ---: | ---: | :--- | :--- |
| `xstd/bits/bit_iterator/detail/bit_advance.hpp` | `auto const bound                 = 0 < n ? bits::detail::storage_access::iterated_bits(it).size() : bits::detail::first_key_position(it);` | 1 | 38 | `<` → `<=` | At `n == 0` only, where `advance_keys` returns `n` before it reads the bound. |
| `xstd/bits/bit_algorithm/bit_mismatch.hpp` | `auto const n  = n1 <= n2 ? bits::detail::first_mismatch(storage_access::bits(r1), storage_access::bits(r2), n1) : bits::detail::first_mismatch(storage_access::bits(r2), storage_access::bits(r1), n2);` | 1 | 20 | `<=` → `<` | At `n1 == n2` only, where both arms call `first_mismatch` on storages of one size, which XORs them block by block and so does not depend on their order. |
| `xstd/bits/detail/storage_compare.hpp` | `if (last <= n) {` | 1 | 10 | `<=` → `<` | At `last == n` only, where `count` is zero: the loop runs zero times and `partial_block_mask(0)` clears the word, the zero the early return gives. |
| `xstd/bits/detail/storage_compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 1 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past `last`, both zero, which compare equal. |
| `xstd/bits/detail/storage_compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 2 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past both sizes, both zero, which compare equal. |
| `xstd/bits/detail/storage_compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 3 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past `last`, both zero, which compare equal. |
| `xstd/bits/detail/storage_compare.hpp` | `return p + 1UZ < c.size() and c.any(p + 1UZ, c.size() - p - 1UZ);` | 1 | 12 | `1UZ` → `0UZ` | The guard's new case is `p + 1 == c.size()`, where `c.any(c.size(), 0)` is false, as the guard was. |
| `xstd/bits/detail/storage_compare.hpp` | `return p + 1UZ < c.size() and c.any(p + 1UZ, c.size() - p - 1UZ);` | 1 | 16 | `<` → `<=` | The guard's new case is `p + 1 == c.size()`, where `c.any(c.size(), 0)` is false, as the guard was. |
| `xstd/bits/detail/storage_compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 72 | `-` → `+` | One more word above both sizes, read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/detail/storage_compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 74 | `1UZ` → `0UZ` | One more word above both sizes, read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/detail/storage_compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 79 | `/` → `*` | Many more words above both sizes, all read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/detail/storage_compare.hpp` | `return xw < yw ? std::strong_ordering::less : std::strong_ordering::greater;` | 1 | 11 | `<` → `<=` | Inside `xw != yw`, where `<` and `<=` agree. |
| `xstd/bits/detail/storage_compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 4 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past both sizes, both zero, which compare equal. |
| `xstd/bits/bit_iterator/detail/select.hpp` | `if (index == (last - 1UZ) / digits) {` | 1 | 22 | `1UZ` → `0UZ` | Differs only when `last` is a multiple of the block width, and there the mask it skips is `partial_block_mask(digits)`, all ones. |
| `xstd/bits/bit_iterator/detail/select.hpp` | `for (auto index = (last - 1UZ) / digits, N = index + 1UZ - lo; index - lo < N; --index) {` | 1 | 58 | `-` → `+` | `N` bounds the countdown from above only: for every `index >= lo`, `index - lo` is below both `index + 1 - lo` and `index + 1 + lo`, and below `lo` it wraps and stops the loop either way. |
| `xstd/bits/bit_iterator/detail/select.hpp` | `auto const m    = static_cast<std::size_t>(n < 0 ? -n : n);` | 1 | 46 | `<` → `<=` | At `n == 0` only, where `-n == n`; `advance_keys` has also returned by then. |
| `xstd/bits/bit_iterator/detail/select.hpp` | `auto const result = ((0 < n) != descends<typename std::iter_reference_t<I>::adaptor_type>)` | 1 | 25 | `<` → `<=` | At `n == 0` only, and `advance_keys` has returned before this line when `n == 0`. |
| `xstd/bits/bit_iterator/detail/select.hpp` | `return n < 0 ? -rest : rest;` | 1 | 10 | `<` → `<=` | At `n == 0` only, and `advance_keys` has returned before this line when `n == 0`. |
| `xstd/bits/detail/word_masks.hpp` | `if constexpr (digits <= word) {` | 1 | 22 | `<=` → `<` | At a 64-bit block only, where the wider-block loop runs once over the one word the first branch shifts by zero, at an index always below `m_words.size()`. |
| `xstd/bits/detail/slice_masks.hpp` | `, m_spans_end(m_first + 1UZ + (((m_last - m_first - std::ranges::min(m_last - m_first, 2UZ)) / m_span) * m_span))` | 1 | 25 | `1UZ` → `0UZ` | The period loop steps by `m_span` from `m_first + 1`, and `m_span` is at least 2 on that branch, so a bound one lower stops it at the same block. |
| `xstd/bits/detail/slice_masks.hpp` | `, m_spans_end(m_first + 1UZ + (((m_last - m_first - std::ranges::min(m_last - m_first, 2UZ)) / m_span) * m_span))` | 1 | 77 | `-` → `+` | `min` gives 2 for `m_last + m_first` as for `m_last - m_first` once the latter is 2 or more, and at 1 `for_each` returns after the first block before it reads `m_spans_end`. |
| `xstd/bits/detail/intrinsic.hpp` | `for (auto const byte : std::views::iota(0UZ, table.size())) {` | 1 | 41 | `0UZ` → `1UZ` | Byte 0's row is all zeros, the value the table starts with. |
