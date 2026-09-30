//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // op_hash
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, pairs, sets
#include <test/spec/view.hpp>       // subject
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Hash)

using namespace test::set;
using test::spec::context;
using test::spec::subject;
namespace inputs = test::spec::set::inputs;

// xstd set: template<> struct hash<X>;
BOOST_AUTO_TEST_CASE(Hash)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // std::set has no std::hash, so the models pass vacuously; the rest are checked, a view as itself.
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        op_hash()(subject(a));
                }
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_hash()(subject(a), subject(b));
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
