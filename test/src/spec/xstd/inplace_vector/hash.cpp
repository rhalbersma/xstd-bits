//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/reference.hpp>       // proxy_reference
#include <test/set/primitives.hpp>  // op_hash
#include <test/spec/input.hpp>      // context
#include <test/spec/sequence.hpp>   // inplace_vector_all, pairs, sequences
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                 // same_as
#include <cstddef>                  // size_t
#include <functional>               // hash

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Hash)

using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// xstd inplace_vector: template<> struct hash<X>;
BOOST_AUTO_TEST_CASE(Hash)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // [vector.bool.pspc]'s hash comes with a proxy reference, which std::inplace_vector<bool, N> lacks.
                static_assert(test::proxy_reference<T> == requires (T const cc) { { std::hash<T>()(cc) } -> std::same_as<std::size_t>; });
                if constexpr (test::proxy_reference<T>) {
                        for (auto const [from, a] : inputs::sequences<T>()) {
                                auto const on_failure = context(from, a);
                                test::set::op_hash()(a);
                        }
                        for (auto const [from, a, b] : inputs::pairs<T>()) {
                                auto const on_failure = context(from, a, b);
                                test::set::op_hash()(a, b);
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
