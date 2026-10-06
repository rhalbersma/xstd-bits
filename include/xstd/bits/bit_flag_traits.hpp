//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_TRAITS_HPP
#define XSTD_BITS_BIT_FLAG_TRAITS_HPP

#include <xstd/bits/detail/shift.hpp> // shl
#include <bit>                        // countr_zero, has_single_bit
#include <cassert>                    // assert
#include <cstddef>                    // size_t
#include <limits>                     // numeric_limits
#include <type_traits>                // is_enum_v, make_unsigned_t, underlying_type_t
#include <utility>                    // to_underlying

// A bitmask enumeration keyed on its own one-bit values, so a set of its flags needs no enumeration of ranks.
namespace xstd {

// A one-bit value ranks at the position of its bit, and rank i is the value 1 << i, both in the unsigned counterpart.
template<class E, std::size_t N = static_cast<std::size_t>(std::numeric_limits<std::make_unsigned_t<std::underlying_type_t<E>>>::digits)>
        requires std::is_enum_v<E>
struct bit_flag_traits
{
private:
        // Unsigned, so an enumerator on a signed type's sign bit is one bit like the others.
        using unsigned_type = std::make_unsigned_t<std::underlying_type_t<E>>;

        static_assert(N <= static_cast<std::size_t>(std::numeric_limits<unsigned_type>::digits));

public:
        static constexpr std::size_t size = N;

        // The key has exactly one bit set; one at or above N ranks at size or above.
        [[nodiscard]] static constexpr auto to_index(E key) noexcept
                -> std::size_t
        {
                auto const bits = static_cast<unsigned_type>(std::to_underlying(key));
                assert(std::has_single_bit(bits));
                return static_cast<std::size_t>(std::countr_zero(bits));
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> E
        {
                assert(index < N);
                return static_cast<E>(bits::detail::shl(unsigned_type{1}, index));
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_TRAITS_HPP
