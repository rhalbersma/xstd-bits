//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_BIT_SLICE_HPP
#define XSTD_BITS_BIT_VALARRAY_BIT_SLICE_HPP

#include <cstddef>  // size_t
#include <valarray> // slice

namespace xstd {

// [class.slice] in a constant expression: start + j * stride for j < size, which std::slice's constructors are not.
class bit_slice
{
        std::size_t m_start  = 0UZ;
        std::size_t m_size   = 0UZ;
        std::size_t m_stride = 0UZ;

public:
        [[nodiscard]] bit_slice() = default;

        [[nodiscard]] constexpr bit_slice(std::size_t start, std::size_t size, std::size_t stride) noexcept
                : m_start(start)
                , m_size(size)
                , m_stride(stride)
        {}

        [[nodiscard]] explicit bit_slice(std::slice const& s) noexcept
                : bit_slice(s.start(), s.size(), s.stride())
        {}

        // [slice.access]
        [[nodiscard]] constexpr auto start() const noexcept
                -> std::size_t
        {
                return m_start;
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_size;
        }

        [[nodiscard]] constexpr auto stride() const noexcept
                -> std::size_t
        {
                return m_stride;
        }

        // [slice.ops]
        [[nodiscard]] friend auto operator==(bit_slice const&, bit_slice const&) -> bool = default;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_VALARRAY_BIT_SLICE_HPP
