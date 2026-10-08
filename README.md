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

## From flag words to containers

An unsigned integer is raw bit storage. Used directly, it is a *flag word*: a row of flags set,
cleared and tested with `&`, `|`, `^`, `~` and shifts, where the number it spells is beside the point.

`std::bitset` generalized the flag word to any width, and in doing so mixed three vocabularies:
that of a **sequence** of `bool` (`operator[]` returning a proxy `reference`, `test`/`set`/`reset`/`flip`
by position), that of a **set** of positions (`&`, `|`, `^`, `count`, `any`/`none`/`all`, and Boost's
`is_subset_of`/`intersects`), and that of a **string** of `'0'`s and `'1'`s. Its `to_ulong`/`to_ullong` and
its `unsigned long long` constructor are not arithmetic — it has no `+`, `-`, `*` or `/` — but access to
the storage when it fits one word. Being all of these, it is none of them cleanly: it has no iterators,
compares only at one width, and prints its bits in the opposite order from how it indexes them.

xstd-bits separates the three. Each becomes the bit-packed counterpart of the standard container it
already resembled, speaking that container's vocabulary, with one interface across every storage. The
hybrid itself is not reproduced: there is no bitset type here. `std::bitset<N>` and `boost::dynamic_bitset`
interoperate through `xstd::bit_convert` instead.

| reading  | standard counterpart                                                       | packed here as                                  |
| :------- | :------------------------------------------------------------------------- | :---------------------------------------------- |
| sequence | `std::array<bool, N>`, `std::inplace_vector<bool, N>`, `std::vector<bool>` | `bit_array`, `bit_bounded_vector`, `bit_vector` |
| set      | `std::set<std::size_t>`                                                    | `bit_fixed_set`, `bit_bounded_set`, `bit_set`   |
| string   | `std::string`                                                              | `bit_string` (planned)                          |

Every one of them reads its raw blocks through `from_blocks`, and converts to and from anything else that has
bit storage through `xstd::bit_convert` — the general form of what `to_ullong` does for one word. Position `i`
stays position `i` at any two widths: two fixed widths must be equal, a run-time width converts into a fixed one by
value and throws `std::overflow_error` for a position the target cannot hold, and anything converts into a bounded,
small or dynamic owner, which adopts the blocks outright from an rvalue holding the same container. Because
positions are kept, a `std::bitset`'s `to_string()` reads reversed against its positions.

xstd-bits is **six containers**: two readings of a block of bits — an ordered set of `std::size_t` and a sequence of `bool` — over three storages, which differ in whether size and capacity are static or dynamic: both static, a dynamic size within a static capacity, and both dynamic.

### Each one is the packing of a standard container, and speaks that container's vocabulary

The relationship is the one `std::flat_set` has to `std::set`: **a different representation under the same interface**, departing from it only where the representation forces a departure. `xstd::bit_vector` answers `std::vector<bool>`'s synopsis line for line; `xstd::bit_array<N>` answers `std::array<bool, N>`'s, `xstd::bit_bounded_vector<N>` answers `std::inplace_vector<bool, N>`'s, and the three sets answer `std::set<std::size_t>`'s. Each is held to its counterpart by a checklist that spells that counterpart's synopsis out as a `requires`-expression and is asserted **on the counterpart first**, so a line the model itself cannot answer can never be asked of the packing.

The yardstick is the [current working draft](https://eel.is/c++draft/), not the standard the library compiles as. Where C++23 and the draft disagree the draft wins, and [design.md](doc/design.md) records which paper moved each line.

Three things the packing genuinely forces, and nothing else:

- **No nodes.** A position is a bit in a block, so there is nothing to unlink and hand over: `node_type`, `extract`, `insert(node_type&&)` and `merge` have no meaning here. `std::flat_set` drops the same four for the same reason.
- **A proxy reference.** A bit has no address, so `operator[]` returns a proxy, `pointer` names nothing, and `data()` goes with it. `std::vector<bool>` makes exactly this trade. The views lose what `std::span` builds on an address: its constructors from a contiguous iterator, an array or a contiguous range of `bool`, which a view over the blocks or the owner stands in for, and `size_bytes()` and `as_bytes`, a window's bits sharing their bytes with bits it does not view. Everything the standard asks *of* the proxy is here — the const-qualified assignment of [P2321R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2321r2.html), `flip()`, and the three hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html).
- **Different invalidation — mostly the other way.** An iterator here is a container and an index, not a pointer into the blocks, so growth that reallocates the blocks leaves it valid. That is `std::set`'s guarantee over storage that is `std::flat_set`'s.

Everything else is addition rather than subtraction: the bitwise operators, `find_first`/`find_next`, the set queries `intersects` and `disjoint` beside the containments `is_subset_of` and `is_superset_of`, each with its proper form, the byte exchange, and the views that give you a second reading of bits you already own.

### Out of range means what each counterpart means by it

The **set reading** takes a key, and a key outside the domain is a lookup that answers no rather than an error: `contains`, `find`, `count`, `lower_bound`, `upper_bound`, `equal_range` and `erase` are total, as they are on `std::set`. So is a value that is no key at all, an enumerator missing from the list or a flag value of several bits: it is no element, and where the key type has an order of its own, the bounds place it among the keys by that order. A shift names no one key, so `<<=` keeps what lands below `max_size()` and drops the rest, as `std::bitset`'s does past `N`. The **sequence reading** indexes, so out of range is out of bounds there: `at(n)` throws `out_of_range` at every width and through every handle, and everything else is the precondition `std::vector`, `std::array` and `std::span` already make it — stated with an `assert`, at the member you called.

`constexpr` is not on that list of advantages, and has not been since [P3372R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3372r3.html) made the standard's own containers `constexpr` throughout. It is table stakes now; what the packing buys is density and the bit-parallel operations over it.

A constant can also be a template argument, as a `std::array<bool, N>` can, wherever the width fills its blocks: `template<xstd::bit_align<xstd::bit_fixed_set<64>> Mask> struct S;` takes a mask by value, and so does `bit_align<bit_array<N>>`. A width that leaves unused bits in its last block is not a structural type, since those bits must stay clear for `==`, `<=>` and hashing to hold; [design.md](doc/design.md#structural-at-aligned-widths) has the reasoning.

Where the type fixes a size, the member that reports it is a constant, after the [static `constexpr std::integral_constant` idiom](https://www.think-cell.com/en/career/devblog/the-new-static-constexpr-std-integral_constant-idiom): `size`, `empty` and `max_size` on `bit_array<N>` and on a `bit_subspan` of static extent, `capacity` and `max_size` on `bit_bounded_vector<N>`, and `max_size` on `bit_fixed_set<N>` and `bit_bounded_set<N>`, whose `size()` counts keys. `a.size()` and `A::size()` still answer a `size_type`, `noexcept`, so every call the counterpart allows means what it does there; `A::size` is also a `std::integral_constant<std::size_t, N>`, which `decltype` reaches through any reference as a constant expression. Two things follow: `auto n = A::size;` deduces the `integral_constant`, so `auto n = a.size();` is the spelling for a `size_type`, and `a.size` without parentheses compiles, naming the constant. [design.md](doc/design.md#constant-sizes) has the details.

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

The calling code for these algorithms is listed below. Here, the pretty-printing as a `set` using `{}` delimiters is triggered by the nested `key_type`. Note that `xstd::bit_fixed_set<N>` acts as a **drop-in replacement** for `std::set<int>` (or `std::flat_set<int>`).

```cpp
int main()
{
    constexpr auto N = 100UZ;
    using X = xstd::bit_fixed_set<N>; /* or xstd::bit_set, std::set<std::size_t>, std::flat_set<std::size_t> */

    auto const primes = opt::sift_primes0<X>(N);
    assert(std::format("{}", primes) == "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}");

    auto const twins = opt::filter_twins(primes);
    assert(std::format("{}", twins)  == "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}");
}
```

Those two assertions are the ones [`test/src/bits/std_set/sieve.cpp`](test/src/bits/std_set/sieve.cpp) makes, over `std::set`, `std::flat_set`, `bit_fixed_set<N>` and `bit_set` alike.

### Sieves that need no bound

`generate_candidates` materializes every candidate below `n` before sifting one, which is what makes the sieve above `O(n)` in space and why it cannot answer "what is the next prime". The same header carries two variants that drop the bound, and they drop it in opposite directions:

```cpp
auto sieve = opt::incremental_sieve();
sieve.next();  // 2, then 3, 5, 7, ... forever, with no n anywhere

auto primes = opt::sift_primes_segmented<xstd::bit_set, xstd::bit_fixed_set<1 << 15>>(n);
```

The **incremental** sieve ([O'Neill 2009](https://www.cs.hmc.edu/~oneill/papers/Sieve-JFP.pdf)) keeps one entry per prime found — the next composite that prime will strike — so its space is `O(π(n))` and it generates without end. It is also about **29× slower**, which is the honest price of unboundedness and the reason it is measured rather than recommended.

The **segmented** sieve is the one that pays. Base primes below `√n` once, then a single reusable window walked over the rest, so peak memory is `O(√n + W)` whatever `n` is. `Window` is a template parameter carrying its own extent, which makes `bit_fixed_set<W>` the natural argument: a compile-time width that allocates nothing in the loop. At `n = 2^20` it is **1.7× faster** than the bounded sieve as well as far thriftier — the window stays in L1 for a whole segment where a megabit sieve is walked with a stride. Less memory *and* less time is the unusual direction for that trade to run.

All three agree, and the test asserts that rather than the README claiming it.

### Hashing, and choosing the algorithm

Every owner, and every set view, hashes, and equal values hash equal whatever holds them. Behind it is [Boost.Hash2](https://www.boost.org/doc/libs/release/libs/hash2/), and there are two hashers, by what they hash:

- **`xstd::hasher<H>` hashes the value**, as its standard model does. It is [xstd-misc](https://github.com/rhalbersma/xstd-misc)'s, and runs Hash2's `hash_append`, which each type here meets through a hook that appends exactly the message Hash2 builds for the model: `std::array<bool, N>` for `bit_array<N>`, `std::vector<bool>` for `bit_vector` and `bit_small_vector`, `std::inplace_vector<bool, N>` for `bit_bounded_vector<N>`, and `std::set<Key, Compare>` for every set. So `xstd::hasher<H>()(x) == xstd::hasher<H>()(model(x))`, and a bit container is a drop-in member of a Hash2-hashed aggregate. It costs what the model costs: a byte per bool, a word per key.
- **`xstd::bit_hasher<H>` hashes the bits**, from [`<xstd/bits/bit_hasher.hpp>`](include/xstd/bits/bit_hasher.hpp). Its message is the value's bit string as bytes, bit `i` at bit `i % 8` of byte `i / 8` with the unused high bits clear, followed by the width only where the width is a run-time value; a set of run-time width stops at the byte of its last key and appends that byte count instead, since equal sets need not share a width. Wherever the storage already holds those bytes, little-endian blocks of `std::uint8_t` up to `unsigned __int128`, that is one `update` over the storage; elsewhere, and in a constant expression, the same bytes are assembled. The block type is no part of the message, so `bit_hasher` is transparent. `xstd::bit_hash_append(h, f, x)` is the same message for a user's own hook, and also takes a contiguous range of one static-width container, nested arrays included, as the planes of a board game's position: where each plane's object is its bytes, the whole range goes in as one `update`.

`std::hash<T>` is `xstd::bit_hasher<boost::hash2::xxhash_64>`, which is also `bit_hasher`'s default: unseeded, which suits keys your program makes. It is xxHash at every length, never FNV-1a, because FNV-1a does not avalanche: the high bits of the last byte it reads never reach the low bits of its result, which are the bits a hash table indexes by. For keys an adversary can choose, name SipHash and seed it per container, as Hash2 advises:

```cpp
#include <xstd/bits/bit_hasher.hpp> // xstd::bit_hasher
#include <xstd/bits/bit_set.hpp>
#include <boost/hash2/siphash.hpp>  // siphash_64
#include <cstdint>
#include <unordered_set>

using hasher = xstd::bit_hasher<boost::hash2::siphash_64>;

auto const seed = std::uint64_t{/* drawn at random, per container */};
auto table      = std::unordered_set<xstd::bit_set, hasher>(0, hasher(seed));
```

The same spelling works for every other owner and set view, with `xstd::hasher` in place of `xstd::bit_hasher`, and with `boost::unordered_flat_set` as with `std::unordered_set`. A sequence view hashes no more than `std::span` does. [design.md](doc/design.md#the-hashing-invariant) has what each hasher appends and why the default is xxHash.

## Headers

Eight containers: two readings of a block of bits, each over four storages.
The reading picks the vocabulary, the storage picks whether size and capacity
are static or dynamic. Each name is a class with the constructors of the standard
container it packs. These eight owners and the three views are the public surface;
the adaptor each reading is built on is internal, under `<xstd/bits/detail/>`.

| Header | Additions | Description | Reference |
| :----- | :-------- | :---------- | :-------- |
| `<xstd/bits/bit_index_mapping.hpp>` | `bit_index_mapping` <br> `sized_bit_index_mapping` <br> `bit_mask_mapping` | What a set asks of a key's mapping: `bit_index_mapping<M, Key>` takes a key to a `std::size_t` position through `M::to_index` and back through `M::from_index`, preserving order, and `sized_bit_index_mapping<M, Key>` adds `M::size`, closing the universe at the positions `0` to `size - 1`, and `M::is_key(key)`, whether a value is one of those keys, which is what `to_index` asks of its argument, and `bit_mask_mapping<M, Key>` adds `M::to_block` and `M::from_block`, which read any value of `Key` as the block of its one-bit keys and back, so that a fixed set over `M` converts with `Key` as a mask; `bit_flag_mapping` alone models it | none |
| `<xstd/bits/bit_key_mapping.hpp>` | `bit_key_mapping` <br> `bit_range_mapping` <br> `bit_find_mapping` <br> `enum_traits` | A key's mapping, an order-preserving bijection onto the positions `0` to `N - 1`. `bit_key_mapping<Key>` is the default a set takes, specialized as `std::char_traits` is: the identity for every unsigned integer key, from `std::uint8_t` to the 128-bit types, and for a strong index type whatever its author writes. `bit_range_mapping<Key, First, N>` maps the `N` consecutive keys from `First`, of an integer or an enumeration, onto positions `0` to `N - 1`; `bit_find_mapping<Key, Keys>` maps each key of a sorted list to its rank by binary search, so a gap costs no bit. An enumeration whose author lists its values in `enum_traits<E>::values` is keyed by `bit_key_mapping<E>` as a range where they run without a gap and as a search of the list otherwise. Each says through `is_key` which values are its keys: a range those from `First` to `First + N - 1`, a list those listed, and the identity every key that names a position | none |
| `<xstd/bits/bit_fixed_set.hpp>` | `bit_fixed_set` <br> `basic_bit_fixed_set` | Ordered set of `std::size_t`, static size and capacity; the `basic_` form takes the key first, `basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>`, `Compare` being `std::less` or `std::greater`. `bit_fixed_set<N>` holds `N` bits in `std::size_t` blocks, and `<xstd/bits/bit_blocks.hpp>`'s transformations change that, the key, mapping and direction kept: `bit_least<bit_fixed_set<N>>` in the smallest block that holds them, `bit_fast<bit_fixed_set<N>>` in the fastest, and `bit_align<bit_fixed_set<N>>` with `N` rounded up to whole blocks. Every set asks `contains(k)` of one key and `x.is_subset_of(y)` of another set, and three free functions ask of two: `includes(x, y)`, all of `y` in `x` as `std::ranges::includes(x, y)` reads it, and `intersects(x, y)` and `disjoint(x, y)`, any and none | [associative.reqmts], [set] |
| `<xstd/bits/bit_enum_set.hpp>` | `bit_enum_set` | Ordered set of an enumeration whose `bit_key_mapping` closes its universe, listed in `enum_traits` or specialized directly, a fixed set in the smallest block that holds it: `bit_enum_set<Enum>` is `bit_least<basic_bit_fixed_set<Enum, std::size_t, N, bit_key_mapping<Enum>>>`, another block being `basic_bit_fixed_set<Enum, Block, N>`'s to spell, deduced from a braced list as `basic_bit_fixed_set{E::a, E::b}`; an enumerator on either side of `\|`, `&`, `^` and `-`, or on the right of their compound forms, is a one-element set, and the enumeration itself gains no operator | [associative.reqmts], [set] |
| `<xstd/bits/bit_flag_mapping.hpp>` | `bit_flag_mapping` | A bitmask type keyed on its own one-bit values: `bit_flag_mapping<Key, N>` ranks a value at the position of its single bit and maps rank `i` to the value with bit `i` alone, for an enumeration in its underlying type's unsigned counterpart, so an enumerator on a signed type's sign bit is a position like the others, for an integer type in its unsigned counterpart, a signed one's sign bit being no position, and for a `std::bitset` as wide as a block through `xstd::bit_convert`; `is_key` holds for a value with exactly one bit set below `N` | none |
| `<xstd/bits/bit_flag_set.hpp>` | `bit_flag_set` | A flag type, `bit_flag_set<Mask, N>`: the set of a bitmask type's one-bit values below `N`, where `Mask` is an enumeration, a built-in integer type or a `std::bitset` as wide as a block. A signed integer keeps its sign bit out: `N` is at most `std::numeric_limits<Mask>::digits`, 31 for `int`, so `bit_flag_set<std::ios_base::fmtflags>` is one spelling whether the implementation makes `fmtflags` an enumeration or an integer, as [bitmask.types] lets it. It is no class of its own but an alias, `bit_least<basic_bit_fixed_set<Mask, std::size_t, N, bit_flag_mapping<Mask, N>, std::greater<Mask>>>`, and what makes it a flag type is the mapping: wherever `bit_mask_mapping<KeyMapping, Key>` holds, a fixed set reads every value of its key as a mask of one-bit keys. It then converts implicitly from and to the mask, so code written against `std::filesystem::perms` keeps its constants and changes only the variable's type; each mixed operator works in both orders, a braced list is the union of its values, `contains(k)` is membership of one flag and false for a value of several bits, `intersects(p, m)` and `disjoint(p, m)` are any-of and none-of with a mask on either side, `p.is_superset_of(m)` is all-of, and every containment, proper or not, takes a mask as the other set, and `~` stays within `N`. Iteration yields the one-bit values from the highest down, and `<=>` is the mask's numeric order; `xstd::bit_convert<Block>(p)` and `bit_flag_set(xstd::from_blocks, word)` give and take the word, and `bit_underlying<bit_flag_set<Mask, N>>` holds it in the mask's own unsigned word. `examples/include/xstd/filesystem.hpp` is `using perms = bit_flag_set<std::filesystem::perms, 16>;` | [bitmask.types] |
| `<xstd/bits/bit_bounded_set.hpp>` | `bit_bounded_set` <br> `basic_bit_bounded_set` | Ordered set, dynamic size within a static capacity; `basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>` | [associative.reqmts], [set] |
| `<xstd/bits/bit_set.hpp>` | `bit_set` <br> `basic_bit_set` | Ordered set, dynamic size and capacity; `basic_bit_set<Key, Block, KeyMapping, Compare, Allocator>` | [associative.reqmts], [set] |
| `<xstd/bits/bit_array.hpp>` | `bit_array` <br> `basic_bit_array` | Sequence of `bool`, static size and capacity; `rotate(n)` and `reverse()` permute it in place a block at a time, as `std::ranges::rotate` and `std::ranges::reverse` would | [array], [P3103R2](https://wg21.link/P3103R2) |
| `<xstd/bits/bit_bounded_vector.hpp>` | `bit_bounded_vector` <br> `basic_bit_bounded_vector` | Sequence of `bool`, dynamic size within a static capacity; `rotate(n)` and `reverse()` as `bit_array` | [inplace.vector], [P3103R2](https://wg21.link/P3103R2) |
| `<xstd/bits/bit_vector.hpp>` | `bit_vector` <br> `basic_bit_vector` | Sequence of `bool`, dynamic size and capacity; `rotate(n)` and `reverse()` as `bit_array` | [vector.bool], [P3103R2](https://wg21.link/P3103R2) |
| `<xstd/bits/ext/boost.hpp>` | `bit_small_set` <br> `bit_small_vector` <br> and their `basic_` forms | Both readings, dynamic size staying inline within a static capacity | [`boost::container::small_vector`](https://www.boost.org/doc/libs/release/doc/html/boost/container/small_vector.html) |
| `<xstd/bits/bit_set_view.hpp>` | `bit_set_view` | Set reading of bits another container owns, or of unsigned blocks in place: `bit_set_view(board)` is a `bit_set_view<std::uint64_t>` | none |
| `<xstd/bits/bit_span.hpp>` <br> `<xstd/bits/bit_subspan.hpp>` | `bit_span` <br> `bit_subspan` | Sequence reading over borrowed bits, whole or sliced, or over unsigned blocks in place: `bit_span(blocks)`; a whole view rotates and reverses what it views as the owners do, a slice does not | [views.span], [P3103R2](https://wg21.link/P3103R2) |
| `<xstd/bits/bit_blocks.hpp>` | `bit_block` <br> `bit_block_range` <br> `bit_blocks` <br> `owned_bit_blocks` <br> `resizable_bit_blocks` <br> `bit_blocks_extent_v` <br> `least_block_t` <br> `fast_block_t` <br> `underlying_block_t` <br> `bit_least` <br> `bit_fast` <br> `bit_align` <br> `bit_underlying` | What every container and view presents a packed interface over: one unsigned block, or a sized contiguous range of them, in no reading of its own. The views take any of it; the owners hold what can be owned, a regular value read-only through `const`, and at a run-time width only what resizes. A built-in array of blocks is read as the `std::array` of the same blocks and is never owned. Also the width its type names, which the views default to, and the smallest and the fastest block holding `N` bits, `<cstdint>`'s least and fast pair, and an enumeration's underlying type or an integer type made unsigned, `underlying_block_t<Key>`, `bool` and the character types having no such counterpart. Four type transformations take a fixed-width owner, an array, a fixed set or a bounded one, to the same owner over another block or width, every other argument kept, as `std::make_unsigned_t` and `std::simd`'s `rebind_t` and `resize_t` do: `bit_least<X>` and `bit_fast<X>` swap the block for the smallest or the fastest holding `N`, `bit_align<X>` rounds `N` up to whole blocks, and `bit_underlying<X>`, for a set whose key is an enumeration or an integer, swaps the block for `underlying_block_t` of the key, the word an existing field or ABI already stores the flags in, so that the set's block is the mask's representation bit for bit. They compose, least first then align being the compact form with no unused tail: `bit_align<bit_least<bit_fixed_set<3>>>` is one byte, where `bit_least<bit_align<bit_fixed_set<3>>>` is eight | none |
| `<xstd/bits/bit_hasher.hpp>` | `bit_hasher` <br> `bit_hash_append` | The hash of the bits: their bytes, bit `i` at bit `i % 8` of byte `i / 8`, then a run-time width, whatever the block type; one `update` over the storage where it holds those bytes. `bit_hash_append` appends the same from a user's own `hash_append` hook, of one container or of a contiguous range of static-width ones; `bit_hasher<H>` runs it under the Hash2 algorithm `H`, seeded as `H` is, and is transparent | none |
| `<xstd/bits/from_blocks.hpp>` | `from_blocks` <br> `from_blocks_t` <br> `bit_constructible_from` | The tag that says an argument's blocks are read as bits, so a static width deduces from them; and the concept for blocks an owner takes as they are | [range.utility.conv] |
| `<xstd/bits/bit.hpp>` <br> `<xstd/bits/bit/bit_convert.hpp>` | `bit_convert` <br> `bit_convertible` <br> `bit_convertible_to` | Positions from anything that has bit storage into anything else that does, at any two widths: ours, blocks, a `std::bitset`; the constraint on the two types, and the concept for a valid call | none |
| `<xstd/bits/ext/boost/dynamic_bitset.hpp>` | `bit_convert` | Both ways between `boost::dynamic_bitset` and the owners, by block range; opt-in, outside every umbrella | [`boost::dynamic_bitset`](https://www.boost.org/doc/libs/release/libs/dynamic_bitset/) |

`<xstd/bits.hpp>` exports the whole surface, so one include brings everything above.
The headers directly under `<xstd/bits/>` are the containers, views and concepts; `<xstd/bits/bit/>` holds free utilities that extend `<bit>`, exported together by `<xstd/bits/bit.hpp>` as in xstd-ints.
A `bit_` prefix names what works on the packed bits: the containers, the concepts and transformations over blocks, and `bit_hasher`. The unprefixed name works on the value as the standard model sees it, as `xstd::hasher` does. The prefix names *what* is hashed, the packed representation, not whether the code is optimized: the hooks behind `xstd::hasher` expand bits to bools with `_pdep_u64` or AVX-512 where the target has them, and still append the model's bytes.
The headers under `<xstd/bits/detail/>` are implementation and carry no stability promise.

## Requirements

- A conforming [C++23](https://wg21.link/N4950) compiler
- CMake 3.28 or later when using the supplied CMake project
- [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc), fetched automatically via CMake `FetchContent` when not already installed
- [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support
- [Boost.Container](https://github.com/boostorg/container) for the bounded column's blocks where the standard library has no `std::inplace_vector`, and for the `ext/` column
- [Boost.DynamicBitset](https://github.com/boostorg/dynamic_bitset), only where `<xstd/bits/ext/boost/dynamic_bitset.hpp>` is included: the `dynamic-bitset` feature in `vcpkg.json`, and a `Boost::dynamic_bitset` the consumer links itself

This library depends on the C++ Standard Library, on [xstd-ints](https://github.com/rhalbersma/xstd-ints) and [xstd-misc](https://github.com/rhalbersma/xstd-misc) (both fetched automatically via CMake `FetchContent` when not already installed), on [Boost.Hash2](https://github.com/boostorg/hash2) for the hashing support, and on [Boost.Container](https://github.com/boostorg/container) for the bounded column where the standard library has no `std::inplace_vector`. It is continuously being tested with the following conforming [C++23](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf) compilers, against all three mainstream standard libraries (libstdc++, the MSVC STL, and libc++). Following the model of [apt.llvm.org](https://apt.llvm.org/), we support the latest two stable releases of each compiler, plus its current development branch.

Two standards are in play and they answer different questions. **C++23 is what the library compiles as** — that is the language it requires, and the tests build at C++26 as well — on GCC 16, GCC 17-SVN and MinGW 16, the three rungs whose libstdc++ carries `<inplace_vector>`. The bounded column exists at either standard and on every standard library: its blocks are a `std::inplace_vector` where the library has one and a `boost::container::static_vector` everywhere else. Only over `std::inplace_vector` are the bounded owners usable in a constant expression, and `XSTD_BITS_HAS_CONSTEXPR_BOUNDED` is defined exactly there, for code to test with `#ifdef`; over `static_vector` their copies are not `noexcept` either, as `static_vector`'s own are not, while their `swap` is `noexcept` over both, as `[inplace.vector.modifiers]` declares it. **The current [working draft](https://eel.is/c++draft/) is what the interfaces are measured against**, because a counterpart's synopsis is a moving target and the newest one is the one worth answering. So `std::bitset`'s `basic_string_view` constructor ([P2697R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2697r1.pdf)) and its `charT` Constraints ([LWG 4294](https://cplusplus.github.io/LWG/issue4294)) are here even though C++23 has neither, and the proxy carries the hidden-friend `swap`s of [P3612R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3612r1.html) beside the static `swap(reference, reference)` that paper moved to `[depr.vector.bool.swap]`.

Note that the benchmarks and unit tests depend on [Boost](https://www.boost.io/), [Google Benchmark](https://github.com/google/benchmark) and [range-v3](https://github.com/ericniebler/range-v3). 

## Installation


The library is header-only and its CMake target carries everything a consumer needs: the include directories, the `xstd-ints`, `xstd-misc`, `Boost::container` and `Boost::hash2` dependencies, and `cxx_std_23`. Link `xstd::bits` and you are done — there is no `target_include_directories` or `CMAKE_CXX_STANDARD` to set on your side.

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

[`vcpkg.json`](vcpkg.json) is this repository's own manifest, not a published port: it is what `VCPKG_ROOT`-based presets install from when you build **this** library. Its `test` feature — Boost.Test, Boost.Dynamic Bitset, Boost.Unordered, Google Benchmark and range-v3 — is a default feature because building the repository normally means building its tests. The `no-tests-vcpkg` preset turns that off with `VCPKG_MANIFEST_NO_DEFAULT_FEATURES`, so a packaging or install build pays for Boost.Hash2 and nothing else. Consuming the library by any of the three methods above does not read this manifest at all.

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
