//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>    // for_each_type
#include <test/sequence/factory.hpp> // model_of
#include <test/spec/input.hpp>       // context
#include <test/spec/sequence.hpp>    // array_all, pairs, sequences
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                  // same_as
#include <vector>                    // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Members)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

namespace {

struct mem_size
{
        template<class X>
        auto operator()(X const& a) const
        {
                static_assert(std::same_as<decltype(a.size()), typename X::size_type>);
                BOOST_CHECK_EQUAL(a.size(), X().size()); // [array.members]/1
        }
};

// Every position the one value, whichever it was before.
struct mem_fill
{
        template<class X>
        auto operator()(X const& a) const
        {
                for (auto const u : {false, true}) {
                        auto b = a;
                        static_assert(std::same_as<decltype(b.fill(u)), void>);
                        b.fill(u);
                        BOOST_CHECK(model_of(b) == std::vector<bool>(a.size(), u)); // [array.members]/3
                }
        }
};

// Every position exchanged, as swap_ranges over the two would.
struct mem_swap
{
        template<class X>
        auto operator()(X const& a, X const& b) const
        {
                auto x = a;
                auto y = b;
                static_assert(std::same_as<decltype(x.swap(y)), void>);
                x.swap(y);
                BOOST_CHECK(model_of(x) == model_of(b)); // [array.members]/4
                BOOST_CHECK(model_of(y) == model_of(a));
        }
};

} // namespace

// [array.members]/1: constexpr size_type size() const noexcept;
BOOST_AUTO_TEST_CASE(Size)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_size()(a);
                }
        });
}

// [array.members]/3: constexpr void fill(const T& u);
BOOST_AUTO_TEST_CASE(Fill)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_fill()(a);
                }
        });
}

// [array.members]/4-5: constexpr void swap(array& y) noexcept(is_nothrow_swappable_v<T>);
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_swap()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
