//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
#define XSTD_BITS_DETAIL_BIT_CASTABLE_HPP

#include <xstd/bits/detail/contiguous_block_range.hpp> // contiguous_block_range
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <bit>                                     // bit_cast, endian
#include <concepts>                                // convertible_to, default_initializable
#include <cstddef>                                 // byte, size_t, to_integer
#include <cstring>                                 // memcpy
#include <limits>                                  // numeric_limits
#include <memory>                                  // addressof
#include <ranges>                                  // contiguous_range, data, range_value_t
#include <type_traits>                             // bool_constant, is_trivially_copyable_v

namespace xstd::detail::bits {

inline constexpr auto bits_per_byte = static_cast<std::size_t>(std::numeric_limits<unsigned char>::digits);
inline constexpr auto bits_per_word = static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);

// The bytes a width needs, which is the currency two fields of bits are exchanged in: byte j holds the positions
// [8j, 8j + 8) least significant bit first, and that is true at every block width, which is what makes a byte the
// common ground where a word is not.
template<std::size_t N>
inline constexpr auto byte_count = (N + bits_per_byte - 1UZ) / bits_per_byte;

// A source of N bits comes in two families, and only one of them has anything to prove.
//
// The STATED family is its own layout, and it has two spellings of one idea. An UNSIGNED INTEGER states it alone:
// bit n of the value is 2^n, said by the language rather than by any implementation, so there is nothing here to
// assume and nothing to probe. That is the family unsigned long long belongs to, which is why to_ullong and the
// constructor taking one need no case of their own below -- they are this case. A CONTIGUOUS SEQUENCE OF BLOCKS
// states the rest of it: block j holds the positions [j*digits, (j+1)*digits), said by the sequence. Put together
// they give every position of the field without a byte of it being assumed, and a scalar is simply the sequence
// of length one -- which is why the two are one family and not two.
//
// Everything in this family is read by SHIFTS on values and never by bit_cast on an object, so no probe runs, no
// padding is reachable and endianness never enters: b[j] >> k is the same number on either byte order. Only the
// second family below, a foreign field of bits whose internals this library cannot name, has to be proved.
template<class B, std::size_t N>
concept integer_source =
        xstd::unsigned_integer<B> and
        N <= static_cast<std::size_t>(xstd::numeric_limits<B>::digits)
;

// The same family said over a sequence. contiguous_block_range already carries what this needs -- contiguous,
// sized, and a value type that is an unsigned integer -- so the only thing added here is the width.
//
// A STATIC width, and that is what keeps the promise the readings make. The size has to be a constant expression
// for "this covers N positions" to be a constraint rather than a run-time check, and asking B().size() is how:
// a std::array<Block, M> answers M, and a std::vector answers zero, which is the honest answer for a container
// that has no bits until one is put in it. So an array converts and a vector does not, on a width and not on a
// preference.
//
// AT LEAST N, not exactly N, which is the rule the scalar spelling already follows: a bit_static_set<32> reads
// the low thirty-two bits of an unsigned long long and a wider block sequence is no different. The tail above N
// is written clear on the way out, so set -> blocks -> set is the identity; blocks -> set -> blocks is not, and
// is not meant to be, exactly as it already is for an integer too wide for the width.
template<class B>
inline constexpr auto block_digits = static_cast<std::size_t>(
        xstd::numeric_limits<std::ranges::range_value_t<B>>::digits
);

template<class B>
concept block_size_is_constant = requires {
        typename std::bool_constant<(B().size(), true)>;
};

template<class B, std::size_t N>
concept block_range_source =
        // CONTIGUOUS FIRST, and the order is load-bearing rather than tidy. contiguous_block_range opens with
        // std::regular, which asks constructible_from, which re-enters the very constructor whose constraint this
        // is -- a concept that depends on itself, and GCC says exactly that. An adaptor's iterator is a proxy and
        // so is never contiguous, so asking that first answers false for every reading here before the recursive
        // question is ever put. A std::array reaches the rest of it unharmed.
        std::ranges::contiguous_range<B> and
        contiguous_block_range<B> and
        std::default_initializable<B> and
        block_size_is_constant<B> and
        B().size() * block_digits<B> >= N
;

// WHEN A COPY ANSWERS WHAT THE SHIFTS DO. This is the question the container already asks one layer down of its
// own blocks, asked here of the SOURCE instead, and the answer has the same two parts. On a little-endian target
// byte j of a value sits at offset j, so the bytes of a value are the bytes of the field. And the object must
// have no padding, because the shifts count by DIGITS where a copy counts by SIZEOF: those agree only when every
// bit of the object is a value bit. No standard type has such padding, and the test is here so that one could not
// quietly turn a copy into the wrong answer.
template<class B>
inline constexpr auto value_bits_fill_object =
        static_cast<std::size_t>(xstd::numeric_limits<B>::digits) == bits_per_byte * sizeof(B);

template<class B>
inline constexpr auto blocks_copy_as_bytes =
        std::endian::native == std::endian::little and value_bits_fill_object<std::ranges::range_value_t<B>>;

template<class B>
[[nodiscard]] constexpr auto object_bytes(B const& b) noexcept
        -> std::array<std::byte, sizeof(B)>
{
        return std::bit_cast<std::array<std::byte, sizeof(B)>>(b);
}

// Whether bit_cast of a value-initialised B is a CONSTANT EXPRESSION, asked so that it answers false instead of
// erroring. It is what rules out a pointer member, a reference member and a union, which are the shapes bit_cast
// refuses to be constexpr for -- and it replaces has_unique_object_representations_v, which cannot be used here:
// GCC 13 through 16 answer false for ANY class with an empty non-static data member, even one [[no_unique_address]]
// costs nothing, where clang answers true. Every container this library defines has such a member, so that trait
// would make this concept false on one compiler and true on the other. Measured on both, at identical layout.
template<class B>
concept bit_cast_is_constant = requires {
        typename std::bool_constant<(object_bytes(B()), true)>;
};

// What the probe below has to be able to ask of a candidate: build an empty one, light one position, count them.
template<class B>
concept probeable_bits =
        std::default_initializable<B> and
        requires (B& b, B const& c, std::size_t n) {
                b.set(n);
                { c.count() } -> std::convertible_to<std::size_t>;
                { c.size()  } -> std::convertible_to<std::size_t>;
        }
;

// One position lit and counted, asked only for whether it is a CONSTANT EXPRESSION. bit_cast_is_constant above
// covers what bit_cast refuses; this covers what the probe itself does, and the two failures are unrelated. A
// block type whose operator|= is not constexpr makes set() unusable in a constant expression while leaving it
// perfectly well-formed -- absl::uint128 is exactly that -- so probeable_bits is satisfied, the probe then runs at
// compile time, and the result is a hard error in the middle of a constraint rather than an unsatisfied concept.
// That is the same shape B().size() == N prevents for a set() that throws, and it needs its own guard because a
// non-constant set() is not an out-of-range one.
//
// ONE position and not five, because this is a gate and not the proof: the probe below is the proof, and paying
// for it twice would halve the width its step budget reaches. count() rides along because it is the other call the
// probe makes on a candidate, and it is as free here as set() is.
//
// NOT noexcept, for the same reason bit_layout_holds is not: a bitset reading's set(pos) throws out_of_range for a
// position it does not have, and a throw out of a noexcept function is a terminate rather than a diagnostic.
template<class B>
[[nodiscard]] constexpr auto probe_once()
        -> bool
{
        auto b = B();
        b.set(0UZ);
        return static_cast<std::size_t>(b.count()) == 1UZ;
}

template<class B>
concept probe_is_constant = requires {
        typename std::bool_constant<(probe_once<B>(), true)>;
};

// FIVE positions, and the count is the budget rather than the taste. Each probe materialises the whole byte array
// through bit_cast, so it costs O(sizeof(B)) however few bytes it then reads, and clang's default
// -fconstexpr-steps admits six of them at a width of 2^20 and refuses seven; GCC's limit is higher. Five leaves
// margin and puts the ceiling at 2^20 -- a 128 KiB object -- past which a caller raises -fconstexpr-steps.
//
// count() == 1 is what makes each probe cheap AND total: with no padding reachable, one position set and the
// expected byte holding exactly its bit, no OTHER byte can hold anything. So this refuses a reordered word, a
// reversed bit order, a tail the implementation does not keep clean, and -- bit_cast handing back the object
// representation rather than the value -- a big-endian target, where bit n of a word lands at the far end of it.
// The endianness guard IS this probe, and there is no second one to fall out of step with it.
//
// NOT noexcept, and deliberately so: a bitset reading's set(pos) throws out_of_range for a position it does not
// have. The loop below never asks for one -- it skips i >= N, and the concept below settles the width before it
// gets here -- but that is reasoning a call graph cannot follow, and a throw out of a noexcept function is a
// terminate rather than a diagnostic. There is nothing to buy back either: every call to this is a constant
// evaluation, and a throw there is already a hard error.
template<class B, std::size_t N>
[[nodiscard]] constexpr auto bit_layout_holds()
        -> bool
{
        for (auto const byte : object_bytes(B())) {
                if (byte != std::byte{}) {
                        return false;
                }
        }
        for (auto const i : { 0UZ, 7UZ, bits_per_byte, bits_per_word, N - 1UZ }) {
                if (i >= N) {
                        continue;
                }
                auto b = B();
                b.set(i);
                if (static_cast<std::size_t>(b.count()) != 1UZ) {
                        return false;
                }
                if (object_bytes(b)[i / bits_per_byte] != static_cast<std::byte>(1U << (i % bits_per_byte))) {
                        return false;
                }
        }
        return true;
}

// The second family: a field of bits whose layout is proved rather than believed. The size window bounds the
// object at its own positions plus at most one spare word, which catches a layout carrying a slot the five fixed
// positions never reach. Order is load-bearing throughout -- atomic constraints are checked left to right, so
// the cheap structural questions come before the probe, and bit_cast_is_constant comes before the probe that
// uses it.
template<class B, std::size_t N>
concept container_source =
        probeable_bits<B> and
        std::is_trivially_copyable_v<B> and
        // ITS width, not merely one that fits. Without this the size window below admits a neighbour -- a
        // std::bitset<9> is eight bytes, so it clears every bound a width of eight sets -- and eight of its nine
        // positions would convert while the ninth vanished. Nothing truncates is a property, so it is a constraint.
        // It also comes BEFORE the probe, which is what keeps the probe from asking a narrower source for a
        // position it does not have: std::bitset<8>::set(8) throws, and a throw is no constant expression, so that
        // would be a hard error where this is an unsatisfied concept.
        B().size() == N and
        sizeof(B) * bits_per_byte >= N and
        sizeof(B) * bits_per_byte <  N + bits_per_word and
        // With no byte to exchange there is nothing to prove, and asking anyway refuses the one width where the
        // question is empty: std::bitset<0> occupies a byte that represents no position, so a bit_cast of it reads
        // an uninitialised one and is no constant expression. The guard is byte_count and not N, because they are
        // zero together and byte_count is what the two functions below actually range over.
        (byte_count<N> == 0UZ or (bit_cast_is_constant<B> and probe_is_constant<B> and bit_layout_holds<B, N>()))
;

template<class B, std::size_t N>
concept bit_castable = integer_source<B, N> or block_range_source<B, N> or container_source<B, N>;

// The shifts, in one place because each direction needs them and a copy cannot take their two cases: a constant
// expression, where memcpy does not exist, and a big-endian target, where those bytes are not this value. They SAY
// where a position goes rather than assuming a byte order, which is what makes them the portable answer and the
// one the copy above has to agree with.
// The scalar pair above, once per block: byte j of the field is byte j % sizeof(block) of block j / sizeof(block).
template<std::size_t N, class B, std::size_t E>
constexpr auto block_bytes_by_shifts(B const& b, std::array<std::byte, E>& bytes) noexcept
        -> void
{
        constexpr auto bytes_per_block = block_digits<B> / bits_per_byte;
        for (auto j = 0UZ; j < bytes.size(); ++j) {
                auto const block = b[j / bytes_per_block];
                auto const shift = bits_per_byte * (j % bytes_per_block);
                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(block >> shift));
        }
}

template<std::size_t N, class B, std::size_t E>
constexpr auto bytes_blocks_by_shifts(std::array<std::byte, E> const& bytes, B& blocks) noexcept
        -> void
{
        using block_type = std::ranges::range_value_t<B>;
        constexpr auto bytes_per_block = block_digits<B> / bits_per_byte;
        for (auto j = 0UZ; j < bytes.size(); ++j) {
                auto const byte  = static_cast<block_type>(std::to_integer<unsigned char>(bytes[j]));
                auto const shift = bits_per_byte * (j % bytes_per_block);
                auto& block = blocks[j / bytes_per_block];
                block = static_cast<block_type>(block | static_cast<block_type>(byte << shift));
        }
}

template<std::size_t N, class B>
        requires bit_castable<B, N>
[[nodiscard]] constexpr auto bit_bytes(B const& b) noexcept
        -> std::array<std::byte, byte_count<N>>
{
        auto bytes = std::array<std::byte, byte_count<N>>();
        if constexpr (byte_count<N> > 0UZ) {
                if constexpr (integer_source<B, N>) {
                        // NO COPY HERE, and that is measured rather than assumed: a value is at most a handful of
                        // bytes, the loop unrolls, and memcpy timed identically at every width (0.31ns either way
                        // on uint8, uint32 and uint64). A branch that buys nothing is worse than no branch.
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(b >> (bits_per_byte * j)));
                        }
                } else if constexpr (block_range_source<B, N>) {
                        if !consteval {
                                if constexpr (blocks_copy_as_bytes<B>) {
                                        std::memcpy(bytes.data(), std::ranges::data(b), bytes.size());
                                        return bytes;
                                }
                        }
                        block_bytes_by_shifts<N>(b, bytes);
                } else {
                        // A field of bits is TRIVIALLY COPYABLE -- container_source says so -- so at run time its
                        // object representation can be read straight into these bytes. The bit_cast is what a
                        // constant expression needs, and it costs a whole second copy of the object, which is the
                        // one this branch was paying twice over.
                        if !consteval {
                                std::memcpy(bytes.data(), std::addressof(b), bytes.size());
                                return bytes;
                        }
                        auto const object = object_bytes(b);
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = object[j];
                        }
                }
        }
        return bytes;
}

template<class B, std::size_t N>
        requires bit_castable<B, N>
[[nodiscard]] constexpr auto bytes_bits(std::array<std::byte, byte_count<N>> const& bytes) noexcept
        -> B
{
        if constexpr (byte_count<N> == 0UZ) {
                return B();
        } else if constexpr (integer_source<B, N>) {
                // The shifts alone, for the reason bit_bytes gives: a copy measured the same and said less.
                auto value = B();
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        auto const byte = static_cast<B>(std::to_integer<unsigned char>(bytes[j]));
                        value = static_cast<B>(value | static_cast<B>(byte << (bits_per_byte * j)));
                }
                return value;
        } else if constexpr (block_range_source<B, N>) {
                // Value-initialised first, so the blocks above N are CLEAR rather than whatever was there: that is
                // what makes set -> blocks -> set the identity at a width the sequence is wider than. The copy
                // below writes only the bytes the field has, so the same value-initialisation is what clears the
                // tail for it too.
                auto blocks = B();
                if !consteval {
                        if constexpr (blocks_copy_as_bytes<B>) {
                                std::memcpy(std::ranges::data(blocks), bytes.data(), bytes.size());
                                return blocks;
                        }
                }
                bytes_blocks_by_shifts<N>(bytes, blocks);
                return blocks;
        } else {
                auto object = std::array<std::byte, sizeof(B)>();
                if !consteval {
                        std::memcpy(object.data(), bytes.data(), bytes.size());
                        return std::bit_cast<B>(object);
                }
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        object[j] = bytes[j];
                }
                return std::bit_cast<B>(object);
        }
}

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
