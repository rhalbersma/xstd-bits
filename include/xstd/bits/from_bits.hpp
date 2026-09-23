//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_FROM_BITS_HPP
#define XSTD_BITS_FROM_BITS_HPP

// The tag that says an argument is a field of bits, as std::from_range says an argument is a range of elements.
namespace xstd {

struct from_bits_t
{
        explicit from_bits_t() = default;
};

inline constexpr auto from_bits = from_bits_t();

} // namespace xstd

#endif // XSTD_BITS_FROM_BITS_HPP
