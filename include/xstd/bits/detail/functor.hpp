//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_FUNCTOR_HPP
#define XSTD_BITS_DETAIL_FUNCTOR_HPP

#include <type_traits> // is_invocable_r_v

namespace xstd::bits::detail {

// A prvalue from a named parameter: MSVC 17 has no auto(x), which is [P0849R8]'s spelling of this.
template<class T>
[[nodiscard]] constexpr auto decay_copy(T value) noexcept
        -> T
{
        return value;
}

// Continue unless the functor says otherwise: a void functor always continues, a bool one says.
template<class F, class T>
[[nodiscard]] constexpr auto invoke_continues(F& f, T value)
        -> bool
{
        if constexpr (std::is_invocable_r_v<bool, F&, T>) {
                return f(decay_copy(value));
        } else {
                f(decay_copy(value));
                return true;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_FUNCTOR_HPP
