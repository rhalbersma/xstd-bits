# `erase`, `erase_if`

- As [[inplace.vector.erasure]](https://eel.is/c++draft/inplace.vector.erasure), with:
- *Constraints:* `c` is an owner of the sequence reading whose width grows, so `c.erase(c.cbegin(), c.cend())` is
  well-formed: `basic_bit_inplace_vector`, `basic_bit_vector` and `basic_bit_small_vector`.
- *Returns:* the number of elements removed.
- They are declared in a namespace of the library's adaptor and found by argument-dependent lookup, not as
  `xstd::erase` and `xstd::erase_if`.
- A `bit_vector` erases as a `vector<bool>` of the same bools does, counting what it removes alike.
