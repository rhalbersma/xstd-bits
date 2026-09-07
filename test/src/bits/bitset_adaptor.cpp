//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/bit_set_view.hpp>             // bit_set_view
#include <xstd/bits/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_traits.hpp>               // bit_traits, block_readable
#include <xstd/bits/bitset.hpp>                   // basic_bitset, bitset
#include <xstd/bits/bitset_adaptor.hpp>           // bitset_adaptor, has_bitops
#include <xstd/bits/block_sequence.hpp>           // block_array, block_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // IWYU pragma: keep; bit_traits<boost::dynamic_bitset>
#include <xstd/bits/ext/std/bitset.hpp>           // IWYU pragma: keep; bit_traits<std::bitset>
#include <algorithm>                              // equal
#include <array>                                  // array
#include <bitset>                                 // bitset
#include <compare>                                // strong_ordering
#include <concepts>                               // regular, same_as, totally_ordered
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t, uint64_t
#include <functional>                             // hash
#include <iterator>                               // back_inserter
#include <ranges>                                 // equal, iota, range, reverse
#include <sstream>                                // istringstream
#include <stdexcept>                              // out_of_range, overflow_error
#include <string>                                 // string
#include <tuple>                                  // tuple
#include <type_traits>                            // is_nothrow_*, is_trivially_*
#include <utility>                                // as_const, declval
#include <vector>                                 // vector

BOOST_AUTO_TEST_SUITE(BitsetAdaptor)

// Dependent, so an unsatisfied class constraint is a false rather than a hard error; an expression rather than a type requirement, which clang-tidy 22 misreads.
template<class B>
constexpr bool wrappable = requires { sizeof(xstd::bitset_adaptor<B>); };

// The vocabulary is our storages': std::bitset's members and boost's set vocabulary, read by block. The counterparts themselves are not wrapped. [design.md#owning-is-ours]
BOOST_AUTO_TEST_CASE(TheVocabularyIsWhatOurStoragesSpeakAndTheCounterpartsDoNot)
{
        static_assert(xstd::has_bitops<xstd::block_array<std::uint8_t, 0>>);
        static_assert(xstd::has_bitops<xstd::block_array<std::uint64_t, 100>>);
        static_assert(xstd::has_bitops<xstd::block_vector<std::size_t>>);
        static_assert(wrappable<xstd::block_array<std::uint8_t, 0>>);
        static_assert(wrappable<xstd::block_vector<std::size_t>>);

        // std::bitset lacks the set vocabulary; boost has it but keeps its blocks to itself.
        static_assert(not xstd::has_bitops<std::bitset<0>>);
        static_assert(not xstd::has_bitops<std::bitset<100>>);
        static_assert(    xstd::has_bitops<boost::dynamic_bitset<>>);
        static_assert(not xstd::block_readable<xstd::bit_traits<boost::dynamic_bitset<>>, boost::dynamic_bitset<>>);
        static_assert(not wrappable<std::bitset<64>>);
        static_assert(not wrappable<boost::dynamic_bitset<>>);

        static_assert(not xstd::has_bitops<std::vector<bool>>);
        static_assert(not xstd::has_bitops<std::vector<std::uint8_t>>);
}

// The public name is the wrapper over a packed array, with the word type in the open.
BOOST_AUTO_TEST_CASE(TheBitsetIsTheWrapperOverAPackedArray)
{
        static_assert(std::same_as<xstd::basic_bitset<9, std::uint8_t>, xstd::bitset_adaptor<xstd::block_array<std::uint8_t, 9>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::bitset_adaptor<xstd::block_array<std::size_t, 64>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::bitset_adaptor<xstd::block_array<std::size_t, 64>, xstd::bit_traits<xstd::block_array<std::size_t, 64>>>>);
}

using Static = std::tuple
<       xstd::basic_bitset<  0, std::uint8_t>
,       xstd::basic_bitset<  1, std::uint8_t>
,       xstd::basic_bitset< 64, std::uint8_t>
,       xstd::basic_bitset< 65, std::uint8_t>
,       xstd::basic_bitset<128, std::uint8_t>
,       xstd::bitset<  0>
,       xstd::bitset< 64>
,       xstd::bitset< 65>
>;

// A regular, nothrow, trivially copyable type that is not a range: what std::bitset is, and what a strict extension keeps. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitsetHasStdBitsetsShape, T, Static)
{
        static_assert(std::regular<T>);
        static_assert(not std::ranges::range<T>);
        static_assert(std::is_nothrow_default_constructible_v<T>);
        static_assert(std::is_nothrow_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_assignable_v<T>);
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(std::ranges::bidirectional_range<decltype(xstd::bit_set_view(std::declval<T&>()))>);
        static_assert(std::ranges::random_access_range<decltype(xstd::bit_span(std::declval<T&>()))>);
}

using Ours = xstd::basic_bitset<9, std::uint8_t>;

// Member by member, ours answers exactly as std::bitset does, throw for throw. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE(OursAnswersAsStdBitsetDoes)
{
        auto w = Ours();
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

// The checked family throws where std::bitset throws: one guard, at a static width. [design.md#the-one-guard]
BOOST_AUTO_TEST_CASE(TheCheckedFamilyThrowsAsStdBitsetDoes)
{
        auto w = Ours();
        BOOST_CHECK_THROW(w.set(9), std::out_of_range);
        BOOST_CHECK_THROW(w.reset(9), std::out_of_range);
        BOOST_CHECK_THROW(w.flip(9), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(w.test(9)), std::out_of_range);
}

// The shifts are total on both counterparts, saturating to none, and the derived operators compose on a copy. [design.md#the-one-guard]
BOOST_AUTO_TEST_CASE(TheShiftsSaturateAsStdBitsetDoes)
{
        auto w = Ours();
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

// The proxy writes and reads through the trait, and swaps as a value.
BOOST_AUTO_TEST_CASE(TheProxyWritesThrough)
{
        auto w = xstd::basic_bitset<8, std::uint8_t>();

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

// The set vocabulary boost has and std::bitset has not is there at a static width too, and so are the two searches. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE(TheExtensionIsThereAtAStaticWidth)
{
        auto d = Ours(0b101ULL);
        auto e = Ours();
        e.set(2);

        BOOST_CHECK(e.is_subset_of(d));
        BOOST_CHECK(e.is_proper_subset_of(d));
        BOOST_CHECK(d.intersects(e));
        BOOST_CHECK(not e.is_proper_subset_of(e));
        BOOST_CHECK(not Ours().intersects(d));

        BOOST_CHECK_EQUAL(d.find_first(), 0UZ);
        BOOST_CHECK_EQUAL(d.find_next(0), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(2), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_first(), Ours::npos);
        using Empty = xstd::basic_bitset<0, std::uint8_t>;
        BOOST_CHECK_EQUAL(Empty().find_first(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_next(0), Empty::npos);

        auto const f = d - e;
        d -= e;
        BOOST_CHECK(f == d);
        BOOST_CHECK_EQUAL(d.count(), 1UZ);
        BOOST_CHECK(d.test(0) and not d.test(2));
}

// The reverse pair mirrors boost's forward pair: the highest set position below pos, npos where none, a pos past the width meaning from the end.
BOOST_AUTO_TEST_CASE(TheReverseSearchesMirrorTheForwardOnes)
{
        auto const d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.find_last(), 2UZ);
        BOOST_CHECK_EQUAL(d.find_prev(2), 0UZ);
        BOOST_CHECK_EQUAL(d.find_prev(1), 0UZ);
        BOOST_CHECK_EQUAL(d.find_prev(0), Ours::npos);
        BOOST_CHECK_EQUAL(d.find_prev(100), 2UZ);
        BOOST_CHECK_EQUAL(d.find_prev(Ours::npos), d.find_last());
        BOOST_CHECK_EQUAL(Ours().find_last(), Ours::npos);

        using Empty = xstd::basic_bitset<0, std::uint8_t>;
        BOOST_CHECK_EQUAL(Empty().find_last(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_prev(0), Empty::npos);

        // The two loops are each other's reverse, across blocks.
        using Wide = xstd::basic_bitset<70, std::uint8_t>;
        auto w = Wide();
        w.set(1); w.set(8); w.set(9); w.set(69);
        auto forward = std::vector<std::size_t>();
        for (auto i = w.find_first(); i != Wide::npos; i = w.find_next(i)) {
                forward.push_back(i);
        }
        auto backward = std::vector<std::size_t>();
        for (auto i = w.find_last(); i != Wide::npos; i = w.find_prev(i)) {
                backward.push_back(i);
        }
        BOOST_CHECK((forward == std::vector<std::size_t>{ 1, 8, 9, 69 }));
        BOOST_CHECK(std::ranges::equal(forward, std::views::reverse(backward)));
}

// The ordering is the bit string's, to_string() compared, which within a word is the number's. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE(TheOrderingIsTheBitStrings)
{
        static_assert(std::totally_ordered<Ours>);
        for (auto const a : std::views::iota(0ULL, 512ULL)) {
                for (auto const b : std::views::iota(0ULL, 512ULL)) {
                        auto const x = Ours(a);
                        auto const y = Ours(b);
                        BOOST_CHECK((x <=> y) == (a <=> b));
                        BOOST_CHECK((x <=> y) == (x.to_string() <=> y.to_string()));
                }
        }

        // Across blocks: the top block decides before the lower ones say anything.
        using Wide = xstd::basic_bitset<70, std::uint8_t>;
        auto top = Wide();
        top.set(69);
        auto rest = Wide();
        rest.set();
        rest.reset(69);
        BOOST_CHECK(rest < top);
        BOOST_CHECK((Wide() <=> Wide()) == std::strong_ordering::equal);
        BOOST_CHECK(Wide() < top);
}

// boost's block interface: the block type and its width, the block count, every block out and at most every block in, the tail kept clear. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE(TheBlockInterfaceIsBoosts)
{
        static_assert(std::same_as<Ours::block_type, std::uint8_t>);
        static_assert(Ours::bits_per_block == 8UZ);
        BOOST_CHECK_EQUAL(Ours().num_blocks(), 2UZ);

        auto const b = Ours("101000001");
        auto out = std::vector<std::uint8_t>();
        to_block_range(b, std::back_inserter(out));
        BOOST_CHECK((out == std::vector<std::uint8_t>{ 0b0100'0001, 0b1 }));

        auto c = Ours();
        from_block_range(out.begin(), out.end(), c);
        BOOST_CHECK(c == b);

        // Fewer blocks than there are leaves the rest alone; a dirty tail is masked rather than kept.
        auto const dirty = std::array<std::uint8_t, 2>{ 0b1000'0000, 0b1111'1111 };
        auto e = Ours();
        from_block_range(dirty.begin(), dirty.end(), e);
        BOOST_CHECK_EQUAL(e.count(), 2UZ);
        BOOST_CHECK(e.test(7) and e.test(8));
        from_block_range(dirty.begin(), dirty.begin() + 1, c);
        BOOST_CHECK_EQUAL(c.count(), 2UZ);
        BOOST_CHECK(c.test(7) and c.test(8));
}

// The views reach a bitset by referring into its storage: the ordering, the keys, the blocks. [design.md#views-over-owners]
BOOST_AUTO_TEST_CASE(TheViewsReachABitset)
{
        using Wide = xstd::basic_bitset<70, std::uint8_t>;
        auto a = Wide();
        auto b = Wide();
        a.set(1); a.set(69);
        b.set(1); b.set(2);

        // Named rather than called on the temporaries: clang 23's lifetime analysis crashes on a deducing-this member of a prvalue.
        auto const va = xstd::bit_set_view(a);
        auto const vb = xstd::bit_set_view(b);
        auto const qa = xstd::bit_span(a);

        auto keys = std::vector<std::size_t>();
        for (auto const k : va) {
                keys.push_back(k);
        }
        BOOST_CHECK((keys == std::vector<std::size_t>{ 1, 69 }));

        BOOST_CHECK(va != vb);
        BOOST_CHECK((vb <=> va) < 0);
        BOOST_CHECK(va.is_subset_of(va));
        BOOST_CHECK_EQUAL(qa[69], true);

        static_assert(std::same_as<decltype(va), xstd::bit_set_view<xstd::block_array<std::uint8_t, 70>> const>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::as_const(a))), xstd::bit_set_view<xstd::block_array<std::uint8_t, 70> const>>);
}

// Built from text, streamed back to text, and hashed: the derived members.
BOOST_AUTO_TEST_CASE(TheDerivedMembersHold)
{
        auto const packed = Ours("101000001");
        BOOST_CHECK_EQUAL(packed.to_string(), std::bitset<9>("101000001").to_string());
        BOOST_CHECK_EQUAL(packed.count(), 3UZ);
        BOOST_CHECK(packed.test(0) and packed.test(6) and packed.test(8));

        auto const hash = std::hash<Ours>();
        BOOST_CHECK_EQUAL(hash(packed), hash(Ours("101000001")));
        BOOST_CHECK(hash(packed) != hash(Ours()));

        // A count on the character-pointer form takes that many characters and no more.
        BOOST_CHECK_EQUAL(Ours("1111", 2).count(), 2UZ);
}

// [bitset.cons]/2 and [bitset.members]/34-37: the word in and the word out, the overflow where a set position lies beyond it.
BOOST_AUTO_TEST_CASE(TheWordConstructorAndConversionsAgreeWithStdBitset)
{
        auto const s = std::bitset<9>(0b101ULL);
        auto const p = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(p.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(p.to_ullong(), s.to_ullong());
        BOOST_CHECK_EQUAL(p.to_ulong(), s.to_ulong());

        // The high bits of the value drop where the width is narrower, as [bitset.cons]/2 has it.
        using Narrow = xstd::basic_bitset<3, std::uint8_t>;
        using Empty  = xstd::basic_bitset<0, std::uint8_t>;
        BOOST_CHECK_EQUAL(Narrow(0b1111ULL).to_ullong(), 7ULL);
        BOOST_CHECK_EQUAL(Narrow(0b1101ULL).to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(Empty(0b1111ULL).to_ullong(), 0ULL);

        auto wide = xstd::basic_bitset<70, std::uint8_t>();
        BOOST_CHECK_EQUAL(wide.to_ullong(), 0ULL);
        wide.set(69);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ullong()), std::overflow_error);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ulong()), std::overflow_error);
}

// [bitset.operators]/6: a short read lands in the low bits, as std::bitset(str) puts it, and an empty read fails the stream.
BOOST_AUTO_TEST_CASE(ExtractionOfAShortInputAgreesWithStdBitset)
{
        auto in = std::istringstream("1");
        auto s = std::bitset<4>();
        in >> s;
        auto ours = std::istringstream("1");
        auto x = xstd::basic_bitset<4, std::uint8_t>();
        ours >> x;
        BOOST_CHECK_EQUAL(x.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(x.to_ullong(), 1ULL);

        auto stop = std::istringstream("01x");
        auto y = xstd::basic_bitset<4, std::uint8_t>();
        stop >> y;
        BOOST_CHECK_EQUAL(y.to_string(), "0001");
        BOOST_CHECK(not stop.fail());
}

// The text constructors reject as std::bitset's do: a stray character, and a position past the end.
BOOST_AUTO_TEST_CASE(TheTextConstructorsRejectWhatStdBitsetRejects)
{
        BOOST_CHECK_THROW(static_cast<void>(Ours("102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(std::string("101"), 4)), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
