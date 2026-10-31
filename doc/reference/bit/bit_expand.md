# `<xstd/bits/bit/bit_expand.hpp>`

## `xstd::bit_expand`

```cpp
template<class T>
  requires see below
constexpr auto bit_expand(const T& x, const T& m) noexcept(see below) -> T;     // (1)
template<unsigned_integer U, class T>
  requires see below
constexpr auto bit_expand(U x, const T& m) noexcept(see below) -> T;            // (2)
```

Lays the low positions of `x` out at the positions `m` holds, in order, as
[[bit.permute]](https://eel.is/c++draft/bit.permute)'s `bit_expand(x, m)` lays an unsigned integer's bits out: PDEP,
over any width. The header also brings xstd-ints' overloads over one unsigned integer.

### Template parameters

| Parameter | Meaning |
| :-------- | :------ |
| `T` | One of our owners, of any reading and any width: a set, a sequence or a valarray. |
| `U` | (2) The unsigned integer the low positions are read from. |

### Parameters

| Parameter | Meaning |
| :-------- | :------ |
| `x` | the source, whose low positions are laid out |
| `m` | the mask, at whose positions they land |

### Return value

1. The `T` whose `k`-th position that `m` holds is set exactly where `x` holds position `k`, and whose positions `m`
   does not hold are clear. Its width is `m`'s, as for `bit_compress`.
2. The same, with bit `k` of `x` as position `k`, and the positions past `U`'s digits clear.

### Exceptions

Only what constructing a `T` of run-time width, or copying a set of run-time width narrower than `m`, throws. Both
are `noexcept` for a fixed width.

### Remarks

- *Constraints:* `T` is one of our owners, neither `const` nor `volatile`; for (2), `U` is an unsigned integer.
- *Preconditions:* For a sequence or a valarray of run-time size, `x.size() == m.size()`, checked by an `assert`.
- A set of run-time width narrower than `m` reads the keys past its width as absent.
- `bit_compress(bit_expand(x, m), m)` holds the positions of `x` below `p`, and `bit_expand(bit_compress(x, m), m)`
  holds those of `x` that `m` holds, `p` being the positions `m` holds.
- It is usable in a constant expression for a fixed width.

### Notes

- A magic or PEXT table is filled by walking the subsets of a mask:
  `for (auto i = 0; i < 1 << mask.size(); ++i) { auto const occupancy = xstd::bit_expand(std::uint64_t(i), mask); }`.
- `bit_expand(std::uint64_t{0b101}, bit_fixed_set<192>{100, 120, 150})` is `bit_fixed_set<192>{100, 150}`.
- The word operation first came before WG21 as [N3864](https://wg21.link/n3864)'s `deposit_bits_right` in 2014, which
  its instruction table maps to PDEP, and the working draft has it from [P3104](https://wg21.link/P3104).

### See also

| Entity | Relation |
| :----- | :------- |
| [`bit_compress`](bit_compress.md) | the inverse: the positions a mask holds, packed |
| [`std::bit_expand`](https://eel.is/c++draft/bit.permute) | the draft's over one unsigned integer, whose shape this generalizes |
