//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // mem_erase_keeps_prefix, mem_insert_keeps_prefix
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // positions, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Modifiers)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [vector.modifiers]/2: constexpr iterator insert(const_iterator position, const T& x);
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert_keeps_prefix()(a, p);
                }
        });
}

// [vector.modifiers]/4-5: constexpr iterator erase(const_iterator position);
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_erase_keeps_prefix()(a, p);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
