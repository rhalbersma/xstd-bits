//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_GSLICE_ARRAY_HPP
#define XSTD_BITS_BIT_GSLICE_ARRAY_HPP

#include <xstd/bits/detail/valarray_proxy.hpp>     // gslice_selection, proxy_access, valarray_proxy
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd {

// [template.gslice.array]: the positions a std::gslice or a bit_gslice selects in a valarray, written through to its blocks.
template<xstd::unsigned_integer Block>
class bit_gslice_array : public bits::detail::valarray_proxy<bits::detail::gslice_selection, Block>
{
        using base_type = bits::detail::valarray_proxy<bits::detail::gslice_selection, Block>;
        using base_type::base_type;

        friend struct bits::detail::proxy_access;

public:
        using base_type::operator=;

        bit_gslice_array() = delete;

        [[nodiscard]] bit_gslice_array(bit_gslice_array const&) = default;

        ~bit_gslice_array() = default;

        // [gslice.array.assign]: the other's elements assigned to this one's, never a rebinding.
        constexpr auto operator=(bit_gslice_array const& other) const // NOLINT(misc-unconventional-assign-operator, cppcoreguidelines-c-copy-assignment-signature): [valarray]'s proxies assign through a const object.
                -> bit_gslice_array const&
        {
                base_type::operator=(other);
                return *this;
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_GSLICE_ARRAY_HPP
