//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // op_bit_and, op_bit_or, op_bit_xor, op_iostream, op_istream_failure
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, bitsets, pairs
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(Operators)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

// [bitset.operators]/1: constexpr bitset<N> operator&(const bitset<N>& lhs, const bitset<N>& rhs) noexcept;
BOOST_AUTO_TEST_CASE(And)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_bit_and()(a, b);
                }
        });
}

// [bitset.operators]/2: constexpr bitset<N> operator|(const bitset<N>& lhs, const bitset<N>& rhs) noexcept;
BOOST_AUTO_TEST_CASE(Or)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_bit_or()(a, b);
                }
        });
}

// [bitset.operators]/3: constexpr bitset<N> operator^(const bitset<N>& lhs, const bitset<N>& rhs) noexcept;
BOOST_AUTO_TEST_CASE(Xor)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_bit_xor()(a, b);
                }
        });
}

// [bitset.operators]/4-8: operator>>(basic_istream& is, bitset<N>& x), operator<<(basic_ostream& os, const bitset<N>&)
BOOST_AUTO_TEST_CASE(ExtractInsert)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // A read that stores nothing fails; boost::dynamic_bitset's own extraction is not checked.
                op_istream_failure<T>()();
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        op_iostream()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
