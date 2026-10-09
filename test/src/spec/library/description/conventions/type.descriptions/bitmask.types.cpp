//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/bitmask.hpp>    // all, elements, for_each_pair, masks, union_but, values, word, word_t, zero
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                   // bitset
#include <concepts>                 // integral, same_as
#include <cstddef>                  // size_t
#include <memory>                   // addressof
#include <ranges>                   // iota
#include <type_traits>              // is_enum_v, is_lvalue_reference_v, remove_cvref_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Library)
BOOST_AUTO_TEST_SUITE(Description)
BOOST_AUTO_TEST_SUITE(Conventions)
BOOST_AUTO_TEST_SUITE(TypeDescriptions)
BOOST_AUTO_TEST_SUITE(BitmaskTypes)

using test::spec::bitmask::for_each_pair;
using test::spec::bitmask::union_but;
using test::spec::bitmask::word;
using test::spec::bitmask::word_t;
using test::spec::bitmask::zero;
namespace inputs = test::spec::bitmask::inputs;

namespace {

template<class X>
inline constexpr bool is_bitset = false;

template<std::size_t N>
inline constexpr bool is_bitset<std::bitset<N>> = true;

// The lvalue a compound assignment returns: [bitmask.types]/2 shows bitmask&, which libstdc++'s streams make const.
template<class R, class X>
concept lvalue_of = std::is_lvalue_reference_v<R> and std::same_as<std::remove_cvref_t<R>, X>;

// [bitmask.types]/4: the value y is set in the object x if x & y is nonzero.
template<class X>
[[nodiscard]] auto is_set(X const& x, X const& y)
        -> bool
{
        return (x & y) != zero<X>();
}

} // namespace

// [bitmask.types]/1: an enumerated type that overloads certain operators, an integer type, or a bitset
BOOST_AUTO_TEST_CASE(Bitmask)
{
        // How the library may implement its own types; a flag type is a class, held to the rest by its operators.
        test::for_each_type<test::spec::bitmask::masks>([]<class C> -> void {
                using X = C::type;
                static_assert(std::is_enum_v<X> or std::integral<X> or is_bitset<X>); // [bitmask.types]/1
        });
        BOOST_CHECK(true);
}

// [bitmask.types]/2: constexpr bitmask operator&(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitAnd)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x & y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x & y) == static_cast<word_t<C>>(word<C>(x) & word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator|(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitOr)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x | y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x | y) == static_cast<word_t<C>>(word<C>(x) | word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator^(bitmask X, bitmask Y);
BOOST_AUTO_TEST_CASE(BitXor)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x, X const y) { { x ^ y } -> std::same_as<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        BOOST_CHECK(word<C>(x ^ y) == static_cast<word_t<C>>(word<C>(x) ^ word<C>(y))); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: constexpr bitmask operator~(bitmask X);
BOOST_AUTO_TEST_CASE(Complement)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X const x) { { ~x } -> std::same_as<X>; }); // [bitmask.types]/2
                // The empty value's complement holds the bits of int_type: every element, and what else a type keeps.
                auto const universe = word<C>(~zero<X>());
                for (auto const& e : inputs::elements<C>()) {
                        BOOST_CHECK(static_cast<word_t<C>>(universe & word<C>(e)) == word<C>(e)); // [bitmask.types]/2
                }
                for (auto const& x : inputs::values<C>()) {
                        BOOST_CHECK(word<C>(~x) == static_cast<word_t<C>>(static_cast<word_t<C>>(~word<C>(x)) & universe)); // [bitmask.types]/2
                }
        });
}

// [bitmask.types]/2: bitmask& operator&=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(AndAssign)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x &= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z &= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x & y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: bitmask& operator|=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(OrAssign)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x |= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z |= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x | y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/2: bitmask& operator^=(bitmask& X, bitmask Y);
BOOST_AUTO_TEST_CASE(XorAssign)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                static_assert(requires (X x, X const y) { { x ^= y } -> lvalue_of<X>; }); // [bitmask.types]/2
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        auto z        = x;
                        auto const& r = (z ^= y);
                        BOOST_CHECK(std::addressof(r) == std::addressof(z) and z == (x ^ y)); // [bitmask.types]/2
                });
        });
}

// [bitmask.types]/3: inline constexpr bitmask C0(V0); and the other bitmask elements
BOOST_AUTO_TEST_CASE(Elements)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X      = C::type;
                auto const e = inputs::elements<C>();
                for (auto const i : std::views::iota(0UZ, e.size())) {
                        BOOST_CHECK(e[i] != zero<X>() and (e[i] & e[i]) != zero<X>()); // [bitmask.types]/3
                        BOOST_CHECK((zero<X>() & e[i]) == zero<X>());                  // [bitmask.types]/3
                        for (auto const j : std::views::iota(0UZ, e.size())) {
                                BOOST_CHECK(i == j or (e[i] != e[j] and (e[i] & e[j]) == zero<X>())); // [bitmask.types]/3
                        }
                }
                for (auto const z : C::names::empties) {
                        BOOST_CHECK(X(z) == zero<X>()); // [bitmask.types]/3
                }
        });
}

// [bitmask.types]/4: to set a value Y in an object X is to evaluate the expression X |= Y
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        auto z = x;
                        z |= y;
                        BOOST_CHECK(y == zero<X>() or is_set(z, y)); // [bitmask.types]/4
                        for (auto const& f : inputs::elements<C>()) {
                                BOOST_CHECK(is_set(z, f) == (is_set(x, f) or is_set(y, f))); // [bitmask.types]/4
                        }
                });
        });
}

// [bitmask.types]/4: to clear a value Y in an object X is to evaluate the expression X &= ~Y
BOOST_AUTO_TEST_CASE(Clear)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X = C::type;
                for_each_pair<C>(inputs::values<C>(), [](X const& x, X const& y) -> void {
                        auto z = x;
                        z &= ~y;
                        BOOST_CHECK(not is_set(z, y)); // [bitmask.types]/4
                        for (auto const& f : inputs::elements<C>()) {
                                BOOST_CHECK(is_set(z, f) == (is_set(x, f) and not is_set(y, f))); // [bitmask.types]/4
                        }
                });
        });
}

// [bitmask.types]/4: the value Y is set in the object X if the expression X & Y is nonzero
BOOST_AUTO_TEST_CASE(IsSet)
{
        test::for_each_type<test::spec::bitmask::all>([]<class C> -> void {
                using X           = C::type;
                auto const e      = inputs::elements<C>();
                auto const in_all = union_but(e, e.size());
                for (auto const i : std::views::iota(0UZ, e.size())) {
                        auto const but_one = union_but(e, i);
                        for (auto const j : std::views::iota(0UZ, e.size())) {
                                BOOST_CHECK(is_set(e[i], e[j]) == (i == j));    // [bitmask.types]/4
                                BOOST_CHECK(is_set(but_one, e[j]) == (i != j)); // [bitmask.types]/4
                        }
                        BOOST_CHECK(is_set(in_all, e[i]) and not is_set(zero<X>(), e[i])); // [bitmask.types]/4
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
