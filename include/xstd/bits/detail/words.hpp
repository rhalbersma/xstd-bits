//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_WORDS_HPP
#define XSTD_BITS_DETAIL_WORDS_HPP

#include <xstd/bits/detail/borrowed_bits.hpp>            // borrowable_word, borrowable_words, borrowed_bits, words_span_t
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, default_extent_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <span>                                          // span
#include <type_traits>                                   // conditional_t, is_const_v, remove_const_t, remove_reference_t

// What the public containers and views are named by: the words, never the storage built over them.
namespace xstd::bits::detail {

// The width the words name by their type: a word's digits, or the width the block storage names.
template<class W>
inline constexpr auto words_extent_v = default_extent_v<std::remove_const_t<W>>;

template<class W>
        requires xstd::unsigned_integer<std::remove_const_t<W>>
inline constexpr auto words_extent_v<W> = static_cast<std::size_t>(xstd::numeric_limits<std::remove_const_t<W>>::digits);

// An owner's storage: one word is held as an array of one, so the storage only ever sees a range.
template<class Blocks, std::size_t N>
struct owner_storage
{
        using type = contiguous_bit_container<Blocks, N>;
};

template<xstd::unsigned_integer Block, std::size_t N>
struct owner_storage<Block, N>
{
        using type = contiguous_bit_container<std::array<Block, 1>, N>;
};

template<class Blocks, std::size_t N>
using owner_storage_t = owner_storage<Blocks, N>::type;

template<class T, class Bits>
using const_as_t = std::conditional_t<std::is_const_v<T>, Bits const, Bits>;

// A view's storage: someone else's words by span, or the storage of an owner over the same words it refers into.
template<class Words, std::size_t N>
struct view_storage_for
{
        using type = const_as_t<Words, contiguous_bit_container<std::remove_const_t<Words>, N>>;
};

template<class Word, std::size_t N>
        requires xstd::unsigned_integer<std::remove_const_t<Word>>
struct view_storage_for<Word, N>
{
        using type = const_as_t<Word, borrowed_bits<std::remove_const_t<Word>, 1>>;
};

template<class Block, std::size_t E, std::size_t N>
struct view_storage_for<std::span<Block, E>, N>
{
        using type = const_as_t<Block, borrowed_bits<std::remove_const_t<Block>, E>>;
};

template<class Words, std::size_t N>
using view_storage_t = view_storage_for<Words, N>::type;

// The words and width a storage is built over, const where it is: how a guide names a view over existing storage.
template<class Bits>
struct words_of;

template<class Blocks, std::size_t N>
struct words_of<contiguous_bit_container<Blocks, N>>
{
        using type = Blocks;
        static constexpr std::size_t width = N;
};

template<class Blocks, std::size_t N>
struct words_of<contiguous_bit_container<Blocks, N> const>
{
        using type = Blocks const;
        static constexpr std::size_t width = N;
};

template<class Block, std::size_t E>
struct words_of<contiguous_bit_container<std::span<Block, E>> const>
{
        using type = std::span<Block const, E>;
        static constexpr std::size_t width = default_extent_v<std::span<Block, E>>;
};

template<class Bits>
using words_of_t = words_of<Bits>::type;

template<class Bits>
inline constexpr std::size_t words_width_v = words_of<Bits>::width;

// The words a guide deduces from what it is handed: a word as itself, a range as the span that lends it.
template<class W>
struct lent_words;

template<borrowable_word W>
struct lent_words<W>
{
        using type = std::remove_reference_t<W>;
};

template<borrowable_words W>
struct lent_words<W>
{
        using type = words_span_t<std::remove_reference_t<W>>;
};

template<class W>
using lent_words_t = lent_words<W>::type;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_WORDS_HPP
