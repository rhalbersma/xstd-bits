//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_MASK_ARRAY_HPP
#define XSTD_BITS_BIT_MASK_ARRAY_HPP

#include <xstd/bits/detail/valarray_proxy.hpp>     // mask_selection, proxy_access, valarray_proxy
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd {

// [template.mask.array]: the positions a mask of the same size selects in a valarray, written through to its blocks.
template<xstd::unsigned_integer Block>
class bit_mask_array : public bits::detail::valarray_proxy<bits::detail::mask_selection<Block>, Block>
{
        using base_type = bits::detail::valarray_proxy<bits::detail::mask_selection<Block>, Block>;
        using base_type::base_type;

        friend struct bits::detail::proxy_access;

public:
        using base_type::operator=;

        bit_mask_array() = delete;

        [[nodiscard]] bit_mask_array(bit_mask_array const&) = default;

        ~bit_mask_array() = default;

        // [mask.array.assign]: the other's elements assigned to this one's, never a rebinding.
        constexpr auto operator=(bit_mask_array const& other) const // NOLINT(misc-unconventional-assign-operator, cppcoreguidelines-c-copy-assignment-signature): [valarray]'s proxies assign through a const object.
                -> bit_mask_array const&
        {
                base_type::operator=(other);
                return *this;
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_MASK_ARRAY_HPP
