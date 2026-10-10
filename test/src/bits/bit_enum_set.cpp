//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>                      // day, listed_enums, nine, perm, piece, wind
#include <test/set/lookup.hpp>                     // lookup_mismatches
#include <xstd/bits/algorithm/bit_disjoint.hpp>    // bit_disjoint
#include <xstd/bits/algorithm/bit_includes.hpp>    // bit_includes
#include <xstd/bits/bit_array.hpp>                 // bit_array
#include <xstd/bits/bit_enum_set.hpp>              // bit_enum_set
#include <xstd/bits/bit_fixed_set.hpp>             // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_key_mapping.hpp>           // bit_key_mapping, bit_range_mapping, enum_traits
#include <xstd/bits/bit_type_traits/bit_least.hpp> // bit_least
#include <xstd/bits/from_blocks.hpp>               // from_blocks
#include <xstd/misc/concepts.hpp>                  // proxy_iterator, proxy_reference
#include <xstd/misc/utility/to_underlying.hpp>     // to_underlying
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                               // max, min, ranges::equal, ranges::includes
#include <bit>                                     // bit_cast
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint16_t, uint32_t, uint8_t
#include <format>                                  // format
#include <functional>                              // greater
#include <limits>                                  // numeric_limits
#include <ranges>                                  // iota, iterator_t, range_reference_t, reverse, size, to, transform
#include <set>                                     // set
#include <stdexcept>                               // out_of_range
#include <type_traits>                             // underlying_type_t
#include <utility>                                 // pair, to_underlying
#include <vector>                                  // vector

namespace {

// An enumeration with no list of values, its default mapping specialized as a range instead.
enum class weekday : std::uint8_t
{
        mon,
        tue,
        wed,
        thu,
        fri,
        sat,
        sun,
};

} // namespace

// Its author names the range of its keys: seven, from mon.
template<>
struct xstd::bit_key_mapping<weekday> : xstd::bit_range_mapping<weekday, weekday::mon, 7UZ>
{};

BOOST_AUTO_TEST_SUITE(BitEnumSet)

namespace {

// The alias's own storage under the descending comparator.
template<class E>
using descending_set = xstd::bit_least<xstd::basic_bit_fixed_set<E, std::size_t, xstd::bit_key_mapping<E>::size, xstd::bit_key_mapping<E>, std::greater<>>>;

// A requires-expression on a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class E>
constexpr bool enumerators_combine = requires (E a, E b) { a | b; } or requires (E a, E b) { a & b; } or requires (E a, E b) { a ^ b; } or requires (E a, E b) { a - b; };

template<class X, class K>
constexpr bool takes_as_key = requires (X x, K k) { x.insert(k); } or requires (X x, K k) { x.contains(k); } or requires (X x, K k) { x.find(k); };

template<class X, class K>
constexpr bool meets_a_key = requires (X x, K k) { x | k; } or requires (X x, K k) { k | x; } or requires (X x, K k) { x & k; } or requires (X x, K k) { k & x; } or requires (X x, K k) { x ^ k; } or requires (X x, K k) { k ^ x; } or requires (X x, K k) { x - k; } or requires (X x, K k) { k - x; } or requires (X x, K k) { x |= k; } or requires (X x, K k) { x &= k; } or requires (X x, K k) { x ^= k; } or requires (X x, K k) { x -= k; };

template<class Reference>
constexpr bool takes_to_underlying = requires (Reference ref) { xstd::to_underlying(ref); };

// The model with one key added, removed or toggled.
template<class M, class E>
auto with(M m, E e)
        -> M
{
        m.insert(e);
        return m;
}

template<class M, class E>
auto without(M m, E e)
        -> M
{
        m.erase(e);
        return m;
}

template<class M, class E>
auto toggled(M const& m, E e)
        -> M
{
        return m.contains(e) ? without(m, e) : with(m, e);
}

template<class M, class E>
auto only(M const& m, E e)
        -> M
{
        return m.contains(e) ? M({e}) : M();
}

// Each lookup and each single-key modifier at one listed key, against the model.
template<class X, class M>
auto looks_up_and_modifies_as_std_set(X const& a, M const& model, typename X::key_type e)
        -> void
{
        BOOST_CHECK_EQUAL(a.contains(e), model.contains(e));
        BOOST_CHECK_EQUAL(a.count(e), model.count(e));
        BOOST_CHECK(a.find(e) == (model.contains(e) ? a.lower_bound(e) : a.end()));
        auto const lb  = a.lower_bound(e);
        auto const mlb = model.lower_bound(e);
        BOOST_CHECK_EQUAL(lb == a.end(), mlb == model.end());
        BOOST_CHECK(mlb == model.end() or *lb == *mlb);
        auto const ub  = a.upper_bound(e);
        auto const mub = model.upper_bound(e);
        BOOST_CHECK_EQUAL(ub == a.end(), mub == model.end());
        BOOST_CHECK(mub == model.end() or *ub == *mub);

        auto x                    = a;
        auto m                    = model;
        auto const [it, inserted] = x.insert(e);
        BOOST_CHECK_EQUAL(inserted, m.insert(e).second);
        BOOST_CHECK(*it == e);
        BOOST_CHECK(std::ranges::equal(x, m));
        BOOST_CHECK_EQUAL(x.erase(e), m.erase(e));
        BOOST_CHECK(std::ranges::equal(x, m));
}

// The enumerator as a one-element set, on either side of each operator and on the right of each compound one.
template<class X, class M>
auto meets_an_enumerator_as_std_set(X const& a, M const& model, typename X::key_type e)
        -> void
{
        static_assert(std::same_as<decltype(a | e), X> and std::same_as<decltype(e - a), X>);
        BOOST_CHECK(std::ranges::equal(a | e, with(model, e)));
        BOOST_CHECK(std::ranges::equal(e | a, with(model, e)));
        BOOST_CHECK(std::ranges::equal(a & e, only(model, e)));
        BOOST_CHECK(std::ranges::equal(e & a, only(model, e)));
        BOOST_CHECK(std::ranges::equal(a ^ e, toggled(model, e)));
        BOOST_CHECK(std::ranges::equal(e ^ a, toggled(model, e)));
        BOOST_CHECK(std::ranges::equal(a - e, without(model, e)));
        BOOST_CHECK(std::ranges::equal(e - a, model.contains(e) ? M() : M({e})));

        auto x = a;
        BOOST_CHECK(&(x |= e) == &x);
        BOOST_CHECK(std::ranges::equal(x, with(model, e)));
        x = a;
        BOOST_CHECK(&(x &= e) == &x);
        BOOST_CHECK(std::ranges::equal(x, only(model, e)));
        x = a;
        BOOST_CHECK(&(x ^= e) == &x);
        BOOST_CHECK(std::ranges::equal(x, toggled(model, e)));
        x = a;
        BOOST_CHECK(&(x -= e) == &x);
        BOOST_CHECK(std::ranges::equal(x, without(model, e)));
}

// The subset of the listed values a mask picks, and the rest of them.
template<class M>
auto split(std::size_t mask)
        -> std::pair<M, M>
{
        constexpr auto values = xstd::enum_traits<typename M::key_type>::values;
        auto nrv              = std::pair<M, M>();
        for (auto const i : std::views::iota(0UZ, std::ranges::size(values))) {
                if (((mask >> i) & 1UZ) != 0UZ) {
                        nrv.first.insert(values[i]);
                } else {
                        nrv.second.insert(values[i]);
                }
        }
        return nrv;
}

// Every value of the underlying type from two below the first listed to two above the last, listed or not.
template<class E>
auto probes()
        -> std::vector<E>
{
        using underlying      = std::underlying_type_t<E>;
        constexpr auto values = xstd::enum_traits<E>::values;
        auto const lo         = std::max(int{std::to_underlying(values.front())} - 2, int{std::numeric_limits<underlying>::min()});
        auto const hi         = std::min(int{std::to_underlying(values.back())} + 2, int{std::numeric_limits<underlying>::max()});
        return std::views::iota(lo, hi + 1) | std::views::transform([](int v) -> E { return static_cast<E>(v); }) | std::ranges::to<std::vector>();
}

// Every subset of the listed values, built, walked, complemented, probed and modified against std::set.
template<class X, class M>
auto agrees_with_std_set_on_every_subset()
        -> void
{
        using E               = X::key_type;
        constexpr auto values = xstd::enum_traits<E>::values;
        constexpr auto n      = std::ranges::size(values);
        for (auto const mask : std::views::iota(0UZ, 1UZ << n)) {
                auto const [model, complement] = split<M>(mask);
                auto const keys                = std::vector<E>(model.begin(), model.end());
                auto const a                   = X(keys.begin(), keys.end());
                BOOST_CHECK(std::ranges::equal(a, model));
                BOOST_CHECK_EQUAL(a.size(), model.size());
                BOOST_CHECK_EQUAL(a.empty(), model.empty());
                if (not model.empty()) {
                        BOOST_CHECK(a.front() == *model.begin());
                        BOOST_CHECK(a.back() == *model.rbegin());
                }
                BOOST_CHECK(std::ranges::equal(~a, complement));
                BOOST_CHECK_EQUAL((~a).size(), n - model.size());
                for (auto const e : values) {
                        looks_up_and_modifies_as_std_set(a, model, e);
                        meets_an_enumerator_as_std_set(a, model, e);
                }
                auto mismatches = 0UZ;
                for (auto const v : probes<E>()) {
                        mismatches += test::set::lookup_mismatches(a, model, v);
                }
                BOOST_CHECK_EQUAL(mismatches, 0UZ);
        }
}

// How often the queries disagree over every pair of subsets, with each other and with std::ranges::includes.
template<class X, class M>
auto query_mismatches_over_pairs()
        -> std::size_t
{
        constexpr auto n = std::ranges::size(xstd::enum_traits<typename X::key_type>::values);
        auto mismatches  = 0UZ;
        for (auto const lhs : std::views::iota(0UZ, 1UZ << n)) {
                auto const xm = split<M>(lhs).first;
                auto const x  = X(xm.begin(), xm.end());
                for (auto const rhs : std::views::iota(0UZ, 1UZ << n)) {
                        auto const ym     = split<M>(rhs).first;
                        auto const y      = X(ym.begin(), ym.end());
                        auto const all_of = xstd::bit_includes(x, y);
                        mismatches += static_cast<std::size_t>(all_of != std::ranges::includes(x, y, x.key_comp()));
                        mismatches += static_cast<std::size_t>(all_of != std::ranges::includes(xm, ym, xm.key_comp()));
                }
        }
        return mismatches;
}

// Every value listed and no other: the whole set walked, counted and compared, padding bits included.
template<class X>
auto holds_the_listed_values_and_no_padding()
        -> void
{
        using E               = X::key_type;
        constexpr auto values = xstd::enum_traits<E>::values;
        auto const all        = ~X();
        auto const listed     = X(values.begin(), values.end());
        BOOST_CHECK_EQUAL(all.size(), std::ranges::size(values));
        BOOST_CHECK(all == listed);
        BOOST_CHECK(~all == X());
        BOOST_CHECK_EQUAL((all << 1UZ).size(), std::ranges::size(values) - 1UZ);
        BOOST_CHECK_EQUAL((all >> 1UZ).size(), std::ranges::size(values) - 1UZ);
        if constexpr (std::same_as<typename X::key_compare, std::greater<>>) {
                BOOST_CHECK(std::ranges::equal(all, values | std::views::reverse));
        } else {
                BOOST_CHECK(std::ranges::equal(all, values));
        }
}

} // namespace

// The block is the narrowest holding every value, and another block is the basic form's to spell.
BOOST_AUTO_TEST_CASE(TheAliasPicksTheSmallestBlock)
{
        using test::set::perm;
        static_assert(sizeof(xstd::bit_enum_set<perm>) == 1UZ);
        static_assert(sizeof(xstd::basic_bit_fixed_set<perm, std::uint32_t, 3UZ>) == 4UZ);
        static_assert(sizeof(xstd::bit_enum_set<test::set::wind>) == 1UZ);
        static_assert(sizeof(xstd::bit_enum_set<test::set::nine>) == 2UZ);
        static_assert(std::same_as<xstd::bit_enum_set<perm>, xstd::basic_bit_fixed_set<perm, std::uint8_t, 3UZ, xstd::bit_key_mapping<perm>>>);
        static_assert(std::same_as<xstd::bit_enum_set<test::set::nine>, xstd::basic_bit_fixed_set<test::set::nine, std::uint16_t, 9UZ, xstd::bit_key_mapping<test::set::nine>>>);

        static_assert(std::same_as<xstd::bit_enum_set<perm>, xstd::bit_least<xstd::basic_bit_fixed_set<perm, std::size_t, 3UZ>>>);
        static_assert(std::same_as<xstd::bit_enum_set<test::set::nine>, xstd::bit_least<xstd::basic_bit_fixed_set<test::set::nine, std::size_t, 9UZ>>>);

        auto const s = xstd::basic_bit_fixed_set<perm, std::uint32_t, 3UZ>{perm::exec, perm::read};
        BOOST_CHECK(std::ranges::equal(s, std::set<perm>{perm::read, perm::exec}));
}

// bit_least takes a mapping of its own too: five workdays from mon in one byte, the weekend no key.
BOOST_AUTO_TEST_CASE(BitLeastTakesAnEnumerationUnderAMappingOfItsOwn)
{
        using workdays = xstd::bit_range_mapping<weekday, weekday::mon, 5UZ>;
        using X        = xstd::bit_least<xstd::basic_bit_fixed_set<weekday, std::size_t, 5UZ, workdays>>;
        static_assert(std::same_as<X, xstd::basic_bit_fixed_set<weekday, std::uint8_t, 5UZ, workdays>>);
        static_assert(std::same_as<X::key_mapping_type, workdays>);
        static_assert(sizeof(X) == 1UZ);
        static_assert(X::max_size() == 5UZ);

        auto x = X{weekday::fri, weekday::mon};
        BOOST_CHECK(std::ranges::equal(x, std::set<weekday>{weekday::mon, weekday::fri}));
        BOOST_CHECK(std::ranges::equal(~x, std::set<weekday>{weekday::tue, weekday::wed, weekday::thu}));
        BOOST_CHECK(not x.contains(weekday::sat));
        BOOST_CHECK_THROW(static_cast<void>(x.insert(weekday::sun)), std::out_of_range);
        BOOST_CHECK(std::ranges::equal(x, std::set<weekday>{weekday::mon, weekday::fri}));
}

// A braced list of enumerators deduces the alias's type, with no mapping or block spelled.
BOOST_AUTO_TEST_CASE(ABracedListOfEnumeratorsDeducesTheEnumSet)
{
        using test::set::perm;
        using test::set::piece;
        auto const s = xstd::basic_bit_fixed_set{perm::read, perm::write};
        static_assert(std::same_as<decltype(s), xstd::bit_enum_set<perm> const>);
        static_assert(std::same_as<decltype(xstd::basic_bit_fixed_set{piece::king}), xstd::bit_enum_set<piece>>);
        BOOST_CHECK(std::ranges::equal(s, std::set<perm>{perm::read, perm::write}));

        // The other guides are left as they were: a copy deduces the copy's type, and blocks a width.
        static_assert(std::same_as<decltype(xstd::basic_bit_fixed_set{s}), xstd::bit_enum_set<perm>>);
        static_assert(std::same_as<decltype(xstd::basic_bit_fixed_set(xstd::from_blocks, std::uint8_t())), xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 8UZ>>);
}

// A specialized default mapping makes an enum set without any list of values.
BOOST_AUTO_TEST_CASE(ASpecializedMappingNeedsNoListOfValues)
{
        static_assert(std::same_as<xstd::bit_enum_set<weekday>, xstd::basic_bit_fixed_set<weekday, std::uint8_t, 7UZ, xstd::bit_key_mapping<weekday>>>);
        auto const s = xstd::basic_bit_fixed_set{weekday::sun, weekday::mon, weekday::wed};
        static_assert(std::same_as<decltype(s), xstd::bit_enum_set<weekday> const>);
        BOOST_CHECK(std::ranges::equal(s, std::set<weekday>{weekday::mon, weekday::wed, weekday::sun}));
        BOOST_CHECK_EQUAL((~s).size(), 4UZ);
}

// Gaps cost no bit: the universe is the six pieces, and the complement of none is all six.
BOOST_AUTO_TEST_CASE(TheUniverseIsTheListedValuesWithoutTheGaps)
{
        using test::set::piece;
        using X = xstd::bit_enum_set<piece>;
        static_assert(X::max_size() == 6UZ);
        static_assert(sizeof(X) == 1UZ);
        BOOST_CHECK(std::ranges::equal(~X(), xstd::enum_traits<piece>::values));

        // A value in a gap is in no set, and one cannot be put there.
        constexpr auto gap = std::bit_cast<piece>(std::uint8_t{2});
        auto x             = X{piece::pawn};
        BOOST_CHECK(not x.contains(gap));
        BOOST_CHECK_THROW(static_cast<void>(x.insert(gap)), std::out_of_range);
        BOOST_CHECK_THROW(x |= gap, std::out_of_range);
        BOOST_CHECK(std::ranges::equal(x, std::set<piece>{piece::pawn}));
}

// The mapping defaults for a listed enumeration, so the basic form with no mapping spelled keys it the same way.
BOOST_AUTO_TEST_CASE(TheBasicFormDefaultsToTheEnumerationsRanks)
{
        using test::set::piece;
        using X = xstd::basic_bit_fixed_set<piece, std::uint8_t, 6UZ>;
        static_assert(std::same_as<X::key_mapping_type, xstd::bit_key_mapping<piece>>);
        auto const x = X{piece::king, piece::pawn, piece::rook};
        BOOST_CHECK(std::ranges::equal(x, std::set<piece>{piece::pawn, piece::rook, piece::king}));
}

// Every subset of every listed enumeration, ascending as std::set<E> and descending as std::set<E, std::greater<>>.
BOOST_AUTO_TEST_CASE_TEMPLATE(EverySubsetAgreesWithStdSet, E, test::set::listed_enums)
{
        agrees_with_std_set_on_every_subset<xstd::bit_enum_set<E>, std::set<E>>();
        agrees_with_std_set_on_every_subset<descending_set<E>, std::set<E, std::greater<>>>();
}

// disjoint is none-of and is_superset_of all-of, over every pair of subsets of every enumeration of at most six values.
BOOST_AUTO_TEST_CASE_TEMPLATE(DisjointIsNotIntersectsAndTheSupersetIsTheSubsetReadTheOtherWay, E, test::set::listed_enums)
{
        if constexpr (std::ranges::size(xstd::enum_traits<E>::values) <= 6UZ) {
                BOOST_CHECK_EQUAL((query_mismatches_over_pairs<xstd::bit_enum_set<E>, std::set<E>>()), 0UZ);
                BOOST_CHECK_EQUAL((query_mismatches_over_pairs<descending_set<E>, std::set<E, std::greater<>>>()), 0UZ);
        } else {
                auto const none = xstd::bit_enum_set<E>();
                auto const all  = ~none;
                BOOST_CHECK(not xstd::bit_includes(none, all) and xstd::bit_includes(all, none) and xstd::bit_disjoint(none, all));
        }
}

// The block's padding stays out of the set: five values in eight bits, eight in eight, nine in sixteen.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheComplementHoldsNoPaddingBit, E, test::set::listed_enums)
{
        holds_the_listed_values_and_no_padding<xstd::bit_enum_set<E>>();
        holds_the_listed_values_and_no_padding<descending_set<E>>();
}

// The enumeration has no operators, an integer is no key, and an integer-keyed set meets no key as a set.
BOOST_AUTO_TEST_CASE(NeitherTwoEnumeratorsNorAnIntegerCombine)
{
        using test::set::perm;
        static_assert(not enumerators_combine<perm>);
        static_assert(not takes_as_key<xstd::bit_enum_set<perm>, int>);
        static_assert(not takes_as_key<xstd::bit_enum_set<perm>, std::size_t>);
        static_assert(not takes_as_key<xstd::bit_enum_set<perm>, test::set::piece>);
        static_assert(takes_as_key<xstd::bit_enum_set<perm>, perm>);
        static_assert(meets_a_key<xstd::bit_enum_set<perm>, perm>);
        static_assert(not meets_a_key<xstd::bit_enum_set<perm>, int>);
        static_assert(not meets_a_key<xstd::bit_fixed_set<8>, std::size_t>);

        BOOST_CHECK(true);
}

// The set prints each enumerator as its own formatter does, in the set's order.
BOOST_AUTO_TEST_CASE(AnEnumSetFormatsItsEnumeratorsByName)
{
        using test::set::piece;
        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_enum_set<piece>{piece::king, piece::pawn, piece::bishop}), "{pawn, bishop, king}");
        BOOST_CHECK_EQUAL(std::format("{}", descending_set<piece>{piece::king, piece::pawn}), "{king, pawn}");
        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_enum_set<piece>()), "{}");
}

// The element proxy is a proxy reference, so the qualified to_underlying reads the key through it in either order.
BOOST_AUTO_TEST_CASE(ToUnderlyingReadsTheKeyThroughTheProxy)
{
        using test::set::piece;
        using underlying = std::underlying_type_t<piece>;
        static_assert(xstd::proxy_iterator<std::ranges::iterator_t<xstd::bit_enum_set<piece>>>);
        static_assert(xstd::proxy_reference<std::ranges::range_reference_t<xstd::bit_enum_set<piece>>>);
        static_assert(std::same_as<decltype(xstd::to_underlying(*xstd::bit_enum_set<piece>().begin())), underlying>);
        static_assert([] -> underlying {
                auto const s = xstd::bit_enum_set<piece>{piece::king};
                return xstd::to_underlying(*s.begin());
        }() == xstd::to_underlying(piece::king));

        auto const keys = std::vector{piece::pawn, piece::bishop, piece::king};
        auto expected   = std::vector<underlying>();
        for (auto const key : keys) {
                expected.push_back(xstd::to_underlying(key));
        }
        auto const ascending = xstd::bit_enum_set<piece>{piece::king, piece::pawn, piece::bishop};
        auto seen            = std::vector<underlying>();
        for (auto const key : ascending) {
                seen.push_back(xstd::to_underlying(key));
        }
        BOOST_CHECK(seen == expected);

        auto const descending = descending_set<piece>{piece::king, piece::pawn, piece::bishop};
        seen.clear();
        for (auto const key : descending) {
                seen.push_back(xstd::to_underlying(key));
        }
        BOOST_CHECK(std::ranges::equal(seen, expected | std::views::reverse));

        // A std::size_t key's proxy and a sequence's bool proxy have no underlying value to give.
        static_assert(takes_to_underlying<std::ranges::range_reference_t<xstd::bit_enum_set<piece>>>);
        static_assert(not takes_to_underlying<std::ranges::range_reference_t<xstd::bit_fixed_set<8>>>);
        static_assert(not takes_to_underlying<std::ranges::range_reference_t<xstd::bit_array<8>>>);
}

BOOST_AUTO_TEST_SUITE_END()
