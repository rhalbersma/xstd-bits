//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/ownership.hpp>                    // ownership
#include <xstd/bits/set_adaptor.hpp>                  // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <cstddef>                                    // size_t
#include <memory>                                     // allocator

namespace xstd {

// The set reading over a heap of blocks: the flagship, and the one name without a qualifier.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
class basic_bit_set : public set_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, ownership::owns, basic_bit_set<Block, Allocator>>
{
        using base_type = set_adaptor<detail::bits::contiguous_bit_vector<Block, Allocator>, ownership::owns, basic_bit_set<Block, Allocator>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

using bit_set = basic_bit_set<std::size_t>;

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_HPP
