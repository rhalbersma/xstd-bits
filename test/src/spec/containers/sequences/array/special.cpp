//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // model_of
#include <test/spec/input.hpp>       // context
#include <test/spec/sequence.hpp>    // array_all, pairs
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <type_traits>               // is_swappable_v

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Special)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [array.special]/1-2: template<class T, size_t N> constexpr void swap(array<T, N>& x, array<T, N>& y) noexcept(...);
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(std::is_swappable_v<T>); // [array.special]/1
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto x = a;
                        auto y = b;
                        auto x1 = a;
                        auto y1 = b;
                        swap(x, y);
                        x1.swap(y1);
                        BOOST_CHECK(x == x1 and y == y1); // [array.special]/2
                        BOOST_CHECK(model_of(x) == model_of(b) and model_of(y) == model_of(a));
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
