//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // fn_erase_if
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, sets
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                  // size_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Erasure)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

namespace {

// Nothing, everything, and every other key, so a removal can be none, all, or interleaved with what stays.
auto const never = [](std::size_t) -> bool { return false; };
auto const always = [](std::size_t) -> bool { return true; };
auto const odd = [](std::size_t x) -> bool { return x % 2 == 1; };

} // namespace

// [set.erasure]/1: set<Key, Compare, Allocator>::size_type erase_if(set<Key, Compare, Allocator>& c, Predicate pred);
BOOST_AUTO_TEST_CASE(EraseIf)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        fn_erase_if()(a, never);
                        fn_erase_if()(a, always);
                        fn_erase_if()(a, odd);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
