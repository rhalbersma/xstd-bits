//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // mem_static_swap
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // index_pairs, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Depr)
BOOST_AUTO_TEST_SUITE(VectorBoolSwap)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [depr.vector.bool.swap]/1-2: static constexpr void swap(reference x, reference y) noexcept;
BOOST_AUTO_TEST_CASE(StaticSwap)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n) { T::swap(c[n], c[n]); }); // [depr.vector.bool.swap]/1
                for (auto const [from, a, i, j] : inputs::index_pairs<T>()) {
                        auto const on_failure = context(from, a, i, j);
                        mem_static_swap()(a, i, j);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
