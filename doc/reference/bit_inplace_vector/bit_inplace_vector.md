# `<xstd/bits/bit_inplace_vector/bit_inplace_vector.hpp>`

## `basic_bit_inplace_vector`, `bit_inplace_vector`

```cpp
template<unsigned_integer Block, size_t N>
  requires see below
using basic_bit_inplace_vector = basic_bit_vector<see below, N>;

template<size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<size_t, N>;

// what basic_bit_vector has over a storage of constant capacity
// capacity
static constexpr integral_constant<size_t, N> max_size = {};
static constexpr integral_constant<size_t, N> capacity = {};
static constexpr auto reserve(size_type n) -> void;
static constexpr auto shrink_to_fit() noexcept -> void;
```

As [[inplace.vector]](https://eel.is/c++draft/inplace.vector) for `inplace_vector<bool, N>`, with:

- `bit_inplace_vector<N>` holds up to `N` bits in `size_t` blocks, and `basic_bit_inplace_vector<Block, N>` in
  `Block`s, a [`basic_bit_vector`](../bit_vector/bit_vector.md) over them capped at `N`. The requires-clause admits a
  cv-unqualified `Block` whose digits are a power of two and at least `unsigned char`'s. `N` is the capacity exactly,
  even where the last block has room for more.
- The storage is `inplace_vector<Block, K>` where the standard library has `<inplace_vector>`, and
  `boost::container::static_vector<Block, K>` elsewhere, for `K = ceil(N / D)`. At `K == 0`, where that type is not
  empty, it is an empty type of the library's own, so `bit_inplace_vector<0>` is an empty type, as
  [[inplace.vector.overview]](https://eel.is/c++draft/inplace.vector.overview) makes `inplace_vector<T, 0>`.
- An alias deduces nothing, so blocks adopted whole are deduced through `basic_bit_vector`:
  `basic_bit_vector(from_blocks, blocks)` over `K` inline blocks is `basic_bit_inplace_vector<Block, K * D>`.
- The reference, the iterator and the departures they force are `basic_bit_array`'s: no `data()`, `pointer` names
  `void`.
- *Capacity:* `capacity` and `max_size` are constants of the type, called as
  [[inplace.vector.capacity]](https://eel.is/c++draft/inplace.vector.capacity) calls them; `size()` and `empty()`
  are functions. `reserve(n)` throws `bad_alloc` where `n > N` and does nothing otherwise; `shrink_to_fit()` does
  nothing.
- *Modifiers:* As [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers): `push_back` and
  `emplace_back` return the proxy for the new element, `try_push_back` and `try_emplace_back` return
  `optional<reference>`, `nullopt` when full, and `unchecked_push_back` and `unchecked_emplace_back` have
  `size() < N` as their precondition. There is no `try_append_range`, which
  [P4022R0](https://wg21.link/P4022R0) takes out of C++26.
- Growth past `N` throws `bad_alloc` before anything is written. `append_range` from a range with no size undoes
  what it appended before the exception leaves, so it has no effects, as
  [[inplace.vector.modifiers]](https://eel.is/c++draft/inplace.vector.modifiers) asks.
- *`from_blocks` constructor:* *Effects:* Moves `blocks` in. Every bit is a position, so `size()` is
  `blocks.size() * D`. *Preconditions:* `blocks.size() * D <= N`.
- *`replace`:* *Effects:* Moves `blocks` in, `size()` becoming `blocks.size() * D`. *Preconditions:* as the
  constructor's.
- *`extract`:* *Returns:* the blocks, moved out, the unused bits of the last one clear. *Postconditions:*
  `size() == 0`. A width short of whole blocks comes back padded with `false`, which `resize` trims.
- `XSTD_BITS_HAS_CONSTEXPR_INPLACE` is defined exactly where the blocks are an `inplace_vector`, and there alone
  the owner is usable in a constant expression. Over `static_vector` its copies are not `noexcept`, as
  `static_vector`'s are not; its `swap` is `noexcept` over either.
- It is not a structural type at any width.
