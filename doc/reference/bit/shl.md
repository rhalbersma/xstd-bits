# `<xstd/bits/bit/shl.hpp>`

This page also specifies `<xstd/bits/bit/shr.hpp>`, whose functions share this text.

## `xstd::shl`, `xstd::shr`

```cpp
// (1)
template<class X, integer S>
    requires see below
  constexpr auto shl(X x, S s) noexcept(see below) -> X;

// (2)
template<class X, integer S>
    requires see below
  constexpr auto shr(X x, S s) noexcept(see below) -> X;
```

Shift a set's keys by any count of any integer type, as [P3793R2](https://wg21.link/p3793r2)'s `shl` and `shr` shift
an integer's bits. They are the total forms beside the set's own `<<` and `>>`, which at a static width take a count
below the width as their precondition, as an integer's built-in shifts do.

1. `shl` moves every key `s` positions up.
2. `shr` moves every key `s` positions down.

### Parameters

| Parameter | Meaning |
| :-------- | :------ |
| `x` | the set to shift |
| `s` | the count to shift by, which may be negative |

*Constraints:* `X` is a set of this library that owns its keys, `basic_bit_fixed_set`, `basic_bit_inplace_set`,
`basic_bit_set` or a type built on them, and `S` models `integer`, signed or unsigned, of any width.

### Return value

1. *Returns:* `x` with each key at position `p` moved to position `p + s`, a key landing outside the positions the set
   can hold dropped.
2. *Returns:* `x` with each key at position `p` moved to position `p - s`, a key landing outside the positions the set
   can hold dropped.

### Exceptions

As the set's own `<<=` and `>>=`: `noexcept` at a static width and an inplace capacity, and otherwise whatever growing
the storage throws.

### Notes

- Every value of `S` is in the domain. A count whose magnitude is the width or more empties a set of static width; a
  set of run-time width grows towards `max_size()` and empties past it, as its `<<=` does.
- A negative count shifts the other way, so `shr(x, s)` equals `shl(x, -s)` wherever `-s` is representable.
- The count's type changes nothing but which counts it can hold.
- A call is a constant expression.

### See also

| Entity | Relation |
| :----- | :------- |
| [`basic_bit_set`](../bit_set/bit_set.md) | its `<<=` and `>>=`, which these call for a count inside the width |
