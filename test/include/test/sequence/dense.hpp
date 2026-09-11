//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_DENSE_HPP
#define TEST_SEQUENCE_DENSE_HPP

#include <boost/test/unit_test.hpp> // BOOST_CHECK_EQUAL
#include <cstddef>                  // size_t
#include <ranges>                   // contiguous_range

// Its own header rather than a corner of sequence/ordering.hpp, which reaches for bit_span and make_bitset that
// this needs none of.
namespace test::sequence {

// The sequence reading yields EVERY position, densely, 0 through size() - 1, each agreeing with the subscript.
// That is the nearest thing a bit sequence can offer to contiguity, and contiguity itself is out of reach:
// std::contiguous_iterator requires iter_reference_t<I> to be a real iter_value_t<I>&, and a single bit has no
// address to hand out. The negative is asserted here so the boundary is stated wherever the density is.
// [design.md#the-iterator-is-the-primitive]
template<class C>
auto yields_every_position(C const& c)
        -> void
{
        static_assert(not std::ranges::contiguous_range<C>);

        auto counted = 0UZ;
        for (auto const value : c) {
                BOOST_CHECK_EQUAL(static_cast<bool>(value), static_cast<bool>(c[counted]));
                ++counted;
        }
        BOOST_CHECK_EQUAL(counted, c.size());
}

}       // namespace test::sequence

#endif  // TEST_SEQUENCE_DENSE_HPP
