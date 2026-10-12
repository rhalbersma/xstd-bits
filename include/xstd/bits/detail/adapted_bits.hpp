//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ADAPTED_BITS_HPP
#define XSTD_BITS_DETAIL_ADAPTED_BITS_HPP

#include <xstd/misc/type_traits/empty_base_type.hpp>   // empty_base_type
#include <xstd/misc/type_traits/no_unique_address.hpp> // XSTD_NO_UNIQUE_ADDRESS
#include <concepts>                                    // same_as
#include <type_traits>                                 // is_nothrow_constructible_v
#include <utility>                                     // forward, in_place_t

namespace xstd::bits::detail {

// What an adaptor wraps, as a base: access cannot follow the width, so which base it is does.
template<class Member, class Base, bool Structural>
class adapted_bits : public Base
{
protected:
        // Overlappable, so that an owner of a capacity of nought, whose storage is empty, is an empty type.
        [[XSTD_NO_UNIQUE_ADDRESS]]
        Member m_bits;

public:
        [[nodiscard]] adapted_bits() = default;

        template<class... Args>
        [[nodiscard]] constexpr explicit adapted_bits(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<Member, Args...>)
                : m_bits(std::forward<Args>(args)...)
        {}

        // An owner's alone: a view's empty base compares nothing, so two views over one storage find no candidate.
        // clang-format off: one line, so "= default;" stays where gcovr's branch exclusion looks for it.
        [[nodiscard]] friend auto operator==(adapted_bits const&, adapted_bits const&) -> bool requires (not std::same_as<Base, xstd::empty_base_type<>>) = default;
        // clang-format on
};

// Storage with no unused bits: the member is public only so that the owner is structural, not for direct use.
template<class Member, class Base>
struct adapted_bits<Member, Base, true> : Base
{
        [[XSTD_NO_UNIQUE_ADDRESS]]
        Member m_bits;

        [[nodiscard]] adapted_bits() = default;

        template<class... Args>
        [[nodiscard]] constexpr explicit adapted_bits(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<Member, Args...>)
                : m_bits(std::forward<Args>(args)...)
        {}

        // An owner's alone: a view's empty base compares nothing, so two views over one storage find no candidate.
        // clang-format off: one line, so "= default;" stays where gcovr's branch exclusion looks for it.
        [[nodiscard]] friend auto operator==(adapted_bits const&, adapted_bits const&) -> bool requires (not std::same_as<Base, xstd::empty_base_type<>>) = default;
        // clang-format on
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_ADAPTED_BITS_HPP
