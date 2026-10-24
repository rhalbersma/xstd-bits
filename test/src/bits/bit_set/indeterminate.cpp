//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>                            // day, wind
#include <xstd/bits/bit_array.hpp>                       // bit_array
#include <xstd/bits/bit_set/bit_enum_set.hpp>            // bit_enum_set
#include <xstd/bits/bit_set/bit_fixed_set.hpp>           // bit_fixed_set
#include <xstd/bits/bit_set/bit_inplace_set.hpp>         // bit_inplace_set
#include <xstd/bits/bit_set/bit_set.hpp>                 // bit_set
#include <xstd/bits/bit_set/indeterminate.hpp>           // XSTD_INDETERMINATE, indeterminate, indeterminate_array, indeterminate_t
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_valarray.hpp>       // bit_valarray
#include <xstd/bits/bit_vector/bit_vector.hpp>           // bit_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                         // array
#include <concepts>                                      // same_as
#include <cstddef>                                       // size_t
#include <tuple>                                         // tuple
#include <type_traits>                                   // is_constructible_v, is_convertible_v, is_default_constructible_v, is_nothrow_constructible_v

BOOST_AUTO_TEST_SUITE(Indeterminate)

namespace {

// Declared only, for the concept below to call in an unevaluated operand.
template<class T>
[[maybe_unused]] auto from_braces(T)
        -> void;

template<class T>
concept braces_convert = requires { from_braces<T>({}); };

// The fixed sets: one whose width fills its blocks, one that pads its last block, and the enumerations' two kinds.
using FixedSets = std::tuple<xstd::bit_fixed_set<64>, xstd::bit_fixed_set<100>, xstd::bit_enum_set<test::set::wind>, xstd::bit_enum_set<test::set::day>>;

// Two keys of each set, to give it a value after the tag left it without one.
template<class S>
[[nodiscard]] constexpr auto two_keys()
        -> S
{
        using key_type = S::key_type;
        if constexpr (std::same_as<key_type, std::size_t>) {
                return S({3UZ, 60UZ});
        } else {
                return S({key_type{1}, key_type{4}});
        }
}

// A per-ply record a search fills before it reads: fields and an array of them, each built from the tag.
struct ply_state
{
        xstd::bit_fixed_set<64> checkers{xstd::indeterminate};
        std::array<xstd::bit_fixed_set<64>, 2> pinners = xstd::indeterminate_array<xstd::bit_fixed_set<64>, 2>();
};

} // namespace

// The tag is from_blocks' twin: an explicit default constructor, so {} cannot stand in for it.
BOOST_AUTO_TEST_CASE(TheTagIsExplicitlyDefaultConstructible)
{
        static_assert(std::is_default_constructible_v<xstd::indeterminate_t>);
        static_assert(not braces_convert<xstd::indeterminate_t>);
        static_assert(std::same_as<decltype(xstd::indeterminate), xstd::indeterminate_t const>);
        BOOST_CHECK(true);
}

// Built from the tag and then assigned, a fixed set holds what it was given, in a constant expression too.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnAssignmentGivesTheSetItsValue, S, FixedSets)
{
        static_assert(std::is_nothrow_constructible_v<S, xstd::indeterminate_t>);
        static_assert(not std::is_convertible_v<xstd::indeterminate_t, S>);
        static_assert([] {
                auto s = S(xstd::indeterminate);
                s      = two_keys<S>();
                return s == two_keys<S>();
        }());
        auto s = S(xstd::indeterminate);
        s      = two_keys<S>();
        BOOST_CHECK(s == two_keys<S>());
}

// The tag is the only way to an indeterminate set: a default or a value-initialized one is empty.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDefaultSetStaysEmpty, S, FixedSets)
{
        static_assert([] {
                S s;
                return s.empty();
        }());
        static_assert([] {
                S s;
                s.insert(two_keys<S>().front());
                return s.size() == 1UZ;
        }());
        S const s;
        BOOST_CHECK(s.empty() and S{}.empty());
}

// The other fixed owners take it too; a run-time width starts at width zero, with nothing to leave unwritten.
BOOST_AUTO_TEST_CASE(OnlyTheFixedOwnersTakeTheTag)
{
        static_assert(std::is_nothrow_constructible_v<xstd::bit_array<64>, xstd::indeterminate_t>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_fixed_valarray<64>, xstd::indeterminate_t>);
        static_assert(not std::is_convertible_v<xstd::indeterminate_t, xstd::bit_array<64>>);
        static_assert(not std::is_convertible_v<xstd::indeterminate_t, xstd::bit_fixed_valarray<64>>);
        static_assert(not std::is_constructible_v<xstd::bit_set, xstd::indeterminate_t>);
        static_assert(not std::is_constructible_v<xstd::bit_inplace_set<64>, xstd::indeterminate_t>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::indeterminate_t>);
        static_assert(not std::is_constructible_v<xstd::bit_valarray, xstd::indeterminate_t>);
        BOOST_CHECK(true);
}

// Built from the tag and then assigned, a fixed array and a fixed valarray hold what they were given.
BOOST_AUTO_TEST_CASE(AnAssignmentGivesTheArraysTheirValue)
{
        static_assert([] {
                auto a = xstd::bit_array<100>(xstd::indeterminate);
                a      = xstd::bit_array<100>({true, false, true});
                auto v = xstd::bit_fixed_valarray<100>(xstd::indeterminate);
                v      = xstd::bit_fixed_valarray<100>({false, true});
                return a[2] and not a[1] and v[1] and not v[0];
        }());
        auto a = xstd::indeterminate_array<xstd::bit_array<100>, 2>();
        a.fill(xstd::bit_array<100>({true}));
        BOOST_CHECK(a[1][0] and not a[1][1]);
}

// Each element is built from the tag in place, and each takes the value its assignment gives it.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnIndeterminateArrayIsAssignedElementByElement, S, FixedSets)
{
        static_assert(std::same_as<decltype(xstd::indeterminate_array<S, 3>()), std::array<S, 3>>);
        static_assert(noexcept(xstd::indeterminate_array<S, 3>()));
        static_assert(xstd::indeterminate_array<S, 0>().empty());
        static_assert([] {
                auto a = xstd::indeterminate_array<S, 3>();
                for (auto& s : a) {
                        s = two_keys<S>();
                }
                return a[0] == two_keys<S>() and a[2] == two_keys<S>();
        }());
        auto a = xstd::indeterminate_array<S, 3>();
        a.fill(two_keys<S>());
        BOOST_CHECK(a[1] == two_keys<S>());
}

// The record a hot path declares on its frame, every field written before it is read, as the tag requires.
BOOST_AUTO_TEST_CASE(AStructOfSetsFillsItsFieldsAfterTheTag)
{
        [[XSTD_INDETERMINATE]] auto state = ply_state();
        state.checkers                    = xstd::bit_fixed_set<64>({12UZ});
        state.pinners[0]                  = xstd::bit_fixed_set<64>({1UZ, 2UZ});
        state.pinners[1]                  = xstd::bit_fixed_set<64>();
        BOOST_CHECK(state.checkers.contains(12UZ) and state.pinners[0].size() == 2UZ and state.pinners[1].empty());
}

BOOST_AUTO_TEST_SUITE_END()
