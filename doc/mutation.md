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
cmake -S . -B build/mutate -G Ninja -DCMAKE_CXX_COMPILER=clang++-22 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG=-O0 \
      -DXSTD_BITS_WARNINGS_AS_ERRORS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=~/.local/opt/boost-1.92.0
cmake --build build/mutate
python3 tools/mutate.py $(cd include && find xstd/bits/bit_algorithm -name '*.hpp') --build build/mutate --output mutants.json
```

`--list` prints the mutants without building anything. `--lines`, `--operators` and `--tests` narrow a run to one
site, one kind of change or some of the tests. `--resume` takes the verdicts an earlier `--output` reached and
decides the rest, and `--limit` decides only that many, so a long run can be split. Warnings are off as errors
because a mutant that only trips a warning has not been caught by a test.

Only executables CTest runs with no arguments and the default pass rule are run. The hardened preconditions, a
death test handed the violation to commit, are not.

## Scope

`include/xstd/bits/bit_algorithm/`, 417 mutants. Every dependent test was run against each survivor. The block
kernels under `include/xstd/bits/detail/` follow: each of their headers is included by some 170 tests, and the
portable and BMI2 paths of `intrinsic.hpp` are compiled out on a machine with AVX-512, so they need builds for those
targets as well.

## Equivalent mutants

Each row is a mutant that survived every dependent test because it cannot change what the code computes, and the
reason why. The tool fails on a survivor missing from this table, and on a row whose mutant a test now kills or that
the code no longer contains. A row names its mutant by the text of its line, which of the header's lines with that text it is, and its column
in that text, so an edit to the line, or a new line with the same text above it, brings the mutant back for review.

| header | line | occurrence | column | mutation | reason |
| :--- | :--- | ---: | ---: | :--- | :--- |
| `xstd/bits/bit_algorithm/bit_advance.hpp` | `auto const bound                 = 0 < n ? bits::detail::storage_access::iterated_bits(it).size() : bits::detail::first_key_position(it);` | 1 | 38 | `<` → `<=` | At `n == 0` only, where `advance_keys` returns `n` before it reads the bound. |
| `xstd/bits/bit_algorithm/bit_mismatch.hpp` | `auto const n  = n1 <= n2 ? bits::detail::first_mismatch(storage_access::bits(r1), storage_access::bits(r2), n1) : bits::detail::first_mismatch(storage_access::bits(r2), storage_access::bits(r1), n2);` | 1 | 20 | `<=` → `<` | At `n1 == n2` only, where both arms call `first_mismatch` on storages of one size, which XORs them block by block and so does not depend on their order. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `if (last <= n) {` | 1 | 10 | `<=` → `<` | At `last == n` only, where `count` is zero: the loop runs zero times and `partial_block_mask(0)` clears the word, the zero the early return gives. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 1 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past `last`, both zero, which compare equal. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 2 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past both sizes, both zero, which compare equal. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 3 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past `last`, both zero, which compare equal. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `return p + 1UZ < c.size() and c.any(p + 1UZ, c.size() - p - 1UZ);` | 1 | 12 | `1UZ` → `0UZ` | The guard's new case is `p + 1 == c.size()`, where `c.any(c.size(), 0)` is false, as the guard was. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `return p + 1UZ < c.size() and c.any(p + 1UZ, c.size() - p - 1UZ);` | 1 | 16 | `<` → `<=` | The guard's new case is `p + 1 == c.size()`, where `c.any(c.size(), 0)` is false, as the guard was. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 72 | `-` → `+` | One more word above both sizes, read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 74 | `1UZ` → `0UZ` | One more word above both sizes, read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;` | 1 | 79 | `/` → `*` | Many more words above both sizes, all read as zero on both sides, which compare equal: the order is unchanged, only its cost. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `return xw < yw ? std::strong_ordering::less : std::strong_ordering::greater;` | 1 | 11 | `<` → `<=` | Inside `xw != yw`, where `<` and `<=` agree. |
| `xstd/bits/bit_algorithm/detail/compare.hpp` | `for (auto k = 0UZ; k < n; k += word_digits) {` | 4 | 22 | `<` → `<=` | An extra pass at `k == n`, reached when `n` is a multiple of 64, reads two words at or past both sizes, both zero, which compare equal. |
| `xstd/bits/bit_algorithm/detail/select.hpp` | `if (index == (last - 1UZ) / digits) {` | 1 | 22 | `1UZ` → `0UZ` | Differs only when `last` is a multiple of the block width, and there the mask it skips is `partial_block_mask(digits)`, all ones. |
| `xstd/bits/bit_algorithm/detail/select.hpp` | `for (auto index = (last - 1UZ) / digits, N = index + 1UZ - lo; index - lo < N; --index) {` | 1 | 58 | `-` → `+` | `N` bounds the countdown from above only: for every `index >= lo`, `index - lo` is below both `index + 1 - lo` and `index + 1 + lo`, and below `lo` it wraps and stops the loop either way. |
| `xstd/bits/bit_algorithm/detail/select.hpp` | `auto const m    = static_cast<std::size_t>(n < 0 ? -n : n);` | 1 | 46 | `<` → `<=` | At `n == 0` only, where `-n == n`; `advance_keys` has also returned by then. |
| `xstd/bits/bit_algorithm/detail/select.hpp` | `auto const result = ((0 < n) != descends<typename std::iter_reference_t<I>::adaptor_type>)` | 1 | 25 | `<` → `<=` | At `n == 0` only, and `advance_keys` has returned before this line when `n == 0`. |
| `xstd/bits/bit_algorithm/detail/select.hpp` | `return n < 0 ? -rest : rest;` | 1 | 10 | `<` → `<=` | At `n == 0` only, and `advance_keys` has returned before this line when `n == 0`. |
