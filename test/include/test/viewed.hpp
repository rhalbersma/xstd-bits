//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_VIEWED_HPP
#define TEST_VIEWED_HPP

#include <xstd/bits/bit_blocks.hpp> // resizable_bit_blocks
#include <cstddef>                  // size_t
#include <limits>                   // numeric_limits
#include <ranges>                   // range_value_t

namespace test {

// What a view is pointed at, all clear: blocks enough for num_bits where they resize, else the type's own width.
template<class T>
[[nodiscard]] auto make_viewed(std::size_t num_bits)
        -> T
{
        if constexpr (xstd::resizable_bit_blocks<T>) {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<std::ranges::range_value_t<T>>::digits);
                return T((num_bits + digits - 1UZ) / digits);
        } else {
                return T();
        }
}

} // namespace test

#endif // TEST_VIEWED_HPP
