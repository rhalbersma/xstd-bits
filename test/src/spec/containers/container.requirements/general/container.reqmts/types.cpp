//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // nested_types
#include <test/set/primitives.hpp>      // mem_const_reference, nested_types
#include <test/spec/container.hpp>      // all, keyed
#include <test/spec/input.hpp>          // context
#include <test/spec/set.hpp>            // sets
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL_COLLECTIONS
#include <algorithm>                    // copy
#include <cstddef>                      // size_t
#include <iterator>                     // inserter
#include <set>                          // set

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Types)

using test::spec::context;

namespace {

class Implicit
{
        std::size_t m_value;

public:
        [[nodiscard]] constexpr explicit(false) Implicit(std::size_t v) noexcept
                : m_value(v)
        {}

        // Implicit is the point: this class exists to convert both ways without a cast.
        [[nodiscard]] constexpr explicit(false) operator std::size_t() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return m_value;
        }
};

} // namespace

// [container.reqmts]/2-9: value_type, reference, const_reference, iterator, const_iterator, difference_type, size_type
BOOST_AUTO_TEST_CASE(NestedTypes)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                // A set's const_reference is a key it holds, converting to a key type constructible from it.
                if constexpr (test::spec::container::keyed<T>) {
                        test::set::nested_types<T>();
                        for (auto const [from, a] : test::spec::set::inputs::sets<T>()) {
                                auto const on_failure = context(from, a);
                                test::set::mem_const_reference()(a);
                                std::set<Implicit> dst;
                                std::ranges::copy(a, std::inserter(dst, dst.end()));
                                BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), dst.begin(), dst.end());
                        }
                } else {
                        test::sequence::nested_types<T>();
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
