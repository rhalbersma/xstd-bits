//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_QUALIFIES_AS_ALLOCATOR_HPP
#define XSTD_BITS_DETAIL_QUALIFIES_AS_ALLOCATOR_HPP

#include <cstddef> // size_t
#include <utility> // declval

namespace xstd::bits::detail {

// [container.reqmts]'s minimum for a deduction guide to treat a type as an allocator: value_type, and allocate(n).
template<class A>
concept qualifies_as_allocator = requires (A& a, std::size_t n) {
        typename A::value_type;
        a.allocate(n);
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_QUALIFIES_AS_ALLOCATOR_HPP
