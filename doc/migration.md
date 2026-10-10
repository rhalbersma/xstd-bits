# Migrating to xstd-bits

Two migrations need no page. `std::vector<bool>` becomes `xstd::bit_vector` and `std::set<std::size_t>` becomes
`xstd::bit_set` by changing the type: each answers its counterpart's synopsis line for line, so the code around it
stays as it is ([README](../README.md#which-one-do-i-want)).

This page is for the sources whose interface differs: `std::bitset`, integer flag words, enumeration flags and Qt's
`QFlags`, `boost::dynamic_bitset` and itsy_bitsy. Each section is a *Tony Table*, the before-and-after form WG21
papers use: what the code says today on the left, what it says here on the right. Every right-hand column compiles
and runs against the library with GCC 15 and Clang 22, and so does every left-hand one except itsy_bitsy's, whose
last release no longer builds with either.

The first question is always which reading the old code meant. `std::bitset` and `boost::dynamic_bitset` offer
both at once; here the type says which:

- **a set of positions**: membership, flags, iterating over what is set. That is `bit_fixed_set<N>`,
  `bit_bounded_set<N>`, `bit_set`, or `bit_flag_set<Mask, N>` for a bitmask type.
- **a sequence of `bool`**: a value at each index, rotating, a row of cells. That is `bit_array<N>`,
  `bit_bounded_vector<N>` or `bit_vector`.

## `std::bitset<N>` as a set of positions

The positions become keys, and `std::bitset`'s verbs become `std::set`'s.

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
#include <xstd/bits/bit_fixed_set.hpp>

auto s = xstd::bit_fixed_set<64>();
s.insert(3);
s.insert(17);
s.erase(3);
s.complement(40);
s.contains(17);  // true
s.size();        // 2
not s.empty();   // true
s.empty();       // false
s.full();        // false
s.fill();        // every position
s.complement();  // complement
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
auto u = s << 1;
auto both = s & other;
auto subset = s.is_subset_of(other);
auto meets = intersects(s, other);
```

</td></tr>
</table>

## `std::bitset<N>` as a row of `bool`

`operator[]` and the whole-row operations stay; the count and the queries keep their names, and rotation is a
member rather than two shifts.

<table>
<tr><th>Before</th><th>After</th></tr>
<tr><td>

```cpp
auto b = std::bitset<8>();
b[1] = true;
b.flip();
b.count();  // 7
b.all();    // false
b = (b << 1) | (b >> 7);  // rotate
b.to_string();  // "11111101", position 0 last
```

</td><td>

```cpp
#include <xstd/bits/bit_array.hpp>

auto a = xstd::bit_array<8>();
a[1] = true;
a.flip();
a.count();  // 7
a.all();    // false
a.rotate(1);
std::format("{}", a);  // "[true, ...]", position 0 first
```

</td></tr>
</table>

## `std::bitset<N>` as a word

`to_ullong` and the `unsigned long long` constructor are the one-word case of `xstd::bit_convert` and the
`from_blocks` tag. A fixed width converts only into an equal one, so `N` is the word's width; `bit_align<bit_least<X>>`
rounds any `N` up to one.

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

## An integer flag word

Owned, the word becomes a fixed set. Kept where it is, in a struct or an ABI, a view gives it the set's interface
in place.

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
#include <xstd/bits/bit_set_view.hpp>

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

## Enumeration flags: `std::filesystem::perms` and other [bitmask.types]

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
#include <xstd/bits/bit_flag_set.hpp>

using perms = xstd::bit_flag_set<fs::perms, 16>;

auto p = perms{fs::perms::owner_read, fs::perms::owner_write};
p |= fs::perms::group_read;
p.contains(fs::perms::owner_write);
p.is_superset_of(mask);
fs::permissions(path, p);  // converts to fs::perms
```

</td></tr>
</table>

## Qt's `QFlags`

`QFlags<Enum>` wraps an enumeration of one-bit values; the set over the same enumeration replaces it, keyed on the
flags themselves through `bit_flag_mapping`.

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
a.toInt();
```

</td><td>

```cpp
#include <xstd/bits/bit_fixed_set.hpp>
#include <xstd/bits/bit_flag_mapping.hpp>

using Alignment = xstd::basic_bit_fixed_set<Qt::AlignmentFlag, std::uint16_t, 16, xstd::bit_flag_mapping<Qt::AlignmentFlag, 16>, std::greater<Qt::AlignmentFlag>>;

auto a = Alignment{Qt::AlignLeft, Qt::AlignTop};
a |= Qt::AlignBottom;
a -= Qt::AlignTop;
a.erase(Qt::AlignLeft);
a.insert(Qt::AlignHCenter);
a.contains(Qt::AlignHCenter);
xstd::bit_convert<std::uint16_t>(a);
```

</td></tr>
</table>

## `boost::dynamic_bitset`

As a set, the run-time width grows to hold a key, where `dynamic_bitset` asks for a `resize` first and `set(i)` past
the size is out of range. As a sequence it is `std::vector<bool>`'s interface. The opt-in header converts between
the two libraries, so a boundary can stay Boost's while the code behind it moves.

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
#include <xstd/bits/bit_set.hpp>

auto s = xstd::bit_set{3, 70};
s.insert(500);  // grows
for (auto i : s) {
        use(i);
}
s.is_subset_of(c);
intersects(s, c);
```

</td></tr>
<tr><td>

```cpp
auto b = boost::dynamic_bitset<>();
b.push_back(true);
b.resize(10);
b.flip();
b.count();
```

</td><td>

```cpp
#include <xstd/bits/bit_vector.hpp>

auto v = xstd::bit_vector();
v.push_back(true);
v.resize(10);
v.flip();
v.count();
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

## itsy_bitsy

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
#include <xstd/bits/bit_vector.hpp>

auto bits = xstd::bit_vector{false, true, true, false, false};
bits.push_back(false);
bits.insert(bits.begin() + 2, {true, true});
bits.count();
```

</td></tr>
<tr><td>

```cpp
std::vector<std::uint32_t> words{0xff00ff00, 0xff00ff00};
bitsy::bit_sequence<std::vector<std::uint32_t>> bits(std::in_place, std::move(words));
```

</td><td>

```cpp
auto bits = xstd::basic_bit_vector<std::uint32_t>(xstd::from_blocks, std::vector<std::uint32_t>{0xff00ff00, 0xff00ff00});
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
