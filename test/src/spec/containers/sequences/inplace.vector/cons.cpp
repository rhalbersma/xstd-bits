//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/factory.hpp>    // model_of
#include <test/sequence/primitives.hpp> // alternating
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // inplace_vector_all, sequences
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_THROW
#include <new>                          // bad_alloc
#include <ranges>                       // from_range
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [inplace.vector.cons]/1-3: constexpr explicit inplace_vector(size_type n);
BOOST_AUTO_TEST_CASE(InplaceVectorCount)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // n default-inserted bools, each of them false, and bad_alloc past the capacity.
                BOOST_CHECK_THROW(static_cast<void>(T(T::capacity() + 1UZ)), std::bad_alloc);
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(model_of(T(a.size())) == std::vector<bool>(a.size())); // [inplace.vector.cons]/2
                }
        });
}

// [inplace.vector.cons]/4-6: constexpr inplace_vector(size_type n, const T& value);
BOOST_AUTO_TEST_CASE(InplaceVectorCountValue)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                BOOST_CHECK_THROW(static_cast<void>(T(T::capacity() + 1UZ, true)), std::bad_alloc);
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(model_of(T(a.size(), true)) == std::vector<bool>(a.size(), true)); // [inplace.vector.cons]/5
                }
        });
}

// [inplace.vector.cons]/7-8: template<class InputIterator> inplace_vector(InputIterator first, InputIterator last);
BOOST_AUTO_TEST_CASE(InplaceVectorFirstLast)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                auto const more = alternating(T::capacity() + 1UZ);
                BOOST_CHECK_THROW(static_cast<void>(T(more.begin(), more.end())), std::bad_alloc);
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto const in = model_of(a);
                        BOOST_CHECK(model_of(T(in.begin(), in.end())) == in); // [inplace.vector.cons]/7
                }
        });
}

// [inplace.vector.cons]/9-11: template<container-compatible-range<T> R> constexpr inplace_vector(from_range_t, R&& rg);
BOOST_AUTO_TEST_CASE(InplaceVectorFromRange)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // The standard libraries without P1206R7 have no from_range constructor to check.
                if constexpr (requires { T(std::from_range, std::vector<bool>()); }) {
                        auto const more = alternating(T::capacity() + 1UZ);
                        BOOST_CHECK_THROW(static_cast<void>(T(std::from_range, more)), std::bad_alloc);
                        for (auto const [from, a] : inputs::sequences<T>()) {
                                auto const on_failure = context(from, a);
                                auto const in = model_of(a);
                                BOOST_CHECK(model_of(T(std::from_range, in)) == in); // [inplace.vector.cons]/10
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
