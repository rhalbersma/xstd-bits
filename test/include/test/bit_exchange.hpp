//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BIT_EXCHANGE_HPP
#define TEST_BIT_EXCHANGE_HPP

#include <xstd/bits/bit/bit_convert.hpp> // bit_convert
#include <xstd/bits/from_blocks.hpp>     // from_blocks
#include <concepts>                      // same_as

namespace test {

// The exchanges asked as templates: a non-dependent requirement hard-errors instead of failing.
template<class Reading, class Bits>
concept exchanges_from_bits = requires (Bits const& b) {
        { Reading(xstd::from_blocks, b) } -> std::same_as<Reading>;
};

// A conversion into Reading, which no view answers.
template<class Reading, class Bits>
concept converts_from = requires (Bits const& b) {
        { xstd::bit_convert<Reading>(b) } -> std::same_as<Reading>;
};

// A conversion out of Reading, which a view over a whole width answers as an owner does.
template<class Reading, class Bits>
concept converts_to = requires (Reading const& r) {
        { xstd::bit_convert<Bits>(r) } -> std::same_as<Bits>;
};

// Both directions, which an owner answers and a view does not.
template<class Reading, class Bits>
concept converts_between = converts_from<Reading, Bits> and converts_to<Reading, Bits>;

} // namespace test

#endif // TEST_BIT_EXCHANGE_HPP
