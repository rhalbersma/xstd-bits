//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/dynamic_bitset.hpp>           // dynamic_bitset
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/basic_bitset.hpp>         // basic_bitset, has_bitops
#include <xstd/bits/bitset.hpp>               // bitset
#include <xstd/bits/block_sequence.hpp>       // block_array, block_vector
#include <xstd/bits/ext/std/bitset.hpp>       // IWYU pragma: keep; bit_traits<std::bitset>
#include <xstd/bits/ranges/sequence_view.hpp> // sequence_view
#include <xstd/bits/ranges/set_view.hpp>      // set_view
#include <bitset>                             // bitset
#include <concepts>                           // regular, same_as
#include <cstddef>                            // size_t
#include <cstdint>                            // uint8_t
#include <functional>                         // hash
#include <ranges>                             // range
#include <stdexcept>                          // out_of_range
#include <string>                             // string
#include <tuple>                              // tuple
#include <type_traits>                        // is_nothrow_*, is_trivially_*
#include <utility>                            // as_const, declval
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(BasicBitset)

// The counterparts that speak the vocabulary, and one that does not.
BOOST_AUTO_TEST_CASE(TheVocabularyIsWhatTheThreeStoragesSpeak)
{
        static_assert(xstd::has_bitops<xstd::block_array<std::uint8_t, 0>>);
        static_assert(xstd::has_bitops<xstd::block_array<std::uint64_t, 100>>);
        static_assert(xstd::has_bitops<xstd::block_vector<std::size_t>>);
        static_assert(xstd::has_bitops<std::bitset<0>>);
        static_assert(xstd::has_bitops<std::bitset<100>>);
        static_assert(xstd::has_bitops<boost::dynamic_bitset<>>);

        static_assert(not xstd::has_bitops<std::vector<bool>>);
        static_assert(not xstd::has_bitops<std::vector<std::uint8_t>>);
}

// The public name is the wrapper over a packed array, with the word type in the open.
BOOST_AUTO_TEST_CASE(TheBitsetIsTheWrapperOverAPackedArray)
{
        static_assert(std::same_as<xstd::bitset<9, std::uint8_t>, xstd::basic_bitset<xstd::block_array<std::uint8_t, 9>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::basic_bitset<xstd::block_array<std::size_t, 64>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::basic_bitset<xstd::block_array<std::size_t, 64>, xstd::bit_traits<xstd::block_array<std::size_t, 64>>>>);
}

using Wrapped = std::tuple
<       xstd::basic_bitset<std::bitset<  0>>
,       xstd::basic_bitset<std::bitset<  1>>
,       xstd::basic_bitset<std::bitset< 64>>
,       xstd::basic_bitset<std::bitset< 65>>
,       xstd::basic_bitset<std::bitset<128>>
>;

// Wrapping std::bitset gives back a regular, nothrow, trivially copyable type that is not a range: what std::bitset is.
BOOST_AUTO_TEST_CASE_TEMPLATE(WrappingStdBitsetKeepsItsShape, T, Wrapped)
{
        static_assert(std::regular<T>);
        static_assert(not std::ranges::range<T>);
        static_assert(std::is_nothrow_default_constructible_v<T>);
        static_assert(std::is_nothrow_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_assignable_v<T>);
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(std::ranges::bidirectional_range<decltype(xstd::set_view(std::declval<T&>()))>);
        static_assert(std::ranges::random_access_range<decltype(xstd::sequence_view(std::declval<T&>()))>);
}

// Idempotence, member by member: the wrapper answers exactly as the std::bitset it wraps, throw for throw.
BOOST_AUTO_TEST_CASE(TheWrapperOverStdBitsetAnswersAsStdBitsetDoes)
{
        auto w = xstd::basic_bitset<std::bitset<9>>();
        auto s = std::bitset<9>();
        BOOST_CHECK_EQUAL(w.size(), s.size());
        BOOST_CHECK(w.none() and s.none());

        w.set(3); s.set(3);
        w.set(8, true); s.set(8, true);
        BOOST_CHECK_EQUAL(w.count(), s.count());
        BOOST_CHECK_EQUAL(w.test(3), s.test(3));
        BOOST_CHECK_EQUAL(w[8], s[8]);
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());

        w.flip(3); s.flip(3);
        w.reset(8); s.reset(8);
        BOOST_CHECK(w.none() and s.none());
}

// The checked family throws where std::bitset throws, forwarded rather than guarded a second time.
BOOST_AUTO_TEST_CASE(TheCheckedFamilyOverStdBitsetThrowsAsStdBitsetDoes)
{
        auto w = xstd::basic_bitset<std::bitset<9>>();
        BOOST_CHECK_THROW(w.set(9), std::out_of_range);
        BOOST_CHECK_THROW(w.reset(9), std::out_of_range);
        BOOST_CHECK_THROW(w.flip(9), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(w.test(9)), std::out_of_range);
}

// The shifts are total on both counterparts, saturating to none, and the derived operators compose on a copy.
BOOST_AUTO_TEST_CASE(TheShiftsOverStdBitsetSaturateAsStdBitsetDoes)
{
        auto w = xstd::basic_bitset<std::bitset<9>>();
        auto s = std::bitset<9>();

        w.set(); s.set();
        w <<= 4; s <<= 4;
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());
        w >>= 9; s >>= 9;
        BOOST_CHECK(w.none() and s.none());
        w.set(0); s.set(0);
        w <<= 9; s <<= 9;
        BOOST_CHECK(w.none() and s.none());

        w.flip(); s.flip();
        BOOST_CHECK_EQUAL((~w).count(), (~s).count());
        BOOST_CHECK_EQUAL((w << 1).to_string(), (s << 1).to_string());
        BOOST_CHECK_EQUAL((w >> 1).to_string(), (s >> 1).to_string());
}

// The proxy over std::bitset storage writes and reads through the trait, and swaps as a value.
BOOST_AUTO_TEST_CASE(TheProxyOverStdBitsetStorageWritesThrough)
{
        auto w = xstd::basic_bitset<std::bitset<8>>();

        w[3] = true;
        BOOST_CHECK(w[3] and w.test(3));
        w[2] = w[3];
        BOOST_CHECK(w[2]);
        BOOST_CHECK_EQUAL(~w[2], false);
        w[2].flip();
        BOOST_CHECK(not w[2]);

        auto const x = w[2];
        auto const y = w[3];
        swap(x, y);
        BOOST_CHECK(w[2] and not w[3]);

        auto z = true;
        swap(w[3], z);
        BOOST_CHECK(w[3] and not z);
        swap(z, w[3]);
        BOOST_CHECK(z and not w[3]);
}

// The views reach a wrapped std::bitset by referring into it: the ordering, the keys, the blocks where readable. [design.md#views-over-owners]
BOOST_AUTO_TEST_CASE(TheViewsReachAWrappedStdBitset)
{
        auto a = xstd::basic_bitset<std::bitset<70>>();
        auto b = xstd::basic_bitset<std::bitset<70>>();
        a.set(1); a.set(69);
        b.set(1); b.set(2);

        // Named rather than called on the temporaries: clang 23's lifetime analysis crashes on a deducing-this member of a prvalue.
        auto const va = xstd::set_view(a);
        auto const vb = xstd::set_view(b);
        auto const qa = xstd::sequence_view(a);

        auto keys = std::vector<std::size_t>();
        for (auto const k : va) {
                keys.push_back(k);
        }
        BOOST_CHECK((keys == std::vector<std::size_t>{ 1, 69 }));

        BOOST_CHECK(va != vb);
        BOOST_CHECK((vb <=> va) < 0);
        BOOST_CHECK(va.is_subset_of(va));
        BOOST_CHECK_EQUAL(qa[69], true);

        static_assert(std::same_as<decltype(va), xstd::set_view<std::bitset<70>> const>);
        static_assert(std::same_as<decltype(xstd::set_view(std::as_const(a))), xstd::set_view<std::bitset<70> const>>);
}

// Built from text, streamed back to text, and hashed: the derived members, over either storage.
BOOST_AUTO_TEST_CASE(TheDerivedMembersHoldOverEitherStorage)
{
        using Packed = xstd::bitset<9, std::uint8_t>;
        auto const packed  = Packed("101000001");
        auto const wrapped = xstd::basic_bitset<std::bitset<9>>(std::string("101000001"));
        BOOST_CHECK_EQUAL(packed.to_string(), wrapped.to_string());
        BOOST_CHECK_EQUAL(packed.count(), 3UZ);
        BOOST_CHECK(packed.test(0) and packed.test(6) and packed.test(8));

        auto const hash = std::hash<Packed>();
        BOOST_CHECK_EQUAL(hash(packed), hash(Packed("101000001")));
        BOOST_CHECK(hash(packed) != hash(Packed()));

        // A count on the character-pointer form takes that many characters and no more.
        BOOST_CHECK_EQUAL(Packed("1111", 2).count(), 2UZ);
}

// The text constructors reject as std::bitset's do: a stray character, and a position past the end.
BOOST_AUTO_TEST_CASE(TheTextConstructorsRejectWhatStdBitsetRejects)
{
        using Packed = xstd::bitset<9, std::uint8_t>;

        BOOST_CHECK_THROW(static_cast<void>(Packed("102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(xstd::basic_bitset<std::bitset<9>>(std::string("102"))), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(xstd::basic_bitset<std::bitset<9>>(std::string("101"), 4)), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
