//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/dynamic.hpp>         // dynamic
#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/bitset.hpp>     // all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL
#include <array>                    // array
#include <cstddef>                  // size_t
#include <ranges>                   // iota
#include <vector>                   // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Cons)

namespace {

// The values a block range reads, as a block of every width converts them: negative, wide and in range.
constexpr auto values = std::array{5, -1, 0x1234, 0};

template<class X>
auto check_converted_block_range()
        -> void
{
        // Two integers of one type are no iterators: a count and a value, as boost dispatches them.
        if (X().max_size() >= 8UZ) {
                auto const y = X(8, 7);
                BOOST_CHECK_EQUAL(y.size(), 8UZ);
                BOOST_CHECK_EQUAL(y.count(), 3UZ);
        }

        using block_type = X::block_type;
        // Boost declares bits_per_block an int, and ours a std::size_t.
        constexpr auto digits = static_cast<std::size_t>(X::bits_per_block);
        auto const ints = std::vector<int>(values.begin(), values.end());
        if (ints.size() * digits > X().max_size()) {
                return;
        }
        auto const x = X(ints.begin(), ints.end());
        BOOST_CHECK_EQUAL(x.size(), ints.size() * digits);
        for (auto const k : std::views::iota(0UZ, ints.size())) {
                auto const block = static_cast<block_type>(ints[k]);
                for (auto const j : std::views::iota(0UZ, digits)) {
                        // Cast back before the mask: a shifted narrow block is an int to bugprone-signed-bitwise.
                        BOOST_CHECK_EQUAL(static_cast<bool>(x[(k * digits) + j]), (static_cast<block_type>(block >> j) & block_type{1}) != block_type{0});
                }
        }
}

} // namespace

// boost::dynamic_bitset: dynamic_bitset(BlockInputIterator first, BlockInputIterator last, ...);
BOOST_AUTO_TEST_CASE(DynamicBitsetBlockRange)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                if constexpr (test::dynamic<T>) {
                        check_converted_block_range<T>();
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
