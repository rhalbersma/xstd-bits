# Gate scorecard

This page holds the two ledgers of [#456](https://github.com/rhalbersma/xstd-bits/issues/456). The bug ledger lists
every bug this repository's history records, the check that found it first, and the cheapest check that would have
found it sooner. The gate ledger lists what each CI workflow costs and what it caught. The bug ledger draws on the
issues labelled bug or upstream report, the bugs pull request descriptions and commits record in passing, the pull
requests from #28 to #468, and seven entries recorded on 17 and 18 October 2026, four of which the harvests already
held. That is 219 rows: 238 harvested, 22 of them the same bug found twice, and three new entries. The gate ledger
covers the last 50 pull-request runs of each workflow, from 2026-10-14 16:14 to 2026-10-18 21:14 UTC.

## The bug ledger

One row per bug, grouped by what was wrong: the library, its tests, its benchmarks, its documentation, a gate, the
CI configuration, or a toolchain or dependency upstream. Within a group the rows run in the order they were found.

- **introduced** is the commit that brought the bug in, where the history pins it. *Inside #N, before merge* is a
  bug a pull request introduced and fixed before it merged. *Not traced* means no harvest followed it back.
- **found** is the pull request, issue or commit where the bug was first recorded. A pull request without a date in
  the record carries the date it merged.
- **found by** is the first check that showed it, in the vocabulary below.
- **cheapest check** is a proposal: an existing gate, or one that does not exist yet. A claim counts once it is
  replayed: check out the fix's parent, run the check and show it fails, then show it passes on the fix. 44 claims
  have been replayed, and each cell says whether its claim was and how it came out.
- **how much earlier** is the time from the introducing commit to the commit that records the finding, the most a
  check run at introduction could have saved. Where the bug was found inside a pull request that merged later, the
  finding is dated by that merge, so the figure is an upper bound. It is measured only where both ends are commits
  with dates; elsewhere it is `—`.

| found by | meaning |
| :--- | :--- |
| `msvc`, `clang_cl`, `apple_clang`, `clang_libcxx`, `clang`, `gcc`, `mingw`, `msvc_analyze`, `clang_tidy`, `coverage`, `sanitizers`, `fuzz`, `consumption`, `audit`, `scorecard` | the CI job family of that name, the first leg to go red |
| `cpp-ci scan` | the Windows stack-ABI scan in the shared cpp-ci workflows |
| `ci (unattributed)` | a CI run whose record names no single first leg |
| `probe workflow` | a one-off diagnostic workflow, not a required gate |
| `local run` | an existing check run on a developer's machine: a build, a sanitizer, clang-tidy, a script, a timing |
| `new instantiation` | a new test, harness or type, compiled or run for the first time |
| `reviewer` | a person reading the code, the documentation or a workflow, including the author and external review |
| `benchmark` | a benchmark or a look at generated code |
| `mutation testing` | a mutant that survived |
| `downstream user` | a consumer of this repository or of cpp-ci |
| `upstream report` | a report from a toolchain or dependency maintainer |
| `unknown` | the record does not say |

Cheapest-check cells use these names for checks that do not exist yet. A **type-matrix compile table** instantiates
every public template over the types users pass: fundamental, class-type, bit-precise and 128-bit blocks, cv-qualified
blocks, spans, views and owners, standard and pmr allocators. A **pin check** asserts that every pin is a tagged commit
reachable from its repository's default branch, and that the workflows pin what `CMakeLists.txt` pins. A **gate
self-test** feeds a gate an input it must reject and fails when the gate passes it.

### Library

103 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| `std::bitset` fast-path probe uses `.` on a pointer; the fast path never runs [6cfc2d5](https://github.com/rhalbersma/xstd-bits/commit/6cfc2d5) | library | `08ebeaf` (2024-01-15) | `6cfc2d5` (2024-01-15) | unknown | a static_assert that the requires-expression holds for `std::bitset`; unreplayed | < 1 h |
| Proxy reference converts only to `int`, so copies into class-type containers need two conversions [#3](https://github.com/rhalbersma/xstd-bits/issues/3) | library | `0f27891` (2018-05-21) | issue #3 (2018-06-18) | reviewer | type-matrix compile table (copies into a container of an int-constructible class); replayed: fails at c14d931, passes at b9f072d | — |
| `operator<=>` for Boost `flat_set` declared for `int` keys only [#23](https://github.com/rhalbersma/xstd-bits/issues/23) | library | `21af348` (2020-05-02) | issue #23 (2024-01-18) | unknown | type-matrix compile table (key types other than `int`); replayed: fails at 444b62e, passes at f729875 | — |
| Vendored `xstd/utility.hpp` shares xstd's include guard and namespace [#26](https://github.com/rhalbersma/xstd-bits/issues/26) | library | not traced | issue #26 | downstream user | the consumption job, run locally (installed beside xstd); unreplayed | — |
| `bit::array::operator<=>` matches set ordering only at equal cardinality [#30](https://github.com/rhalbersma/xstd-bits/pull/30) | library | `529dbba` (2025-02-26) | #30 (merged 2026-07-20) | local run | differential test against the standard model (against `std::set<int>`); unreplayed | 509 d |
| Friend template-ids name templates declared later; Clang rejects [#30](https://github.com/rhalbersma/xstd-bits/pull/30) | library | not traced | #30 (merged 2026-07-20) | clang | compile on every local compiler (`tools/ci-local.sh`); replayed: fails at 146d985, passes at 146d985 | — |
| Single-block shifts narrow `int` back to the block implicitly [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | library | `4c648fb` (#33, 2026-07-21) | #45 (merged 2026-08-28) | sanitizers | an implicit-conversion sanitizer leg run locally; unreplayed | 38 d |
| `all()` uses else-if on constants: C4127 under `/W4 /WX` [#59](https://github.com/rhalbersma/xstd-bits/pull/59) | library | not traced | #59 (merged 2026-08-30) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| `bitset` extractor peeks twice; a short read sets `failbit` [#60](https://github.com/rhalbersma/xstd-bits/pull/60) | library | `f3a444c` (2022-09-03) | #60 (merged 2026-08-30) | coverage | differential test against the standard model (against `std::bitset`'s extractor); unreplayed | 1,456 d |
| Draft `set_ops` matched `dynamic_bitset::clear()` and destroyed the container [#65](https://github.com/rhalbersma/xstd-bits/pull/65) | library | inside #65, before merge | #65 (merged 2026-09-02) | local run | the local run that found it; none cheaper; unreplayed | — |
| `set_range` constraint needs a complete type; self-referential containers recurse [#66](https://github.com/rhalbersma/xstd-bits/pull/66) | library | `9017bd3` (#65, 2026-09-02) | #66 (merged 2026-09-02) | new instantiation | type-matrix compile table; replayed: fails at 9017bd3, passes at d1f8940 | 2 h |
| Test CMake hardcodes `-Werror`; subproject consumers inherit it [#72](https://github.com/rhalbersma/xstd-bits/issues/72) | library | not traced | #73 (merged 2026-09-03) | reviewer | the consumption job, run locally (as a subproject); unreplayed | — |
| `${CMAKE_PROJECT_NAME}` at three link sites [#78](https://github.com/rhalbersma/xstd-bits/issues/78) | library | not traced | issue #78/#89 | reviewer | the consumption job, run locally; unreplayed | — |
| `set_ops::contains` is `noexcept` over a throwing `std::bitset::test` [#79](https://github.com/rhalbersma/xstd-bits/pull/79) | library | `9017bd3` (#65, 2026-09-02) | #79 (merged 2026-09-04) | clang_tidy | clang-tidy `bugprone-exception-escape` over the test TUs; unreplayed | 2 d |
| `set_ops::insert`/`erase` and `array_ops::assign` are `noexcept` over a throwing setter [#79](https://github.com/rhalbersma/xstd-bits/pull/79) | library | `9017bd3` (#65, 2026-09-02) | #79 (merged 2026-09-04) | clang_tidy | clang-tidy `bugprone-exception-escape` over the test TUs; unreplayed | 2 d |
| `HOMEPAGE_URL` uses `${PROJECT_NAME}` before it is set [#81](https://github.com/rhalbersma/xstd-bits/pull/81) | library | `7722b29` (2019-12-23) | #81 (merged 2026-09-04) | reviewer | a check on the installed package metadata; unreplayed | 2,447 d |
| `bit_finite_set` lookups are UB for keys `>= N` [#86](https://github.com/rhalbersma/xstd-bits/issues/86) | library | not traced | issue #86 (2026-09-04) | reviewer | differential fuzzing; unreplayed | — |
| `find_next` past the end skips the bound [#88](https://github.com/rhalbersma/xstd-bits/pull/88) | library | not traced | #88 (merged 2026-09-05) | reviewer | differential test against the standard model (arguments past the end); unreplayed | — |
| Dropping a helper also drops `<iterator>`; libstdc++ masks it [#88](https://github.com/rhalbersma/xstd-bits/pull/88) | library | inside #88, before merge | #88 (merged 2026-09-05) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | — |
| Reverse iteration over `std::bitset` is quadratic on MSVC [#95](https://github.com/rhalbersma/xstd-bits/pull/95) | library | not traced | #95 (merged 2026-09-05) | reviewer | an operation-count test on MSVC STL; unreplayed | — |
| Install file set misses four new headers [#98](https://github.com/rhalbersma/xstd-bits/pull/98) | library | inside #98, before merge | #98 (merged 2026-09-06) | ci (unattributed) | the consumption job, run locally; unreplayed | — |
| Defaulted `==` compares blocks before width [#100](https://github.com/rhalbersma/xstd-bits/pull/100) | library | `9e97313` (#99, 2026-09-06) | #100 (merged 2026-09-06) | benchmark | an instruction-count check on the hot paths; unreplayed | 2 h |
| `bitset` extractor places a short read at the high positions [#103](https://github.com/rhalbersma/xstd-bits/pull/103) | library | not traced | #103 (merged 2026-09-07) | reviewer | differential test against the standard model (against `std::bitset`'s extractor); unreplayed | — |
| Set adaptor's `hash_append` hook is unreachable [#103](https://github.com/rhalbersma/xstd-bits/pull/103) | library | not traced | #103 (merged 2026-09-07) | reviewer | a test that the hook is reached; unreplayed | — |
| `max_size()` gives three answers over one storage [#111](https://github.com/rhalbersma/xstd-bits/pull/111) | library | `3fe57ba` (2026-09-07) | #111 (merged 2026-09-07) | new instantiation | type-matrix compile table; on replay, reclassified as an agreement check across readings (a run of the cell, or its static_assert), not a compile table | 5 h |
| Width member leaves padding before over-aligned blocks (`-Wpadded`) [#111](https://github.com/rhalbersma/xstd-bits/pull/111) | library | `31c1a6b` (#102, 2026-09-06) | #111 (merged 2026-09-07) | local run | the local run that found it; none cheaper; unreplayed | 1 d |
| `set_adaptor` has no `get_allocator`; `bitset_adaptor` no `swap` [#116](https://github.com/rhalbersma/xstd-bits/issues/116) | library | not traced | #120 (merged 2026-09-08) | reviewer | type-matrix compile table (member surface per counterpart); replayed: fails at 4f82c68, passes at a38f7d1 | — |
| `generate_candidates` calls `iota(2, n)` for `n < 2` [#118](https://github.com/rhalbersma/xstd-bits/issues/118) | library | `d90208f` (2024-01-17) | #122 (merged 2026-09-08) | new instantiation | boundary tests below the first prime; unreplayed | 965 d |
| `for_each` hands a named lvalue; mutating functors write to a discarded local [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | library | `865b9e6` (#125, 2026-09-08) | #132 (merged 2026-09-10) | reviewer | type-matrix compile table (functors taking references); replayed: fails at 0e8682c, passes at ded4d46 | 2 d |
| ADL `begin`/`end` make `xstd::bitset` a range [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | library | not traced | #132 (merged 2026-09-10) | reviewer | type-matrix compile table; replayed: fails at 0e8682c, passes at ded4d46 | — |
| Twenty-odd constraints test a literal `0UZ`, not the argument [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | library | not traced | #132 (merged 2026-09-10) | reviewer | type-matrix compile table; replayed: fails at 0e8682c, passes at ded4d46 | — |
| `block_storage` omits the range subscript requirement [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | library | `a659b82` (#83, 2026-09-04) | #132 (merged 2026-09-10) | reviewer | type-matrix compile table; withdrawn on replay: no existing type falls in the gap, and only an archetype, which the tests exclude, would | 6 d |
| `block_sequence` has only a member swap; `ranges::swap` falls back to moves [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | library | `a659b82` (#83, 2026-09-04) | #132 (merged 2026-09-10) | reviewer | a swap-and-move counting test; unreplayed | 6 d |
| `detail/intrin` constrained wider than its implementation [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | library | `d1f8940` (#66, 2026-09-02) | #141 (merged 2026-09-12) | new instantiation | type-matrix compile table; replayed: fails at f62bf19, passes at 9d816f7 | 9 d |
| `pred`'s `intersects` needs an implicit conversion to `bool` [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | library | not traced | #141 (merged 2026-09-12) | new instantiation | type-matrix compile table; replayed: fails at f62bf19, passes at 9d816f7 | — |
| Sequence proxy converts to any class constructible from `bool`; comparisons ambiguous [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | library | not traced | #141 (merged 2026-09-12) | new instantiation | type-matrix compile table; replayed: fails at f62bf19, passes at 9d816f7 | — |
| Block shifts pass `size_t` counts to integer classes taking `int` [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | library | not traced | #141 (merged 2026-09-12) | new instantiation | type-matrix compile table; replayed: fails at f62bf19, passes at 9d816f7 | — |
| Exact set-proxy comparisons break `ranges::equal` across proxies [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | library | inside #141, before merge | #141 (merged 2026-09-12) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Trait removal makes `bitset_adaptor::find_prev` assert past the width [#151](https://github.com/rhalbersma/xstd-bits/pull/151) | library | inside #151, before merge | #151 (merged 2026-09-14) | local run | the local run that found it; none cheaper; unreplayed | — |
| Sequence `<<=`/`>>=` shift in the reverse direction [#161](https://github.com/rhalbersma/xstd-bits/pull/161) | library | not traced | #161 (merged 2026-09-15) | reviewer | differential test against the standard model (shifts against `std::bitset`); unreplayed | — |
| `operator~` on a run-time-width set is not a function of its value [#162](https://github.com/rhalbersma/xstd-bits/pull/162) | library | not traced | #162 (merged 2026-09-15) | reviewer | a value-semantics law test; unreplayed | — |
| `block_writable` asks the const-stripped type; const windows declare `&=` [#164](https://github.com/rhalbersma/xstd-bits/pull/164) | library | `bed313d` (#110, 2026-09-07) | #164 (merged 2026-09-15) | reviewer | type-matrix compile table (const windows); replayed: fails at 062f16f, passes at 719f50a | 8 d |
| Growth widths computed by addition wrap near `SIZE_MAX` [#165](https://github.com/rhalbersma/xstd-bits/pull/165) | library | not traced | #165 (merged 2026-09-16) | reviewer | boundary tests at `SIZE_MAX`; unreplayed | — |
| `guard_range` tests `pos + len`, which wraps [#165](https://github.com/rhalbersma/xstd-bits/pull/165) | library | `3fe57ba` (#109, 2026-09-07) | #165 (merged 2026-09-16) | reviewer | boundary tests at `SIZE_MAX`; unreplayed | 9 d |
| `blocks_for` wraps to zero blocks near `SIZE_MAX` [#165](https://github.com/rhalbersma/xstd-bits/pull/165) | library | `a659b82` (#83, 2026-09-04) | #165 (merged 2026-09-16) | reviewer | boundary tests at `SIZE_MAX`; unreplayed | 11 d |
| `find_next(pos)` past the width overflows the heap buffer [#167](https://github.com/rhalbersma/xstd-bits/pull/167) | library | not traced | #167 (merged 2026-09-16) | local run | differential fuzzing; unreplayed | — |
| `complement(x)` writes through a block a dynamic set lacks [#167](https://github.com/rhalbersma/xstd-bits/pull/167) | library | not traced | #167 (merged 2026-09-16) | reviewer | differential fuzzing; unreplayed | — |
| `pack`'s size sum wraps for an `iota(0, SIZE_MAX)` source [#167](https://github.com/rhalbersma/xstd-bits/pull/167) | library | `bed313d` (#110, 2026-09-07) | #167 (merged 2026-09-16) | reviewer | boundary tests at `SIZE_MAX`; unreplayed | 9 d |
| `max_size` disagrees with Boost and `vector<bool>` [#167](https://github.com/rhalbersma/xstd-bits/pull/167) | library | `4a7dadb` (#112, 2026-09-07) | #167 (merged 2026-09-16) | local run | differential test against the standard model (`max_size` against the counterparts); unreplayed | 9 d |
| `resize_to` dirties the tail before a refused growth [#167](https://github.com/rhalbersma/xstd-bits/pull/167) | library | `31c1a6b` (#102, 2026-09-06) | #167 (merged 2026-09-16) | new instantiation | refusing-allocator harness; unreplayed | 10 d |
| Set reading's conversion reaches `m_bits`, a pointer in a view [#171](https://github.com/rhalbersma/xstd-bits/pull/171) | library | `36c3226` (#170, 2026-09-17) | #171 (merged 2026-09-18) | reviewer | type-matrix compile table (views converted); replayed: fails at 36c3226, passes at a20d8f3 | 12 h |
| Non-`constexpr` `operator\|=` turns a layout probe into a hard error [#171](https://github.com/rhalbersma/xstd-bits/pull/171) | library | inside #171, before merge | #171 (merged 2026-09-18) | ci (unattributed) | type-matrix compile table (`absl::uint128` column); replayed: fails at 8998c0a, passes at cf92150 | — |
| Block sequences give `bit_static_set<256>(words)` two meanings [#172](https://github.com/rhalbersma/xstd-bits/pull/172) | library | `f473b7f` (#172, 2026-09-18) | #172 (merged 2026-09-18) | reviewer | type-matrix compile table (constructor overloads per argument shape); withdrawn on replay: both spellings compile and run correctly at the parent; only review sees two meanings | — |
| An assert-only count fails every Release rung under `NDEBUG` [#180](https://github.com/rhalbersma/xstd-bits/pull/180) | library | inside #180, before merge | #180 (merged 2026-09-19) | ci (unattributed) | compile on every local compiler (`tools/ci-local.sh`) (Release build); replayed: fails at c8455d2, passes at 73b427d | — |
| Eight places offer less than the counterpart [#183](https://github.com/rhalbersma/xstd-bits/pull/183) | library | not traced | #183 (merged 2026-09-19) | reviewer | type-matrix compile table (member surface per counterpart); replayed: fails at 40a6a03, passes at 9082238 | — |
| `erase_unused` masks the last block on every call [#184](https://github.com/rhalbersma/xstd-bits/pull/184) | library | not traced | #184 (merged 2026-09-19) | benchmark | the check that found it; none cheaper; unreplayed | — |
| Value-returning operations return the base, not the owner [#206](https://github.com/rhalbersma/xstd-bits/pull/206) | library | not traced | #206 (merged 2026-09-21) | reviewer | type-matrix compile table (return type per owner); withdrawn on replay: every owner is an alias of its vehicle at the parent, so no check there can fail | — |
| Concept-constrained `std::` trait specializations break libc++'s `<tuple>` [#206](https://github.com/rhalbersma/xstd-bits/pull/206) | library | inside #206, before merge | #206 (merged 2026-09-21) | local run | a local libc++ leg (`tools/ci-local.sh libcxx-22`); unreplayed | — |
| The library's own swap goes unused; a fallback exchanges values [#213](https://github.com/rhalbersma/xstd-bits/pull/213) | library | not traced | #213 (merged 2026-09-22) | reviewer | a swap-counting test; unreplayed | — |
| `Boost::container` linked without `find_dependency` [#214](https://github.com/rhalbersma/xstd-bits/pull/214) | library | inside #214, before merge | #214 (merged 2026-09-22) | consumption | the consumption job, run locally; unreplayed | — |
| `bit_array.hpp` specializes tuple traits without `<tuple>` [#215](https://github.com/rhalbersma/xstd-bits/pull/215) | library | inside #215, before merge | #215 | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | — |
| Implicit moves copy the width and move the blocks; moved-from owner reports its old size [#220](https://github.com/rhalbersma/xstd-bits/pull/220) | library | `a659b82` (#83, 2026-09-04) | #220 (2026-09-23) | unknown | a moved-from-state test under ASan/UBSan; unreplayed | 18 d |
| Public owner constraint admits spans and const blocks that break inside [#232](https://github.com/rhalbersma/xstd-bits/issues/232) | library | not traced | #254 (merged 2026-09-26) | reviewer | type-matrix compile table (spans, const blocks, `dynamic_extent`); replayed: fails at 7be0005, passes at 03f4ed8 | — |
| Run-time-width vehicle needs a count constructor no concept requires [#254](https://github.com/rhalbersma/xstd-bits/pull/254) | library | `a659b82` (#83, 2026-09-04) | #254 (merged 2026-09-26) | new instantiation | type-matrix compile table (minimal storage); replayed: fails at 7be0005, passes at 03f4ed8 | 21 d |
| Allocator parameters as `same_as` templates refuse conversions [#258](https://github.com/rhalbersma/xstd-bits/issues/258) | library | not traced | #259 (merged 2026-09-26) | reviewer | type-matrix compile table (pmr, rebound allocators, `{}`); replayed: fails at 8e16e5e, passes at 9116067 | — |
| `basic_bitset` text constructor checks only the first N characters [#271](https://github.com/rhalbersma/xstd-bits/pull/271) | library | not traced | #271 (merged 2026-09-27) | new instantiation | differential test against the standard model (against `std::bitset`'s text constructor); unreplayed | — |
| Appending a whole last word overshoots a bounded owner's capacity [#274](https://github.com/rhalbersma/xstd-bits/pull/274) | library | `c18be3f` (#256, 2026-09-26) | #274 (2026-09-28) | new instantiation | differential test against the standard model (bounded owners against `std::inplace_vector`); unreplayed | 2 d |
| Bounded set's left shift refused by its width, not its highest key [#274](https://github.com/rhalbersma/xstd-bits/pull/274) | library | #274 | #274 (2026-09-28) | new instantiation | differential test against the standard model (bounded owners against `std::inplace_vector`); unreplayed | — |
| Bounded owner's unsized `append_range` leaves packed words when capacity throws [#274](https://github.com/rhalbersma/xstd-bits/pull/274) | library | `bed313d` (#110, 2026-09-07) | #274 (2026-09-28) | new instantiation | differential test against the standard model (no-effects check on capacity overflow); unreplayed | 20 d |
| `basic_bit_bounded_vector<B, 0>` neither empty nor trivially copyable [#282](https://github.com/rhalbersma/xstd-bits/pull/282) | library | not traced | #282 (merged 2026-09-29) | audit | type-matrix compile table (capacity 0); on replay, reclassified as a clause assertion at capacity 0 ([inplace.vector.overview]/5 traits), not a compile table | — |
| Bounded and small owners miss the draft's `noexcept` [#281](https://github.com/rhalbersma/xstd-bits/issues/281) | library | not traced | #282 (merged 2026-09-29) | audit | type-matrix compile table (`noexcept` against the draft); replayed: fails at beb1d53, passes at 96aa8f1 | — |
| `bit_members` as a struct makes every `std::array` width structural [#283](https://github.com/rhalbersma/xstd-bits/issues/283) | library | `aa5fc55` (#282, 2026-09-29) | #288 (merged 2026-09-30) | reviewer | type-matrix compile table (structural at unaligned widths); on replay, reclassified as a structural assertion on the detail vehicle (bit_container), not the owner/view table | 19 h |
| `sequence_adaptor` rebuild uses a default-constructed allocator [#267](https://github.com/rhalbersma/xstd-bits/issues/267) | library | `bed313d` (#110, 2026-09-07) | #295 (2026-10-01) | new instantiation | refusing-allocator harness (found by it); unreplayed | 24 d |
| Rebuild's swap of inline and heap blocks can allocate after the sizes are exchanged [#267](https://github.com/rhalbersma/xstd-bits/issues/267) | library | `bed313d` (#110, 2026-09-07) | #295 (2026-10-01) | new instantiation | refusing-allocator harness (found by it); unreplayed | 24 d |
| `append_range` from an unsized range keeps a partial append on allocation failure [#267](https://github.com/rhalbersma/xstd-bits/issues/267) | library | `bed313d` (#110, 2026-09-07) | #295 (2026-10-01) | new instantiation | refusing-allocator harness (found by it); unreplayed | 24 d |
| Move assignment between unequal allocators leaves empty blocks under the old width [#295](https://github.com/rhalbersma/xstd-bits/pull/295) | library | not traced | #295 (2026-10-01) | ci (unattributed) | refusing-allocator harness (on a local libc++ leg); unreplayed | — |
| Static-width set shifts by `n >= N` are UB [#268](https://github.com/rhalbersma/xstd-bits/issues/268) | library | not traced | #296 (merged 2026-10-02) | local run | differential fuzzing (found by it); unreplayed | — |
| Sequence erase goes through an allocating rebuild and can throw [#267](https://github.com/rhalbersma/xstd-bits/issues/267) | library | `bed313d` (#110, 2026-09-07) | #306 (2026-10-02) | new instantiation | refusing-allocator harness; unreplayed | 25 d |
| Copy assignment assigns the width before the blocks; a throwing copy breaks the basic guarantee [#309](https://github.com/rhalbersma/xstd-bits/issues/309) | library | `39f1e03` (#100, 2026-09-06) | issue #309 (2026-10-02) | unknown | refusing-allocator harness (with an lvalue copy-assignment case); unreplayed | — |
| Single-pass `append` keeps appended blocks on refusal [#310](https://github.com/rhalbersma/xstd-bits/issues/310) | library | `31c1a6b` (#102, 2026-09-06) | issue #310 | reviewer | refusing-allocator harness; unreplayed | — |
| Set proxy comparisons fail over 128-bit class blocks [#340](https://github.com/rhalbersma/xstd-bits/pull/340) | library | `9d816f7` (#141, 2026-09-12) | #340 (merged 2026-10-05) | reviewer | type-matrix compile table (128-bit blocks); replayed: fails at 6cb437d, passes at bd5079e | 23 d |
| Proxies over user types let a catch-all `operator==` hijack `r == r` [#343](https://github.com/rhalbersma/xstd-bits/pull/343) | library | not traced | #343 (merged 2026-10-05) | reviewer | none proposed | — |
| Signed flag enumeration with a sign-bit flag is not order-preserving [#347](https://github.com/rhalbersma/xstd-bits/pull/347) | library | #347 | #347 (merged 2026-10-06) | new instantiation | type-matrix compile table (signed flag enumerations); on replay, reclassified as a run of the cell, not a compile table | — |
| `lower_bound` of a non-key returns `end()` [#353](https://github.com/rhalbersma/xstd-bits/pull/353) | library | not traced | #353 (merged 2026-10-07) | reviewer | differential fuzzing (against `std::set`); unreplayed | — |
| Growable sets with a closed key mapping grow to a non-key [#361](https://github.com/rhalbersma/xstd-bits/pull/361) | library | not traced | #361 (merged 2026-10-09) | reviewer | differential fuzzing (against `std::set`); unreplayed | — |
| Width and count hashed as `size_t`; digests differ between 32 and 64 bits [#364](https://github.com/rhalbersma/xstd-bits/issues/364) | library | not traced | issue #364 | reviewer | an i686 (`-m32`) leg (digests compared across targets); unreplayed | — |
| Short-message hash uses FNV-1a, which does not avalanche [#367](https://github.com/rhalbersma/xstd-bits/issues/367) | library | #365 | issue #367 | reviewer | an avalanche test; unreplayed | — |
| `invoke_continues` leaves an untakeable branch for a void functor [#371](https://github.com/rhalbersma/xstd-bits/pull/371) | library | inside #371, before merge | #371 (2026-10-09) | coverage | local gcovr (`tools/ci-local.sh coverage`); unreplayed | — |
| `bit_bounded_set<0>` on `no_blocks` adds branches width 0 never takes [#371](https://github.com/rhalbersma/xstd-bits/pull/371) | library | inside #371, before merge | #371 (2026-10-09) | coverage | local gcovr (`tools/ci-local.sh coverage`); unreplayed | — |
| `or_block_byte` casts `unsigned char` to the block; `absl::uint128` takes the signed constructor (`-Wsign-promo`) [#374](https://github.com/rhalbersma/xstd-bits/pull/374) | library | `1c229c7` (#326, 2026-10-03), latent; reachable inside #374, before merge | #374 (2026-10-09) | new instantiation | type-matrix compile table (`absl::uint128` column); replayed: fails at cd1344c, passes at d8ee901 | — |
| Class blocks copied as raw objects (no wrong result) [#374](https://github.com/rhalbersma/xstd-bits/pull/374) | library | not traced | #374 (merged 2026-10-10) | reviewer | none proposed | — |
| `blit_source` misses owners: `append_range(owner)` packs bit by bit, `window &= owner` fails to compile [#375](https://github.com/rhalbersma/xstd-bits/pull/375) | library | `625e2f0` (#260, 2026-09-26) | #375 (2026-10-09) | new instantiation | type-matrix compile table (owners as blit sources); replayed: fails at 1018920, passes at 4b599a9 | 14 d |
| Set views over raw blocks have `<=>` but no `==` [#377](https://github.com/rhalbersma/xstd-bits/issues/377) | library | not traced | #384 (merged 2026-10-11) | new instantiation | type-matrix compile table; replayed: fails at e9016b5, passes at 5061f22 | — |
| `unsigned _BitInt(24)` blocks fail to compile [#377](https://github.com/rhalbersma/xstd-bits/issues/377) | library | not traced | #385 (merged 2026-10-12) | new instantiation | type-matrix compile table (`_BitInt` blocks); replayed: fails at 5061f22, passes at 6fbfd8f | — |
| `from_blocks` into a non-whole-block width compiles to byte shuffling on GCC [#390](https://github.com/rhalbersma/xstd-bits/issues/390) | library | `a20d8f3` (#171, 2026-09-18) | issue #390 (2026-10-10) | benchmark | an instruction-count check on the hot paths; unreplayed | — |
| `key_distance` wraps for keys wider than `size_t` [#398](https://github.com/rhalbersma/xstd-bits/pull/398) | library | `8b8f5da` (#353, 2026-10-07) | #398 (2026-10-13) | local run | an i686 (`-m32`) leg; unreplayed | 6 d |
| `volatile` blocks pass the constraints and fail deep in the storage [#421](https://github.com/rhalbersma/xstd-bits/issues/421) | library | not traced | issue #421 (2026-10-14) | reviewer | type-matrix compile table (cv-qualified blocks); replayed: fails at 746f59c, passes at 9ecacd9 | — |
| BMI2 dispatch keyed on `__BMI2__`, not the fast-BMI2 macro; no workflow passes `-march`, so neither branch compiles in CI (open gap) [#422](https://github.com/rhalbersma/xstd-bits/issues/422) | library | `2ecd6d8` (#368, 2026-10-09) | issue #422 (2026-10-14) | reviewer | an `-march=x86-64-v3` leg; withdrawn on replay: the leg compiles the chosen branch at both commits and both pass: the bug is which correct branch is picked | — |
| Insert rebuilds the whole sequence per call (quadratic) [#423](https://github.com/rhalbersma/xstd-bits/issues/423) | library | `bed313d` (#110, 2026-09-07) | issue #423 | reviewer | an operation-count test for repeated insertion; unreplayed | — |
| `NDEBUG` turns off every precondition [#427](https://github.com/rhalbersma/xstd-bits/issues/427) | library | not traced | issue #427 | reviewer | a hardened Release leg with the precondition tests; unreplayed | — |
| Ranged `count` calls unqualified `popcount`, which finds Boost.Int128's by ADL [#442](https://github.com/rhalbersma/xstd-bits/pull/442) | library | `28bd43c` (#378, 2026-10-11) | #442 (2026-10-14) | unknown | type-matrix compile table (Boost.Int128 blocks); replayed: fails at 9e296be, passes at 40081fc | 4 d |
| Concepts and traits not cv-transparent [#447](https://github.com/rhalbersma/xstd-bits/pull/447) | library | not traced | #447 (merged 2026-10-16) | reviewer | type-matrix compile table (cv-qualified owners and spans); replayed: fails at 7dd8022, passes at e33c605 | — |
| Sized appends reserve exactly, reallocating every call [#450](https://github.com/rhalbersma/xstd-bits/pull/450) | library | `bed313d` (#110, 2026-09-07) | #450 (merged 2026-10-17) | new instantiation | an allocation-counting test (found by it); unreplayed | 39 d |

### Tests

24 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| Tests include `<flat_set>` unconditionally; VS 2022's STL has none [#32](https://github.com/rhalbersma/xstd-bits/pull/32) | test | not traced | #32 (merged 2026-07-21) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Sieve test erases from the container it iterates (UB for vector-backed `flat_set`) [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | test | `5651f3a` (2024-01-20), latent; UB once `std::flat_set` joined the types (not traced) | #45 (merged 2026-08-28) | msvc | a `_GLIBCXX_DEBUG` leg; unreplayed | — |
| Boost.Mp11 used by tests but never declared [#59](https://github.com/rhalbersma/xstd-bits/pull/59) | test | not traced | #59 (merged 2026-08-30) | reviewer | the consumption job, run locally (with declared dependencies only); unreplayed | — |
| Test guard right only on local Boost 1.83 [#60](https://github.com/rhalbersma/xstd-bits/pull/60) | test | inside #60, before merge | #60 (merged 2026-08-30) | ci (unattributed) | compile on every local compiler (`tools/ci-local.sh`) (against the Boost CI resolves); on replay, reclassified as a local build-and-run (ctest) against the Boost CI resolves, not a compile | — |
| Range test asserts libc++'s proxy behaviour, not the library's [#65](https://github.com/rhalbersma/xstd-bits/pull/65) | test | inside #65, before merge | #65 (merged 2026-09-02) | apple_clang | a local libc++ leg (`tools/ci-local.sh libcxx-22`); unreplayed | — |
| Two exhaustive harnesses declare the same names in `xstd` [#66](https://github.com/rhalbersma/xstd-bits/pull/66) | test | not traced | #66 (merged 2026-09-02) | new instantiation | a TU that includes every test header; unreplayed | — |
| Boost adaptor test claims agreement with Boost's ordering, which differs on 88 of 256 pairs [#66](https://github.com/rhalbersma/xstd-bits/pull/66) | test | not traced | #66 (merged 2026-09-02) | local run | the local run that found it; none cheaper; unreplayed | — |
| `views::set_union` resolves to range-v3's only through include order [#67](https://github.com/rhalbersma/xstd-bits/pull/67) | test | `9017bd3` (#65, 2026-09-02) | #67 (merged 2026-09-02) | reviewer | a per-header self-sufficiency build; unreplayed | 7 h |
| Unqualified `numeric_limits` silently finds `xstd::numeric_limits` [#67](https://github.com/rhalbersma/xstd-bits/pull/67) | test | `d1f8940` (#66, 2026-09-02) | #67 (merged 2026-09-02) | ci (unattributed) | compile on every local compiler (`tools/ci-local.sh`); replayed: fails at 9e0281c, passes at 2335ad6 | 5 h |
| Test target named `test..bits` [#67](https://github.com/rhalbersma/xstd-bits/pull/67) | test | `d1f8940` (#66, 2026-09-02) | #67 (merged 2026-09-02) | local run | the local run that found it; none cheaper; unreplayed | 5 h |
| 36 includes arrive transitively through `ext/` adapters [#79](https://github.com/rhalbersma/xstd-bits/pull/79) | test | not traced | #79 (merged 2026-09-04) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | — |
| 128-bit block gate keyed on `__GNUG__` [#85](https://github.com/rhalbersma/xstd-bits/pull/85) | test | not traced | #85 (merged 2026-09-04) | reviewer | type-matrix compile table (128-bit blocks under `-std=c++23`); replayed: fails at a659b82, passes at 59b4b72 (strict `-std=c++23` on libstdc++ only) | — |
| `mem_emplace` forwards arguments three times (use after move) [#125](https://github.com/rhalbersma/xstd-bits/pull/125) | test | `43636ef` (2018-12-23) | #125 (merged 2026-09-08) | local run | local clang-tidy (`tools/ci-local.sh tidy-*`) (over `test/include/`); unreplayed | 2,816 d |
| `bugprone-signed-bitwise` on `lcg >> 11` sits on main unseen [#129](https://github.com/rhalbersma/xstd-bits/pull/129) | test | `865b9e6` (#125, 2026-09-08) | #129 (merged 2026-09-09) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`) (`ci-local.sh tidy-24`); unreplayed | 10 h |
| Fixture assumes `std::bitset` has no block access; MSVC's STL has `_Getword` [#130](https://github.com/rhalbersma/xstd-bits/pull/130) | test | inside #130, before merge | #130 (merged 2026-09-09) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Thirteen forwarded trait entries never called by a test [#133](https://github.com/rhalbersma/xstd-bits/pull/133) | test | inside #133, before merge | #133 (merged 2026-09-10) | coverage | local gcovr (`tools/ci-local.sh coverage`); unreplayed | — |
| Widened 128-bit flag puts a class block into `word_types` on MSVC [#141](https://github.com/rhalbersma/xstd-bits/pull/141) | test | inside #141, before merge | #141 (merged 2026-09-12) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Checklists cannot see the surface gaps [#183](https://github.com/rhalbersma/xstd-bits/pull/183) | test | not traced | #183 (merged 2026-09-19) | reviewer | none proposed | — |
| Test primitive `composable::includes` compares the subset relation backwards [#269](https://github.com/rhalbersma/xstd-bits/pull/269) | test | `48f85e6` (2020-05-08) | #269 (2026-09-27) | new instantiation | differential test against the standard model (random sizes); unreplayed | 2,333 d |
| Test citations carry wrong paragraph numbers [#279](https://github.com/rhalbersma/xstd-bits/pull/279) | test | not traced | #279 (merged 2026-09-28) | audit | the Audit workflow (found by it); unreplayed | — |
| Associative-container cases check something else than ¶13 and ¶15 [#280](https://github.com/rhalbersma/xstd-bits/pull/280) | test | not traced | #280 (merged 2026-09-29) | audit | the Audit workflow (found by it); unreplayed | — |
| Tests assert capacity-0 bounded owners are empty types; the MSVC ABI gives them a byte [#371](https://github.com/rhalbersma/xstd-bits/pull/371) | test | inside #371, before merge | #371 (2026-10-09) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Tests assume 64-bit `size_t` blocks [#398](https://github.com/rhalbersma/xstd-bits/pull/398) | test | not traced | #398 (merged 2026-10-13) | local run | an i686 (`-m32`) leg; unreplayed | — |
| Redundant `<cstddef>` for `std::dynamic_extent` in a test [#469](https://github.com/rhalbersma/xstd-bits/pull/469) | test | inside #469, before merge | #469 (2026-10-18) | local run | the local run that found it; none cheaper; unreplayed | — |

### Benchmarks

6 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| Sieve benchmark registered as a Debug test trips ctest's timeout on Windows Debug [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | benchmark | #45 | #45 (merged 2026-08-28) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| `flat_set` sieve ladder to 2^20 takes ctest from 6 s to 7 min [#117](https://github.com/rhalbersma/xstd-bits/issues/117) | benchmark | inside #120, before merge | #120 (merged 2026-09-08) | local run | the local run that found it; none cheaper; unreplayed | — |
| Macro's only user preprocessed away on VS 2022: `-Wunused-macros` [#120](https://github.com/rhalbersma/xstd-bits/pull/120) | benchmark | inside #120, before merge | #120 (merged 2026-09-08) | clang_cl | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| Bench ladder holds 16 KB on one frame (C6262) [#120](https://github.com/rhalbersma/xstd-bits/pull/120) | benchmark | inside #120, before merge | #120 (merged 2026-09-08) | msvc_analyze | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| `bm_scan` compares unlike walks (1.8x slower vs 0.86x like for like) [#177](https://github.com/rhalbersma/xstd-bits/pull/177) | benchmark | `3324e09` (#122, 2026-09-08) | #177 (merged 2026-09-19) | benchmark | none proposed | 11 d |
| Random-read benchmark measures a 64-bit divide and overlapped misses (open) [#424](https://github.com/rhalbersma/xstd-bits/issues/424) | benchmark | `3324e09` (#122, 2026-09-08) | issue #424 (2026-10-14) | reviewer | none proposed | — |

### Documentation

10 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| Workaround comment cites the wrong GCC bug and mislabels a snapshot [#56](https://github.com/rhalbersma/xstd-bits/pull/56) | doc | #54 | #56 (merged 2026-08-29) | reviewer | none proposed | 5 h |
| `sanitizers.yml` header comment contradicts the inputs below it [#57](https://github.com/rhalbersma/xstd-bits/pull/57) | doc | #45 | #57 (merged 2026-08-29) | reviewer | none proposed | 1 d |
| design.md calls Boost's `operator<` magnitude ordering; it is reverse-lexicographic [#93](https://github.com/rhalbersma/xstd-bits/pull/93) | doc | `9017bd3` (#65, 2026-09-02) | #93 (merged 2026-09-05) | local run | the local run that found it; none cheaper; unreplayed | 3 d |
| README calls `max_size` static [#115](https://github.com/rhalbersma/xstd-bits/issues/115) | doc | not traced | #122 (merged 2026-09-08) | reviewer | a doc test that compiles the claim; unreplayed | — |
| README table gives non-templates template parameter lists [#125](https://github.com/rhalbersma/xstd-bits/pull/125) | doc | `f98512f` (#106, 2026-09-07) | #125 (merged 2026-09-08) | reviewer | a doc test that compiles the claim; unreplayed | 1 d |
| README labels the set row `int` with the wrong break-even [#133](https://github.com/rhalbersma/xstd-bits/pull/133) | doc | not traced | #133 (merged 2026-09-10) | reviewer | a doc test that compiles the claim; unreplayed | — |
| CONTRIBUTING.md advertises a dropped MSVC rung [#138](https://github.com/rhalbersma/xstd-bits/pull/138) | doc | #134 | #138 (merged 2026-09-11) | reviewer | none proposed | 5 h |
| design.md's 47x `count` claim compares two libstdc++ columns [#121](https://github.com/rhalbersma/xstd-bits/issues/121) | doc | `0e8682c` (#130, 2026-09-09) | #162 (merged 2026-09-15) | benchmark | the benchmark on a libc++ leg; unreplayed | 6 d |
| README describes the bit layout mirrored [#174](https://github.com/rhalbersma/xstd-bits/pull/174) | doc | not traced | #174 (merged 2026-09-18) | reviewer | a doc test that compiles the claim; unreplayed | — |
| Include comments name 85 names their files do not use [#461](https://github.com/rhalbersma/xstd-bits/pull/461) | doc | not traced | #461 (merged 2026-10-17) | audit | the Audit workflow (`include_comments.py`, found by it); unreplayed | — |

### Gates

14 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| `clang-tidy.yml` pipes into `tee` without `pipefail`; the job always passes [#34](https://github.com/rhalbersma/xstd-bits/pull/34) | gate | `4c648fb` (#33, 2026-07-21) | #34 (merged 2026-07-21) | reviewer | a gate self-test that seeds a failing input; unreplayed | 10 h |
| clang-tidy per-header pass never runs when the first pass fails, hiding findings [#51](https://github.com/rhalbersma/xstd-bits/pull/51) | gate | not traced | #51 (merged 2026-08-28) | clang_tidy | a gate self-test that seeds a failing input; unreplayed | — |
| Prose inside `.clang-tidy`'s folded `Checks` scalar disables two exclusions [#51](https://github.com/rhalbersma/xstd-bits/pull/51) | gate | inside #51, before merge | #51 (merged 2026-08-28) | local run | the local run that found it; none cheaper; unreplayed | — |
| gcov unwind-resume records on closing braces make 100% unattainable [#60](https://github.com/rhalbersma/xstd-bits/pull/60) | gate | in cpp-ci | #60 (merged 2026-08-30) | coverage | the check that found it; none cheaper; unreplayed | — |
| Trailing-return fixer strands three `NOLINT` markers [#74](https://github.com/rhalbersma/xstd-bits/pull/74) | gate | inside #74, before merge | #74 (merged 2026-09-03) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | — |
| gcovr merges branches across TUs by gcc's per-TU block ids [#103](https://github.com/rhalbersma/xstd-bits/pull/103) | gate | not traced | #103 (merged 2026-09-07) | coverage | the check that found it; none cheaper; unreplayed | — |
| `HeaderFilterRegex` hides 112+ findings in `test/include/` [#126](https://github.com/rhalbersma/xstd-bits/issues/126) | gate | not traced | #125 (merged 2026-09-08) | local run | a gate self-test that seeds a failing input; unreplayed | — |
| One `NOLINTBEGIN` per file suppresses 46 sites with the 17 that needed it [#129](https://github.com/rhalbersma/xstd-bits/pull/129) | gate | inside #129, before merge | #129 (merged 2026-09-09) | mutation testing | the check that found it; none cheaper; unreplayed | — |
| Benchmarks built with `--coverage` add branches to the denominator [#133](https://github.com/rhalbersma/xstd-bits/pull/133) | gate | not traced | #133 (merged 2026-09-10) | coverage | the check that found it; none cheaper; unreplayed | — |
| gcovr's `= default;` exclusion is line-bound [#183](https://github.com/rhalbersma/xstd-bits/pull/183) | gate | not traced | #183 (merged 2026-09-19) | coverage | local gcovr (`tools/ci-local.sh coverage`); unreplayed | — |
| Reformat moves nineteen `NOLINT` markers off their lines [#199](https://github.com/rhalbersma/xstd-bits/pull/199) | gate | inside #199, before merge | #199 (merged 2026-09-20) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | — |
| A pre-existing `NOLINT` on the wrong line [#199](https://github.com/rhalbersma/xstd-bits/pull/199) | gate | not traced | #199 (merged 2026-09-20) | reviewer | a lint that each `NOLINT` line raises its diagnostic without the marker; unreplayed | — |
| Canary-called stubs share one concurrency group, so badges stay red [#405](https://github.com/rhalbersma/xstd-bits/issues/405) | gate | `662899c` (#75, 2026-09-03) | issue #405 (2026-10-13) | reviewer | a check that each canary-dispatched run concludes rather than cancels; unreplayed | — |
| cpp-ci clang-tidy loop piped into `tee`: only the last header's status counts [#465](https://github.com/rhalbersma/xstd-bits/pull/465) | gate | in cpp-ci | #465 (merged 2026-10-18) | reviewer | a gate self-test that seeds a failing input; unreplayed | — |

### CI configuration

14 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| xstd fetched from its `main`; an upstream move turns every compiling job red [#43](https://github.com/rhalbersma/xstd-bits/pull/43) | ci-config | `348bcb8` (2026-07-19) | #43 (merged 2026-08-28) | ci (unattributed) | a pin check: every pin a tagged commit reachable from its default branch; unreplayed | 40 d |
| Withholding `-Wa,-mbig-obj` from MinGW Debug fails assembly [#48](https://github.com/rhalbersma/xstd-bits/pull/48) | ci-config | #47 | #48 (merged 2026-08-28) | mingw | the Windows leg that found it; nothing local reaches it; unreplayed | 2 h |
| Per-header clang-tidy pass compiles the Boost adapter without Boost on the path [#52](https://github.com/rhalbersma/xstd-bits/pull/52) | ci-config | #45 | #52 (merged 2026-08-28) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`); unreplayed | 12 h |
| MinGW vector-width cap scoped to Debug; Release objects still misaligned [#55](https://github.com/rhalbersma/xstd-bits/pull/55) | ci-config | #54 | #55 (merged 2026-08-29) | cpp-ci scan | the check that found it; none cheaper; unreplayed | 2 h |
| Repin sweep's filename regex misses `clang-libc++.yml` [#57](https://github.com/rhalbersma/xstd-bits/pull/57) | ci-config | inside #57, before merge | #57 (merged 2026-08-29) | reviewer | a pin check: every pin a tagged commit reachable from its default branch; unreplayed | — |
| xstd pinned at a PR-branch commit the squash left unreachable [#62](https://github.com/rhalbersma/xstd-bits/pull/62) | ci-config | `0df620b` (2026-08-31) | #62 (merged 2026-08-31) | consumption | a pin check: every pin a tagged commit reachable from its default branch; unreplayed | < 1 h |
| `coverage.yml` resolves cpp-ci from an untagged commit [#156](https://github.com/rhalbersma/xstd-bits/pull/156) | ci-config | `33ddd7e` (#76, 2026-09-03) | #156 (merged 2026-09-14) | reviewer | a pin check: every pin a tagged commit reachable from its default branch; unreplayed | 11 d |
| No-tests preset lacks the vcpkg toolchain file [#169](https://github.com/rhalbersma/xstd-bits/pull/169) | ci-config | not traced | #169 (merged 2026-09-16) | local run | the local run that found it; none cheaper; unreplayed | — |
| `.git-blame-ignore-revs` names an unreachable commit [#200](https://github.com/rhalbersma/xstd-bits/pull/200) | ci-config | `d42a9cc` (#199, 2026-09-20) | #200 (merged 2026-09-21) | reviewer | a check that each blame-ignore commit is reachable from main; unreplayed | 7 h |
| `setup-toolchain.sh` installs clang-format by version, not hash [#240](https://github.com/rhalbersma/xstd-bits/pull/240) | ci-config | `d42a9cc` (#199, 2026-09-20) | #240 (merged 2026-09-23) | scorecard | the check that found it; none cheaper; unreplayed | 3 d |
| `setup-toolchain.sh` leaves CMake 3.28 below the 3.30 floor [#293](https://github.com/rhalbersma/xstd-bits/pull/293) | ci-config | #211 | #293 (merged 2026-10-01) | local run | the local run that found it; none cheaper; unreplayed | 10 d |
| ASan libc++ leg killed mid-build running six Ninja jobs [#443](https://github.com/rhalbersma/xstd-bits/pull/443) | ci-config | in cpp-ci | #443 (merged 2026-10-15) | sanitizers | none proposed (runner capacity) | — |
| libc++ Release legs killed at exit 143 (a TU peaks at 5.1 GiB) [#465](https://github.com/rhalbersma/xstd-bits/pull/465) | ci-config | in cpp-ci | #465 (merged 2026-10-18) | clang_libcxx | none proposed (runner capacity) | — |
| Stale xstd-misc pin kept in `consumption.yml` by a file-wide conflict resolution [#465](https://github.com/rhalbersma/xstd-bits/pull/465) | ci-config | inside #465, before merge | #465 (merged 2026-10-18) | consumption | a pin check (consumption pins checked against CMakeLists.txt); unreplayed | — |

### Toolchains, upstream

32 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| `-march=native` emits 32-byte aligned stack stores under the 16-byte Windows ABI (GCC PR target/54412) [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | toolchain-upstream | upstream | #45 (merged 2026-08-28) | mingw | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| cl rejects an explicit-object member in a trailing return type (C2228) [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | toolchain-upstream | upstream | #45 (merged 2026-08-28) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| libc++ lacks `cartesian_product`, `stride` and `adjacent`; the suite does not compile [#45](https://github.com/rhalbersma/xstd-bits/pull/45) | toolchain-upstream | upstream | #45 (merged 2026-08-28) | clang_libcxx | a local libc++ leg (`tools/ci-local.sh libcxx-22`); unreplayed | — |
| MSVC `/O2` optimizer crash on `bit_static_set<0>` through range-v3 [#98](https://github.com/rhalbersma/xstd-bits/pull/98) | toolchain-upstream | upstream | #98 (merged 2026-09-06) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| clang 23 lifetime analysis crashes on a deducing-this call with a temporary [#98](https://github.com/rhalbersma/xstd-bits/pull/98) | toolchain-upstream | upstream | #98 (merged 2026-09-06) | clang | compile on every local compiler (`tools/ci-local.sh`) (`ci-local.sh clang-23`); replayed: fails at e5f5c7b, passes at 766e735 | — |
| MSVC 17 cannot deduce through alias templates (P1814) [#101](https://github.com/rhalbersma/xstd-bits/pull/101) | toolchain-upstream | upstream | #101 (merged 2026-09-06) | msvc_analyze | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| GCC 17 Release false `-Wrestrict`/`-Wstringop-overread` in `std::vector` [#103](https://github.com/rhalbersma/xstd-bits/pull/103) | toolchain-upstream | upstream | #103 (merged 2026-09-07) | gcc | compile on every local compiler (`tools/ci-local.sh`) (Release build); replayed: fails at f826410, passes at 2e5ffb4 | — |
| clang-tidy reads the `0` in `ordering < 0` as a null pointer under libstdc++ 16 [#130](https://github.com/rhalbersma/xstd-bits/pull/130) | toolchain-upstream | upstream | #130 (merged 2026-09-09) | local run | the local run that found it; none cheaper; unreplayed | — |
| MSVC 2022 lacks `auto(x)` (P0849R8) [#132](https://github.com/rhalbersma/xstd-bits/pull/132) | toolchain-upstream | upstream | #132 (merged 2026-09-10) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| GCC 15 declines to fold an `lea` into `shrx` (documented, not fixed) [#133](https://github.com/rhalbersma/xstd-bits/pull/133) | toolchain-upstream | upstream | #133 (merged 2026-09-10) | benchmark | an instruction-count check on the hot paths; unreplayed | — |
| Xcode 16.4's libc++ lacks `range_const_reference_t` [#147](https://github.com/rhalbersma/xstd-bits/pull/147) | toolchain-upstream | upstream | #147 (merged 2026-09-13) | apple_clang | the Apple Clang leg that found it; nothing local reaches it; unreplayed | — |
| Local clang-tidy 20 crash reported as a pass [#171](https://github.com/rhalbersma/xstd-bits/pull/171) | toolchain-upstream | upstream | #171 (merged 2026-09-18) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`) (with its exit code checked); unreplayed | — |
| clang-tidy 23 `avoid-c-style-cast` false positive [#183](https://github.com/rhalbersma/xstd-bits/pull/183) | toolchain-upstream | upstream | #183 (merged 2026-09-19) | clang_tidy | local clang-tidy (`tools/ci-local.sh tidy-*`) (`ci-local.sh tidy-23`); unreplayed | — |
| `readability-redundant-typename` asks for code GCC and Clang reject [#206](https://github.com/rhalbersma/xstd-bits/pull/206) | toolchain-upstream | upstream | #206 (merged 2026-09-21) | clang_tidy | the check that found it; none cheaper; unreplayed | — |
| `bugprone-exception-escape` false positive over `small_vector` [#214](https://github.com/rhalbersma/xstd-bits/pull/214) | toolchain-upstream | upstream | #214 (merged 2026-09-22) | clang_tidy | the check that found it; none cheaper; unreplayed | — |
| MSVC alias deduction fails with the extent rule in the requires clause [#256](https://github.com/rhalbersma/xstd-bits/pull/256) | toolchain-upstream | upstream | #256 (merged 2026-09-26) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| libstdc++ 15 `std::bitset` text constructor has the same gap [#271](https://github.com/rhalbersma/xstd-bits/pull/271) | toolchain-upstream | upstream | #271 (merged 2026-09-27) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| clang-analyzer false shift-by-64 in Boost's `to_ulong` [#271](https://github.com/rhalbersma/xstd-bits/pull/271) | toolchain-upstream | upstream | #271 (merged 2026-09-27) | clang_tidy | the check that found it; none cheaper; unreplayed | — |
| MSVC STL `vector<bool>::reserve` past `max_size()` throws `bad_alloc`, not `length_error` [#270](https://github.com/rhalbersma/xstd-bits/issues/270) | toolchain-upstream | upstream | #274 (2026-09-28) | clang_cl | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| libstdc++ 16 `inplace_vector` single-pass append fills then throws (no-effects broken) [#270](https://github.com/rhalbersma/xstd-bits/issues/270) | toolchain-upstream | upstream | #276 (2026-09-28) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| libc++ `bitset::reference::flip()` returns `void` [#279](https://github.com/rhalbersma/xstd-bits/pull/279) | toolchain-upstream | upstream | #279 (merged 2026-09-28) | audit | the Audit workflow (found by it); unreplayed | — |
| libc++ `set`/`map` assignment corrupts the tree when a node allocation throws [#297](https://github.com/rhalbersma/xstd-bits/issues/297) | toolchain-upstream | upstream | #295 (2026-10-01) | apple_clang | a local libc++ leg (`tools/ci-local.sh libcxx-22`); unreplayed | — |
| MSVC STL debug-mode `noexcept` vector move allocates a proxy; a refusing allocator terminates [#298](https://github.com/rhalbersma/xstd-bits/issues/298) | toolchain-upstream | upstream | #295 (2026-10-01) | clang_cl | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| MSVC C1001 on a by-reference lambda inside `if constexpr` in a generic lambda [#299](https://github.com/rhalbersma/xstd-bits/issues/299) | toolchain-upstream | upstream | #295 (2026-10-01) | msvc | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| MSVC STL single-pass range insert keeps a partial insertion on allocation failure [#300](https://github.com/rhalbersma/xstd-bits/issues/300) | toolchain-upstream | upstream | #295 (2026-10-01) | probe workflow | the Windows leg that found it; nothing local reaches it; unreplayed | — |
| libstdc++ single-pass range insert keeps a partial insertion on allocation failure [#301](https://github.com/rhalbersma/xstd-bits/issues/301) | toolchain-upstream | upstream | #295 (2026-10-01) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| libc++ `vector<bool>` mid-vector insert shifts a word by 64 (UB) [#305](https://github.com/rhalbersma/xstd-bits/issues/305) | toolchain-upstream | upstream | #304 (2026-10-02) | fuzz | the check that found it; none cheaper; unreplayed | — |
| libc++ `std::equal` on `vector<bool>` at different unaligned offsets answers wrongly [#307](https://github.com/rhalbersma/xstd-bits/issues/307) | toolchain-upstream | upstream | #306 (2026-10-02) | apple_clang | a local libc++ leg (`tools/ci-local.sh libcxx-22`); unreplayed | — |
| libstdc++ before 16 `ranges::rotate` loses a proxy bit (GCC PR 121913) [#352](https://github.com/rhalbersma/xstd-bits/pull/352) | toolchain-upstream | upstream | #352 (merged 2026-10-07) | local run | the local run that found it; none cheaper; unreplayed | — |
| libc++ `syntax_option_type` breaks the bitmask requirements [#358](https://github.com/rhalbersma/xstd-bits/pull/358) | toolchain-upstream | upstream | #358 (merged 2026-10-08) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| GCC 15 mangles class-type template arguments without implicit zeros (PR c++/121231) [#396](https://github.com/rhalbersma/xstd-bits/issues/396) | toolchain-upstream | upstream | issue #396 (2026-10-12) | local run | an i686 (`-m32`) leg (running the structural test); unreplayed | — |
| clang 23.1.4 lifetime-safety segfault on an explicit-object rvalue member call (open upstream) [#470](https://github.com/rhalbersma/xstd-bits/issues/470) | toolchain-upstream | upstream | #469 (2026-10-18) | sanitizers | compile on every local compiler (`tools/ci-local.sh`) (`ci-local.sh clang-23` on that test); replayed: fails at c447af8, passes at ae47094 | — |

### Dependencies, upstream

16 rows.

| bug | kind | introduced | found | found by | cheapest check that would have caught it | how much earlier |
| :--- | :--- | :--- | :--- | :--- | :--- | ---: |
| cpp-ci v0.8.0's internal pin carries a stale toolchain list [#57](https://github.com/rhalbersma/xstd-bits/pull/57) | dependency-upstream | upstream | rhalbersma/xstd#214 | downstream user | a cpp-ci release check that callers see the published toolchain list; unreplayed | — |
| cpp-ci v0.10.0's Ninja generator breaks clang-tidy on `.modmap` files [#168](https://github.com/rhalbersma/xstd-bits/pull/168) | dependency-upstream | upstream | rhalbersma/xstd-ints#249 | downstream user | cpp-ci's own CI running its clang-tidy workflow; unreplayed | — |
| cpp-ci v0.5.0 expands an empty array under `set -u` (bash 3.2) [#50](https://github.com/rhalbersma/xstd-bits/pull/50) | dependency-upstream | upstream | #50 (merged 2026-08-28) | apple_clang | cpp-ci's scripts run under bash 3.2 in cpp-ci's own CI; unreplayed | — |
| cpp-ci's MinGW 17-SVN rung is a GCC 16.0.1 snapshot [#56](https://github.com/rhalbersma/xstd-bits/pull/56) | dependency-upstream | upstream | #56 (merged 2026-08-29) | reviewer | a cpp-ci check that a rung's version matches its label; unreplayed | — |
| Transient 403 from packages.microsoft.com [#239](https://github.com/rhalbersma/xstd-bits/pull/239) | dependency-upstream | upstream | #239 (merged 2026-09-24) | sanitizers | none proposed (transient) | — |
| Boost 1.92 `small_vector::get_allocator()` returns its internal allocator type [#270](https://github.com/rhalbersma/xstd-bits/issues/270) | dependency-upstream | upstream | #269 (2026-09-27) | coverage | compile on every local compiler (`tools/ci-local.sh`) (with vcpkg's Boost 1.92); replayed: fails at 8609d7e, passes at 9684e89 | — |
| Boost `small_vector` allocator-extended move steals the source buffer under unequal allocators [#270](https://github.com/rhalbersma/xstd-bits/issues/270) | dependency-upstream | upstream | #269 (2026-09-27) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| Boost `flat_set::reverse_iterator` is not `std::reverse_iterator` [#270](https://github.com/rhalbersma/xstd-bits/issues/270) | dependency-upstream | upstream | comment on #270 (2026-09-27) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| Boost `static_vector::swap` is never `noexcept` [#302](https://github.com/rhalbersma/xstd-bits/issues/302) | dependency-upstream | upstream | #290 (2026-09-30) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| Boost `small_vector` swap friend bypassed by generic swap; member swap has no exception specification [#302](https://github.com/rhalbersma/xstd-bits/issues/302) | dependency-upstream | upstream | #290 (2026-09-30) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| Boost `small_vector` input-iterator insert keeps a partial insertion [#311](https://github.com/rhalbersma/xstd-bits/issues/311) | dependency-upstream | upstream | issue #311 (2026-10-02) | new instantiation | the local run that found it; none cheaper; unreplayed | — |
| Boost `static_vector` throws its own `bad_alloc`, not derived from `std::bad_alloc` (by design) [#312](https://github.com/rhalbersma/xstd-bits/issues/312) | dependency-upstream | upstream | issue #312 (2026-10-02) | unknown | none proposed | — |
| Boost `dynamic_bitset` moves are not `noexcept` [#313](https://github.com/rhalbersma/xstd-bits/issues/313) | dependency-upstream | upstream | issue #313 (2026-10-02) | unknown | a `noexcept` static_assert over the dependency's moves; unreplayed | — |
| Boost `dynamic_bitset::append(first, last)` gives only the basic guarantee for input iterators [#314](https://github.com/rhalbersma/xstd-bits/issues/314) | dependency-upstream | upstream | issue #314 (2026-10-02) | unknown | refusing-allocator harness; unreplayed | — |
| `stdx::bitset` leaves unused high bits set and mishandles wide shifts (not a dependency) [#330](https://github.com/rhalbersma/xstd-bits/issues/330) | dependency-upstream | upstream | issue #330 (2026-10-03) | reviewer | none proposed | — |
| cpp-ci `install-gcc` exits 0 when the PPA index fails [#450](https://github.com/rhalbersma/xstd-bits/pull/450) | dependency-upstream | upstream | #450 (merged 2026-10-17) | ci (unattributed) | none proposed (upstream infrastructure) | — |
## The gate ledger

One row per workflow, ordered by runner minutes per pull request. A workflow that runs several job families names
them all; the timings are per workflow run. The window is about four days and about 16 pull requests (#419 and #442
to #469). The counts are incidents, not rates: a failure is one failed job attempt, including attempts in runs that
were later cancelled.

- **bugs found first** counts the bug-ledger rows whose finder is one of the workflow's job families, over the whole
  history, and those found on a pull request in the window.
- **median queue**, **median run wall** and **median runner minutes per PR** are medians over runs that concluded in
  success or failure. Runner minutes are summed job wall minutes, with no operating-system multiplier.
- **red on the PR's own code** counts failed jobs whose cause was the pull request's change.
- **red for another reason** counts the rest by cause: `mirror/network` (a package mirror failing), `runner/infra` (a
  runner lost or killed), `gate-config` (the gate's own configuration), `upstream toolchain` (a compiler crash).

| workflow | job families | bugs found first, all history | of which in the window | median queue min | median run wall min | median runner min per PR | red on the PR's own code | red for another reason |
| :--- | :--- | ---: | ---: | ---: | ---: | ---: | ---: | :--- |
| Sanitizers | `sanitizers` | 4 | 2 | 83.7 | 174.9 | 456.6 | 0 | mirror/network 8, runner/infra 2, upstream toolchain 1 |
| Clang-CL | `clang_cl` | 3 | 0 | 13.3 | 124.8 | 334.2 | 0 | — |
| GCC | `gcc`, `gcc_cxx26`, `hardened`, `i686` | 1 | 0 | 58.9 | 193.0 | 331.1 | 0 | mirror/network 8 |
| MSVC | `msvc` | 13 | 0 | 9.2 | 118.9 | 304.6 | 0 | — |
| Clang-Tidy | `clang_tidy` | 15 | 0 | 23.3 | 178.2 | 283.3 | 7 | mirror/network 3 |
| MinGW | `mingw`, `mingw_cxx26` | 2 | 0 | 18.7 | 123.0 | 225.1 | 0 | — |
| Clang libc++ | `clang_libcxx`, `hardened` | 2 | 1 | 33.4 | 160.1 | 204.5 | 0 | runner/infra 4 |
| MSVC-Analyze | `msvc_analyze` | 2 | 0 | 2.5 | 110.4 | 196.5 | 4 | — |
| Clang | `clang` | 2 | 0 | 26.4 | 142.3 | 176.6 | 2 | mirror/network 4 |
| Apple Clang | `apple_clang` | 5 | 0 | 3.2 | 67.5 | 105.4 | 1 | — |
| CodeQL | `codeql` | 0 | 0 | 0.2 | 84.7 | 84.6 | 0 | mirror/network 2 |
| Coverage | `coverage` | 9 | 0 | 0.8 | 35.8 | 33.7 | 3 | mirror/network 1 |
| Emscripten | `wasm32` | 0 | 0 | 5.8 | 68.0 | 21.7 | 2 | — |
| ClusterFuzzLite PR fuzzing | `fuzz` | 1 | 0 | 0.2 | 9.2 | 17.6 | 0 | — |
| Consumption | `consumption` | 3 | 1 | 0.8 | 1.7 | 1.0 | 1 | gate-config 1, mirror/network 1 |
| Actionlint | `actionlint` | 0 | 0 | 0.5 | 0.9 | 0.4 | 0 | — |
| Clang-Format | `clang_format` | 0 | 0 | 0.4 | 0.8 | 0.4 | 0 | — |
| Audit | `audit` | 6 | 1 | 0.5 | 0.6 | 0.1 | 0 | — |

Five finders sit outside this table. Scorecard (1 bug) runs on pushes to `main` and on a schedule, so it has no
pull-request runs. The mutation-testing find (1) was a local run, before the scheduled mutation workflow existed.
The cpp-ci stack-ABI scan (1) runs inside cpp-ci's workflows, `ci (unattributed)` (8) names no workflow, and the
probe workflow (1) no longer exists.

Of the 104,511 runner minutes the 900 listed runs used, 44,311 (42%) went to runs cancelled because a newer push
superseded them; the per-PR medians above exclude those runs.

## The replays

Four checks were proposed most often, and 44 of their claims were replayed, one cell per bug, at the fix's parent and
at the fix. A replay *confirms* a claim when the cell fails at the parent and passes at the fix. It *reclassifies* one
when a different check does that, and *withdraws* one when no check of the kind can.

| proposed check | claims replayed | confirmed | reclassified | withdrawn |
| :--- | ---: | ---: | ---: | ---: |
| type-matrix compile table | 35 | 28 | 4 | 3 |
| compile on every local compiler (`tools/ci-local.sh`) | 8 | 7 | 1 | 0 |
| an `-march=x86-64-v3` leg | 1 | 0 | 0 | 1 |
| actionlint, run locally | 1 | 1 | 0 | 0 |

The actionlint claim was replayed on a bug from after the ledger's window: #471's `audit.yml` named a step with an
unquoted `ranges:: says`, which YAML reads as a mapping. actionlint fails at f07aa64 and passes at c4ef151.

What the replays say about each check:

- **A type-matrix cell has to assert, not just instantiate.** Four of the seven claims it lost compile and run at the
  parent: a wrong `max_size()` that only an agreement check across the readings catches, an `inplace_vector<T, 0>`
  that only the [inplace.vector.overview]/5 traits catch, a structural accident in the detail vehicle that no public
  owner shows, and a signed flag enumeration that only a run against `std::set`'s order catches. The table that would
  have found them carries the clause's assertions and runs its cells. Three claims cannot replay at all: one needs a
  type that does not exist, one is two correct spellings with different meanings, one has no observable form before
  its own fix.
- **A type-matrix cell has to compile as users do.** The 128-bit row replays only under a strict `-std=c++23` on
  libstdc++; the CI builds use `gnu++23`, CMake's default, and pass.
- **Compile on every local compiler means every one, in both build types and against the CI's dependencies.** Its
  seven confirmed claims needed clang 23 (twice, a frontend crash), a Release build (twice, once on GCC 17 at `-O3`),
  clang beside GCC (once), Boost 1.92 from vcpkg (once) and plain compilation (once). The claim it lost needed a test
  run against the newer Boost the CI resolves, not a compile.
- **An `-march` leg would not have caught the BMI2 dispatch bug.** It compiles the chosen branch at both commits and
  both pass; the two headers disagree only under `-march=znver1`, `znver2` and MSVC's `/arch:AVX2`, and no test fails
  there either. A lint that keeps `__BMI2__` inside xstd-ints, or a test that asserts which path a target selects,
  would have. `test/CMakeLists.txt` passes `-march=native` to every test, so which branch CI compiles depends on the
  runner's hardware.
- **One replay found a live gap.** libc++ 22 counts `unsigned __int128` as `std::unsigned_integral` even under
  `-std=c++23`, against the premise of the 128-bit row's fix. That fix's two-way `static_assert` fires on libc++ 22
  strict; `main` has dropped the assertion and keeps the `#if`, so there the 128-bit row is silently left out.

## What the ledgers say so far

- **Most bugs were found first outside CI.** Of the 219 rows, a CI gate found 78 first (36%, 6 of them the Audit
  checks on their first run), a reviewer 65 (30%), a new instantiation 34 (16%) and a local run 23 (11%);
  benchmarks, downstream users, mutation testing and a probe found 11, and for 8 the record does not say. For the 103
  library bugs the split is sharper: a reviewer 44 (43%), a new instantiation 23 (22%), a local run 9 (9%) and a CI
  gate 18 (17%).
- **CI gates find upstream defects more than library ones.** They found 27 of the 48 toolchain and dependency
  defects first (56%), 13 of them on the Windows and Apple legs, against 18 of the 103 library bugs.
- **Where the time can be measured, the median is under a week.** Of the 60 rows with both ends dated, the median
  latency is 5.7 days; 18 were found within a day and 28 after more than a week. Six waited more than a year,
  the longest 2,816 days: a use-after-move in the test helpers, found by a local clang-tidy run over `test/include/`,
  which the CI configuration's header filter hid. 28 bugs were introduced and fixed inside one pull request.
- **One proposed check covers the most rows.** A type-matrix compile table is the cheapest check proposed for 35
  rows, 34 of them library bugs, which reviewers (18) and new instantiations (12) found first. Replayed, it confirms 28
  of them, and four more once its cells assert the clause and run.
- **Several gates have no first find in the ledger.** Actionlint, Clang-Format, CodeQL and Emscripten found no
  ledger bug first in the whole history, and GCC one. In the window, seven workflows failed no job for any reason:
  Actionlint, Audit, ClusterFuzzLite, Clang-CL, Clang-Format, MinGW and MSVC.
- **The bug ledger records few of the defects CI turns away inside a pull request.** In the window, 20 jobs went red
  on the pull request's own code, for 14 distinct reasons across 7 pull requests: clang-tidy 7 jobs, MSVC-Analyze 4,
  Coverage 3, Clang 2, Emscripten 2, Apple Clang 1, Consumption 1. One of them, the stale xstd-misc pin, is a ledger
  row; the 11 ledger rows found in the window came first from a reviewer (2), the sanitizers (2), and one each from a
  local run, a new instantiation, Audit, Clang libc++, Consumption, an unattributed CI run, and an unknown finder.
- **Most red jobs in the window were not the pull request's fault.** Of 55 failed jobs, 35 failed for another reason:
  27 on a package-mirror outage, all on #450; 6 on runners killed or lost, all on libc++ 22 legs; 1 on a gate's
  configuration; and 1 on a compiler crash.
- **Cost is concentrated.** The per-PR runner-minute medians sum to about 2,780 minutes. The four Windows workflows
  (Clang-CL, MSVC, MinGW, MSVC-Analyze) take 1,060 of them (38%) and found 20 ledger bugs first, 9 of them upstream
  defects; Sanitizers alone takes 457 (16%) and has the longest median queue, 84 minutes. A further 42% of all runner
  minutes went to superseded runs.
