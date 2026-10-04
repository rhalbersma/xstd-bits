//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>     // exchanges_from_bits
#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // make_sequence, stripes
#include <test/spec/rejection.hpp>   // covers_static_width_v
#include <test/spec/sequence.hpp>    // array_all
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstdint>                   // uint8_t
#include <type_traits>               // is_trivially_copy_assignable_v, is_trivially_copy_constructible_v, is_trivially_destructible_v, is_trivially_move_assignable_v, is_trivially_move_constructible_v
#include <utility>                   // move

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;

// [array.cons]/1: the implicitly-declared special member functions
BOOST_AUTO_TEST_CASE(SpecialMemberFunctions)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // Implicit members over bools are trivial, which a user-provided one never is.
                static_assert(std::is_trivially_copy_constructible_v<T> and std::is_trivially_move_constructible_v<T>); // [array.cons]/1
                static_assert(std::is_trivially_copy_assignable_v<T> and std::is_trivially_move_assignable_v<T>);       // [array.cons]/1
                static_assert(std::is_trivially_destructible_v<T>);                                                     // [array.cons]/1
                auto const a = make_sequence<T>(T().size(), stripes);
                auto b       = a;
                auto const c = std::move(b);
                BOOST_CHECK(c == a); // [array.cons]/1
        });
}

// xstd array: template<class Bits> constexpr X(from_blocks_t, const Bits& b) noexcept;
BOOST_AUTO_TEST_CASE(ArrayFromBlocks)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // An integer is read as bools only where it covers the width, by this library's design.
                static_assert(test::exchanges_from_bits<T, std::uint8_t> == test::spec::covers_static_width_v<T, std::uint8_t>);
                static_assert(test::exchanges_from_bits<T, unsigned long long> == test::spec::covers_static_width_v<T, unsigned long long>);
                BOOST_CHECK(true);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
