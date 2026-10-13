//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_FLAG_P4313_HPP
#define TEST_FLAG_P4313_HPP

#include <type_traits> // is_enum_v
#include <utility>     // to_underlying

// P4313R1's bitmask operators before any compiler has its annotation: a trait marks the enumeration instead.
namespace test::p4313 {

// Specialized true per enumeration, standing in for the annotation query.
template<class T>
inline constexpr bool bitmask_type = false;

// P4313R1's exposition-only bitmask-like.
template<class T>
concept bitmask_like = std::is_enum_v<T> and bitmask_type<T>;

} // namespace test::p4313

// P4313R1 22.x.4: each in the global namespace, each returning the enumeration.
template<test::p4313::bitmask_like T>
[[nodiscard]] constexpr auto operator|(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

template<test::p4313::bitmask_like T>
[[nodiscard]] constexpr auto operator&(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

template<test::p4313::bitmask_like T>
[[nodiscard]] constexpr auto operator^(T lhs, T rhs) noexcept
        -> T
{
        return static_cast<T>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

template<test::p4313::bitmask_like T>
[[nodiscard]] constexpr auto operator~(T lhs) noexcept
        -> T
{
        return static_cast<T>(~std::to_underlying(lhs));
}

template<test::p4313::bitmask_like T>
constexpr auto operator|=(T& lhs, T rhs) noexcept
        -> T&
{
        lhs = lhs | rhs;
        return lhs;
}

template<test::p4313::bitmask_like T>
constexpr auto operator&=(T& lhs, T rhs) noexcept
        -> T&
{
        lhs = lhs & rhs;
        return lhs;
}

template<test::p4313::bitmask_like T>
constexpr auto operator^=(T& lhs, T rhs) noexcept
        -> T&
{
        lhs = lhs ^ rhs;
        return lhs;
}

#endif // TEST_FLAG_P4313_HPP
