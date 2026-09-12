//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP
#define XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP

namespace xstd::detail::bits {

// The allocator's name where the storage below has one and nothing where it does not: an empty base, a class having no conditional typedef.
// Equality is defaulted so a derived class's defaulted == still compares, this base having nothing to compare. [design.md#a-strict-extension]
// Neither constexpr nor noexcept is written: a defaulted comparison deduces both, and xstd::empty_type's own
// defaulted <=> writes neither either. [design.md#a-strict-extension]
template<class Storage>
struct allocator_typedef
{
        [[nodiscard]] friend auto operator==(allocator_typedef const&, allocator_typedef const&) -> bool = default;
};

template<class Storage>
        requires requires { typename Storage::allocator_type; }
struct allocator_typedef<Storage>
{
        using allocator_type = Storage::allocator_type;

        [[nodiscard]] friend auto operator==(allocator_typedef const&, allocator_typedef const&) -> bool = default;
};

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_ALLOCATOR_TYPEDEF_HPP
