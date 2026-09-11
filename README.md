# Rebooting the bits franchise

> "The reasonable man adapts himself to the world: the unreasonable one persists
> in trying to adapt the world to himself. Therefore all progress depends on the
> unreasonable man."
>
> — George Bernard Shaw, *Man and Superman* (1903), "Maxims for Revolutionists"

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
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/rhalbersma/xstd-bits/badge)](https://scorecard.dev/viewer/?uri=github.com/rhalbersma/xstd-bits)

xstd-bits is a modern and opinionated reimagining of `std::bitset<N>`, keeping what time has proven to be effective, and throwing out what is not. It is **nine containers**: three readings of a block of bits — an ordered set of `std::size_t`, a sequence of `bool`, and the `bitset` that deliberately offers both — over three storages, which differ in whether size and capacity are static or dynamic: both static, a dynamic size within a static capacity, and both dynamic.

Each does less work than `std::bitset` (e.g. no bounds-checking and no throwing of `out_of_range` exceptions) yet offers more (e.g. full `constexpr`-ness and bidirectional iterators over individual 1-bits). This enables **bit-twiddling with set-like syntax** (identical to `std::set<int>`), typically leading to cleaner, more expressive code that seamlessly interacts with the rest of the Standard Library.

The flagship of the set reading is `xstd::bit_static_set<N>`, a fixed-size ordered set of integers that is compact and fast; most of this document is about it, because it is the cell where the design questions are sharpest. `xstd::bit_set` is its allocating counterpart.

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

The C++ Standard Library and Boost provide the following optimized data structures in the landscape spanned by the aforementioned design decisions and optimization directives, as shown in the table below.

|                                 | static size and capacity | dynamic size and capacity |
| :------------------------------ | :----------------------- | :------------------------ |
| **ordered set of `std::size_t`** | `std::bitset<N>`        | `std::flat_set<std::size_t>` (sparse) <br> `boost::dynamic_bitset<>` (dense) |
| **sequence of `bool`**          | `std::bitset<N>`         | `std::vector<bool>` <br> `boost::dynamic_bitset<>` |

Notes:

1. Both `std::bitset` and `boost::dynamic_bitset` are not clear about the interface they provide. E.g. both offer a hybrid of sequence-like random element access (through `operator[]`) as well as primitives for set-like bidirectional iteration (using non-Standard GCC extensions `_Find_first` and `_Find_next` in the case of `std::bitset`).
2. It [has been known](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2130.html#96) for over two decades that providing a variable-size sequence of `bool` through specializing `std::vector<bool>` was an unfortunate design choice.
3. For ordered sets, there is a further design choice whether to optimize for **dense** sets or for **sparse** sets. Dense sets require a single bit per **potential** element, whereas sparse sets require a whole element per **actual** element. The break-even is therefore the element's width: at a 64-bit `std::size_t`, if less (more) than 1 in 64 elements (1.5625%) are actually present, a dense representation will be less (more) compact than a sparse one — 1 in 32 (3.125%) if the sparse set stores 32-bit integers instead.
4. Only `boost::dynamic_bitset` allows storage configuration through its `Block` template parameter (defaulted to `unsigned long`).
5. The `Block`, `Allocator` and `Compare` parameters are omitted from both tables. They configure a storage; they do not place a container in the landscape, and spelling them out obscured the one axis the columns are measuring.

## A reimagined `bit` landscape

The aforementioned issues with the current `bit` landscape can be resolved by implementing a single-purpose container for each cell of the design space. Two single readings over three storages is six containers, and the `bitset` that offers **both** readings is the seventh, eighth and ninth — **and this library implements all nine**. But a `bitset` is not a third reading; it is a storage you have not yet said how to read. So it does not get a row of its own: it appears in both rows, under the view that picks a reading of it.

|                                  | static size and capacity                                            | dynamic size, static capacity                                              | dynamic size and capacity                                     |
| :------------------------------- | :------------------------------------------------------------------ | :------------------------------------------------------------------------- | :------------------------------------------------------------ |
| **ordered set of `std::size_t`** | `xstd::bit_static_set<N>` <br> `xstd::bit_set_view<xstd::bitset<N>>` <br> `xstd::bit_set_view<std::bitset<N>>` | `xstd::bit_inplace_set<N>` <br> `xstd::bit_set_view<xstd::inplace_bitset<N>>` | `xstd::bit_set` <br> `xstd::bit_set_view<xstd::dynamic_bitset>` <br> `xstd::bit_set_view<boost::dynamic_bitset<>>` |
| **sequence of `bool`**           | `xstd::bit_array<N>` <br> `xstd::bit_span<xstd::bitset<N>>` <br> `xstd::bit_span<std::bitset<N>>` | `xstd::bit_inplace_vector<N>` <br> `xstd::bit_span<xstd::inplace_bitset<N>>` | `xstd::bit_vector` <br> `xstd::bit_span<xstd::dynamic_bitset>` <br> `xstd::bit_span<boost::dynamic_bitset<>>` |

The columns are the three storages the one underlying vehicle is parameterized on — `std::array` fixes size and capacity, `std::inplace_vector` varies size within a fixed capacity, `std::vector` varies both — so a cell is a reading crossed with a storage, and nothing else. The outer two columns are the ones the current landscape already has; the middle is the pairing it never named.

Each cell offers the same reading three ways: as a container built for it, as a view onto that column's own `bitset`, and as a view onto the **standard or Boost** bitset of the same shape. That last one is the whole of what [retrofitting](#retrofitting-set-like-behaviour-onto-stdbitsetn-and-boostdynamic_bitset) means — you do not have to adopt a container to get a reading, you point a view at the bits you already have. The middle column offers only two, because a run-time size over static capacity is the pairing neither the standard nor Boost has anything in.

Notes:

1. Each data structure is clear about the interface it provides: sequences are random access containers and ordered sets are bidirectional containers.
2. The `bitset` column entries are the point the old two-by-two could not express. `std::bitset` and `boost::dynamic_bitset` are faulted above for being unclear about which interface they offer; the answer here is not to abolish the hybrid but to make choosing between its two readings **explicit at the call site**. `xstd::bitset<N>` is a strict extension of `std::bitset<N>` and `xstd::dynamic_bitset` is one of `boost::dynamic_bitset<>` — every expression valid on the counterpart is valid here, with the same result — and neither has iterators of its own, because `begin` is one name and there are two readings. `bit_set_view` and `bit_span` are how you say which you meant, and they appear in both rows for exactly that reason.
3. A view names the bitset directly because a bitset has a `bit_traits` of its own, and **one** specialization — on `bitset_adaptor`, which all three bitsets are aliases of — gives all three of them that at once. The foreign bitsets get theirs from [`include/xstd/bits/ext/`](include/xstd/bits/ext/), which is the same mechanism from the outside. It relays each of the storage trait's twenty entries under its own guard, so the word-parallel paths are not quietly lost in the forwarding: `block_readable` still holds through it. Deduction is unaffected and still binds the storage a container wraps, so `xstd::bit_set_view(bs)` remains `xstd::bit_set_view<xstd::contiguous_bit_array<std::size_t, N>>` — the two spellings coexist, and the deduction guide for a plain storage is constrained to non-owners so that the two do not tie.
4. The variable-size sequence of `bool` is named `xstd::bit_vector` and decoupled from the general `std::vector` class template.
5. All containers use a dense (single bit per element) representation. Variable-size sparse sets can be provided by `flat_set`, either in [Boost](https://www.boost.org/doc/libs/1_80_0/doc/html/boost/container/flat_set.html) or in [C++ 23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1222r4.pdf).
6. The names above are the short ones, which fix `Block` to `std::size_t` and so take only the width, or nothing at all in the dynamic column where there is no width to give. Each has a `basic_` form that leaves the block open: `xstd::basic_bit_static_set<Block, N>`, `xstd::basic_bit_array<Block, N>`, `xstd::basic_bitset<Block, N>` and their inplace siblings, and `xstd::basic_bit_set<Block, Allocator>`, `xstd::basic_bit_vector<Block, Allocator>`, `xstd::basic_dynamic_bitset<Block, Allocator>` down the dynamic column. So `xstd::bit_set` is an alias, not a template, and `xstd::basic_bit_set<std::uint8_t>` is how a block is chosen.
7. Each static-width name has an `aligned` form in a nested namespace, its width rounded up to whole blocks so that no block carries an unused tail: `xstd::aligned::bitset<120>` is `xstd::bitset<128>`. That costs nothing in storage at a width already spanning whole blocks, and removes the tail-restoring mask from `fill`, `flip` and the left shift.

The **middle column** is what allocates nothing and yet carries a run-time width. It depends on `std::inplace_vector`, so those three names exist only where the standard library provides it (`__cpp_lib_inplace_vector`); an alias withholds a name rather than a capability.

Ownership is deliberately **not** a fourth column. A view is not a fourth storage: it is one of the two readings pointed at storage someone else owns, which is why `bit_set_view` and `bit_span` sit inside the cells rather than beside them. `xstd::bit_subspan` is the one that stays out of the table, because it narrows a sequence to a window rather than choosing a reading. All of them are described under [retrofitting](#retrofitting-set-like-behaviour-onto-stdbitsetn-and-boostdynamic_bitset) below.

### Hello World: generating (twin) primes

The code below demonstrates how a standards conforming `set` implementation can be used to implement the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes). This algorithm generates all [prime numbers](https://en.wikipedia.org/wiki/Prime_number) below a number `n`. It is the library's worked example and its benchmark, and it lives in [`include/opt/set/sieve.hpp`](include/opt/set/sieve.hpp); the listing below is that header, not a paraphrase of it.

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

    auto const primes = xstd::sift_primes0<X>(N);
    assert(fmt::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto const twins = xstd::filter_twins(primes);
    assert(fmt::format("{}", twins)  == "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}");
}
```

Those two assertions are the ones [`test/src/bits/std_set/sieve.cpp`](test/src/bits/std_set/sieve.cpp) makes, over `std::set`, `std::flat_set`, `bit_static_set<N>` and `bit_set` alike.

### Sieves that need no bound

`generate_candidates` materializes every candidate below `n` before sifting one, which is what makes the sieve above `O(n)` in space and why it cannot answer "what is the next prime". The same header carries two variants that drop the bound, and they drop it in opposite directions:

```cpp
auto sieve = xstd::incremental_sieve();
sieve.next();  // 2, then 3, 5, 7, ... forever, with no n anywhere

auto primes = xstd::sift_primes_segmented<xstd::bit_set, xstd::bit_static_set<1 << 15>>(n);
```

The **incremental** sieve ([O'Neill 2009](https://www.cs.hmc.edu/~oneill/papers/Sieve-JFP.pdf)) keeps one entry per prime found — the next composite that prime will strike — so its space is `O(π(n))` and it generates without end. It is also about **29× slower**, which is the honest price of unboundedness and the reason it is measured rather than recommended.

The **segmented** sieve is the one that pays. Base primes below `√n` once, then a single reusable window walked over the rest, so peak memory is `O(√n + W)` whatever `n` is. `Window` is a template parameter carrying its own extent, which makes `bit_static_set<W>` the natural argument: a compile-time width that allocates nothing in the loop. At `n = 2^20` it is **1.7× faster** than the bounded sieve as well as far thriftier — the window stays in L1 for a whole segment where a megabit sieve is walked with a stride. Less memory *and* less time is the unusual direction for that trade to run.

All three agree, and the test asserts that rather than the README claiming it.

## Requirements for `set`-like behaviour

Looking at the above code, the following four ingredients are necessary to implement the Sieve of Eratosthenes:

1. **Bidirectional iterators** `begin` and `end` in the `set`'s own namespace (to work with range-`for` and the `<ranges>` library);
2. **Constructors** taking a pair of iterators or a range (in order for `std::ranges::to` to construct a `set`);
3. A **nested type** `key_type` (in order for the `{fmt}` library to use `{}` delimiters);
4. A **member function** `erase` to remove elements (for other applications: the rest of a `set`'s interface).

`xstd::bit_static_set<N>` implements all four of the above requirements. Note that Visual C++ support is finicky at the moment because its `<ranges>` implementation cannot (yet) handle the `xstd::bit_static_set<N>` proxy iterators and proxy references correctly.

## Retrofitting `set`-like behaviour onto `std::bitset<N>` and `boost::dynamic_bitset<>`

The four ingredients above are exactly what `std::bitset<N>` and `boost::dynamic_bitset<>` lack. Without access to their implementations it is impossible to add nested types and member functions to them — you cannot give `std::bitset` a `key_type`, or an `erase`, or iterators in its own namespace.

You can, however, add all four from the **outside**. `xstd::bit_set_view` is an adaptor that wraps any bit storage and presents the set reading of it: bidirectional iterators, `key_type`, `insert`/`erase`/`contains`, and the set predicates — reconciled onto whatever the underlying container spells them as, `set(pos)`/`reset(pos)` for `std::bitset` and `boost::dynamic_bitset` alike. `xstd::bit_span` does the same for the sequence reading, and `xstd::bit_subspan` for a window into one.

```cpp
auto bs = std::bitset<100>();
auto const s = xstd::bit_set_view(bs);   // a set view onto someone else's bits

s.insert(42);                            // bs.set(42)
assert(s.contains(42));                  // bs.test(42)
assert(fmt::format("{}", s) == "{42}");  // formats as a set, which std::bitset cannot
```

What makes this work for a foreign container is `xstd::bit_traits`, a trait the adaptors read the storage through. The library ships specializations for `std::bitset` and `boost::dynamic_bitset` in [`include/xstd/bits/ext/`](include/xstd/bits/ext/), kept out of the umbrella header so that including `<xstd/bits.hpp>` never puts Boost on your include path. A third-party bit container can join by specializing that trait, without changing this library or that one.

This is why ownership is not a fourth column of the table above: a view is not a fourth kind of container, it is the same three readings pointed at storage someone else owns.

### Printing

The snippets above use `fmt::format`, which finds the proxies through fmt's own `format_as`. `std::format` and `std::print` work too, with nothing to include and nothing to switch on:

```cpp
std::print("{}\n", primes);   // {2, 3, 5, 7, 11, ...}   the set reading, in braces
std::print("{}\n", flags);    // [false, true, ...]      the sequence reading, in brackets
std::print("{::#x}\n", primes);
```

Each proxy reference carries its own `std::formatter`, so a container that hands the proxy out brings the formatter with it. Nothing is specialized for a container: every one of them is already a range, so [`[format.range.formatter]`](https://eel.is/c++draft/format.range.formatter) formats it once its reference is formattable. The braces-versus-brackets split is the standard's, not ours — `[format.range.fmtkind]` picks `range_format::set` for a range with a `key_type` — so each reading prints in its own vocabulary without being told to. Both hooks read the same value: the `std::formatter` defers to the `format_as` that fmt calls, so the two libraries cannot drift.

## Data-parallelism

The `filter_twins` above walks the primes one at a time, because that is all `std::set` can do. A dense container can answer the same question a **word at a time**, without iterating at all. A prime is a twin exactly when it has a neighbour two away, so shifting the whole set by two in each direction and intersecting gives every twin in a handful of instructions per block:

```cpp
template<class X>
auto filter_twins_parallel(X const& primes)
{
    return primes & (primes << 2 | primes >> 2);
}
```

That is the same set the loop produces, and on a `bit_static_set<128>` it is four shifts, two ors and an and over two words — no iterator, no branch per element, no comparison. The elementwise form remains the one in `opt/set/sieve.hpp`, because it is the form `std::set` and `std::flat_set` can also run and the benchmark needs all three on the same algorithm.

The shifts are why the bit layout is what it is: element `0` is the most significant bit of the first word, so `<<` moves toward larger elements and the set order matches the bitstring order. The FAQ below draws it.

which has as output:
<pre>
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}
{3, 5, 11, 17, 29, 41, 59, 71}
</pre>

### Sequence of bits

What does a foreign bitset have to offer for the set reading to be built on top of it? The essential difference (apart from differently named member functions) is the lack of iterators. The GCC Standard Library `libstdc++` provides member functions `_Find_first` and `_Find_next` for `std::bitset<N>` as **non-standard extensions**. For `boost::dynamic_bitset<>`, similarly named member functions `find_first` and `find_next` exist.

Those are what the `bit_traits` specializations in [`include/xstd/bits/ext/`](include/xstd/bits/ext/) read, and `xstd::bit_set_view` turns them into bidirectional iterators `begin`/`end` (and `cbegin`/`cend`/`rbegin`/`rend`/`crbegin`/`crend`) plus a nested `key_type`, so a `std::bitset` formats like a set out of the box — as the snippet under [retrofitting](#retrofitting-set-like-behaviour-onto-stdbitsetn-and-boostdynamic_bitset) shows. Where a storage offers no such scan, the trait falls back to reading it block by block, which is why `bit_set_view` works over containers that never anticipated it.

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

Minor **semantic differences** between common functionality in `xstd::bit_static_set<N>` and `std::set<int>` are:

- the `xstd::bit_static_set` member function `max_size` is `constexpr`, and at a static width its value is a constant expression usable wherever `N` is. It is a **member** rather than a `static` member function, because the same `max_size()` has to answer for the dynamic and inplace readings too, where it is the storage that knows ([design.md#max-size-is-the-bits](design.md#max-size-is-the-bits)). So `s.max_size()` is a constant expression and `decltype(s)::max_size()` does not compile.
- the `xstd::bit_static_set` iterators are **proxy iterators**, and taking their address yields **proxy references**. The difference should be undetectable. See the FAQ at the end of this document.
- the `xstd::bit_static_set` members `fill`, `complement`, `replace` and `full` do not exist for `std::set`.

With these caveats in mind, all fixed-size, defaulted comparing, non-allocating, non-splicing `std::set<int>` code in the wild should continue to work out-of-the-box with `xstd::bit_static_set<N>`.

### 2 An almost complete translation of `std::bitset<N>`

Almost all existing `std::bitset<N>` code has **a direct translation** (i.e. achievable through search-and-replace) to an equivalent `xstd::bit_static_set<N>` expression, with the same and familiar semantics as `std::set<int>` or `boost::flat_set<int>`.

| `std::bitset<N>`                | `xstd::bit_static_set<N>`              | Notes                                           |
| :---------------                | :-----------------              | :----                                           |
| `bs.set()`                      | `bs.fill()`                     | not a member of `std::set<int>`                 |
| `bs.set(n)`                     | `bs.add(n)` <br> `bs.insert(n)` | no bounds-checking or `out_of_range` exceptions |
| `bs.set(n, v)` <br> `bs[n] = v` | `v ? bs.add(n) : bs.pop(n)`     | no bounds-checking or `out_of_range` exceptions |
| `bs.reset()`                    | `bs.clear()`                    | returns `void` as `std::set<int>`, not `*this` as `std::bitset<N>`  |
| `bs.reset(n)`                   | `bs.pop(n)` <br> `bs.erase(n)`  | no bounds-checking or `out_of_range` exceptions |
| `bs.flip()`                     | `bs.complement()`               | not a member of `std::set<int>`                 |
| `bs.flip(n)`                    | `bs.complement(n)`              | no bounds-checking or `out_of_range` exceptions <br> not a member of `std::set<int>` |
| `bs.count()`                    | `bs.size()`                     | |
| `bs.size()`                     | `bs.max_size()`                 | `constexpr`; a constant expression at a static width |
| `bs.test(n)` <br> `bs[n]`       | `bs.contains(n)`                | no bounds-checking or `out_of_range` exceptions |
| `bs.all()`                      | `bs.full()`                     | not a member of `std::set<int>`                 |
| `bs.any()`                      | `not bs.empty()`                | |
| `bs.none()`                     | `bs.empty()`                    | |

The semantic differences between `xstd::bit_static_set<N>` and `std::bitset<N>` are:

- `xstd::bit_static_set<N>` answers `max_size()` as a `constexpr` member, where `std::bitset<N>` answers the same question with `size()`;
- `xstd::bit_static_set<N>` does not do bounds-checking for its members `insert`, `erase`, `replace` and `contains`. Instead of throwing an `out_of_range` exception for argument values outside the range `[0, N)`, this **behavior is undefined**. This gives `xstd::bit_static_set<N>` a small performance benefit over `std::bitset<N>`.

Functionality from `std::bitset<N>` that is not in `xstd::bit_static_set<N>`:

- **No integer or string constructors**: `xstd::bit_static_set` cannot be constructed from `unsigned long long`, `std::string` or `const char*`.
- **No integer or string conversion operators**: `xstd::bit_static_set` does not convert to `unsigned long`, `unsigned long long` or `std::string`.
- **No I/O streaming operators**: `xstd::bit_static_set` does not provide overloaded I/O streaming `operator<<` and `operator>>`.
- **No hashing**: `xstd::bit_static_set` does not provide a specialization for `std::hash<>`.

I/O functionality can be obtained through third-party libraries such as [{fmt}](https://fmt.dev/latest/), which has generic support for ranges such as `xstd::bit_static_set`. Similarly, hashing functionality can be obtained through third-party libraries such as [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html) by streaming the `xstd::bit_static_set<N>` elements and size through an overloaded `hash_append` function.

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

**Q**: How is the set ordering mapped to the array's bit layout?  
**A**: The most significant bit of the first array word maps onto set value `0`.
**A**: The least significant bit of the last array word maps onto set value `N - 1`.

**Q**: I'm visually oriented, can you draw a diagram?  
**A**: Sure, it looks like this for `basic_bit_static_set<std::uint8_t, 16>`:

|value |01234567|89ABCDEF|
|:---- |-------:|-------:|
|word  |       0|       1|
|offset|76543210|76543210|

**Q**: Why is the set order mapped this way onto the array's bit-layout?  
**A**: To be able to use **data-parallelism** for `(a < b) == std::ranges::lexicographical_compare(a, b)`.

**Q**: How is efficient set comparison connected to the bit-ordering within words?  
**A**: Take `basic_bit_static_set<std::uint8_t, 8>` and consider when `sL < sR` for ordered sets of integers `sL` and `sR`.

**Q**: Ah, lexicographical set comparison corresponds to bit comparison from most to least significant?  
**A**: Indeed, and this is equivalent to doing the integer comparison `wL > wR` on the underlying words `wL` and `wR`.

**Q**: So the set ordering a `bit_static_set` is equivalent to its representation as a bitstring?  
**A**: Yes, indeed, and this property has been known for several decades:

> "bit-0 is the leftmost, just like char-0 is the leftmost in character strings. [...]
> This makes converting from and to unsigned integers a little counter-intuitive,
> but the string-ness (or "array-ness") is the foundation of this abstraction.
>
> Chuck Allison, [ISO/WG21/N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf), May 26, 1992

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

| Platform | Compiler | Standard Library | Stable | Qualification | Development | CI |
| :------- | :------- | :--------------- | :----- | :------------ | :---------- | :- |
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

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [fmtlib](https://github.com/fmtlib/fmt), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## License

<pre>
         Copyright Rein Halbersma 2014-2026.
Distributed under the <a href="http://www.boost.org/users/license.html">Boost Software License, Version 1.0</a>.
   (See accompanying file LICENSE_1_0.txt or copy at
         <a href="http://www.boost.org/LICENSE_1_0.txt">http://www.boost.org/LICENSE_1_0.txt</a>)
</pre>
