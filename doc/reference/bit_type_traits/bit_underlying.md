# `<xstd/bits/bit_type_traits/bit_underlying.hpp>`

## `underlying_block_t`, `bit_underlying`

- `underlying_block_t<Key>`:
  - *Constraints:* `Key` is an enumeration, or models xstd-ints's `integer`; its underlying type, or `Key` itself,
    is an integral type; and xstd-ints's `make_unsigned_t` of that type models `unsigned_integer`.
  - *Result:* `make_unsigned_t<U>`, where `U` is the enumeration's underlying type, or `Key` itself: the block a
    field or an ABI of `Key` already uses.
  - An enumeration on `uint32_t` gives `uint32_t`, one on `int16_t` gives `uint16_t`, and one on `char` gives
    `unsigned char`. `underlying_block_t<int>` is `unsigned`, of `int8_t` is `uint8_t`, of `uint32_t` is `uint32_t`
    and of `int64_t` is `uint64_t`.
  - `bool`, `char`, `char8_t` and `wchar_t` have no such counterpart, nor has an enumeration on `bool`, and a class
    such as `bitset<8>` is turned away.
- `bit_underlying<Owner>`:
  - *Constraints:* `remove_cv_t<Owner>` is a rebindable owner with a `key_type` for which `underlying_block_t` names
    a type: a set keyed by an enumeration or an integer.
  - *Result:* `bit_rebind<underlying_block_t<typename Owner::key_type>, Owner>`, the width kept.
  - For a flag set at its mask's full width the result is the type itself, `bit_least` having chosen that word; for
    `bit_fast` of it, or a narrower `bit_flag_set<E, 9>`, it is the set in `E`'s own word.
    `bit_underlying<bit_flag_set<int, 5>>` is `basic_bit_fixed_set<int, unsigned, 5, bit_flag_mapping<int, 5>,
    greater<int>>`, in the word an `int` field stores.
  - The rule is by key type, not by mapping: `bit_underlying<bit_set>` is `bit_set`, and
    `bit_underlying<bit_least<bit_fixed_set<9>>>` is `bit_fixed_set<9>`.
  - The set's block is then the mask's representation bit for bit, so `bit_convert`, `from_blocks` and `bit_cast`
    to and from that word agree with `to_underlying` of the mask.
  - A sequence such as `bit_array<9>` has no key and is turned away, and so are `bit_flag_set<bitset<16>>` and a set
    keyed by an enumeration on `bool`.
