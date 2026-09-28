//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // model_of
#include <test/spec/input.hpp>       // context
#include <test/spec/sequence.hpp>    // sequences, vector_all
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <vector>                    // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [vector.cons]/1-2: constexpr explicit vector(const Allocator&) noexcept;
BOOST_AUTO_TEST_CASE(VectorAllocator)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                // The default constructor delegates to this one.
                BOOST_CHECK(T().empty()); // [vector.cons]/1
        });
}

// [vector.cons]/3-5: constexpr explicit vector(size_type n, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(VectorCount)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                // n default-inserted bools, each of them false.
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(model_of(T(a.size())) == std::vector<bool>(a.size())); // [vector.cons]/4
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
