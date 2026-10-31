# `<xstd/bits/bit/has_single_bit.hpp>`

## `xstd::has_single_bit`

```cpp
template<class T>
  requires see below
constexpr auto has_single_bit(const T& x) noexcept -> bool;
```

Whether exactly one position of `x` is set, as [[bit.pow.two]](https://eel.is/c++draft/bit.pow.two)'s
`has_single_bit(x)` asks whether an unsigned integer is a power of two, over any width. The header also brings
xstd-ints' overload over one unsigned integer.

### Template parameters

| Parameter | Meaning |
| :-------- | :------ |
| `T` | One of our owners, of any reading and any width: a set, a sequence or a valarray. |

### Parameters

| Parameter | Meaning |
| :-------- | :------ |
| `x` | the owner whose positions are tested |

### Return value

`true` if exactly one position of `x` is set, `false` if none or more than one is.

### Complexity

Linear in the blocks up to and including the first that holds a position, then in the blocks after it until one
holds another, and no more: at most every block once.

### Remarks

- *Constraints:* `T` is one of our owners.
- It reads the blocks, not the positions: the first block holding any is tested with `has_single_bit`, and every
  block after it must be nought.
- It is usable in a constant expression for a fixed width.

### Notes

- For a set it is `x.size() == 1`, which a set of stored count answers in constant time, and which otherwise
  counts every block; `has_single_bit` stops at the second block holding a position.
- `has_single_bit(bit_fixed_set<192>{100})` is `true`, and `has_single_bit(bit_fixed_set<192>{100, 150})` is `false`.

### See also

| Entity | Relation |
| :----- | :------- |
| [`std::has_single_bit`](https://eel.is/c++draft/bit.pow.two) | the draft's over one unsigned integer, whose question this asks of an owner |
