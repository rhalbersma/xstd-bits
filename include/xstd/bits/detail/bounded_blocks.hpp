//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP
#define XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <cstddef>        // size_t
#include <inplace_vector> // inplace_vector

// The bounded owners are constant-evaluable exactly where std::inplace_vector holds their blocks.
#define XSTD_BITS_HAS_CONSTEXPR_BOUNDED 1

namespace xstd::bits::detail {

template<class Block, std::size_t K>
using bounded_blocks = std::inplace_vector<Block, K>;

} // namespace xstd::bits::detail

#else

#include <boost/container/static_vector.hpp> // static_vector
#include <cstddef>                           // size_t

namespace xstd::bits::detail {

// Inline blocks under a capacity the type carries, as std::inplace_vector's are, but not constant-evaluable.
template<class Block, std::size_t K>
using bounded_blocks = boost::container::static_vector<Block, K>;

} // namespace xstd::bits::detail

#endif

#endif // XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP
