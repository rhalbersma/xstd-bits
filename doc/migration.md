# Migrating to xstd-bits

A move to xstd-bits serves one or more of four goals, each with its own payoff:

1. **A class interface where none existed**, over raw integers, enumerations and flags, gives type safety and
   expressiveness: the keys keep their own types, and the shifts and masks become named members.
2. **A focused interface in place of a hybrid one**, where `std::bitset` and `boost::dynamic_bitset` offer every
   reading at once, gives focus: the type says which reading the code means.
3. **A space-optimized container**, in place of the standard's containers of `bool` and `std::size_t`, spends a bit
   per position.
4. **A speed-optimized algorithm** over those containers works a block at a time, which turns the space optimization
   into a time optimization too.

Where the code changes, a section shows how as a *Tony Table*, the before-and-after form WG21 papers use: what the
code says today on the left, what it says here on the right. Every right-hand column compiles and runs against the
library with GCC 15 and Clang 22, and so does every left-hand one except itsy_bitsy's, whose last release no longer
builds with either.

## A class interface where none existed

Code that keeps bits in an integer spells them as positions, enumerators or flags, and works them with shifts and
masks; the class takes the keys the code already has, and the shifts and masks become its members:

- **positions in a word**: `bit_fixed_set<N>` for a word of its own, or `bit_array<N>` where the word is a row of
  `bool`; `bit_set_view` for one kept where it is.
- **an enumeration's keys**: `bit_enum_set<Enum>`.
- **a bitmask type's flags**: `bit_flag_set<Mask, N>`, for the standard library's
  [[bitmask.types]](https://eel.is/c++draft/bitmask.types) and for other libraries' flags, such as Qt's `QFlags` and
  POSIX's `sa_flags`.

### A bitboard or a flag word

A word whose bits are positions, as a chess engine's bitboard or a flag word, becomes a `bit_fixed_set<64>`. The
shift-and-mask primitives become `insert`, `erase` and `contains`, `std::popcount` becomes `size`, and the
`w &= w - 1` loop over `std::countr_zero` becomes a range-for, lowest position first. `&`, `|`, `^` and the shifts
stay as the word's: the same precondition, `n < 64` for a shift, and the same instructions, so code that already
works on the word keeps working and keeps its speed; `a & ~b` is also `a - b`. A word read as a row of `bool`,
indexed, rotated or reversed, is a `bit_array<64>` instead.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
std::uint64_t bb = 0;
bb |= std::uint64_t{1} << sq;
bb &= ~(std::uint64_t{1} << sq);
(bb >> sq) & 1;
std::popcount(bb);
for (auto x = bb; x != 0; x &= x - 1) {
        use(std::countr_zero(x));
}
auto north = bb << 8;
auto both = bb & other;
```

</td><td>

```cpp
#include <xstd/bits/bit_set/bit_fixed_set.hpp>

auto bb = xstd::bit_fixed_set<64>();
bb.insert(sq);
bb.erase(sq);
bb.contains(sq);
bb.size();
for (auto i : bb) {
        use(i);
}
auto north = bb << 8;
auto both = bb & other;
```

</td></tr>
</table>

Kept where it is, in a struct or an ABI, the word takes the set's interface in place through a view.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
std::uint64_t w = 0;
w |= std::uint64_t{1} << 5;
w &= ~(std::uint64_t{1} << 5);
(w >> 9) & 1;
std::popcount(w);
for (auto x = w; x != 0; x &= x - 1) {
        use(std::countr_zero(x));
}
```

</td><td>

```cpp
#include <xstd/bits/bit_set/bit_set_view.hpp>

auto v = xstd::bit_set_view(w);  // w stays where it is
v.insert(5);
v.erase(5);
v.contains(9);
v.size();
for (auto i : v) {
        use(i);
}
```

</td></tr>
</table>

### An enumeration's keys

`bit_enum_set<Enum>` is the set of an enumeration's enumerators, one bit each in the smallest unsigned type that holds
them, as Java's `EnumSet` is. An enumeration that ends in a `count` enumerator needs nothing more; another lists its
values in a specialization of `xstd::enum_traits`. The shifts by `std::to_underlying` go, and iteration yields the
keys in the enumeration's own type.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
enum class weekday { mon, tue, wed, thu, fri, sat, sun, count };

auto days = 0U;
days |= 1U << std::to_underlying(weekday::sat);
days &= ~(1U << std::to_underlying(weekday::sat));
(days >> std::to_underlying(weekday::sun)) & 1U;
std::popcount(days);
for (auto x = days; x != 0; x &= x - 1) {
        use(static_cast<weekday>(std::countr_zero(x)));
}
```

</td><td>

```cpp
#include <xstd/bits/bit_set/bit_enum_set.hpp>

auto days = xstd::bit_enum_set<weekday>();  // one byte
days.insert(weekday::sat);
days.erase(weekday::sat);
days.contains(weekday::sun);
days.size();
for (auto d : days) {
        use(d);
}
```

</td></tr>
</table>

### Enumeration flags: `std::filesystem::perms` and other [[bitmask.types]](https://eel.is/c++draft/bitmask.types)

`bit_flag_set<Mask, N>` is the set of the mask's one-bit values. It converts implicitly from and to the mask, so
the constants and the functions that take the mask stay as they are; what changes is the type of the variable.
Iteration yields the flags from the highest down.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
namespace fs = std::filesystem;

auto p = fs::perms::owner_read | fs::perms::owner_write;
p |= fs::perms::group_read;
(p & fs::perms::owner_write) != fs::perms::none;
(p & mask) == mask;
fs::permissions(path, p);
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit_set/bit_flag_set.hpp>

using perms = xstd::bit_flag_set<fs::perms, 16>;

auto p = perms{fs::perms::owner_read, fs::perms::owner_write};
p |= fs::perms::group_read;
p.contains(fs::perms::owner_write);
xstd::ranges::bit_includes(p, mask);
fs::permissions(path, p);  // converts to fs::perms
```

</td></tr>
</table>

### Qt's `QFlags`

`QFlags<Enum>` wraps an enumeration of one-bit values; `bit_flag_set` over the same enumeration replaces it, keyed on
the flags themselves. It takes the enumeration although `Qt::AlignLeft | Qt::AlignTop` is a `QFlags` rather than an
`Qt::AlignmentFlag`: a flag type accepts any enumeration whose `|` compiles, whatever it returns. `Qt::AlignmentFlag`
has no fixed underlying type, so its values are only those of the nine-bit field its enumerators need, and the width
is written out: with a wider one, `~a` would form a value the enumeration cannot hold.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <QtCore/QFlags>

Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop;
a |= Qt::AlignBottom;
a &= ~Qt::AlignTop;
a.setFlag(Qt::AlignLeft, false);
a.setFlag(Qt::AlignHCenter);
a.testFlag(Qt::AlignHCenter);
a.testFlags(Qt::AlignCenter);
a.testAnyFlags(Qt::AlignVertical_Mask);
a.toInt();
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit_set/bit_flag_set.hpp>

using Alignment = xstd::bit_flag_set<Qt::AlignmentFlag, 9>;

auto a = Alignment{Qt::AlignLeft, Qt::AlignTop};
a |= Qt::AlignBottom;
a -= Qt::AlignTop;
a.erase(Qt::AlignLeft);
a.insert(Qt::AlignHCenter);
a.contains(Qt::AlignHCenter);
xstd::ranges::bit_includes(a, Qt::AlignCenter);
xstd::ranges::bit_intersects(a, Qt::AlignVertical_Mask);
Qt::AlignmentFlag(a);
```

</td></tr>
</table>

Member by member:

| `QFlags` | Flag type |
| :-- | :-- |
| `testFlag(f)`, `f` one bit | `contains(f)` |
| `testFlags(m)` | `xstd::ranges::bit_includes(a, m)`, except that `testFlags(0)` asks whether `a` is empty, and `bit_includes(a, 0)` is always true |
| `testAnyFlags(m)` | `xstd::ranges::bit_intersects(a, m)` |
| `setFlag(f, on)`, `f` one bit | `on ? a.insert(f) : a.erase(f)` |
| `setFlag(m, on)`, `m` several bits | `on ? a \|= m : a -= m` |
| `toInt()` | the conversion to the mask, `Qt::AlignmentFlag(a)`, or `xstd::bit_convert<unsigned>(a)` for the word |

An unscoped enumeration converts to `bool`, so `if (a & Qt::AlignLeft)` still reads as any-of, through the
conversion to the mask. A scoped enumeration does not, and its flag type then asks `xstd::ranges::bit_intersects` or
`contains` instead.

### A signed word of unsigned flags: POSIX `sa_flags`

A flag type keeps a signed word's sign bit out of its universe, so a flag that sits there needs the word's unsigned
type. glibc's `SA_RESETHAND` is `0x80000000`, an `unsigned` constant, stored in `struct sigaction`'s `int sa_flags`.
The word is `int`, the flags are `unsigned`: key on `unsigned`, and cast once on each side of the field. Iteration
is then `std::set<unsigned, std::greater<unsigned>>`'s, `SA_RESETHAND` first.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <signal.h>

struct sigaction sa{};
sa.sa_flags = SA_RESTART | SA_RESETHAND;
sa.sa_flags |= SA_NODEFER;
(sa.sa_flags & SA_RESETHAND) != 0;
```

</td><td>

```cpp
#include <xstd/bits/bit_set/bit_flag_set.hpp>
#include <signal.h>

using sa_flags = xstd::bit_flag_set<unsigned>;

auto f = sa_flags{SA_RESTART, SA_RESETHAND};
f |= SA_NODEFER;
f.contains(SA_RESETHAND);
sa.sa_flags = static_cast<int>(unsigned(f));
auto g = sa_flags(static_cast<unsigned>(sa.sa_flags));
```

</td></tr>
</table>

`xstd::bit_flag_set<int>{SA_RESETHAND}` does not compile on glibc: the constant narrows into `int`. A run-time value
with the sign bit set fails the flag type's assertion on the way in.

## A focused interface in place of a hybrid one

`std::bitset` and `boost::dynamic_bitset` each offer three readings at once; here the type says which, at
`std::bitset<N>`'s fixed width or `boost::dynamic_bitset`'s run-time one:

- **a set of positions**: membership, flags, iterating over what is set. That is `bit_fixed_set<N>` or `bit_set`.
- **a sequence of `bool`**: a value at each index, rotating, a row of cells. That is `bit_array<N>` or `bit_vector`.
- **a row combined whole**: `&`, `|`, `^`, `~` and shifts over every element at once. That is `std::valarray<bool>`'s
  reading, `bit_fixed_valarray<N>` or `bit_valarray`.

itsy_bitsy offers the sequence reading alone, and takes the same destinations.

### `std::bitset<N>` as a set of positions

The positions become keys, and `std::bitset`'s verbs become `std::set`'s. The shifts are an integer's rather than
`std::bitset`'s: `b << n` empties a bitset once `n >= N`, while `s << n` takes `n < N` as its precondition, as a shift
of a `std::uint64_t` does, and a hardened build checks it. Where the count can reach the width, `xstd::shl(s, n)` and
`xstd::shr(s, n)` are the total forms, which empty the set there and shift the other way for a negative count.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <bitset>

auto b = std::bitset<64>();
b.set(3);
b.set(17);
b.reset(3);
b.flip(40);
b.test(17);  // true
b.count();   // 2
b.any();     // true
b.none();    // false
b.all();     // false
b.set();     // every position
b.flip();    // complement
```

</td><td>

```cpp
#include <xstd/bits/bit_set/bit_fixed_set.hpp>

auto s = xstd::bit_fixed_set<64>();
s.insert(3);
s.insert(17);
s.erase(3);
if (s.contains(40)) { s.erase(40); } else { s.insert(40); }
s.contains(17);            // true
s.size();                  // 2
not s.empty();             // true
s.empty();                 // false
s.size() == s.max_size();  // false
s = ~decltype(s)();        // every position
s = ~s;                    // complement
```

</td></tr>
<tr><td>

```cpp
// libstdc++'s extension; elsewhere a loop over N
for (auto i = b._Find_first(); i < b.size(); i = b._Find_next(i)) {
        use(i);
}
```

</td><td>

```cpp
for (auto i : s) {
        use(i);
}
```

</td></tr>
<tr><td>

```cpp
auto u = b << 1;
auto both = b & other;
auto subset = (b & other) == b;
auto meets = (b & other).any();
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>

auto u = s << 1;
auto both = s & other;
auto subset = xstd::ranges::bit_includes(other, s);
auto meets = xstd::ranges::bit_intersects(s, other);
```

</td></tr>
</table>

### `std::bitset<N>` as a row of `bool`

A row combined whole is a `bit_fixed_valarray<N>`, `std::valarray<bool>` packed: `operator[]` stays, and every
operator works element by element over two rows of one size, or a row and a `bool`. `~` is logical not, as `!` is,
and the comparisons answer a row of bools, so `b == c` becomes `(v == w).min()`. The count and the queries are
`xstd::ranges::bit_count`, `bit_all_of`, `bit_any_of` and `bit_none_of`, or `valarray`'s own `sum`, `min` and `max`; a
rotation is `cshift`. `std::bitset`'s shifts stay, in its sense: `v << n` moves position `i` to `i + n`, which is
`valarray`'s `v.shift(-n)`. A row that is indexed and permuted rather than combined is a `bit_array<N>`, with
`xstd::ranges::bit_rotate` and `xstd::ranges::bit_reverse`.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
auto b = std::bitset<8>();
b[1] = true;
b.flip();
b.count();  // 7
b.all();    // false
b.any();    // true
b = (b << 1) | (b >> 7);  // rotate
b.to_string();  // "11111101", position 0 last
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>

auto v = xstd::bit_fixed_valarray<8>();
v[1] = true;
v = ~v;
xstd::ranges::bit_count(v, true);  // 7
v.min();             // false, or xstd::ranges::bit_all_of(v)
v.max();             // true, or xstd::ranges::bit_any_of(v)
v = v.cshift(-1);    // position i to i + 1
std::format("{}", v);  // "[true, ...]", position 0 first
```

</td></tr>
<tr><td>

```cpp
auto r = (b & c) | (b ^ c);
auto n = ~b;
auto e = b == c;
b <<= 2;  // position i to i + 2
b >>= 1;  // position i to i - 1
```

</td><td>

```cpp
auto r = (v & w) | (v ^ w);
auto n = ~v;
auto e = (v == w).min();
v <<= 2;  // or v = v.shift(-2)
v >>= 1;  // or v = v.shift(1)
```

</td></tr>
</table>

### `std::bitset<N>` as a word

`to_ullong` and the `unsigned long long` constructor are the one-word case of `xstd::bit_convert` and the
`from_blocks` tag. A fixed width converts into any at least as wide, zero-extended, so `N` need not be the word's
width, and into a narrower one it does not compile.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
auto b = std::bitset<64>(0xF0);
auto w = b.to_ullong();  // 0xF0
```

</td><td>

```cpp
#include <xstd/bits/bit.hpp>

auto s = xstd::bit_fixed_set<64>(xstd::from_blocks, std::uint64_t{0xF0});
auto w = xstd::bit_convert<std::uint64_t>(s);  // 0xF0
```

</td></tr>
<tr><td>

```cpp
auto b = std::bitset<9>(0x1F0);
auto w = b.to_ulong();  // 0x1F0
```

</td><td>

```cpp
auto s = xstd::bit_fixed_set<9>{4, 5, 6, 7, 8};
auto w = xstd::bit_convert<unsigned long>(s);  // 0x1F0
```

</td></tr>
<tr><td>

```cpp
// an API that takes or returns std::bitset<64>
void legacy(std::bitset<64>);
std::bitset<64> legacy_result();
```

</td><td>

```cpp
legacy(xstd::bit_convert<std::bitset<64>>(s));
auto t = xstd::bit_convert<xstd::bit_fixed_set<64>>(legacy_result());
auto a = xstd::bit_convert<xstd::bit_array<64>>(legacy_result());
```

</td></tr>
</table>

### `boost::dynamic_bitset`

As a set, the run-time width grows to hold a key, where `dynamic_bitset` asks for a `resize` first and `set(i)` past
the size is out of range. As a sequence it is `std::vector<bool>`'s interface, and its bitwise operators are
`bit_valarray`'s, one `xstd::bit_convert` away. The opt-in header converts between the two libraries, so a boundary
can stay Boost's while the code behind it moves.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <boost/dynamic_bitset.hpp>

auto b = boost::dynamic_bitset<>(100);
b.set(3);
b.set(70);
for (auto i = b.find_first(); i != b.npos; i = b.find_next(i)) {
        use(i);
}
b.is_subset_of(c);
b.intersects(c);
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit_set/bit_set.hpp>

auto s = xstd::bit_set{3, 70};
s.insert(500);  // grows
for (auto i : s) {
        use(i);
}
xstd::ranges::bit_includes(c, s);
xstd::ranges::bit_intersects(s, c);
```

</td></tr>
<tr><td>

```cpp
auto b = boost::dynamic_bitset<>();
b.push_back(true);
b.resize(10);
b.flip();
b.count();
b &= c;
b ^= c;
auto n = ~b;
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit.hpp>
#include <xstd/bits/bit_valarray/bit_valarray.hpp>
#include <xstd/bits/bit_vector.hpp>

auto v = xstd::bit_vector();
v.push_back(true);
v.resize(10);
auto a = xstd::bit_convert<xstd::bit_valarray>(v);
a = ~a;
xstd::ranges::bit_count(a, true);
a &= c;
a ^= c;
auto n = ~a;
```

</td></tr>
<tr><td>

```cpp
void legacy(boost::dynamic_bitset<> const&);
```

</td><td>

```cpp
#include <xstd/bits/ext/boost/dynamic_bitset.hpp>

legacy(xstd::bit_convert<boost::dynamic_bitset<>>(v));
auto t = xstd::bit_convert<xstd::bit_set>(boost_result);
```

</td></tr>
</table>

### itsy_bitsy

`bitsy::bit_sequence<Container>` is the sequence reading over a container of words; here the storage is in the type's
name rather than its argument. `bitsy::bit_view<Range>` is `bit_span`, and the set reading has no itsy_bitsy
counterpart. The left column follows itsy_bitsy's README: its last release, of August 2022, no longer builds with
GCC 15 or Clang 22.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <itsy/bitsy.hpp>

bitsy::bit_sequence<std::vector<std::size_t>> bits{false, true, true, false, false};
bits.push_back(false);
bits.insert(bits.begin() + 2, {true, true});
bits.popcount();
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>
#include <xstd/bits/bit_vector.hpp>

auto bits = xstd::bit_vector{false, true, true, false, false};
bits.push_back(false);
bits.insert(bits.begin() + 2, {true, true});
xstd::ranges::bit_count(bits, true);
```

</td></tr>
<tr><td>

```cpp
std::vector<std::uint32_t> words{0xff00ff00, 0xff00ff00};
bitsy::bit_sequence<std::vector<std::uint32_t>> bits(std::in_place, std::move(words));
```

</td><td>

```cpp
auto bits = xstd::basic_bit_vector(xstd::from_blocks, std::vector<std::uint32_t>{0xff00ff00, 0xff00ff00});
```

</td></tr>
<tr><td>

```cpp
std::uint16_t storage[2]{};
bitsy::bit_view<std::span<std::uint16_t>> view(storage);
view[17] = true;
```

</td><td>

```cpp
#include <xstd/bits/bit_span.hpp>

std::uint16_t storage[2]{};
auto view = xstd::bit_span(storage);
view[17] = true;
```

</td></tr>
</table>

## A space-optimized container

`std::array<bool, N>`, `std::inplace_vector<bool, N>` and `std::valarray<bool>` spend a byte per position and
`std::set<std::size_t>` a node per key; `bit_array<N>`, `bit_inplace_vector<N>`, `bit_valarray` and `bit_set` spend a
bit. `std::vector<bool>` is packed already, and `bit_vector` is that packing under a vector's name. None of these
needs a section: the type changes and the code around it stays, as each answers its counterpart's synopsis
([README](../README.md#which-one-do-i-want)), with these exceptions:

- `[vector.bool]`'s `flip()` and deprecated static `swap` are left out, and so is `array`'s and `inplace_vector`'s
  `data()`, which a packed bit has no address for.
- `bit_valarray` has no arithmetic, so `+`, `*` and the rest stop compiling. Two operators compile and change
  meaning: `~` is logical not, where `valarray<bool>`'s promotes and answers all `true`, and `<<` and `>>` move
  positions as `std::bitset`'s do, where `valarray<bool>`'s shift each element's value.

## A speed-optimized algorithm

Packing pays in time only where the algorithms are adapted to it. Howard Hinnant measured as much in
[On `vector<bool>`](https://howardhinnant.github.io/onvectorbool.html) (2012): over 100,000 positions, a
word-at-a-time `find`, `count`, `fill`, `copy`, `swap_ranges`, `rotate` and `equal` ran from 1.7 times (`rotate`) to
about 75 times (`find`) as fast as over an array of `bool`, and the same algorithms a bit at a time up to 38 times as slow
(`fill`). `xstd::ranges` adapts each of them.

On a packed sequence a `std::ranges` algorithm still compiles, and visits one proxy at a time. Its counterpart in
`xstd::ranges` carries a `bit_` prefix and works a block at a time, in O(`N`/`D`) for `D` bits per block, over any
sequence of the library's, a `bit_span` and its windows included. Over `bool` a predicate is the identity, so
`bit_all_of`, `bit_any_of` and `bit_none_of` drop it, and `bit_count` and `bit_find` take the value they look for.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
#include <algorithm>
#include <functional>

auto v = xstd::bit_vector(n);
std::ranges::count(v, true);
std::ranges::find(v, true);
std::ranges::all_of(v, std::identity());
std::ranges::equal(v, w);
std::ranges::reverse(v);
std::ranges::sort(v);
```

</td><td>

```cpp
#include <xstd/bits/bit_algorithm.hpp>

auto v = xstd::bit_vector(n);
xstd::ranges::bit_count(v, true);
xstd::ranges::bit_find(v, true);
xstd::ranges::bit_all_of(v);
xstd::ranges::bit_equal(v, w);
xstd::ranges::bit_reverse(v);
xstd::ranges::bit_sort(v);
```

</td></tr>
</table>
