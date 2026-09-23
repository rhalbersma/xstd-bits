# Rebooting the bits franchise

> "The reasonable man adapts himself to the world: the unreasonable one persists
> in trying to adapt the world to himself. Therefore all progress depends on the
> unreasonable man."
>
> — George Bernard Shaw, *Man and Superman* (1903), "Maxims for Revolutionists"

[![Project Status: WIP](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/c%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-Boost-blue.svg)](https://opensource.org/licenses/BSL-1.0)
[![GCC](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml)
[![MinGW](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml)
[![Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml)
[![Clang-libc++](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml)
[![Apple Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml)
[![Clang-CL](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml)
[![MSVC](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml)
[![Coverage](https://codecov.io/gh/rhalbersma/xstd-bits/branch/main/graph/badge.svg)](https://codecov.io/gh/rhalbersma/xstd-bits)
[![Consumption](https://github.com/rhalbersma/xstd-bits/actions/workflows/consumption.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/consumption.yml)
[![Sanitizers](https://github.com/rhalbersma/xstd-bits/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/sanitizers.yml)
[![Clang-Tidy](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-tidy.yml)
[![MSVC-Analyze](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc-analyze.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc-analyze.yml)
[![CodeQL](https://github.com/rhalbersma/xstd-bits/actions/workflows/codeql.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/codeql.yml)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/rhalbersma/xstd-bits/badge)](https://scorecard.dev/viewer/?uri=github.com/rhalbersma/xstd-bits)

xstd-bits is **nine containers**: three readings of a block of bits — an ordered set of `std::size_t`, a sequence of `bool`, and the `bitset` that deliberately offers both — over three storages, which differ in whether size and capacity are static or dynamic: both static, a dynamic size within a static capacity, and both dynamic.

### Each one is the packing of a standard container, and speaks that container's vocabulary

The relationship is the one `std::flat_set` has to `std::set`: **a different representation under the same interface**, departing from it only where the representation forces a departure. `xstd::bit_vector` answers `std::vector<bool>`'s synopsis line for line; `xstd::bit_array<N>` answers `std::array<bool, N>`'s, `xstd::bit_inplace_vector<N>` answers `std::inplace_vector<bool, N>`'s, the three `bitset`s answer `std::bitset<N>`'s and `boost::dynamic_bitset<>`'s, and the three sets answer `std::set<std::size_t>`'s. Each is held to its counterpart by a checklist that spells that counterpart's synopsis out as a `requires`-expression and is asserted **on the counterpart first**, so a line the model itself cannot answer can never be asked of the packing.

The yardstick is the [current working draft](https://eel.is/c++draft/), not the standard the library compiles as. Where C++23 and the draft disagree the draft wins, and [design.md](doc/design.md) records which paper moved each line.

Three things the packing genuinely forces, and nothing else:

- **No nodes.** A position is a bit in a word, so there is nothing to unlink and hand over: `node_type`, `extract`, `insert(node_type&&)` and `merge` have no meaning here. `std::flat_set` drops the same four for the same reason.
- **A proxy reference.** A bit has no address, so `operator[]` returns a proxy, `pointer` names nothing, and `data()` goes with it. `std::vector<bool>` makes exactly this trade. Everything the standard asks *of* the proxy is here — the const-qualified assignment of [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html), `flip()`, and the three hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html).
- **Different invalidation — mostly the other way.** An iterator here is a container and an index, not a pointer into the blocks, so growth that reallocates the blocks leaves it valid. That is `std::set`'s guarantee over storage that is `std::flat_set`'s.

Everything else is addition rather than subtraction: the bitwise operators, `find_first`/`find_next`, the subset and intersection tests, the byte exchange, and the views that give you a second reading of bits you already own.

### Out of range means what each counterpart means by it

The **set reading** takes a key, and a key outside the domain is a lookup that answers no rather than an error: `contains`, `find`, `count`, `lower_bound`, `upper_bound` and `equal_range` are total, as they are on `std::set`. The **bitset reading** keeps `std::bitset`'s checked members and their `out_of_range`; a dynamic width asked to grow past `max_size()` throws `std::length_error`, as a container does for a size it cannot represent. The **sequence reading** indexes, so out of range is out of bounds there: `at(n)` throws `out_of_range` at every width and through every handle, and everything else is the precondition `std::vector`, `std::array` and `std::span` already make it — stated with an `assert`, at the member you called.

`constexpr` is not on that list of advantages, and has not been since [P3372R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3372r3.html) made the standard's own containers `constexpr` throughout. It is table stakes now; what the packing buys is density and the bit-parallel operations over it.

## Usage

### Hello World: generating (twin) primes

The code below demonstrates how a standards conforming `set` implementation can be used to implement the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes). This algorithm generates all [prime numbers](https://en.wikipedia.org/wiki/Prime_number) below a number `n`. It is the library's worked example and its benchmark, and it lives in [`examples/include/opt/set/sieve.hpp`](examples/include/opt/set/sieve.hpp) — outside `include/`, because a workload is a subject rather than library code. The listing below is that header's body, not a paraphrase of it; the names are in namespace `opt`.

```cpp
template<class X>
auto sift(X& primes, std::size_t m)
{
        primes.erase(m);
}

template<class X>
auto generate_candidates(std::size_t n)
{
        return std::views::iota(2UZ, n) | std::ranges::to<X>();
}

// Iterate a snapshot and guard with contains(): sift() erases, which invalidates a vector-backed X's cached end().
template<class X>
auto sift_primes0(std::size_t n)
{
        auto primes = generate_candidates<X>(n);
        auto const candidates = primes;
        for (auto p
                : candidates
                | std::views::take_while([&](auto x) { return x * x < n; })
        ) {
                if (not primes.contains(p)) {
                        continue;
                }
                for (auto m = p * p; m < n; m += p) {
                        sift(primes, m);
                }
        }
        return primes;
}
```

Two details are worth pausing on, because both were learned the hard way. The inner walk is `m += p` rather than `views::iota(p * p, n) | views::stride(p)`: `views::stride` is a C++23 adaptor that libc++ has not implemented, and the library supports all three standard libraries. And the outer loop iterates a **snapshot** while guarding with `contains()`, because `sift()` erases from `primes`, which invalidates a cached `end()` for any vector-backed container — `std::flat_set` among them.

Given a set of primes, generating the [twin primes](https://en.wikipedia.org/wiki/Twin_prime) means keeping every prime that has a neighbour exactly 2 away:

```cpp
template<class X>
auto filter_twins(X const& primes)
{
        using key = std::ranges::range_value_t<X>;
        auto twins = X();
        auto first = std::ranges::begin(primes);
        auto const last = std::ranges::end(primes);
        if (first == last) {
                return twins;
        }
        auto prev = static_cast<key>(*first++);
        if (first == last) {
                return twins;
        }
        auto self = static_cast<key>(*first++);
        for (; first != last; ++first) {
                auto const next = static_cast<key>(*first);
                if (self - 2 == prev or self + 2 == next) {
                        twins.insert(self);
                }
                prev = self;
                self = next;
        }
        return twins;
}
```

This returns **both** members of each twin pair — `{3, 5, 7, 11, 13, ...}`, [OEIS A001097](https://oeis.org/A001097) — rather than the lesser of each pair, `{3, 5, 11, 17, ...}`, [A001359](https://oeis.org/A001359). Both are called "the twin primes" in the wild, so the choice is worth stating.

The calling code for these algorithms is listed below. Here, the pretty-printing as a `set` using `{}` delimiters is triggered by the nested `key_type`. Note that `xstd::bit_static_set<N>` acts as a **drop-in replacement** for `std::set<int>` (or `std::flat_set<int>`).

```cpp
int main()
{
    constexpr auto N = 100UZ;
    using X = xstd::bit_static_set<N>; /* or xstd::bit_set, std::set<std::size_t>, std::flat_set<std::size_t> */

    auto const primes = opt::sift_primes0<X>(N);
    assert(std::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto const twins = opt::filter_twins(primes);
    assert(std::format("{}", twins)  == "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}");
}
```

Those two assertions are the ones [`test/src/bits/std_set/sieve.cpp`](test/src/bits/std_set/sieve.cpp) makes, over `std::set`, `std::flat_set`, `bit_static_set<N>` and `bit_set` alike.

### Sieves that need no bound

`generate_candidates` materializes every candidate below `n` before sifting one, which is what makes the sieve above `O(n)` in space and why it cannot answer "what is the next prime". The same header carries two variants that drop the bound, and they drop it in opposite directions:

```cpp
auto sieve = opt::incremental_sieve();
sieve.next();  // 2, then 3, 5, 7, ... forever, with no n anywhere

auto primes = opt::sift_primes_segmented<xstd::bit_set, xstd::bit_static_set<1 << 15>>(n);
```

The **incremental** sieve ([O'Neill 2009](https://www.cs.hmc.edu/~oneill/papers/Sieve-JFP.pdf)) keeps one entry per prime found — the next composite that prime will strike — so its space is `O(π(n))` and it generates without end. It is also about **29× slower**, which is the honest price of unboundedness and the reason it is measured rather than recommended.

The **segmented** sieve is the one that pays. Base primes below `√n` once, then a single reusable window walked over the rest, so peak memory is `O(√n + W)` whatever `n` is. `Window` is a template parameter carrying its own extent, which makes `bit_static_set<W>` the natural argument: a compile-time width that allocates nothing in the loop. At `n = 2^20` it is **1.7× faster** than the bounded sieve as well as far thriftier — the window stays in L1 for a whole segment where a megabit sieve is walked with a stride. Less memory *and* less time is the unusual direction for that trade to run.

All three agree, and the test asserts that rather than the README claiming it.

## Headers

Nine containers: three readings of a block of bits, each over three storages.
The reading picks the vocabulary, the storage picks whether size and capacity
are static or dynamic.

| Header | Additions | Description | Reference |
| :----- | :-------- | :---------- | :-------- |
| `<xstd/bits/bit_static_set.hpp>` | `bit_static_set` <br> `basic_bit_static_set` | Ordered set of `std::size_t`, static size and capacity | [associative.reqmts], [set] |
| `<xstd/bits/bit_inplace_set.hpp>` | `bit_inplace_set` <br> `basic_bit_inplace_set` | Ordered set, dynamic size within a static capacity | [associative.reqmts], [set] |
| `<xstd/bits/bit_set.hpp>` | `bit_set` <br> `basic_bit_set` | Ordered set, dynamic size and capacity | [associative.reqmts], [set] |
| `<xstd/bits/bit_array.hpp>` | `bit_array` <br> `basic_bit_array` | Sequence of `bool`, static size and capacity | [array] |
| `<xstd/bits/bit_inplace_vector.hpp>` | `bit_inplace_vector` <br> `basic_bit_inplace_vector` | Sequence of `bool`, dynamic size within a static capacity | [inplace.vector] |
| `<xstd/bits/bit_vector.hpp>` | `bit_vector` <br> `basic_bit_vector` | Sequence of `bool`, dynamic size and capacity | [vector.bool] |
| `<xstd/bits/bitset.hpp>` | `bitset` <br> `basic_bitset` | Both readings at once, static size and capacity | [template.bitset] |
| `<xstd/bits/inplace_bitset.hpp>` | `inplace_bitset` <br> `basic_inplace_bitset` | Both readings, dynamic size within a static capacity | [template.bitset] |
| `<xstd/bits/dynamic_bitset.hpp>` | `dynamic_bitset` <br> `basic_dynamic_bitset` | Both readings, dynamic size and capacity | [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/dynamic_bitset.html) |
| `<xstd/bits/ext/boost.hpp>` | `bit_small_set` <br> `bit_small_vector` <br> `small_bitset` <br> and their `basic_` forms | All three readings, dynamic size staying inline within a static capacity | [`boost::container::small_vector`](https://www.boost.org/doc/libs/release/doc/html/boost/container/small_vector.html) |
| `<xstd/bits/bit_set_view.hpp>` | `bit_set_view` | Set reading of bits another container owns | none |
| `<xstd/bits/bit_span.hpp>` <br> `<xstd/bits/bit_subspan.hpp>` | `bit_span` <br> `bit_subspan` | Sequence reading over borrowed bits, whole or sliced | [views.span] |
| `<xstd/bits/borrowed_bits.hpp>` | `borrowed_bits` <br> `borrow_bits` | Bits in words someone else owns, for `bit_set_view` and `bit_span` to read and write in place | [views.span] |
| `<xstd/bits/contiguous_bit_sequence.hpp>` | `contiguous_bit_sequence` | What a storage must model to be adapted | none |

`<xstd/bits.hpp>` exports the whole surface, so one include brings everything above.
The headers under `<xstd/bits/detail/>` are implementation and carry no stability promise.

## Requirements

- A conforming [C++23](https://wg21.link/N4950) compiler
- CMake 3.28 or later when using the supplied CMake project
- [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc), fetched automatically via CMake `FetchContent` when not already installed
- [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support

This library depends on the C++ Standard Library, on [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc) (both fetched automatically via CMake `FetchContent` when not already installed), and on [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support. It is continuously being tested with the following conforming [C++23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf) compilers, against all three mainstream standard libraries (libstdc++, the MSVC STL, and libc++). Following the model of [apt.llvm.org](https://apt.llvm.org/), we support the latest two stable releases of each compiler, plus its current development branch.

Two standards are in play and they answer different questions. **C++23 is what the library compiles as** — that is the language it requires, and the tests build at C++26 as well — on GCC 16, GCC 17-SVN and MinGW 16, the three rungs whose libstdc++ carries `<inplace_vector>` — which is what reaches the inplace column. **The current [working draft](https://eel.is/c++draft/) is what the interfaces are measured against**, because a counterpart's synopsis is a moving target and the newest one is the one worth answering. So `std::bitset`'s `basic_string_view` constructor ([P2697R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2697r1.pdf)) and its `charT` Constraints ([LWG 4294](https://cplusplus.github.io/LWG/issue4294)) are here even though C++23 has neither, and the proxy carries the hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html) beside the static `swap(reference, reference)` that paper moved to `[depr.vector.bool.swap]`.

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## Installation


The library is header-only and its CMake target carries everything a consumer needs: the include directories, the `xstd-ints`, `xstd-misc` and `Boost::hash2` dependencies, and `cxx_std_23`. Link `xstd::bits` and you are done — there is no `target_include_directories` or `CMAKE_CXX_STANDARD` to set on your side.

All three methods below are built by the [Consumption workflow](.github/workflows/consumption.yml), on every pull request and on every push to `main`, so what is written here is what is tested.

### `find_package`, against an installed copy

```cmake
find_package(xstd-bits 0.1.0 CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE xstd::bits)
```

Installing needs no test dependencies:

```sh
cmake --preset no-tests-vcpkg          # or --preset no-tests, if Boost.Hash2 is already findable
cmake --build --preset no-tests-vcpkg
cmake --install build/no-tests-vcpkg --prefix /where/you/want/it
```

The installed package config calls `find_dependency` for `xstd-ints`, `xstd-misc` and `boost_hash2`, so those three have to be findable from the consuming project too. The version file is written `SameMinorVersion`, so `find_package(xstd-bits 0.1.0)` accepts 0.1.x and rejects 0.2.0.

### `add_subdirectory`, against a vendored copy

```cmake
add_subdirectory(external/xstd-bits)
target_link_libraries(my_target PRIVATE xstd::bits)
```

`xstd::bits` is an alias of the real target, so it spells the same thing here as under `find_package`. The tests and benchmarks are guarded by `PROJECT_IS_TOP_LEVEL` and do not configure when the library is a subdirectory, whatever your `BUILD_TESTING` is set to.

### `FetchContent`, against the repository

```cmake
include(FetchContent)
FetchContent_Declare(
    xstd-bits
    GIT_REPOSITORY https://github.com/rhalbersma/xstd-bits.git
    GIT_TAG        main          # pin a commit for a reproducible build
)
FetchContent_MakeAvailable(xstd-bits)
target_link_libraries(my_target PRIVATE xstd::bits)
```

`xstd-ints` and `xstd-misc` are fetched in turn if they are not already installed, at the commits `CMakeLists.txt` pins. Boost.Hash2 is not fetched — it is a `find_package(... REQUIRED)`, so it has to be installed, whether through vcpkg, your distribution, or a Boost tree you already have.

### The vcpkg manifest

[`vcpkg.json`](vcpkg.json) is this repository's own manifest, not a published port: it is what `VCPKG_ROOT`-based presets install from when you build **this** library. Its `test` feature — Boost.Test, Boost.Dynamic Bitset, Google Benchmark and range-v3 — is a default feature because building the repository normally means building its tests. The `no-tests-vcpkg` preset turns that off with `VCPKG_MANIFEST_NO_DEFAULT_FEATURES`, so a packaging or install build pays for Boost.Hash2 and nothing else. Consuming the library by any of the three methods above does not read this manifest at all.

## Continuous Integration

We continuously test the stable, qualification, and development branches of the
major [C++23](https://wg21.link/N4950) toolchains (compilers and standard
libraries) in both Debug and Release mode:

| Platform | Compiler | Standard Library | Stable | Qualification | Development | Status |
| :------- | :------- | :--------------- | :----- | :------------ | :---------- | :----- |
| Linux | GCC | libstdc++ | 15 | 16 | 17-SVN | [![GCC](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml) |
| Windows | MinGW | libstdc++ | 15 | 16 | — | [![MinGW](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml) |
| Linux | Clang | libstdc++ | 22 (libstdc++ 15) | 23 (libstdc++ 16) | 24-SVN (libstdc++ 17-SVN) | [![Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml) |
| Linux | Clang | libc++ | 22 | 23 | 24-SVN | [![Clang-libc++](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml) |
| macOS | Apple Clang | libc++ | 17.0.0 (Xcode 16.4) | 21.0.0 (Xcode 26.6) | — | [![Apple Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml) |
| Windows | Clang-CL | MSVC | 19.1.5 (VS 2022) | 20.1.8 (VS 2026) | 20.1.8 (VS 2026-Preview) | [![Clang-CL](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml) |
| Windows | MSVC | MSVC | 2022 | 2026 | 2026-Preview | [![MSVC](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml) |

`Apple Clang` has no `Development` entry because Apple doesn't publish Apple Clang dev snapshots the way LLVM does; that leg tests the latest stable Xcode release from each of the two supported series. `MinGW` has none either: WinLibs publishes no GCC trunk build between stable branches, and the shared workflow drops that rung with a notice rather than failing, so the row names the two that actually run. The `Linux | GCC` row keeps its `17-SVN` — that ladder is unaffected.

`MSVC` has its `Stable` entry back, and what it cost is worth recording. Two things stood between MSVC 17 (VS 2022) and this library. The first was alias-template deduction: `bit_set_view`, `bit_span` and `bit_subspan` were aliases, and `error C2976: 'xstd::bit_set_view': too few template arguments` is what MSVC 17 made of that. Making the views class templates with their own deduction guides settled it, for reasons of its own. The second was a ledger the rung's absence had opened: with no MSVC 17 leg to answer to, `decay_copy` became `auto(x)` ([P0849R8](https://wg21.link/P0849R8), which MSVC 17 does not implement) and a `typename` came off a constrained type-parameter's default argument. Both are back — a three-line helper in each of two adaptors, and one keyword under a `NOLINT` — which is the whole price of the rung. [design.md#the-views-are-the-adaptors](doc/design.md#the-views-are-the-adaptors) records the trade.

Every leg above passes. The library no longer uses the C++23 range adaptors libc++ has not implemented (`views::cartesian_product`, `views::adjacent`, `views::pairwise_transform`, `views::stride`), and the tests probe for `<flat_set>` rather than assuming it, so the `Clang | libc++` and `Apple Clang` rows and the `Clang-CL` VS 2022 legs build and run like the rest.

## License


<pre>
         Copyright Rein Halbersma 2014-2026.
Distributed under the <a href="http://www.boost.org/users/license.html">Boost Software License, Version 1.0</a>.
   (See accompanying file LICENSE_1_0.txt or copy at
         <a href="http://www.boost.org/LICENSE_1_0.txt">http://www.boost.org/LICENSE_1_0.txt</a>)
</pre>
