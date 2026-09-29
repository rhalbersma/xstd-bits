//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // heterogeneous_key
#include <test/spec/set.hpp>        // all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Modifiers)

// [set.modifiers]/1: template<class K> pair<iterator, bool> insert(K&& x); iterator insert(const_iterator hint, K&& x);
BOOST_AUTO_TEST_CASE(InsertHeterogeneous)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // Each key_compare is std::less<key_type>, with no is_transparent to let a key of another type in.
                using K = test::set::heterogeneous_key;
                static_assert(not requires (T a, K k) { a.insert(k); });             // [set.modifiers]/1
                static_assert(not requires (T a, K k) { a.insert(a.cbegin(), k); }); // [set.modifiers]/1
                BOOST_CHECK(true);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
