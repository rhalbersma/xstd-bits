//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitmask.hpp>      // flag_set, for_each_pair, masks, named_values, word, word_t, zero
#include <xstd/bits/bit_flag_set.hpp> // bit_flag_set
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                   // convertible_to

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(FlagSet)
BOOST_AUTO_TEST_SUITE(Mask)

using test::spec::bitmask::flag_set;
using test::spec::bitmask::for_each_pair;
using test::spec::bitmask::word;
using test::spec::bitmask::word_t;
using test::spec::bitmask::zero;
namespace inputs = test::spec::bitmask::inputs;

namespace {

// A flag type's operators, on two flag types or on one and a mask, against the mask's own.
template<class C>
auto operators_agree(typename C::type const& a, typename C::type const& b)
        -> void
{
        using T = C::type;
        using F = flag_set<typename C::names>;
        using S = F::type;
        BOOST_CHECK(static_cast<T>(S(a) & S(b)) == (a & b) and static_cast<T>(S(a) & b) == (a & b) and static_cast<T>(a & S(b)) == (a & b));
        BOOST_CHECK(static_cast<T>(S(a) | S(b)) == (a | b) and static_cast<T>(S(a) | b) == (a | b) and static_cast<T>(a | S(b)) == (a | b));
        BOOST_CHECK(static_cast<T>(S(a) ^ S(b)) == (a ^ b) and static_cast<T>(S(a) ^ b) == (a ^ b) and static_cast<T>(a ^ S(b)) == (a ^ b));

        auto x = S(a);
        auto y = S(a);
        BOOST_CHECK(static_cast<T>(x &= S(b)) == (a & b) and static_cast<T>(y &= b) == (a & b));
        x = S(a);
        y = S(a);
        BOOST_CHECK(static_cast<T>(x |= S(b)) == (a | b) and static_cast<T>(y |= b) == (a | b));
        x = S(a);
        y = S(a);
        BOOST_CHECK(static_cast<T>(x ^= S(b)) == (a ^ b) and static_cast<T>(y ^= b) == (a ^ b));

        // The complements agree on every bit that both of them complement.
        auto const both = static_cast<word_t<C>>(word<C>(~zero<T>()) & word<F>(~zero<S>()));
        BOOST_CHECK(static_cast<word_t<C>>(word<F>(~S(a)) & both) == static_cast<word_t<C>>(word<C>(~a) & both));
}

} // namespace

// xstd flag set: constexpr X(const Mask& mask) noexcept; constexpr operator Mask() const noexcept;
BOOST_AUTO_TEST_CASE(MaskConversion)
{
        test::for_each_type<test::spec::bitmask::masks>([]<class C> -> void {
                using T = C::type;
                using S = xstd::bit_flag_set<T>;
                static_assert(std::convertible_to<T, S> and std::convertible_to<S, T>);
                for (auto const& a : inputs::named_values<C>()) {
                        S const s = a;
                        BOOST_CHECK(static_cast<T>(s) == a and s == a);
                        BOOST_CHECK(word<flag_set<typename C::names>>(s) == word<C>(a));
                }
        });
}

// xstd flag set: constexpr X operator&(const X& lhs, const Mask& rhs) noexcept; and fifteen more
BOOST_AUTO_TEST_CASE(MaskOperators)
{
        test::for_each_type<test::spec::bitmask::masks>([]<class C> -> void {
                using T = C::type;
                for_each_pair<C>(inputs::named_values<C>(), [](T const& a, T const& b) -> void { operators_agree<C>(a, b); });
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
