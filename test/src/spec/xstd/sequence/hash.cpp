//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>            // for_each_type
#include <test/inplace_vector.hpp>           // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/spec/hash.hpp>                // check_hashes_as
#include <test/spec/input.hpp>               // context
#include <test/spec/sequence.hpp>            // array_all, inplace_vector_all, sequences, vector_all
#include <boost/container/static_vector.hpp> // static_vector
#include <boost/container/vector.hpp>        // vector
#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <algorithm>                         // copy
#include <array>                             // array
#include <tuple>                             // tuple_size_v
#include <vector>                            // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Sequence)
BOOST_AUTO_TEST_SUITE(Hash)

using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// xstd sequence: void tag_invoke(hash_append_tag, const Provider&, Hash&, const Flavor&, const X*);
BOOST_AUTO_TEST_CASE(HashAppend)
{
        // std::array<bool, N>'s message: a byte of 0 or 1 per position, no size, and one '\x00' where there is none.
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto model            = std::array<bool, std::tuple_size_v<T>>();
                        std::ranges::copy(a, model.begin());
                        test::spec::check_hashes_as(a, model);
                }
        });

        // std::vector<bool>'s, then its size; Boost's vector<bool> is not specialized and writes the same bytes.
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        test::spec::check_hashes_as(a, std::vector<bool>(a.begin(), a.end()));
                        test::spec::check_hashes_as(a, boost::container::vector<bool>(a.begin(), a.end()));
                }
        });

        // std::inplace_vector<bool, N>'s, std::vector<bool>'s again, and Boost's static_vector<bool, N> as well.
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        test::spec::check_hashes_as(a, boost::container::static_vector<bool, T::capacity()>(a.begin(), a.end()));
#ifdef TEST_HAS_INPLACE_VECTOR
                        test::spec::check_hashes_as(a, std::inplace_vector<bool, T::capacity()>(a.begin(), a.end()));
#endif
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
