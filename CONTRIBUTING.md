# Contributing to xstd-bits

## What a PR must satisfy before it can merge

This repository enforces its quality bar through CI rather than through review discretion. A PR is mergeable once every required check below is green.

- **Every compiler/platform leg passes.** See the table in [README.md](README.md) for the current matrix (GCC, Clang, Clang libc++, Clang-CL, MSVC, MinGW, Apple Clang). Every leg counts, including every `Development` leg (`17-SVN`, `24-SVN`, `2026-Preview`), the `libc++` legs, and the `Clang-CL` VS 2022 legs; none of them are advisory. Each ladder workflow ends in an `all` job that is red unless every one of its legs succeeded, and that gate is what branch protection requires: a leg name changes whenever a rung moves, the gate name does not.
- **`clang-tidy` is clean.** [`.clang-tidy`](.clang-tidy) sets `WarningsAsErrors: '*'`, so any finding over the public headers fails the job outright.
- **MSVC's `/analyze` is clean.** The [MSVC-Analyze workflow](.github/workflows/msvc-analyze.yml) fills the same role on the MSVC side, on the two Visual Studio 2026 rungs the `msvc` ladder covers.
- **No sanitizer failures.** The [Sanitizers workflow](.github/workflows/sanitizers.yml) runs ASan+LSan, UBSan and the implicit-conversion sanitizer, against both libstdc++ and libc++. Leak detection is on.
- **The public headers stay self-sufficient.** Each header is compiled as its own translation unit (see `test/CMakeLists.txt`); don't rely on include order from another header.
- **Workflow files pass `actionlint`.** The [Actionlint workflow](.github/workflows/actionlint.yml) validates GitHub Actions syntax and expressions.
- **The documented consumption methods work.** The [Consumption workflow](.github/workflows/consumption.yml) builds a consumer using `find_package`, `add_subdirectory`, and `FetchContent`.
- **CodeQL analysis is clean.** The [CodeQL workflow](.github/workflows/codeql.yml) runs the C/C++ `security-extended` query suite.
- **Line and branch coverage are both 100%, project-wide and for the PR's own diff.** The [Coverage workflow](.github/workflows/coverage.yml) passes `--fail-under-line 100 --fail-under-branch 100`, and [`codecov.yml`](.github/codecov.yml) sets both Codecov statuses to a 100% target with zero tolerance. New code needs a test that exercises every line and branch it adds; existing coverage may not regress.

  Excluded from that bar, by the shared workflow: `assert(...)` contract checks, compiler-synthesized `= default;` members, the exception-unwinding branches gcov attaches to any call that could throw, and a line holding nothing but a closing brace — which carries no code of yours, only the unwind resume for a function with a local to destroy.

  **Code that cannot be tested does not get an exemption; it gets commented out.** If you cannot reach a line from a test, comment it out and say why, so that whoever uncomments it has to write the test. One arm in the library is unreachable rather than untested: `bit_array<0>::is_valid`'s empty case, only ever called from an `assert` that a zero-size array has no member to reach, and not removable because MSVC's `/W4` rejects a bare `n < N` as always false when `N` is zero. It carries a trailing `// GCOVR_EXCL_LINE`, which drops that one line from the denominator rather than counting it as reached. The marker has to sit on the line it excludes — above or below it does nothing — and `grep -rn EXCL_LINE include/` is the whole list of such concessions. Keep it short.

  Keep an `assert` on a line of its own, as the library does. The workflow drops assert branches by matching the start of a line, so an assert sharing a line with real code keeps a branch no test can take.

  Write a conditional across lines rather than packing it onto one. gcov counts per line, so a single-line `if`/`else` can only read as wholly covered or wholly uncovered, and a half-tested one reads as covered.

## Test layout and naming

The test tree mirrors the header tree, and `test/CMakeLists.txt` fails configuration if it ever stops doing so: `include/xstd/bits/ranges/set_view.hpp` is answered by `test/src/bits/ranges/set_view.cpp`. The rule runs one way — every public header needs a source, `detail/` excepted, being machinery rather than interface. It does not forbid a source that answers no header, and some directories are exactly that.

**A directory whose name is a header's is that header's tests. A `std_`-prefixed one is a contract's.** The sweeps in `bits/std_set/` and `bits/std_bitset/` are not tests of `std::set` or `std::bitset`, and not tests of ours either: each runs one Standard clause's requirements — `[set]`, `[bitset]` — over every type claiming to meet it, ours and the standard library's and Boost's alike, so the claim is checked against the same primitives on all of them. `std_` names which clause; it does not name a type. `bits/std_array/` follows for the sequence contract.

That prefix is also what keeps `bits/std_bitset/` from being read as more tests of `xstd::bits::bitset` — those are in `bits/bitset.cpp`, one mirrored source, sitting right beside it. Note the near-neighbour it does *not* mean either: `bits/ext/std/bitset.cpp` (`Ext` / `Std` / `Bitset`) tests the adaptor that teaches `std::bitset` to be viewed, one header, and is mirrored like any other.

`bits/block/` carries no prefix on purpose. Block is this library's own named requirement rather than a Standard clause, so there is no `std_` to claim.

**In a clause sweep no implementation gets a shorter name than another.** Every candidate is spelled with its namespace — `std::set`, `std::flat_set`, `xstd::bit_static_set`; `std::bitset`, `boost::dynamic_bitset`, `xstd::bitset` — and these sources carry no `using namespace xstd;`. Ours is one implementation among those being checked and gets no name lookup it has not earned: an unqualified `bit_static_set` sitting in a list where `std::flat_set` had to be spelled out makes ours read as the local default, which is exactly the thing a conformance sweep must not assume. The harness directives (`test`, `test::set`, `test::bitset`) stay — they name the checking primitives, which are applied identically to every candidate, so they privilege nobody.

**The harness sits outside the library, on purpose.** It is a top-level `namespace test`, reached as `<test/...>`, with no `xstd` above it. Nested inside `xstd` — where it used to be — an unqualified name in a harness header falls back to the library and finds *our* overload for every candidate, `std::` ones included, at which point the sweep quietly stops comparing implementations and starts confirming ours. That is not a discipline to keep up; there is now no enclosing namespace to fall back to.

It was not hypothetical. `set/composable.hpp` wrote `ranges::views::set_union` meaning range-v3's, and got it only because that header happens to sort before the one declaring `xstd::ranges` in all three sources that include it. Re-order the includes and it would have found `xstd::ranges` instead and stopped compiling. It says `::ranges` now, and the arrangement no longer depends on either.

Boost is worth knowing here, because it splits both ways: Boost.Unordered puts its harness in a top-level `namespace test` with no `using namespace boost` anywhere, while Boost.Container and Abseil nest theirs in the library namespace — and Boost.Container's tests show the cost, naming its own container unqualified 45 times against an always-spelled-out `std::`.

**Suites nest directory-wise.** One `BOOST_AUTO_TEST_SUITE` per path component under `test/src/`, the file stem included, each component PascalCased. That makes the suite path the CTest target id, spelled the other way round, so a failing target says where to look:

| source | target | suites |
| :--- | :--- | :--- |
| `test/src/bits/ranges/set_view.cpp` | `test.bits.ranges.set_view` | `Ranges` / `SetView` |
| `test/src/bits/ext/std/bitset.cpp` | `test.bits.ext.std.bitset` | `Ext` / `Std` / `Bitset` |
| `test/src/bits/std_set/o2.cpp` | `test.bits.std_set.o2` | `StdSet` / `O2` |

With one subtraction: `bits` is every source's first component, so as a suite it distinguishes nothing and is left out. `test/src/bits.cpp` is what remains — the umbrella over the whole library, and the one source whose cases sit in the master suite. Should this library ever grow a second top-level directory, `Bits` comes back at the front of every row above.

An umbrella source takes the stem like any other, so `test/src/bits/ranges.cpp` is `Ranges`, alongside the `Ranges` / `SetView` of the directory beside it. Nothing else goes in a suite name: the tier a sweep runs at (constant, linear, quadratic) belongs in its case names, not in place of the directory it lives in.

**Cases are declarative.** A case name is a sentence about what holds, with the thing under test as the implied subject — `TheOrderingsAgreeInsideASingleBlock`, `AWidthInTheTypeIsAStaticExtent`, `DefaultConstructionYieldsAnEmptySet`. A name that only says which members were exercised (`Observers`, `Operators`, `Constructors`) names where the test looked rather than what it claims, and reads as nothing at all in a failure log. A conformance check may be the predicate itself, since that already is the claim: `IsRegular`, `IsTrivial`, `IsABitSequence`.

This is [xstd](https://github.com/rhalbersma/xstd-ints)'s convention as well, including the subtraction: `Ints` goes the same way once xstd splits into xstd-ints and xstd-core and `ints/` stops distinguishing anything. The two libraries' test trees are meant to read the same way.

One check runs without being required:

- **`clang-format` does not run on a PR.** This repository has no `.clang-format`, so [its workflow](.github/workflows/clang-format.yml) is dispatch-only; enabling it means adopting a style and reformatting the tree.

The [Scorecard workflow](.github/workflows/scorecard.yml) cannot be required either: it runs on pushes to `main` and on a schedule, never on a pull request.

## Required status checks

The names to tick under branch protection, exactly as GitHub reports them:

| Check | Covers |
| :--- | :--- |
| `actionlint / actionlint` | workflow syntax |
| `apple_clang / all` | Xcode 16.4 and 26.6, Debug and Release |
| `clang / all` | Clang 22, 23, 24-SVN with libstdc++ |
| `clang_cl / all` | clang-cl on VS 2022, 2026, 2026-Preview |
| `clang_libcxx / all` | Clang 22, 23, 24-SVN with libc++ |
| `clang_tidy / all` | clang-tidy on all three rungs |
| `codeql / Analyze` | `security-extended` |
| `consumption / Consume` | the three CMake consumption models |
| `coverage / gcovr` | 100% of lines and branches |
| `gcc / all` | GCC 15, 16, 17-SVN |
| `mingw / all` | MinGW 15 and 16 |
| `msvc / all` | cl on VS 2026 and 2026-Preview |
| `msvc_analyze / all` | `/analyze` on the same two rungs |
| `sanitizers / all` | all fifteen sanitizer legs |

Codecov posts two more, `codecov/project` and `codecov/patch`, which carry the same 100% bar for the whole tree and for the diff.

## License

By contributing, you agree that your contributions will be licensed under the [Boost Software License, Version 1.0](LICENSE_1_0.txt), the same license that covers the rest of this repository.
