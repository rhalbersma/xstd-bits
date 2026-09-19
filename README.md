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

The yardstick is the [current working draft](https://eel.is/c++draft/), not the standard the library compiles as. Where C++23 and the draft disagree the draft wins, and [design.md](design.md) records which paper moved each line.

Three things the packing genuinely forces, and nothing else:

- **No nodes.** A position is a bit in a word, so there is nothing to unlink and hand over: `node_type`, `extract`, `insert(node_type&&)` and `merge` have no meaning here. `std::flat_set` drops the same four for the same reason.
- **A proxy reference.** A bit has no address, so `operator[]` returns a proxy, `pointer` names nothing, and `data()` goes with it. `std::vector<bool>` makes exactly this trade. Everything the standard asks *of* the proxy is here — the const-qualified assignment of [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html), `flip()`, and the three hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html).
- **Different invalidation — mostly the other way.** An iterator here is a container and an index, not a pointer into the blocks, so growth that reallocates the blocks leaves it valid. That is `std::set`'s guarantee over storage that is `std::flat_set`'s.

Everything else is addition rather than subtraction: the bitwise operators, `find_first`/`find_next`, the subset and intersection tests, the byte exchange, and the views that give you a second reading of bits you already own.

### Out of range means what each counterpart means by it

The **set reading** takes a key, and a key outside the domain is a lookup that answers no rather than an error: `contains`, `find`, `count`, `lower_bound`, `upper_bound` and `equal_range` are total, as they are on `std::set`. The **bitset reading** keeps `std::bitset`'s checked members and their `out_of_range`; a dynamic width asked to grow past `max_size()` throws `std::length_error`, as a container does for a size it cannot represent. The **sequence reading** indexes, so out of range is out of bounds there: `at(n)` throws `out_of_range` at every width and through every handle, and everything else is the precondition `std::vector`, `std::array` and `std::span` already make it — stated with an `assert`, at the member you called.

`constexpr` is not on that list of advantages, and has not been since [P3372R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3372r3.html) made the standard's own containers `constexpr` throughout. It is table stakes now; what the packing buys is density and the bit-parallel operations over it.

## Design choices for a `bitset` data structure

> "A `bitset` can be seen as either an array of bits or a set of integers. [...]
> Common usage suggests that dynamic-length `bitsets` are seldom needed."
>
> Chuck Allison, [ISO/WG21/N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf), November 25, 1991

The above quote is from the first C++ Standard Committee proposal on what would eventually become `std::bitset<N>`. The quote highlights two design choices to be made for a `bitset` data structure:

1. a sequence of `bool` versus an ordered set of `int`;
2. fixed-size versus variable-size storage.

Thirty years of use have added a value to each axis. The first choice has a third answer that the quote itself takes for granted — a `bitset` that offers **both** readings on purpose, which is what `std::bitset` and `boost::dynamic_bitset` actually are. And the second is not one choice but two, because **size** and **capacity** need not move together: fixing both gives `std::bitset`, letting both vary gives `std::vector<bool>`, and the pairing the quote had no word for is a **dynamic size over static capacity**, which allocates nothing and yet resizes, and which C++26's `std::inplace_vector` finally makes expressible. Both tables below are laid out on that one axis, so they read cell for cell.

A `bitset` should also optimize for both space (using contiguous storage) and time (using CPU-intrinsics for data-parallelism) wherever possible.

## The current `bit` landscape

The landscape is three readings crossed with three storages, so it is a three-by-three, and the C++ Standard Library and Boost between them fill six of its nine cells:

|                                  | static size and capacity     | dynamic size, static capacity        | dynamic size and capacity |
| :------------------------------- | :--------------------------- | :----------------------------------- | :------------------------ |
| **ordered set of `std::size_t`** | —                            | —                                    | `std::set<std::size_t>` <br> `std::flat_set<std::size_t>` |
| **sequence of `bool`**           | `std::array<bool, N>`        | `std::inplace_vector<bool, N>`       | `std::vector<bool>` |
| **`bitset`**                     | `std::bitset<N>`             | —                                    | `boost::dynamic_bitset<>` |

Three cells are empty and three more are filled by something of the **wrong representation**, which are two different complaints:

- `std::set` and `std::flat_set` are **sparse**: a whole element per *actual* element, where a dense set spends a single bit per *potential* one.
- `std::array<bool, N>` and `std::inplace_vector<bool, N>` are **unpacked**: a whole byte per `bool`, eight times what the bits need. `std::vector<bool>` is the one standard sequence that packs, and it is the one everybody regrets.

Notes:

1. Both `std::bitset` and `boost::dynamic_bitset` are not clear about the interface they provide. E.g. both offer a hybrid of sequence-like random element access (through `operator[]`) as well as primitives for set-like bidirectional iteration (using non-Standard GCC extensions `_Find_first` and `_Find_next` in the case of `std::bitset`). That ambiguity is why they have a row of their own here rather than appearing in one of the two above it: a `bitset` is a width of bits that has not said how it is to be read.
2. It [has been known](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2130.html#96) for over two decades that providing a variable-size sequence of `bool` through specializing `std::vector<bool>` was an unfortunate design choice.
3. For ordered sets, there is a further design choice whether to optimize for **dense** sets or for **sparse** sets. Dense sets require a single bit per **potential** element, whereas sparse sets require a whole element per **actual** element. The break-even is therefore the element's width: at a 64-bit `std::size_t`, if less (more) than 1 in 64 elements (1.5625%) are actually present, a dense representation will be less (more) compact than a sparse one — 1 in 32 (3.125%) if the sparse set stores 32-bit integers instead.
4. `std::inplace_vector` is C++26 and its `bool` is not a specialization: [P0843](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0843r7.html) declined to repeat `vector<bool>`, so that cell is occupied but unpacked.
5. Only `boost::dynamic_bitset` allows storage configuration through its `Block` template parameter (defaulted to `unsigned long`).
6. The `Block`, `Allocator` and `Compare` parameters are omitted from both tables. They configure a storage; they do not place a container in the landscape, and spelling them out obscured the one axis the columns are measuring.

## A reimagined `bit` landscape

The aforementioned issues can be resolved by implementing a single-purpose container for each cell of the design space. **This library fills all nine**, every one of them dense and packed, and adds a fourth row that the other table has no answer to at all. Every name below is in namespace `xstd`, so only the foreign ones carry a qualifier:

|                                  | static size and capacity     | dynamic size, static capacity     | dynamic size and capacity |
| :------------------------------- | :--------------------------- | :-------------------------------- | :------------------------ |
| **ordered set of `std::size_t`** | `bit_static_set<N>`          | `bit_inplace_set<N>`              | `bit_set` |
| **sequence of `bool`**           | `bit_array<N>`               | `bit_inplace_vector<N>`           | `bit_vector` |
| **`bitset`**                     | `bitset<N>`                  | `inplace_bitset<N>`               | `dynamic_bitset` |
| **a reading of a `bitset`**      | `bit_set_view` / `bit_span` over <br> `bitset<N>`, `std::bitset<N>` | over <br> `inplace_bitset<N>` | over <br> `dynamic_bitset`, `boost::dynamic_bitset<>` |

The columns are the three storages the one underlying vehicle is parameterized on — `std::array` fixes size and capacity, `std::inplace_vector` varies size within a fixed capacity, `std::vector` varies both — so a cell is a reading crossed with a storage, and nothing else. The outer two columns are the ones the current landscape already has; the middle is the pairing it never named.

The first three rows are **containers**, one per cell, and the fourth is not a container at all. A `bitset` is a width of bits that has not said how it is to be read, so the row above gives you the bits and this one is how you say which reading you meant — over one of ours, or over the **standard or Boost** bitset of the same shape, which is the whole of what [retrofitting](#choosing-a-reading-over-a-bitset) means. You do not have to adopt a container to get a reading; you point a view at the bits you already have. The middle column has one bitset to view rather than two, a run-time size over static capacity being the pairing neither the standard nor Boost has anything in.

Notes:

1. Each container in the first two rows is clear about the interface it provides: sequences are random access containers and ordered sets are bidirectional containers. The third row is the deliberate exception, and the fourth is what resolves it.
2. The `bitset` row is the point the old two-by-two could not express. `std::bitset` and `boost::dynamic_bitset` are faulted above for being unclear about which interface they offer; the answer here is not to abolish the hybrid but to make choosing between its two readings **explicit at the call site**. `xstd::bitset<N>` is a strict extension of `std::bitset<N>` and `xstd::dynamic_bitset` is one of `boost::dynamic_bitset<>` — every expression valid on the counterpart is valid here, with the same result — and neither has iterators of its own, because `begin` is one name and there are two readings. `bit_set_view` and `bit_span` are how you say which you meant, which is why they are a row and not a cell.
3. A view over a bitset is a view over the storage that bitset wraps: `xstd::bit_set_view(bs)` deduces `xstd::bit_set_view<xstd::detail::bits::contiguous_bit_array<std::size_t, N>>`, and `decltype` is how you name the result. The deduction guide for a plain storage is constrained to non-owners, so an owner and the storage inside it do not tie.
4. The variable-size sequence of `bool` is named `xstd::bit_vector` and decoupled from the general `std::vector` class template.
5. All containers use a dense (single bit per element) representation. Variable-size sparse sets can be provided by `flat_set`, either in [Boost](https://www.boost.org/doc/libs/1_80_0/doc/html/boost/container/flat_set.html) or in [C++ 23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1222r4.pdf).
6. The names above are the short ones, which fix `Block` to `std::size_t` and so take only the width, or nothing at all in the dynamic column where there is no width to give. Each has a `basic_` form that leaves the block open: `xstd::basic_bit_static_set<Block, N>`, `xstd::basic_bit_array<Block, N>`, `xstd::basic_bitset<Block, N>` and their inplace siblings, and `xstd::basic_bit_set<Block, Allocator>`, `xstd::basic_bit_vector<Block, Allocator>`, `xstd::basic_dynamic_bitset<Block, Allocator>` down the dynamic column. So `xstd::bit_set` is an alias, not a template, and `xstd::basic_bit_set<std::uint8_t>` is how a block is chosen.
7. Each static-width name has an `aligned` form in a nested namespace, its width rounded up to whole blocks so that no block carries an unused tail: `xstd::aligned::bitset<120>` is `xstd::bitset<128>`. That costs nothing in storage at a width already spanning whole blocks, and removes the tail-restoring mask from `fill`, `flip` and the left shift.

The **middle column** is what allocates nothing and yet carries a run-time width. It depends on `std::inplace_vector`, so those three names exist only where the standard library provides it (`__cpp_lib_inplace_vector`); an alias withholds a name rather than a capability.

Ownership is deliberately **not** a fourth **column**. A view is not a fourth storage: it takes the shape of whatever it views, which is why the fourth row spans the same three columns as the three above it rather than standing beside them. `xstd::bit_subspan` is the one that stays out of the table, because it narrows a sequence to a window rather than choosing a reading. All of them are described under [retrofitting](#choosing-a-reading-over-a-bitset) below.

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

## Requirements for `set`-like behaviour

Looking at the above code, the following four ingredients are necessary to implement the Sieve of Eratosthenes:

1. **Bidirectional iterators** `begin` and `end` in the `set`'s own namespace (to work with range-`for` and the `<ranges>` library);
2. **Constructors** taking a pair of iterators or a range (in order for `std::ranges::to` to construct a `set`);
3. A **nested type** `key_type` (in order for `std::format` to use `{}` delimiters: [format.range.fmtkind] picks `range_format::set` for a range that has one);
4. A **member function** `erase` to remove elements (for other applications: the rest of a `set`'s interface).

`xstd::bit_static_set<N>` implements all four of the above requirements. Note that Visual C++ support is finicky at the moment because its `<ranges>` implementation cannot (yet) handle the `xstd::bit_static_set<N>` proxy iterators and proxy references correctly.

## Choosing a reading over a bitset

`xstd::bitset<N>` is a strict extension of `std::bitset<N>`, which means it inherits the ambiguity too: it is
neither a set nor a sequence, and it has no `begin` of its own, because `begin` is one name and there are two
readings. `xstd::bit_set_view` and `xstd::bit_span` are how you say which you meant.

```cpp
auto bs = xstd::bitset<100>();
auto const s = xstd::bit_set_view(bs);   // the set reading of those bits

s.insert(42);                            // bs.set(42)
assert(s.contains(42));                  // bs.test(42)
assert(std::format("{}", s) == "{42}"); // formats as a set, which a bitset cannot
```

The view supplies what the bitset lacks: bidirectional iterators, a nested `key_type`,
`insert`/`erase`/`contains`, and the set predicates. `xstd::bit_span` does the same for the sequence reading,
and `xstd::bit_subspan` for a window into one. A view binds the storage the bitset wraps, so it reaches the
word-parallel paths rather than reading a position at a time.

This is why ownership is not a fourth column of the table above: a view is not a fourth kind of container, it is
the same three readings pointed at storage someone else owns.

### Printing

The snippets above use `std::format`, and `std::print` works the same way, with nothing to include beyond `<format>` and nothing to switch on:

```cpp
std::print("{}\n", primes);   // {2, 3, 5, 7, 11, ...}   the set reading, in braces
std::print("{}\n", flags);    // [false, true, ...]      the sequence reading, in brackets
std::print("{::#x}\n", primes);
```

Each proxy reference carries its own `std::formatter`, so a container that hands the proxy out brings the formatter with it. Nothing is specialized for a container: every one of them is already a range, so [`[format.range.formatter]`](https://eel.is/c++draft/format.range.formatter) formats it once its reference is formattable. The braces-versus-brackets split is the standard's, not ours — `[format.range.fmtkind]` picks `range_format::set` for a range with a `key_type` — so each reading prints in its own vocabulary without being told to. Each proxy also keeps a hidden-friend `format_as`, which is fmt's own per-type hook, so a consumer who formats with fmt gets the same output without this library depending on fmt to build or to test. The two hooks cannot drift: the `std::formatter` reads the value by calling `format_as`, rather than reaching for it a second way of its own.

## Data-parallelism

The `filter_twins` above walks the primes one at a time, because that is all `std::set` can do. A dense container can answer the same question a **word at a time**, without iterating at all. A prime is a twin exactly when it has a neighbour two away, so shifting the whole set by two in each direction and intersecting gives every twin in a handful of instructions per block:

```cpp
template<class X>
auto filter_twins_parallel(X const& primes)
{
    return primes & (primes << 2 | primes >> 2);
}
```

That is the same set the loop produces, and on a `bit_static_set<128>` it is four shifts, two ors and an and over two words — no iterator, no branch per element, no comparison. The elementwise form remains the one in `examples/include/opt/set/sieve.hpp`, because it is the form `std::set` and `std::flat_set` can also run and the benchmark needs all three on the same algorithm.

The shifts read the way they do because of the bit layout: element `0` is the least significant bit of the first word, so `<<` moves toward larger elements. What it does **not** do is make the set order the bitstring order — under this layout those are two different walks over the same blocks, and the FAQ below draws both.

which has as output:
<pre>
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}
{3, 5, 11, 17, 29, 41, 59, 71}
</pre>

### Sequence of bits

What a bitset lacks for the set reading to be built on top of it, apart from differently named member functions, is iterators. `xstd::bit_set_view` supplies them: bidirectional `begin`/`end` (and `cbegin`/`cend`/`rbegin`/`rend`/`crbegin`/`crend`) plus a nested `key_type`, so a bitset formats like a set out of the box — as the snippet under [choosing a reading](#choosing-a-reading-over-a-bitset) shows. It reads the storage a block at a time rather than a position at a time, which is what makes the scan word-parallel.

## Documentation

The interface for the class template `xstd::bit_static_set<N>` is the coherent union of the following building blocks:

1. An almost **drop-in** implementation of the full interface of `std::set<int>`.
2. An almost complete **translation** of the [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) member functions to the [`std::set<int>`](http://en.cppreference.com/w/cpp/container/set) naming convention.
3. The single-pass and short-circuiting **set predicates** from [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html).
4. The bitwise operators from [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) and [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html) reimagined as composable and data-parallel **set algorithms**.

The **full** interface of `xstd::bit_static_set` is `constexpr`.

### 1 An almost drop-in replacement for `std::set<int>`

`xstd::bit_static_set<N>` is a fixed-size ordered set of integers, providing conceptually the same functionality as `std::set<int, std::less<int>, Allocator>`, where `Allocator` statically allocates memory to store `N` integers. In particular, `xstd::bit_static_set<N>` has:

- **No customized key comparison**: `xstd::bit_static_set` uses `std::less<int>` as its fixed comparator (accessible through its nested types `key_compare` and `value_compare`). In particular, the `xstd::bit_static_set` constructors do not take a comparator argument.
- **No allocators**: `xstd::bit_static_set` is a fixed-size set of non-negative integers and does not dynamically allocate memory. In particular, `xstd::bit_static_set` does **not provide** a `get_allocator()` member function and its constructors do not take an allocator argument. Its allocating counterpart `xstd::bit_set` does provide both — the allocator follows the storage column, not the set reading.
- **No splicing**: `xstd::bit_static_set` is **not a node-based container**, and does not provide the splicing operations as defined in [p0083r3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0083r3.pdf). In particular, `xstd::bit_static_set` does **not provide** the nested types `node_type` and `insert_return_type`, the `extract()` or `merge()` member functions, or the `insert()` overloads taking a node handle.

- **No container exchange**: `std::flat_set` hands its underlying container out with `extract() &&` and takes one back with `replace(container_type&&)`, which is how you build one cheaply and how you get the sorted vector back out. `xstd::bit_static_set` has neither name, and has the capability twice over — see the `from_bits`/`to_bits` bullet below, and [the comparison in design.md](design.md#the-bytes-they-agree-on).

Minor **semantic differences** between common functionality in `xstd::bit_static_set<N>` and `std::set<int>` are:

- the `xstd::bit_static_set` member function `max_size` is `constexpr`, and at a static width its value is a constant expression usable wherever `N` is. It is a **member** rather than a `static` member function because `std::set`'s is a member: each reading takes the shape its own counterpart spells, which is why the sequence reading's middle column has a `static` one instead — `[inplace.vector.capacity]` spells all four of `capacity`, `max_size`, `reserve` and `shrink_to_fit` static there, and `xstd::bit_inplace_vector<N>::capacity()` answers without an object accordingly ([design.md#max-size-is-the-bits](design.md#max-size-is-the-bits)). So for the set reading `s.max_size()` is a constant expression and `decltype(s)::max_size()` does not compile.
- the `xstd::bit_static_set` iterators are **proxy iterators**, and taking their address yields **proxy references**. The difference should be undetectable. See the FAQ at the end of this document.
- the `xstd::bit_static_set` members `fill`, `complement` and `full` do not exist for `std::set`.
- `xstd::bit_static_set<N>` exchanges bits with any field of `N` bits through a **named pair**, `from_bits` and `to_bits<B>()`, and not through a constructor or a conversion operator. What they admit is named by a concept rather than by a type: an unsigned integer or a sequence of them, whose layout the language and the sequence state between them, or a field of bits whose layout `bit_castable` probes and proves — so `std::bitset<N>` and `xstd::bitset<N>` ride in on the same rule as `unsigned long long`, and an implementation that laid its bits out otherwise fails to compile rather than converting quietly. The widths are the same `N` and a static width is a capacity under this reading, so position `n` here is bit `n` there: nothing truncates, nothing grows, nothing throws, and the round trip is the identity. It is `constexpr` at every width, and a copy rather than a walk over positions.

  A name rather than a conversion, because the integer family is the one a set reader can still misread, and only a name answers it at the call site, where the reader is: `bit_static_set<32>::from_bits(5u)` is the set of positions the **value** five has, `{0, 2}`, not the set `{5}` ([design.md#the-bytes-they-agree-on](design.md#the-bytes-they-agree-on)). `from_bits` is a static factory on an owner; `to_bits<B>()` is also there on a set view, which spans a whole container and so has that container's bytes. The run-time-width `xstd::bit_set` has neither, a `std::bitset` naming one `N` that a growing set has no single value for.

With these caveats in mind, all fixed-size, defaulted comparing, non-allocating, non-splicing `std::set<int>` code in the wild should continue to work out-of-the-box with `xstd::bit_static_set<N>`.

### 2 An almost complete translation of `std::bitset<N>`

Almost all existing `std::bitset<N>` code has **a direct translation** (i.e. achievable through search-and-replace) to an equivalent `xstd::bit_static_set<N>` expression, with the same and familiar semantics as `std::set<int>` or `boost::flat_set<int>`.

| `std::bitset<N>`                | `xstd::bit_static_set<N>`              | Notes                                           |
| :---------------                | :-----------------              | :----                                           |
| `bs.set()`                      | `bs.fill()`                     | not a member of `std::set<int>`                 |
| `bs.set(n)`                     | `bs.add(n)` <br> `bs.insert(n)` | `out_of_range` past `N`, where `std::bitset` throws it too |
| `bs.set(n, v)` <br> `bs[n] = v` | `v ? bs.add(n) : bs.pop(n)`     | `out_of_range` past `N` on the insert; the erase is total |
| `bs.reset()`                    | `bs.clear()`                    | returns `void` as `std::set<int>`, not `*this` as `std::bitset<N>`  |
| `bs.reset(n)`                   | `bs.pop(n)` <br> `bs.erase(n)`  | total over the key: erasing what is not there is the no-op returning zero |
| `bs.flip()`                     | `bs.complement()`               | not a member of `std::set<int>`                 |
| `bs.flip(n)`                    | `bs.complement(n)`              | `out_of_range` past `N`, as the insert it is <br> not a member of `std::set<int>` |
| `bs.count()`                    | `bs.size()`                     | |
| `bs.size()`                     | `bs.max_size()`                 | `constexpr`; a constant expression at a static width |
| `bs.test(n)` <br> `bs[n]`       | `bs.contains(n)`                | total over the key: a position past `N` is one the set does not hold |
| `bs.all()`                      | `bs.full()`                     | not a member of `std::set<int>`                 |
| `bs.any()`                      | `not bs.empty()`                | |
| `bs.none()`                     | `bs.empty()`                    | |

The semantic differences between `xstd::bit_static_set<N>` and `std::bitset<N>` are:

- `xstd::bit_static_set<N>` answers `max_size()` as a `constexpr` member, where `std::bitset<N>` answers the same question with `size()`;
- `xstd::bit_static_set<N>` splits its members by what `[set]` can promise. **Asking is total**: `contains`, `count`, `find`, `lower_bound`, `upper_bound`, `equal_range` and `erase(key)` all answer for a key outside `[0, N)` — it is a key the set does not hold, which is an answer and not a precondition violation, exactly as `std::set::find` returns `end()` for any key it does not hold. **Writing is not**: `insert` and `complement` have nowhere to put such a key, and throw `out_of_range` as `std::bitset<N>` does for a position past `N`. This used to be undefined instead, on the grounds of a performance benefit; measured on the sieve at `N = 2^16`, best of twenty-five, the guard costs nothing — 188.0µs against 188.1µs, and 75.1µs against 75.1µs over 65536 inserts — because the comparison is against a compile-time constant and never taken.

Functionality from `std::bitset<N>` that is not in `xstd::bit_static_set<N>`:

- **No string constructors and no `to_string`**: a bit string is the **`bitset` reading's** vocabulary, and it is there across that whole row — `xstd::bitset<N>`, `xstd::inplace_bitset<N>` and `xstd::dynamic_bitset` all take `std::bitset`'s string constructors and answer `to_string()`, at every width. The set reading declines it as it declines the rest of that vocabulary, and crossing costs one call either way: `s.to_bits<xstd::bitset<N>>().to_string()`, and `xstd::bit_static_set<N>::from_bits(xstd::bitset<N>(str))` back. What a set prints *as itself* is `{2, 3, 5}`, which is [printing](#printing) above.
- **No integer constructor and no integer conversion operator**: here what is missing is the language's *unnamed* doors and not the capability. The byte exchange does both under a name, `xstd::bit_static_set<32>::from_bits(5u)` and `s.to_bits<unsigned>()` — and it is named for exactly the reason a constructor would be the wrong spelling: `from_bits(5u)` is the set of positions the **value** five has, `{0, 2}`, and not the set `{5}`. A constructor cannot say which of those it meant; a name can.
- **No I/O streaming operators**: `operator<<` and `operator>>` are `[bitset.operators]`'s, so they sit on that row beside `to_string`, at every width. A set formats instead, in its own vocabulary.

I/O functionality can be obtained through third-party libraries such as [{fmt}](https://fmt.dev/latest/), which has generic support for ranges such as `xstd::bit_static_set` — and `std::format` and `std::print` need nothing at all, as [printing](#printing) above shows. Hashing is **not** on that list: `std::hash<xstd::bit_static_set<N>>` is specialized, over [Boost.Hash2](https://github.com/boostorg/hash2), and every value this library compares it also hashes, so `a == b` implies `hash(a) == hash(b)` under every reading ([design.md#the-hashing-invariant](design.md#the-hashing-invariant)). The `hash_append` hook is there beside it, for a caller wanting an algorithm other than the defaulted `fnv1a_64`.

### 3 Set predicates from `boost::dynamic_bitset`

The set predicates `is_subset`, `is_proper_subset` and `intersects` from `boost::dynamic_bitset` are present in `xstd::bit_static_set` with **identical syntax** and **identical semantics**. Note that these set predicates are not present in `std::bitset`. Efficient emulation of these set predicates for `std::bitset` is not possible using **single-pass** and **short-circuiting** semantics.

| `xstd::bit_static_set<N>` <br> `boost::dynamic_bitset<>`  | `std::bitset<N>`             |
| :------------------------------------------------  | :---------------             |
| `a.is_subset_of(b)`                                | `(a & ~b).none()`            |
| `a.is_proper_subset_of(b)`                         | `(a & ~b).none() and a != b` |
| `a.intersects(b)`                                  | `(a & b).any()`              |

### 4 The bitwise operators from `std::bitset` and `boost::dynamic_bitset` reimagined as set algorithms

The bitwise operators (`&=`, `|=`, `^=`, `-=`, `~`, `&`, `|`, `^`, `-`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_static_set` with **identical syntax** and **identical semantics**. Note that the bitwise difference operators (`-=` and `-`) from `boost::dynamic_bitset` are not present in `std::bitset`. The `operator-` can be emulated for `std::bitset` using the identity `a - b == a & ~b`.

The bitwise-shift operators (`<<=`, `>>=`, `<<`, `>>`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_static_set` with **identical syntax**, but with the **semantic difference** that `xstd::bit_static_set<N>` does not support bit-shifting for lengths `>= N`. Instead of calling `clear()` for argument values outside the range `[0, N)`, this **behavior is undefined**. Note that these semantics for `xstd::bit_static_set<N>` are identical to bit-shifting on native unsigned integers. This gives `xstd::bit_static_set<N>` a small performance benefit over `std::bitset<N>`.

With the exception of `operator~`, the non-member bitwise operators can be reimagined as **composable** and **data-parallel** versions of the set algorithms on sorted ranges. In C++23, the set algorithms are not (yet) composable, but the [range-v3](https://ericniebler.github.io/range-v3/) library contains lazy views for them.


| `xstd::bit_static_set<N>`      | `std::set<int>` with the range-v3 set algorithm views                                                |
| :----------------       | :----------------------------------------------------------------------------------------------------|
| `a.is_subset_of(b)`     | `std::ranges::includes(a, b)`                                                                        |
| <code>a &vert; b</code> | <code>ranges::views::set_union(a, b)                &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a & b`                 | <code>ranges::views::set_intersection(a, b)         &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a - b`                 | <code>ranges::views::set_difference(a, b)           &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a ^ b`                 | <code>ranges::views::set_symmetric_difference(a, b) &vert; std::ranges::to&lt;std::set&gt;() </code> |

The bitwise shift operators of `xstd::bit_static_set<N>` can be reimagined as set **transformations** that add or subtract a non-negative constant to all set elements, followed by **filtering** out elements that would fall outside the range `[0, N)`. This can also be formulated in a composable way for `std::set<int>`, albeit without the data-parallelism that `xstd::bit_static_set<N>` provides.

<table>
<tr>
    <th>
        xstd::bit_static_set&ltN&gt
    </th>
    <th>
        std::set&ltint&gt
    </th>
</tr>
<tr>
    <td>
        <pre lang="cpp">auto b = a << n;</pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | std::views::transform([=](auto x) { return x + n; })
    | std::views::filter   ([=](auto x) { return x < N; })
    | std::ranges::to&ltstd::set&gt;()
;</pre>
    </td>
</tr>
<tr>
    <td>
        <pre lang="cpp">auto b = a >> n;</pre>
    </td>
    <td>
        <pre lang="cpp">
auto b = a
    | std::views::transform([=](auto x) { return x - n;  })
    | std::views::filter   ([=](auto x) { return 0 <= x; })
    | std::ranges::to&ltstd::set&gt;()
;</pre>
    </td>
</tr>
</table>

## Frequently Asked Questions

### Iterators

**Q**: How can you iterate over individual bits? I thought a byte was the unit of addressing?  
**A**: Using proxy iterators, which hold a pointer and an offset.

**Q**: What happens if you dereference a proxy iterator?  
**A**: You get a proxy reference: `ref == *it`.

**Q**: What happens if you take the address of a proxy reference?  
**A**: You get a proxy iterator: `it == &ref`.

**Q**: How do you get any value out of a proxy reference?  
**A**: They implicitly convert to `int`.

**Q**: How can proxy references work if C++ does not allow overloading of `operator.`?  
**A**: Indeed, proxy references break the equivalence between functions calls like `ref.mem_fn()` and `it->mem_fn()`.

**Q**: How do you work around this?  
**A**: `int` is not a class-type and does not have member functions, so this situation never occurs.

**Q**: Aren't there too many implicit conversions when assigning a proxy reference to an implicitly `int`-constructible class?  
**A**: No, proxy references also implicity convert to any class type that is implicitly constructible from an `int`.

**Q**: So iterating over an `xstd::bit_static_set` is really fool-proof?  
**A**: Yes, `xstd::bit_static_set` iterators are [easy to use correctly and hard to use incorrectly](http://www.aristeia.com/Papers/IEEE_Software_JulAug_2004_revised.htm).

### Bit-layout

**Q**: How is `xstd::bit_static_set` implemented?  
**A**: `bit_static_set` uses a `std::array` of unsigned integers, so its storage goes wherever the object does. That is true of the static-width column only: the inplace column holds a `std::inplace_vector` inline, and the dynamic column a `std::vector`. All three are the same storage vehicle over a different container, which is an implementation detail rather than a name you reach for.

**Q**: How is a set value mapped onto the array's bit layout?  
**A**: Position `n` is bit `n % W` of word `n / W`, for a word of `W` bits. So the **least** significant bit of the first array word maps onto set value `0`, and the most significant bit of the last array word onto set value `N - 1`. That is the conventional layout: `boost::dynamic_bitset` and the mainstream `std::bitset` and `std::vector<bool>` implementations all lay their bits out the same way.

**Q**: I'm visually oriented, can you draw a diagram?  
**A**: Sure, it looks like this for `basic_bit_static_set<std::uint8_t, 16>`, each word drawn most significant bit first:

|word  |       0|       1|
|:---- |-------:|-------:|
|offset|76543210|76543210|
|value |76543210|FEDCBA98|

**Q**: Why that layout, and not the mirrored one?  
**A**: Because it is very nearly forced. It is not a convention this library follows for the sake of following one, and there are four reasons, of which the first is not really negotiable at all.

**A one-word bitset must *be* its integer.** `std::bitset` mandates the round trip `bitset<N>(v).to_ullong() == v`, and the language fixes the value side of it: bit `n` of an unsigned integer is `2^n`. So `from_ullong` is `if ((val >> i) & 1) assign(i, true)` and `to_ullong` is its inverse, and at one word the stored block is the integer, bit for bit, with no work at all. Mirror the layout and the standard's own round trip becomes a bit reversal in each direction — which is why `design.md` admits the integer family with **nothing to prove**, where a foreign field of bits has to pass a probe ([design.md#the-bytes-they-agree-on](design.md#the-bytes-they-agree-on)).

**The scan primitive is the layout.** A forward step is `bits_per_block * i + countr_zero(m_blocks[i])` — `ctz` and nothing else, because position `n` is bit `n`. A mask is `unit << offset`, one instruction. Mirror it and each grows a correction: `digits - 1 - clz` for the step, `highbit >> offset` for the mask, and a `<<` that lowers to `shr`. That cost is visible here rather than hypothetical — the **reverse** scan pays exactly that subtraction, `last_bit() - countl_zero(...)`, and mirroring would move it onto `find_first`/`find_next`, which is the set reading's common path.

**Growth appends, so the numbering must append too.** The dynamic column is a `std::vector` of blocks and `push_back` is `resize(size() + 1, value)`, so a new position lands at bit `size() % digits` of the last block: the used region of that block grows upward from its low bits, the unused tail stays above it, and nothing already stored moves. That tail-above-the-used-bits invariant is what the bitset ordering then leans on, comparing blocks as unsigned integers from the top block down.

**And the exchange is a copy only because everyone agrees.** `std::bitset`, `std::vector<bool>` and `boost::dynamic_bitset` are all LSB-first, so a field of the same width agrees byte for byte and converts by `memcpy` rather than by reversing the bits of every byte. The per-byte path is measured at 9.70µs against 0.07µs over 2^16 positions, about a hundred and forty-five times; a mirrored layout would pay at least that on every conversion, in both directions, forever.

**Q**: Does that make little-endian mandatory too?  
**A**: For this library's own containers, no. Positions are computed by shifts on block **values**, and `b[j] >> k` is the same number on either byte order, so a big-endian target is correct and merely takes the shift path where a little-endian one copies.

For **interop**, yes, and both doors close at once. The block-range family names `std::endian::native == std::endian::little` in its own constraint, and a foreign bitset fails the layout probe, `std::bit_cast` handing back the object representation rather than the value — where bit `n` of a word lands at the far end of it. So a big-endian build keeps every container and loses every conversion from a field of bits it did not lay out itself.

**Q**: Then how does set comparison stay word-parallel, if the set order is not the words' integer order?  
**A**: It is three walks over the one layout, one per reading, each a word at a time rather than a position at a time:

| reading         | walk                                             | the rule it applies |
| :-------------- | :----------------------------------------------- | :------------------ |
| `bitset`        | the blocks **in reverse**                        | the bit string, most significant position first, *is* the blocks from the top down, so it is `std::lexicographical_compare_three_way` over the reversed blocks and nothing hand-rolled |
| ordered set     | the blocks ascending, through `first_difference` | whoever holds the lowest differing position is **less** — unless the other holds nothing above it, in which case that other is a prefix, and a prefix is less |
| sequence        | the same primitive, ascending                    | whoever holds the lowest differing position is **greater**, position `0` being the sequence's first element |

**Q**: So a set's order is *not* its bitstring's order?  
**A**: No, and that is why there are three functions rather than one ([design.md#two-readings-disagree](design.md#two-readings-disagree)). The two ascending readings share `first_difference`; the bitset reading deliberately does not use it and walks the other way, which is why that helper keeps its own name on the storage rather than being called `mismatch` there.

**Q**: Didn't the first `bitset` proposal argue for the other layout?  
**A**: It did, and this library does not follow it:

> "bit-0 is the leftmost, just like char-0 is the leftmost in character strings. [...]
> This makes converting from and to unsigned integers a little counter-intuitive,
> but the string-ness (or "array-ness") is the foundation of this abstraction.
>
> Chuck Allison, [ISO/WG21/N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf), May 26, 1992

Bit-0 leftmost buys one thing: the bitstring order and the array order agree, which is worth having when a `bitset` is one container. Here it is three readings over one storage and they disagree about order whatever the layout, so that agreement was never on offer — where the copy the conventional layout buys is.

### Storage type

**Q**: What storage type does `xstd::bit_static_set` use?  
**A**: By default, `xstd::bit_static_set` uses an array of `std::size_t` integers.

**Q**: Can I customize the storage type?  
**A**: Yes. The alias carrying the default is `template<std::size_t N> using bit_static_set = basic_bit_static_set<std::size_t, N>`; the underlying `template<xstd::unsigned_integer Block, std::size_t N> basic_bit_static_set` requires the block explicitly. Every cell of the table follows that pattern: a short name that defaults `Block` to `std::size_t`, and a `basic_` name that does not.

**Q**: What other storage types can be used as template argument for `Block`?  
**A**: Any type modelling the Standard Library `unsigned_integral` concept, which includes (for GCC and Clang) `xstd::uint128`.

**Q**: Does the `xstd::bit_static_set` implementation optimize for the case of a small number of words of storage?  
**A**: Yes, there are three special cases for 0, 1 and 2 words of storage, as well as the general case of 3 or more words.

## Requirements

This library depends on the C++ Standard Library, on [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc) (both fetched automatically via CMake `FetchContent` when not already installed), and on [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support. It is continuously being tested with the following conforming [C++23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf) compilers, against all three mainstream standard libraries (libstdc++, the MSVC STL, and libc++). Following the model of [apt.llvm.org](https://apt.llvm.org/), we support the latest two stable releases of each compiler, plus its current development branch.

Two standards are in play and they answer different questions. **C++23 is what the library compiles as** — that is the language it requires, and the tests build at C++26 as well, which is what reaches the inplace column. **The current [working draft](https://eel.is/c++draft/) is what the interfaces are measured against**, because a counterpart's synopsis is a moving target and the newest one is the one worth answering. So `std::bitset`'s `basic_string_view` constructor ([P2697R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2697r1.pdf)) and its `charT` Constraints ([LWG 4294](https://cplusplus.github.io/LWG/issue4294)) are here even though C++23 has neither, and the proxy carries the hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html) beside the static `swap(reference, reference)` that paper moved to `[depr.vector.bool.swap]`.

| Platform | Compiler | Standard Library | Stable | Qualification | Development | Status |
| :------- | :------- | :--------------- | :----- | :------------ | :---------- | :----- |
| Linux | GCC | libstdc++ | 15 | 16 | 17-SVN | [![GCC](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml) |
| Windows | MinGW | libstdc++ | 15 | 16 | — | [![MinGW](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml) |
| Linux | Clang | libstdc++ | 22 (libstdc++ 15) | 23 (libstdc++ 16) | 24-SVN (libstdc++ 17-SVN) | [![Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml) |
| Linux | Clang | libc++ | 22 | 23 | 24-SVN | [![Clang-libc++](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml) |
| macOS | Apple Clang | libc++ | 17.0.0 (Xcode 16.4) | 21.0.0 (Xcode 26.6) | — | [![Apple Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml) |
| Windows | Clang-CL | MSVC | 19.1.5 (VS 2022) | 20.1.8 (VS 2026) | 20.1.8 (VS 2026-Preview) | [![Clang-CL](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml) |
| Windows | MSVC | MSVC | — | 2026 | 2026-Preview | [![MSVC](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml) |

`Apple Clang` has no `Development` entry because Apple doesn't publish Apple Clang dev snapshots the way LLVM does; that leg tests the latest stable Xcode release from each of the two supported series. `MinGW` has none either: WinLibs publishes no GCC trunk build between stable branches, and the shared workflow drops that rung with a notice rather than failing, so the row names the two that actually run. The `Linux | GCC` row keeps its `17-SVN` — that ladder is unaffected.

`MSVC` has no `Stable` entry, and that one is a deliberate cost rather than a gap in what the vendor publishes. `bit_set_view` and `bit_span` are alias templates, and MSVC 17 (VS 2022) cannot deduce through an alias template of the shape they need — `error C2976: 'xstd::bit_set_view': too few template arguments` — while MSVC 18 (VS 2026) compiles them clean. **The `Windows | Clang-CL` row keeps its VS 2022 rung and passes on it**, because Clang-CL is Clang and Clang has had [P1814](https://wg21.link/P1814R0) since 19: what left the matrix is the MSVC 17 front end, not the VS 2022 runner, STL or platform. [design.md#the-views-are-the-adaptors](design.md#the-views-are-the-adaptors) records the trade.

Every leg above passes. The library no longer uses the C++23 range adaptors libc++ has not implemented (`views::cartesian_product`, `views::adjacent`, `views::pairwise_transform`, `views::stride`), and the tests probe for `<flat_set>` rather than assuming it, so the `Clang | libc++` and `Apple Clang` rows and the `Clang-CL` VS 2022 legs build and run like the rest.

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## Consuming this library

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


## License

<pre>
         Copyright Rein Halbersma 2014-2026.
Distributed under the <a href="http://www.boost.org/users/license.html">Boost Software License, Version 1.0</a>.
   (See accompanying file LICENSE_1_0.txt or copy at
         <a href="http://www.boost.org/LICENSE_1_0.txt">http://www.boost.org/LICENSE_1_0.txt</a>)
</pre>
