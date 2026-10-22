//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP
#define XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP

#include <xstd/bits/bit_concepts/bit_storage.hpp>              // bit_storage
#include <xstd/bits/bit_valarray/detail/valarray_adaptor.hpp> // valarray_adaptor
#include <xstd/bits/detail/bit_block_container.hpp>           // bit_block_container
#include <xstd/bits/detail/rebind.hpp>                        // rebind
#include <xstd/bits/detail/storage_rebind.hpp>                // fits_storage, storage_owner, storage_rebind, storage_width_v
#include <cstddef>                                            // size_t
#include <span>                                               // dynamic_extent
#include <vector>                                             // vector

namespace xstd {

// The packed std::valarray<bool> over a storage of blocks: every operator elementwise, at a size set at run time.
template<bit_storage Blocks = std::vector<std::size_t>, std::size_t N = std::dynamic_extent>
        requires bits::detail::fits_storage<Blocks, N>
class basic_bit_valarray : public bits::detail::valarray_adaptor<bits::detail::bit_block_container<Blocks, bits::detail::storage_width_v<Blocks, N>>, basic_bit_valarray<Blocks, N>>
{
        using base_type = bits::detail::valarray_adaptor<bits::detail::bit_block_container<Blocks, bits::detail::storage_width_v<Blocks, N>>, basic_bit_valarray<Blocks, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

using bit_valarray = basic_bit_valarray<>;

// Another block or width goes through the storage's bit_storage_rebind, and a counted storage recounts its blocks.
template<class Blocks, std::size_t N>
struct bits::detail::storage_owner<basic_bit_valarray<Blocks, N>>
{
        template<class OtherBlocks, std::size_t M>
        using type = basic_bit_valarray<OtherBlocks, M>;
};

template<class Blocks, std::size_t N>
struct bits::detail::rebind<basic_bit_valarray<Blocks, N>> : bits::detail::storage_rebind<basic_bit_valarray<Blocks, N>, Blocks, N>
{};

} // namespace xstd

#endif // XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP
