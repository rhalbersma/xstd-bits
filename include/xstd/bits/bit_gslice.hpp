//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_GSLICE_HPP
#define XSTD_BITS_BIT_GSLICE_HPP

#include <xstd/bits/detail/gslice_index.hpp> // gslice_index
#include <cstddef>                           // size_t
#include <memory>                            // make_shared, shared_ptr
#include <valarray>                          // gslice, valarray
#include <vector>                            // vector

namespace xstd {

// [class.gslice] with its positions worked out once, as packed bits where they ascend, for every subscript that reuses it.
class bit_gslice
{
        std::shared_ptr<bits::detail::gslice_index const> m_index;

        [[nodiscard]] static auto vector_of(std::valarray<std::size_t> const& a)
                -> std::vector<std::size_t>
        {
                return {std::begin(a), std::end(a)};
        }

        // The proxies read the index, which a copy shares, so a proxy holding one never outlives what it reads.
        friend struct bits::detail::gslice_access;

public:
        // [gslice.cons]: no lengths select nothing.
        [[nodiscard]] bit_gslice()
                : m_index(std::make_shared<bits::detail::gslice_index const>())
        {}

        [[nodiscard]] bit_gslice(std::size_t start, std::valarray<std::size_t> const& lengths, std::valarray<std::size_t> const& strides)
                : m_index(std::make_shared<bits::detail::gslice_index const>(start, vector_of(lengths), vector_of(strides)))
        {}

        [[nodiscard]] explicit bit_gslice(std::gslice const& g)
                : bit_gslice(g.start(), g.size(), g.stride())
        {}

        // [gslice.access]
        [[nodiscard]] auto start() const noexcept
                -> std::size_t
        {
                return m_index->start;
        }

        [[nodiscard]] auto size() const
                -> std::valarray<std::size_t>
        {
                return {m_index->lengths.data(), m_index->lengths.size()};
        }

        [[nodiscard]] auto stride() const
                -> std::valarray<std::size_t>
        {
                return {m_index->strides.data(), m_index->strides.size()};
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_GSLICE_HPP
