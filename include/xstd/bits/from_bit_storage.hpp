//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_FROM_BIT_STORAGE_HPP
#define XSTD_BITS_FROM_BIT_STORAGE_HPP

// The tag that says an argument's words are read as bits, as std::from_range says a range's elements are read.
namespace xstd {

struct from_bit_storage_t
{
        explicit from_bit_storage_t() = default;
};

inline constexpr auto from_bit_storage = from_bit_storage_t();

} // namespace xstd

#endif // XSTD_BITS_FROM_BIT_STORAGE_HPP
