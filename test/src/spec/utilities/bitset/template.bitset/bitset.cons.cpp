//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // any_value
#include <test/bitset/primitives.hpp> // constructor, string_constructor
#include <test/dynamic.hpp>           // dynamic
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, bitsets
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_TEST_INFO_SCOPE
#include <array>                      // array

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(TemplateBitset)
BOOST_AUTO_TEST_SUITE(BitsetCons)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

namespace {

// Empty, the lowest and highest digit, both alternations, a mixed word, and full.
constexpr auto integers = std::array{0ULL, 1ULL, 1ULL << 63U, 0x5555'5555'5555'5555ULL, 0xAAAA'AAAA'AAAA'AAAAULL, 0x0123'4567'89AB'CDEFULL, ~0ULL};

} // namespace

// [bitset.cons]/1: constexpr bitset() noexcept;
BOOST_AUTO_TEST_CASE(Bitset)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                constructor<T>()();
        });
}

// [bitset.cons]/2: constexpr bitset(unsigned long long val) noexcept;
BOOST_AUTO_TEST_CASE(BitsetVal)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // A run-time width takes the integer after the count, at every count up to the sweep's limit.
                for (auto const val : integers) {
                        BOOST_TEST_INFO_SCOPE("val " << val);
                        if constexpr (test::dynamic<T>) {
                                on1::any_value<T>([&](auto num_bits) -> void {
                                        constructor<T>()(num_bits, static_cast<unsigned long>(val));
                                });
                        } else {
                                constructor<T>()(val);
                        }
                }
        });
}

// [bitset.cons]/3-9: explicit bitset(const basic_string<charT, traits, Allocator>& str, ...), bitset(const charT*, ...)
BOOST_AUTO_TEST_CASE(BitsetStr)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        string_constructor<T>()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
