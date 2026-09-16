//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/contiguous_bit_sequence.hpp>          // contiguous_bit_sequence
#include <xstd/bits/bitset.hpp>                          // basic_bitset, bitset
#include <xstd/bits/bitset_adaptor.hpp>                  // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp>     // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp>    // contiguous_bit_vector
#include <xstd/bits/dynamic_bitset.hpp>                  // basic_dynamic_bitset
#include <boost/dynamic_bitset.hpp>                      // dynamic_bitset
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                     // equal
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <compare>                                       // is_lt, strong_ordering
#include <concepts>                                      // regular, same_as, totally_ordered
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t, uint64_t
#include <functional>                                    // hash
#include <iterator>                                      // back_inserter
#include <limits>                                        // numeric_limits
#include <ranges>                                        // equal, iota, range, reverse
#include <sstream>                                       // istringstream
#include <stdexcept>                                     // out_of_range, overflow_error
#include <string>                                        // string
#include <tuple>                                         // tuple
#include <type_traits>                                   // is_nothrow_*, is_trivially_*
#include <utility>                                       // as_const, declval
#include <vector>                                        // vector

BOOST_AUTO_TEST_SUITE(BitsetAdaptor)

// Dependent, so an unsatisfied class constraint is a false rather than a hard error; an expression rather than a type requirement, which clang-tidy 22 misreads.
template<class B>
constexpr bool wrappable = requires { sizeof(xstd::bitset_adaptor<B>); };

// Dependent likewise, so a storage without an allocator answers false; the alias spells the typedef without a typename, which clang-tidy 22 reads as redundant.
template<class X>
using allocator_of = X::allocator_type;

template<class X>
constexpr bool has_allocator = requires (X const& x) { sizeof(allocator_of<X>); x.get_allocator(); };

// Only our storages can be wrapped, and the refusal is nominal now rather than a vocabulary the candidate fails to speak.
BOOST_AUTO_TEST_CASE(TheWrappedStoragesAreOursAndTheCounterpartsAreNot)
{
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 0>>);
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_array<std::uint64_t, 100>>);
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_vector<std::size_t>>);

        // Not for want of the vocabulary: boost::dynamic_bitset speaks all of it and is still refused, because being ours is the question and not what a candidate's members answer.
        static_assert(not wrappable<std::bitset<64>>);
        static_assert(not wrappable<boost::dynamic_bitset<>>);
        static_assert(not wrappable<std::vector<bool>>);
        static_assert(not wrappable<std::vector<std::uint8_t>>);
}

// The public name is the wrapper over a packed array, with the word type in the open.
BOOST_AUTO_TEST_CASE(TheBitsetIsTheWrapperOverAPackedArray)
{
        static_assert(std::same_as<xstd::basic_bitset<std::uint8_t, 9>, xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_array<std::size_t, 64>>>);
}

using Static = std::tuple
<       xstd::basic_bitset<std::uint8_t, 0>
,       xstd::basic_bitset<std::uint8_t, 1>
,       xstd::basic_bitset<std::uint8_t, 64>
,       xstd::basic_bitset<std::uint8_t, 65>
,       xstd::basic_bitset<std::uint8_t, 128>
,       xstd::bitset<  0>
,       xstd::bitset< 64>
,       xstd::bitset< 65>
>;

// A regular, nothrow, trivially copyable type that is not a range: what std::bitset is, and what a strict extension keeps.
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

using Ours = xstd::basic_bitset<std::uint8_t, 9>;

// Member by member, ours answers exactly as std::bitset does, throw for throw.
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

// The checked family throws where std::bitset throws: one guard, at a static width.
BOOST_AUTO_TEST_CASE(TheCheckedFamilyThrowsAsStdBitsetDoes)
{
        auto w = Ours();
        BOOST_CHECK_THROW(w.set(9), std::out_of_range);
        BOOST_CHECK_THROW(w.reset(9), std::out_of_range);
        BOOST_CHECK_THROW(w.flip(9), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(w.test(9)), std::out_of_range);
}

// The shifts are total on both counterparts, saturating to none, and the derived operators compose on a copy.
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
        auto w = xstd::basic_bitset<std::uint8_t, 8>();

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

// The set vocabulary boost has and std::bitset has not is there at a static width too, and so are the two searches.
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

        // The symmetric spelling beside boost's member, both orders alike.
        BOOST_CHECK(intersects(d, e) and intersects(e, d));
        BOOST_CHECK(not intersects(Ours(), d) and not intersects(d, Ours()));

        BOOST_CHECK_EQUAL(d.find_first(), 0UZ);
        BOOST_CHECK_EQUAL(d.find_next(0), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(2), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_first(), Ours::npos);
        using Empty = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Empty().find_first(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_next(0), Empty::npos);

        auto const f = d - e;
        d -= e;
        BOOST_CHECK(f == d);
        BOOST_CHECK_EQUAL(d.count(), 1UZ);
        BOOST_CHECK(d.test(0) and not d.test(2));
}

// The forward pair answers for the positions the reverse pair below was already asked about, and did not: boost's find_next is total -- `if (pos >= sz - 1 || sz == 0) return npos` -- where the storage's step asserts is_valid(n) and moves to n + 1. So a pos past the width read a block index the storage need not have, and find_next(npos) was the worst of it: n + 1 wraps to zero, the scan starts at the beginning, and the answer is the FIRST set position rather than none.
BOOST_AUTO_TEST_CASE(TheForwardSearchesAreTotalPastTheWidth)
{
        auto const d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.find_next(1), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(8), Ours::npos);          // the last position this width has
        BOOST_CHECK_EQUAL(d.find_next(9), Ours::npos);          // the first it has not
        BOOST_CHECK_EQUAL(d.find_next(100), Ours::npos);
        BOOST_CHECK_EQUAL(d.find_next(Ours::npos), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_next(0), Ours::npos);

        // And at a width of more than one block, which is where the walk below steps through find_next: the last block is the one a step past the width would read beyond.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto w = Wide();
        w.set(69);
        BOOST_CHECK_EQUAL(w.find_next(68), 69UZ);
        BOOST_CHECK_EQUAL(w.find_next(69), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(70), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(Wide::npos), Wide::npos);
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

        using Empty = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Empty().find_last(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_prev(0), Empty::npos);

        // The two loops are each other's reverse, across blocks.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
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

// The ordering is the bit string's, to_string() compared, which within a word is the number's.
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
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto top = Wide();
        top.set(69);
        auto rest = Wide();
        rest.set();
        rest.reset(69);
        BOOST_CHECK(rest < top);
        BOOST_CHECK((Wide() <=> Wide()) == std::strong_ordering::equal);
        BOOST_CHECK(Wide() < top);
}

// boost's block interface: the block type and its width, the block count, every block out and at most every block in, the tail kept clear.
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

// boost's remaining members at a static width, where every guard throws: at, test_set, the ranged forms, max_size; no allocator, the storage having none.
BOOST_AUTO_TEST_CASE(TheRestOfBoostsSurfaceIsThereAtAStaticWidth)
{
        static_assert(not has_allocator<Ours>);
        static_assert(has_allocator<xstd::basic_dynamic_bitset<std::uint8_t>>);
        BOOST_CHECK_EQUAL(Ours().max_size(), 9UZ);

        auto d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.at(0), true);
        BOOST_CHECK_EQUAL(std::as_const(d).at(1), false);
        d.at(1) = true;
        BOOST_CHECK(d.test(1));
        BOOST_CHECK_THROW(static_cast<void>(d.at(9)), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(std::as_const(d).at(9)), std::out_of_range);

        BOOST_CHECK_EQUAL(d.test_set(1, false), true);
        BOOST_CHECK_EQUAL(d.test_set(1), false);
        BOOST_CHECK(d.test(1));
        BOOST_CHECK_THROW(static_cast<void>(d.test_set(9)), std::out_of_range);

        d.set(3, 5, true);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1111'1111ULL - 0b100ULL + 0b100ULL);
        d.reset(0, 2);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1111'1100ULL);
        d.flip(0, 9);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1'0000'0011ULL);
        d.set(9, 0, true);
        BOOST_CHECK_THROW(d.set(8, 2, true), std::out_of_range);
        BOOST_CHECK_THROW(d.reset(9, 1), std::out_of_range);
        BOOST_CHECK_THROW(d.flip(5, 5), std::out_of_range);

        // pos + len is the sum the guard refuses to make: near the top of size_t it wraps to a value below every width, so the check the range was meant to fail is the one it would pass, and the ranged forms would then write nothing and report nothing.
        constexpr auto top = std::numeric_limits<std::size_t>::max();
        BOOST_CHECK_THROW(d.set(top, 1, true), std::out_of_range);
        BOOST_CHECK_THROW(d.reset(top - 3, 8), std::out_of_range);
        BOOST_CHECK_THROW(d.flip(top, top), std::out_of_range);

        // A position past the width is out of range whatever the length, and a refused range writes nothing.
        BOOST_CHECK_THROW(d.set(10, 0, true), std::out_of_range);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1'0000'0011ULL);
}

// The views reach a bitset by referring into its storage: the ordering, the keys, the blocks.
BOOST_AUTO_TEST_CASE(TheViewsReachABitset)
{
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
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
        BOOST_CHECK(std::is_lt(vb <=> va));
        BOOST_CHECK(va.is_subset_of(va));
        BOOST_CHECK_EQUAL(qa[69], true);

        static_assert(std::same_as<decltype(va), xstd::bit_set_view<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 70>> const>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::as_const(a))), xstd::bit_set_view<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 70> const>>);
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
        using Narrow = xstd::basic_bitset<std::uint8_t, 3>;
        using Empty  = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Narrow(0b1111ULL).to_ullong(), 7ULL);
        BOOST_CHECK_EQUAL(Narrow(0b1101ULL).to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(Empty(0b1111ULL).to_ullong(), 0ULL);

        auto wide = xstd::basic_bitset<std::uint8_t, 70>();
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
        auto x = xstd::basic_bitset<std::uint8_t, 4>();
        ours >> x;
        BOOST_CHECK_EQUAL(x.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(x.to_ullong(), 1ULL);

        auto stop = std::istringstream("01x");
        auto y = xstd::basic_bitset<std::uint8_t, 4>();
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

// A view over a bitset binds the storage it wraps, and that is the only spelling there is.
BOOST_AUTO_TEST_CASE(ABitsetReadsAsItsStorage)
{
        using B = xstd::bitset<100>;

        // A view over a bitset binds the storage it wraps, which is now the only spelling: naming the bitset itself as a view's Bits is what the constraint refuses, the storage being the thing a view refers into.
        using Blocks = xstd::detail::bits::contiguous_bit_array<std::size_t, 100>;
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::declval<B&>())), xstd::bit_set_view<Blocks>>);
        static_assert(std::same_as<decltype(xstd::bit_span(std::declval<B&>())),     xstd::bit_span<Blocks>>);

        // Naming the bitset changes how a view is spelled, not what the bitset offers.
        static_assert(not std::ranges::range<B>);

        auto bs = B();
        bs.set(3);
        bs.set(41);

        auto const sv = xstd::bit_set_view(bs);
        auto const sp = xstd::bit_span(bs);
        BOOST_CHECK_EQUAL(std::ranges::distance(sv), 2);
        BOOST_CHECK_EQUAL(std::ranges::distance(sp), 100);
        BOOST_CHECK(std::ranges::bidirectional_range<decltype(sv)>);
        BOOST_CHECK(std::ranges::random_access_range<decltype(sp)>);
}

BOOST_AUTO_TEST_SUITE_END()

// contiguous_bit_sequence is structural and says so: it asks the positional members -- test(n), set(n), reset(n), flip(n) -- and every field of bits that has them answers, ours and the counterparts alike. That is a different question from which storages this library wraps, which is nominal and asked by the constraint above.
BOOST_AUTO_TEST_SUITE(TheStructuralQuestionIsNotTheNominalOne)

static_assert(xstd::contiguous_bit_sequence<xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>>);
static_assert(xstd::contiguous_bit_sequence<boost::dynamic_bitset<>>);
static_assert(xstd::contiguous_bit_sequence<std::bitset<64>>);

BOOST_AUTO_TEST_SUITE_END()
