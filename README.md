# Rebooting the bits franchise

> "The reasonable man adapts himself to the world: the unreasonable one persists
> in trying to adapt the world to himself. Therefore all progress depends on the
> unreasonable man."
>
> -- George Bernard Shaw, *Man and Superman* (1903), "Maxims for Revolutionists"

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

`xstd::bit_set<N>` is a modern and opinionated reimagining of `std::bitset<N>`, keeping what time has proven to be effective, and throwing out what is not. `xstd::bit_set` is a **fixed-size ordered set of integers** that is compact and fast. It does less work than `std::bitset` (e.g. no bounds-checking and no throwing of `out_of_range` exceptions) yet offers more (e.g. fulll `constexpr`-ness and bidirectional iterators over individual 1-bits). This enables **fixed-size bit-twiddling with set-like syntax** (identical to `std::set<int>`), typically leading to cleaner, more expressive code that seamlessly interacts with the rest of the Standard Library.

## Design choices for a `bitset` data structure

> "A `bitset` can be seen as either an array of bits or a set of integers. [...]
> Common usage suggests that dynamic-length `bitsets` are seldom needed."
>
> Chuck Allison, [ISO/WG21/N0075](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1991/WG21%201991/X3J16_91-0142%20WG21_N0075.pdf), November 25, 1991

The above quote is from the first C++ Standard Committee proposal on what would eventually become `std::bitset<N>`. The quote highlights two design choices to be made for a `bitset` data structure:

1. a sequence of `bool` versus an ordered set of `int`;
2. fixed-size versus variable-size storage.

A `bitset` should also optimize for both space (using contiguous storage) and time (using CPU-intrinsics for data-parallelism) wherever possible.

## The current `bit` landscape

The C++ Standard Library and Boost provide the following optimized data structures in the landscape spanned by the aforementioned design decisions and optimization directives, as shown in the table below.

|                          | fixed-size        | variable-size |
| :--------------------    | :---------        | :------------ |
| **sequence of `bool`**   | `std::bitset<N>`  | `std::vector<bool, Allocator>` <br> `boost::dynamic_bitset<Block, Allocator>` |
| **ordered set of `int`** | `std::bitset<N>`  | `boost::dynamic_bitset<Block, Allocator>` (dense) <br> `std:flat_set<int, Compare, Allocator>` (sparse) |

Notes:

1. Both `std::bitset` and `boost::dynamic_bitset` are not clear about the interface they provide. E.g. both offer a hybrid of sequence-like random element access (through `operator[]`) as well as primitives for set-like bidirectional iteration (using non-Standard GCC extensions `_Find_first` and `_Find_next` in the case of `std::bitset`).
2. It [has been known](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2130.html#96) for over two decades that providing a variable-size sequence of `bool` through specializing `std::vector<bool>` was an unfortunate design choice.
3. For ordered sets, there is a further design choice whether to optimize for **dense** sets or for **sparse** sets. Dense sets require a single bit per **potential** element, whereas sparse sets require a single `int` per **actual** element. For 32-bit integers, if less (more) than 1 in 32 elements (3.125%) are actually present in a set, a dense representation will be less (more) compact than a sparse representation.
4. Only `boost::dynamic_bitset` allows storage configuration through its `Block` template parameter (defaulted to `unsigned long`).

## A reimagined `bit` landscape

The aforementioned issues with the current `bit` landscape can be resolved by implementing a single-purpose container for each of the four quadrants in the design space.

|                          | fixed-size                                   | variable-size                             |
| :--------------------    | :---------                                   | :------------                             |
| **sequence of `bool`**   | `xstd::bit_array<N, Block>`                  | `xstd::bit_vector<Block, Allocator>`      |
| **ordered set of `int`** | `xstd::bit_set<N, Block>` (**this library**) | `xstd::dynamic_bit_set<Block, Allocator>` |

Notes:

1. Each data structure is clear about the interface it provides: sequences are random access containers and ordered sets are bidirectional containers.
2. The variable-size sequence of `bool` is named `xstd::bit_vector` and decoupled from the general `std::vector` class template.
3. All containers use a dense (single bit per element) representation. Variable-size sparse sets of `int` can be provided by `flat_set`, either in [Boost](https://www.boost.org/doc/libs/1_80_0/doc/html/boost/container/flat_set.html) or in [C++ 23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1222r4.pdf).
4. All containers allow storage configuration through their `Block` template parameter (defaulted to `std::size_t`).

This library provides one of the four outlined quadrants: `xstd::bit_set<N>` as a **fixed-size ordered set of `int`**. The other three quadrants are not implemented.

### Hello World: generating (twin) primes

The code below demonstrates how a standards conforming `set` implementation can be used to implement the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes). This algorithm generates all [prime numbers](https://en.wikipedia.org/wiki/Prime_number) below a number `n`. Note that the implementation below requires seamless interaction with the C++20 (and beyond) `<ranges>` library and does not use manual iterator manipulation or index arithmetic.

```cpp
template<class C>
auto sift(C& primes, int m)
{
    primes.erase(m);
}

template<class C>
struct generate_candidates
{
    auto operator()(int n) const
    {
        return std::views::iota(2, n) | std::ranges::to<C>();
    }
};

template<class C>
auto sift_primes(int n)
{
    auto primes = generate_candidates<C>()(n);
    for (auto p
        : primes
        | std::views::take_while([&](auto x) { return x * x < n; })
    ) {
        for (auto m
            : std::views::iota(p * p, n)
            | std::views::stride(p)
        ) {
            sift(primes, m);
        }
    }
    return primes;
}
```

Given a set of primes, generating the [twin primes](https://en.wikipedia.org/wiki/Twin_prime) can be done by iterating over pairs of adjacent primes, filtering on their difference being exactly 2, selecting the first element of each pair, and converting that to a new set:

```cpp
template<class C>
auto filter_twins(C const& primes)
{
    return primes
        | std::views::pairwise
        | std::views::filter([](auto&& x) { auto&& [ first, second ] = x; return first + 2 == second; })
        | std::views::elements<0>
        | std::ranges::to<C>()
    ;
}
```

The calling code for these algorithms is listed below. Here, the pretty-printing as a `set` using `{}` delimiters is triggered by the nested `key_type`. Note that `xstd::bit_set<N>` acts as a **drop-in replacement** for `std::set<int>` (or `std::flat_set<int>`).

[![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/7bfjz15bP)
```cpp
int main()
{
    constexpr auto N = 100;
    using C = xstd::bit_set<N>; /* or std::set<int> or std::flat_set<int> */

    auto primes = sift_primes<C>(N);
    assert(fmt::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto twins = filter_twins(primes);
    assert(fmt::format("{}", twins)  == "{3, 5, 11, 17, 29, 41, 59, 71}");
}
```

## Requirements for `set`-like behaviour

Looking at the above code, the following four ingredients are necessary to implement the Sieve of Eratosthenes:

1. **Bidirectional iterators** `begin` and `end` in the `set`'s own namespace (to work with range-`for` and the `<ranges>` library);
2. **Constructors** taking a pair of iterators or a range (in order for `std::ranges::to` to construct a `set`);
3. A **nested type** `key_type` (in order for the `{fmt}` library to use `{}` delimiters);
4. A **member function** `erase` to remove elements (for other applications: the rest of a `set`'s interface).

`xstd::bit_set<N>` implements all four of the above requirements. Note that Visual C++ support is finicky at the moment because its `<ranges>` implementation cannot (yet) handle the `xstd::bit_set<N>` proxy iterators and proxy references correctly.

## Retrofitting `set`-like behaviour onto `std::bitset<N>` and `boost::dynamic_bitset<>`

Can we use Without access to their implementations, it's impossible to add nested types and member functions

## Data-parallelism

```cpp
template<class C>
struct generate_empty
{
    auto operator()(auto) const
    {
        return C();
    }
};

template<int N, std::unsigned_integral Block>
auto fill(xstd::bit_set<N, Block>& empty)
{
    empty.fill();
}

template<class C>
struct generate_candidates
{
    auto operator()(auto n) const
    {
        auto candidates = generate_empty<C>()(n);
        fill(candidates);
        sift(candidates, 0);
        sift(candidates, 1);
        return candidates;
    }
};

template<class C>
auto filter_twins(C const& primes)
{
    return primes & primes >> 2;
}
```

[![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/7GWced7Wa)
```cpp
int main()
{
    constexpr auto N = 100;
    using C = xstd::bit_set<N>; /* or std::bitset<N> or boost::dynamic_bitset<> */

    auto const primes = sift_primes<C>(N);
    assert(fmt::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto const twins = filter_twins(primes);
    assert(fmt::format("{}", twins) == "{3, 5, 11, 17, 29, 41, 59, 71}");
}
```

which has as output:
<pre>
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}
{3, 5, 11, 17, 29, 41, 59, 71}
</pre>

### Sequence of bits

How would the Sieve of Eratosthenes code look when using a sequence of bits? The essential difference (apart from differently named member functions) is the lack of iterators. The GCC Standard Library `libstdc++` provides member functions `_Find_first` and `_Find_next` for `std::bitset<N>` as **non-standard extensions**. For `boost::dynamic_bitset<>`, similarly named member functions `find_first` and `find_next` exist. We combine these functions into an `xstd::proxy::bidirectional::view`, a lightweight adaptor providing bidirectional iterators `begin`/`end` (and `cbegin`/`cend`/`rbegin`/`rend`/`crbegin`/`crend`) plus a nested `key_type`, so it formats like a set out of the box. The links in the table below provide the full code examples for `std::bitset<N>` and `boost::dynamic_bitset<>`.

| Library                   | Try it online |
| :------                   | :------------ |
| `xstd::bit_set<N>`        | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/shr5TEP9M) |
| `std::bitset<N>`          | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/zGjofjToj) |
| `boost::dynamic_bitset<>` | [![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/PeKoK8zvd) |

## Documentation

The interface for the class template `xstd::bit_set<N>` is the coherent union of the following building blocks:

1. An almost **drop-in** implementation of the full interface of `std::set<int>`.
2. An almost complete **translation** of the [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) member functions to the [`std::set<int>`](http://en.cppreference.com/w/cpp/container/set) naming convention.
3. The single-pass and short-circuiting **set predicates** from [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html).
4. The bitwise operators from [`std::bitset<N>`](http://en.cppreference.com/w/cpp/utility/bitset) and [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/1_80_0/libs/dynamic_bitset/dynamic_bitset.html) reimagined as composable and data-parallel **set algorithms**.

The **full** interface of `xstd::bit_set` is `constexpr`.

### 1 An almost drop-in replacement for `std::set<int>`

`xstd::bit_set<N>` is a fixed-size ordered set of integers, providing conceptually the same functionality as `std::set<int, std::less<int>, Allocator>`, where `Allocator` statically allocates memory to store `N` integers. In particular, `xstd::bit_set<N>` has:

- **No customized key comparison**: `xstd::bit_set` uses `std::less<int>` as its fixed comparator (accessible through its nested types `key_compare` and `value_compare`). In particular, the `xstd::bit_set` constructors do not take a comparator argument.
- **No allocators**: `xstd::bit_set` is a fixed-size set of non-negative integers and does not dynamically allocate memory. In particular, `xstd::bit_set` does **not provide** a `get_allocator()` member function and its constructors do not take an allocator argument.
- **No splicing**: `xstd::bit_set` is **not a node-based container**, and does not provide the splicing operations as defined in [p0083r3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0083r3.pdf). In particular, `xstd::bit_set` does **not provide** the nested types `node_type` and `insert_return_type`, the `extract()` or `merge()` member functions, or the `insert()` overloads taking a node handle.

Minor **semantic differences** between common functionality in `xstd::bit_set<N>` and `std::set<int>` are:

- the `xstd::bit_set` member function `max_size` is a `static` member function (because `N` is a constant expression).
- the `xstd::bit_set` iterators are **proxy iterators**, and taking their address yields **proxy references**. The difference should be undetectable. See the FAQ at the end of this document.
- the `xstd::bit_set` members `fill`, `complement`, `replace` and `full` do not exist for `std::set`.

With these caveats in mind, all fixed-size, defaulted comparing, non-allocating, non-splicing `std::set<int>` code in the wild should continue to work out-of-the-box with `xstd::bit_set<N>`.

### 2 An almost complete translation of `std::bitset<N>`

Almost all existing `std::bitset<N>` code has **a direct translation** (i.e. achievable through search-and-replace) to an equivalent `xstd::bit_set<N>` expression, with the same and familiar semantics as `std::set<int>` or `boost::flat_set<int>`.

| `std::bitset<N>`                | `xstd::bit_set<N>`              | Notes                                           |
| :---------------                | :-----------------              | :----                                           |
| `bs.set()`                      | `bs.fill()`                     | not a member of `std::set<int>`                 |
| `bs.set(n)`                     | `bs.add(n)` <br> `bs.insert(n)` | no bounds-checking or `out_of_range` exceptions |
| `bs.set(n, v)` <br> `bs[n] = v` | `v ? bs.add(n) : bs.pop(n)`     | no bounds-checking or `out_of_range` exceptions |
| `bs.reset()`                    | `bs.clear()`                    | returns `void` as `std::set<int>`, not `*this` as `std::bitset<N>`  |
| `bs.reset(n)`                   | `bs.pop(n)` <br> `bs.erase(n)`  | no bounds-checking or `out_of_range` exceptions |
| `bs.flip()`                     | `bs.complement()`               | not a member of `std::set<int>`                 |
| `bs.flip(n)`                    | `bs.complement(n)`              | no bounds-checking or `out_of_range` exceptions <br> not a member of `std::set<int>` |
| `bs.count()`                    | `bs.size()`                     | |
| `bs.size()`                     | `bs.max_size()`                 | a `static` member                               |
| `bs.test(n)` <br> `bs[n]`       | `bs.contains(n)`                | no bounds-checking or `out_of_range` exceptions |
| `bs.all()`                      | `bs.full()`                     | not a member of `std::set<int>`                 |
| `bs.any()`                      | `not bs.empty()`                | |
| `bs.none()`                     | `bs.empty()`                    | |

The semantic differences between `xstd::bit_set<N>` and `std::bitset<N>` are:

- `xstd::bit_set<N>` has a `static` member function `max_size()`;
- `xstd::bit_set<N>` does not do bounds-checking for its members `insert`, `erase`, `replace` and `contains`. Instead of throwing an `out_of_range` exception for argument values outside the range `[0, N)`, this **behavior is undefined**. This gives `xstd::bit_set<N>` a small performance benefit over `std::bitset<N>`.

Functionality from `std::bitset<N>` that is not in `xstd::bit_set<N>`:

- **No integer or string constructors**: `xstd::bit_set` cannot be constructed from `unsigned long long`, `std::string` or `const char*`.
- **No integer or string conversion operators**: `xstd::bit_set` does not convert to `unsigned long`, `unsigned long long` or `std::string`.
- **No I/O streaming operators**: `xstd::bit_set` does not provide overloaded I/O streaming `operator<<` and `operator>>`.
- **No hashing**: `xstd::bit_set` does not provide a specialization for `std::hash<>`.

I/O functionality can be obtained through third-party libraries such as [{fmt}](https://fmt.dev/latest/), which has generic support for ranges such as `xstd::bit_set`. Similarly, hashing functionality can be obtained through third-party libraries such as [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html) by streaming the `xstd::bit_set<N>` elements and size through an overloaded `hash_append` function.

### 3 Set predicates from `boost::dynamic_bitset`

The set predicates `is_subset`, `is_proper_subset` and `intersects` from `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax** and **identical semantics**. Note that these set predicates are not present in `std::bitset`. Efficient emulation of these set predicates for `std::bitset` is not possible using **single-pass** and **short-circuiting** semantics.

| `xstd::bit_set<N>` <br> `boost::dynamic_bitset<>`  | `std::bitset<N>`             |
| :------------------------------------------------  | :---------------             |
| `a.is_subset_of(b)`                                | `(a & ~b).none()`            |
| `a.is_proper_subset_of(b)`                         | `(a & ~b).none() and a != b` |
| `a.intersects(b)`                                  | `(a & b).any()`              |

### 4 The bitwise operators from `std::bitset` and `boost::dynamic_bitset` reimagined as set algorithms

The bitwise operators (`&=`, `|=`, `^=`, `-=`, `~`, `&`, `|`, `^`, `-`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax** and **identical semantics**. Note that the bitwise difference operators (`-=` and `-`) from `boost::dynamic_bitset` are not present in `std::bitset`. The `operator-` can be emulated for `std::bitset` using the identity `a - b == a & ~b`.

The bitwise-shift operators (`<<=`, `>>=`, `<<`, `>>`) from `std::bitset` and `boost::dynamic_bitset` are present in `xstd::bit_set` with **identical syntax**, but with the **semantic difference** that `xstd::bit_set<N>` does not support bit-shifting for lengths `>= N`. Instead of calling `clear()` for argument values outside the range `[0, N)`, this **behavior is undefined**. Note that these semantics for `xstd::bit_set<N>` are identical to bit-shifting on native unsigned integers. This gives `xstd::bit_set<N>` a small performance benefit over `std::bitset<N>`.

With the exception of `operator~`, the non-member bitwise operators can be reimagined as **composable** and **data-parallel** versions of the set algorithms on sorted ranges. In C++23, the set algorithms are not (yet) composable, but the [range-v3](https://ericniebler.github.io/range-v3/) library contains lazy views for them.

[![Try it online](https://img.shields.io/badge/try%20it-online-brightgreen.svg)](https://godbolt.org/z/G1caEeMPb)

| `xstd::bit_set<N>`      | `std::set<int>` with the range-v3 set algorithm views                                                |
| :----------------       | :----------------------------------------------------------------------------------------------------|
| `a.is_subset_of(b)`     | `std::ranges::includes(a, b)`                                                                        |
| <code>a &vert; b</code> | <code>ranges::views::set_union(a, b)                &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a & b`                 | <code>ranges::views::set_intersection(a, b)         &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a - b`                 | <code>ranges::views::set_difference(a, b)           &vert; std::ranges::to&lt;std::set&gt;() </code> |
| `a ^ b`                 | <code>ranges::views::set_symmetric_difference(a, b) &vert; std::ranges::to&lt;std::set&gt;() </code> |

The bitwise shift operators of `xstd::bit_set<N>` can be reimagined as set **transformations** that add or subtract a non-negative constant to all set elements, followed by **filtering** out elements that would fall outside the range `[0, N)`. This can also be formulated in a composable way for `std::set<int>`, albeit without the data-parallelism that `xstd::bit_set<N>` provides.

<table>
<tr>
    <th>
        xstd::bit_set&ltN&gt
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

**Q**: So iterating over an `xstd::bit_set` is really fool-proof?  
**A**: Yes, `xstd::bit_set` iterators are [easy to use correctly and hard to use incorrectly](http://www.aristeia.com/Papers/IEEE_Software_JulAug_2004_revised.htm).

### Bit-layout

**Q**: How is `xstd::bit_set` implemented?  
**A**: It uses a stack-allocated array of unsigned integers as bit storage.

**Q**: How is the set ordering mapped to the array's bit layout?  
**A**: The most significant bit of the first array word maps onto set value `0`.
**A**: The least significant bit of the last array word maps onto set value `N - 1`.

**Q**: I'm visually oriented, can you draw a diagram?  
**A**: Sure, it looks like this for `bit_set<16, uint8_t>`:

|value |01234567|89ABCDEF|
|:---- |-------:|-------:|
|word  |       0|       1|
|offset|76543210|76543210|

**Q**: Why is the set order mapped this way onto the array's bit-layout?  
**A**: To be able to use **data-parallelism** for `(a < b) == std::ranges::lexicographical_compare(a, b)`.

**Q**: How is efficient set comparison connected to the bit-ordering within words?  
**A**: Take `bit_set<8, uint8_t>` and consider when `sL < sR` for ordered sets of integers `sL` and `sR`.

**Q**: Ah, lexicographical set comparison corresponds to bit comparison from most to least significant?  
**A**: Indeed, and this is equivalent to doing the integer comparison `wL > wR` on the underlying words `wL` and `wR`.

**Q**: So the set ordering a `bit_set` is equivalent to its representation as a bitstring?  
**A**: Yes, indeed, and this property has been known for several decades:

> "bit-0 is the leftmost, just like char-0 is the leftmost in character strings. [...]
> This makes converting from and to unsigned integers a little counter-intuitive,
> but the string-ness (or "array-ness") is the foundation of this abstraction.
>
> Chuck Allison, [ISO/WG21/N0128](http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/1992/WG21%201992/X3J16_92-0051%20WG21_N0128.pdf), May 26, 1992

### Storage type

**Q**: What storage type does `xstd::bit_set` use?  
**A**: By default, `xstd::bit_set` uses an array of `std::size_t` integers.

**Q**: Can I customize the storage type?  
**A**: Yes, the full class template signature is `template<int N, std::unsigned_integral Block = std::size_t> xstd::bit_set`.

**Q**: What other storage types can be used as template argument for `Block`?  
**A**: Any type modelling the Standard Library `unsigned_integral` concept, which includes (for GCC and Clang) `xstd::uint128`.

**Q**: Does the `xstd::bit_set` implementation optimize for the case of a small number of words of storage?  
**A**: Yes, there are three special cases for 0, 1 and 2 words of storage, as well as the general case of 3 or more words.

## Requirements

This library depends on the C++ Standard Library and [xstd](https://github.com/rhalbersma/xstd-ints) (fetched automatically via CMake `FetchContent`, for `xstd::align_up`), and is continuously being tested with the following conforming [C++23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf) compilers, against all three mainstream standard libraries (libstdc++, the MSVC STL, and libc++). Following the model of [apt.llvm.org](https://apt.llvm.org/), we support the latest two stable releases of each compiler, plus its current development branch.

| Platform | Compiler | Standard Library | Stable | Qualification | Development | CI |
| :------- | :------- | :--------------- | :----- | :------------ | :---------- | :- |
| Linux | GCC | libstdc++ | 15 | 16 | 17-SVN | [![GCC](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/gcc.yml) |
| Windows | MinGW | libstdc++ | 15 | 16 | — | [![MinGW](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/mingw.yml) |
| Linux | Clang | libstdc++ | 22 (libstdc++ 15) | 23 (libstdc++ 16) | 24-SVN (libstdc++ 17-SVN) | [![Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang.yml) |
| Linux | Clang | libc++ | 22 | 23 | 24-SVN | [![Clang-libc++](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-libc%2B%2B.yml) |
| macOS | Apple Clang | libc++ | 17.0.0 (Xcode 16.4) | 21.0.0 (Xcode 26.6) | — | [![Apple Clang](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/apple-clang.yml) |
| Windows | Clang-CL | MSVC | 19.1.5 (VS 2022) | 20.1.8 (VS 2026) | 20.1.8 (VS 2026-Preview) | [![Clang-CL](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/clang-cl.yml) |
| Windows | MSVC | MSVC | 2022 (17.11+) | 2026 | 2026-Preview | [![MSVC](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml/badge.svg)](https://github.com/rhalbersma/xstd-bits/actions/workflows/msvc.yml) |

`Apple Clang` has no `Development` entry because Apple doesn't publish Apple Clang dev snapshots the way LLVM does; that leg tests the latest stable Xcode release from each of the two supported series. `MinGW` has none either: WinLibs publishes no GCC trunk build between stable branches, and the shared workflow drops that rung with a notice rather than failing, so the row names the two that actually run. The `Linux | GCC` row keeps its `17-SVN` — that ladder is unaffected.

Every leg above passes. The library no longer uses the C++23 range adaptors libc++ has not implemented (`views::cartesian_product`, `views::adjacent`, `views::pairwise_transform`, `views::stride`), and the tests probe for `<flat_set>` rather than assuming it, so the `Clang | libc++` and `Apple Clang` rows and the VS 2022 legs build and run like the rest.

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [fmtlib](https://github.com/fmtlib/fmt), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## License

<pre>
         Copyright Rein Halbersma 2014-2026.
Distributed under the <a href="http://www.boost.org/users/license.html">Boost Software License, Version 1.0</a>.
   (See accompanying file LICENSE_1_0.txt or copy at
         <a href="http://www.boost.org/LICENSE_1_0.txt">http://www.boost.org/LICENSE_1_0.txt</a>)
</pre>
