//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // model_of
#include <test/spec/input.hpp>       // context
#include <test/spec/sequence.hpp>    // sequences, vector_all
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <initializer_list>          // initializer_list
#include <ranges>                    // from_range
#include <vector>                    // vector
#include <version>                   // IWYU pragma: keep; __cpp_lib_containers_ranges

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [vector.cons]/1: constexpr explicit vector(const Allocator&) noexcept;
BOOST_AUTO_TEST_CASE(VectorAllocator)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                auto const m = typename T::allocator_type();
                auto const u = T(m);
                BOOST_CHECK(u.empty() and u.get_allocator() == m); // [vector.cons]/1
        });
}

// [vector.cons]/4: constexpr explicit vector(size_type n, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(VectorCount)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T::size_type n, T::allocator_type a) {
                        T(n);
                        T(n, a);
                });
                // n default-inserted bools, each of them false.
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(model_of(T(a.size())) == std::vector<bool>(a.size())); // [vector.cons]/4
                }
        });
}

// [vector.cons]/7: constexpr vector(size_type n, const T& value, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(VectorCountValue)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T::size_type n, bool b, T::allocator_type a) { T(n, b, a); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        BOOST_CHECK(model_of(T(a.size(), true, typename T::allocator_type())) == std::vector<bool>(a.size(), true)); // [vector.cons]/7
                }
        });
}

// [vector.cons]/9: vector(InputIterator first, InputIterator last, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(VectorFirstLast)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (bool const* first, bool const* last, T::allocator_type a) { T(first, last, a); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto const in = model_of(a);
                        BOOST_CHECK(model_of(T(in.begin(), in.end(), typename T::allocator_type())) == in); // [vector.cons]/9
                }
        });
}

// [vector.cons]/11: vector(from_range_t, R&& rg, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(VectorFromRange)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (std::initializer_list<bool> il, T::allocator_type a) { T(std::from_range, il, a); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto const in = model_of(a);
                        BOOST_CHECK(model_of(T(std::from_range, in, typename T::allocator_type())) == in); // [vector.cons]/11
                }
#endif
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
