//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_NON_VOLATILE_HPP
#define XSTD_BITS_DETAIL_NON_VOLATILE_HPP

#include <type_traits> // is_volatile_v, remove_reference_t

namespace xstd::bits::detail {

// An explicit object parameter's type, refused where volatile, as a member without a volatile overload refuses it.
template<class Self>
concept non_volatile = not std::is_volatile_v<std::remove_reference_t<Self>>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_NON_VOLATILE_HPP
