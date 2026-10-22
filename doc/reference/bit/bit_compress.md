# `<xstd/bits/bit/bit_compress.hpp>`

## `xstd::bit_compress`

```cpp
template<class T>
  requires see below
constexpr auto bit_compress(const T& x, const T& m) noexcept(see below) -> T;   // (1)
template<unsigned_integer U, class T>
  requires see below
constexpr auto bit_compress(const T& x, const T& m) noexcept -> U;            // (2)
```

Packs the positions of `x` that `m` holds, in order, from position zero, as
[[bit.permute]](https://eel.is/c++draft/bit.permute)'s `bit_compress(x, m)` packs an unsigned integer's bits: PEXT,
over any width. The header also brings xstd-ints' overloads over one unsigned integer.

### Template parameters

| Parameter | Meaning |
| :-------- | :------ |
| `T` | One of our owners, of any reading and any width: a set, a sequence or a valarray. Deduced from both arguments, which have this one type. |
| `U` | (2) The unsigned integer the packed positions are read into. |

### Parameters

| Parameter | Meaning |
| :-------- | :------ |
| `x` | the source |
| `m` | the mask, whose positions select from `x` |

### Return value

1. The `T` whose position `k` is set exactly where `x` holds the `k`-th position `m` holds, counting from zero, and
   whose positions from `p` on are clear, `p` being the positions `m` holds: `m.size()` for a set, and
   `xstd::ranges::bit_count(m, true)` for a sequence or a valarray. A set's result holds the ranks, among `m`'s keys,
   of the keys `x` holds there. Its width is `m`'s: the type's for a fixed width, `m.size()` for a sequence or a
   valarray of run-time size, and `m`'s for a set of run-time width.
2. The same positions as the bits of a `U`, bit `k` being position `k`.

### Exceptions

(1) throws only what constructing a `T` of run-time width throws, and is `noexcept` for a fixed width; (2) throws
nothing.

### Remarks

- *Constraints:* `T` is one of our owners, neither `const` nor `volatile`. A view, `bitset` and two owners of
  different types are turned away.
- *Preconditions:* For a sequence or a valarray of run-time size, `x.size() == m.size()`, as a valarray mask's size
  is the array's. For (2), `p <= numeric_limits<U>::digits`. Each is checked by an `assert`.
- A set of run-time width reads the keys past its width as absent, whichever of `x` and `m` is the wider.
- It runs a block at a time: each block of `m` selects from the block of `x` beside it through xstd-ints'
  `bit_compress`, PEXT where the target has BMI2 fast, and the result lands directly above what the blocks below it
  selected. (2) packs into a register and builds no owner: two `bit_fixed_set<64>` compile to one `pext`, and two
  `bit_fixed_set<192>` to three.
- It is usable in a constant expression for a fixed width.

### Notes

- A rook's attack-table index on a board of 192 squares is
  `table[sq][xstd::bit_compress<std::uint64_t>(occupied, rook_mask[sq])]`, `occupied` and `rook_mask[sq]` each a
  `bit_fixed_set<192>`.
- `bit_compress(bit_fixed_set<192>{3, 70, 130, 191}, bit_fixed_set<192>{3, 4, 70, 71, 130, 190, 191})` is
  `bit_fixed_set<192>{0, 2, 4, 6}`, and `bit_compress<uint64_t>` of the same is `0b1010101`.
- `bit_compress(bit_set{1, 500}, bit_set{1, 2, 3, 500, 900})` is `bit_set{0, 3}`.
- For a valarray `v` and a mask `m` of its size, `bit_compress(v, m)` packs what `std::as_const(v)[m]` gathers, at
  `v`'s width rather than at `p`.

### See also

| Entity | Relation |
| :----- | :------- |
| [`bit_expand`](bit_expand.md) | the inverse: the low positions laid out at a mask's positions |
| [`bit_convert`](bit_convert.md) | a source's positions at another width, where `bit_compress` selects them |
| [`std::bit_compress`](https://eel.is/c++draft/bit.permute) | the draft's over one unsigned integer, whose shape this generalizes |
