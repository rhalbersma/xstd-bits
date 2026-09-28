//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // fn_format
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // sequences, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(VectorBool)
BOOST_AUTO_TEST_SUITE(Fmt)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [vector.bool.fmt]/1-2: ParseContext::iterator parse(ParseContext& ctx); FormatContext::iterator format(...) const;
BOOST_AUTO_TEST_CASE(Format)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        fn_format()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
