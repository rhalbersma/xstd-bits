# `<xstd/bits/bit_array/tuple_element.hpp>`

## `tuple_size`, `tuple_element`, `get`

- As [[array.tuple]](https://eel.is/c++draft/array.tuple), with:
- `tuple_size<basic_bit_array<Block, N>>` is `integral_constant<size_t, N>`.
- `tuple_element<I, basic_bit_array<Block, N>>::type` is the proxy `reference`, and for a `const` array the proxy
  `const_reference`, so a structured binding over the array writes through to it. *Mandates:* `I < N`.
- `get<I>(a)`, for an lvalue, a `const` lvalue, an rvalue and a `const` rvalue, returns `a[I]`. *Constraints:*
  `I < N`. It is found by argument-dependent lookup, not as a member of namespace `xstd`.
