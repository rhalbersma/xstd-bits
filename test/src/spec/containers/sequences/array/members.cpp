//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_prefix_sequences, all_singleton_sequence_pairs, all_singleton_sequences, empty_sequence
#include <test/sequence/factory.hpp>    // model_of
#include <test/spec/random.hpp>         // all_sequence_pairs, all_sequences
#include <test/spec/sequence.hpp>       // array_boundary_widths, array_every_width, array_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                     // same_as
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Members)

using namespace test::sequence;

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

// [array.members]/1: size()
BOOST_AUTO_TEST_SUITE(Size)

BOOST_AUTO_TEST_CASE_TEMPLATE(IsTheWidthInTheTypeAtEveryWidth, T, test::spec::sequence::array_every_width)
{
        on0::empty_sequence<T>(mem_size());
}

BOOST_AUTO_TEST_SUITE_END()

// [array.members]/3: fill(u)
BOOST_AUTO_TEST_SUITE(Fill)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryPrefixAndSingleton, T, test::spec::sequence::array_boundary_widths)
{
        on1::all_prefix_sequences<T>(mem_fill());
        on1::all_singleton_sequences<T>(mem_fill());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::array_random_widths)
{
        test::spec::random::all_sequences<T>(mem_fill());
}

BOOST_AUTO_TEST_SUITE_END()

// [array.members]/4-5: swap(y)
BOOST_AUTO_TEST_SUITE(Swap)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySingletonPair, T, test::spec::sequence::array_boundary_widths)
{
        on2::all_singleton_sequence_pairs<T>(mem_swap());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomPairs, T, test::spec::sequence::array_random_widths)
{
        test::spec::random::all_sequence_pairs<T>(mem_swap());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
