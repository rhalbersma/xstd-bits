//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ALLOCATOR_BASE_TYPE_HPP
#define XSTD_BITS_DETAIL_ALLOCATOR_BASE_TYPE_HPP

namespace xstd::bits::detail {

// The allocator's name where the storage has one, else an empty base; one per owner, so no two compare through it.
template<class Storage, class Owner = void>
struct allocator_base_type
{
        [[nodiscard]] friend auto operator==(allocator_base_type const&, allocator_base_type const&) -> bool = default;
};

template<class Storage, class Owner>
        requires requires { typename Storage::allocator_type; }
struct allocator_base_type<Storage, Owner>
{
        using allocator_type = Storage::allocator_type;

        [[nodiscard]] friend auto operator==(allocator_base_type const&, allocator_base_type const&) -> bool = default;
};

// Whether an allocator argument means anything to this storage.
template<class Storage>
inline constexpr bool has_allocator_v = false;

template<class Storage>
        requires requires { typename Storage::allocator_type; }
inline constexpr bool has_allocator_v<Storage> = true;

// Stands in for the allocator a storage lacks; explicit, so no argument, {} included, ever becomes one.
struct no_allocator
{
        explicit no_allocator() = default;
};

// An allocator parameter as [container.alloc.reqmts] spells it, non-deduced and converting, over any storage.
template<class Storage>
struct allocator_param
{
        using type = no_allocator;
};

template<class Storage>
        requires has_allocator_v<Storage>
struct allocator_param<Storage>
{
        using type = Storage::allocator_type;
};

template<class Storage>
using allocator_param_t = allocator_param<Storage>::type;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_ALLOCATOR_BASE_TYPE_HPP
