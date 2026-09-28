//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // fn_erase, fn_erase_if
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // sequences, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Erasure)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

namespace {

// Either value.
auto check_erase(auto const& a)
        -> void
{
        fn_erase()(a, false);
        fn_erase()(a, true);
}

// A predicate that keeps everything, nothing, or one of the two values.
auto check_erase_if(auto const& a)
        -> void
{
        fn_erase_if()(a, [](bool) -> bool { return false; });
        fn_erase_if()(a, [](bool) -> bool { return true; });
        fn_erase_if()(a, [](bool x) -> bool { return x; });
}

} // namespace

// [vector.erasure]/1: vector<T, Allocator>::size_type erase(vector<T, Allocator>& c, const U& value);
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        check_erase(a);
                }
        });
}

// [vector.erasure]/2: vector<T, Allocator>::size_type erase_if(vector<T, Allocator>& c, Predicate pred);
BOOST_AUTO_TEST_CASE(EraseIf)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        check_erase_if(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
