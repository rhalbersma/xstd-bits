//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/contiguous_bit_sequence.hpp>      // contiguous_bit_sequence
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <boost/dynamic_bitset/dynamic_bitset.hpp>    // dynamic_bitset
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <bitset>                                     // bitset
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint64_t
#include <tuple>                                      // tuple

// The common vocabulary the three bit containers answer in their own names. [design.md#the-common-vocabulary]
namespace {

using ours_static  = xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>;
using ours_dynamic = xstd::detail::bits::contiguous_bit_vector<std::uint64_t>;
using theirs       = std::bitset<64>;
using boosts       = boost::dynamic_bitset<>;

// Each probe is a template: a requires-expression over a concrete type is evaluated eagerly and hard-errors rather than answering false, so "does not have" can only be asked through a parameter.
template<class C> concept has_subscript  = requires (C const& c, std::size_t n)  { c[n];               };
template<class C> concept has_complement = requires (C const& c)                 { ~c;                 };
template<class C> concept has_set_value  = requires (C& b, std::size_t n, bool v) { b.set(n, v);       };
template<class C> concept has_difference = requires (C& b, C const& c)           { b -= c;             };
template<class C> concept has_subset_of  = requires (C const& c)                 { c.is_subset_of(c);  };
template<class C> concept has_to_string  = requires (C const& c)                 { c.to_string();      };

// A storage carrying none of this vocabulary; nothing is constrained on the concept, so it is still a type the library never wraps rather than one it rejects.
struct word { std::uint64_t bits = 0; };

inline constexpr auto width = 64UZ;

// Two of the four carry their width in the type and two take it at construction. Named rather than detected:
// std::bitset<N> is constructible from a std::size_t and reads it as a VALUE, so a detector spelled that way
// hands back std::bitset<64>(64), which is bit 6 set rather than a width.
template<class C>
[[nodiscard]] auto make()
        -> C
{
        return C();
}

template<> [[nodiscard]] auto make<ours_dynamic>() -> ours_dynamic { return ours_dynamic(width); }
template<> [[nodiscard]] auto make<boosts>()       -> boosts       { return boosts(width);       }

}       // namespace

using Models = std::tuple<ours_static, ours_dynamic, theirs, boosts>;

BOOST_AUTO_TEST_SUITE(TheCommonVocabulary)

// All three model it, at both widths of ours.
static_assert(xstd::contiguous_bit_sequence<ours_static>);
static_assert(xstd::contiguous_bit_sequence<ours_dynamic>);
static_assert(xstd::contiguous_bit_sequence<theirs>);
static_assert(xstd::contiguous_bit_sequence<boosts>);

// It is the intersection and not the union: every one of these is absent from at least one of the three, so asking for it would drop a model.
static_assert(not has_subscript<ours_static>);   // ours reads through test, never a subscript [design.md#test-not-subscript]
static_assert(not has_complement<ours_static>);  // nor does it complement in place
static_assert(not has_set_value<ours_static>);   // nor take the two-argument set, assign being spelled apart from it
static_assert(not has_difference<theirs>);       // std::bitset has no difference
static_assert(not has_subset_of<theirs>);        // nor boost's set vocabulary
static_assert(not has_to_string<boosts>);        // to_string is std::bitset's alone

// Structural and nothing more: it describes a shape the three containers share, and the adaptors admit their storage by name instead. [design.md#the-common-vocabulary]
static_assert(not xstd::contiguous_bit_sequence<word>);

// Asserted only, the concept would say the names exist; asked of each model in turn, it says they mean the same
// thing. Three cases rather than one, because one walk of the whole vocabulary is past the cognitive-complexity
// threshold and the three groups are the reading's own: the whole, a position, and the bitwise operators.
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryModelAnswersTheWhole, C, Models)
{
        auto a = make<C>();

        BOOST_CHECK_EQUAL(a.size(), width);

        a.set();
        BOOST_CHECK_EQUAL(a.count(), width);
        BOOST_CHECK(a.all());
        BOOST_CHECK(a.any());
        BOOST_CHECK(not a.none());

        a.reset();
        BOOST_CHECK_EQUAL(a.count(), 0UZ);
        BOOST_CHECK(not a.all());
        BOOST_CHECK(not a.any());
        BOOST_CHECK(a.none());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EveryModelAnswersAPosition, C, Models)
{
        auto a = make<C>();

        a.set(3);
        BOOST_CHECK(a.test(3));
        BOOST_CHECK(not a.test(4));
        BOOST_CHECK_EQUAL(a.count(), 1UZ);

        a.reset(3);
        BOOST_CHECK(not a.test(3));
        BOOST_CHECK(a.none());

        a.flip(3);
        BOOST_CHECK(a.test(3));

        a.flip();
        BOOST_CHECK(not a.test(3));
        BOOST_CHECK_EQUAL(a.count(), width - 1UZ);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EveryModelAnswersTheBitwiseOperators, C, Models)
{
        auto a = make<C>();
        a.set(3);

        auto b = a;
        BOOST_CHECK(b == a);

        b |= a;
        BOOST_CHECK(b == a);
        b &= a;
        BOOST_CHECK(b == a);
        b ^= a;
        BOOST_CHECK(b.none());

        b = a;
        b <<= 1;
        BOOST_CHECK(b.test(4));
        b >>= 1;
        BOOST_CHECK(b == a);
}

BOOST_AUTO_TEST_SUITE_END()
