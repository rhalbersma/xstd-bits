//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_TO_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_TO_HPP

#include <xstd/bits/bit/bit_convert.hpp> // bit_convert
#include <concepts>                      // same_as
#include <utility>                       // forward

namespace xstd {

// Whether bit_convert<To>(from) is valid: the positions map, though a run-time source may still throw.
template<class From, class To>
concept bit_convertible_to = requires (From&& from) {
        { xstd::bit_convert<To>(std::forward<From>(from)) } -> std::same_as<To>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_TO_HPP
