//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_INDETERMINATE_HPP
#define XSTD_BITS_BIT_SET_INDETERMINATE_HPP

#include <array>       // array
#include <concepts>    // constructible_from
#include <cstddef>     // size_t
#include <type_traits> // is_nothrow_constructible_v
#include <utility>     // index_sequence, make_index_sequence

// The attribute that keeps an automatic variable's storage unwritten under C++26's erroneous values, where it exists.
#ifndef XSTD_INDETERMINATE
#if __cplusplus > 202302L and defined(__has_cpp_attribute)
#if __has_cpp_attribute(indeterminate)
#define XSTD_INDETERMINATE indeterminate
#endif
#endif
#endif
#ifndef XSTD_INDETERMINATE
#define XSTD_INDETERMINATE
#endif

namespace xstd {

// The tag that leaves an owner's blocks default-initialized, as C++26's [[indeterminate]] leaves a variable's storage.
struct indeterminate_t
{
        explicit indeterminate_t() = default;
};

inline constexpr auto indeterminate = indeterminate_t();

// K owners, each built from the tag in its own element: a prvalue initializer is copy-elided, so nothing is zeroed.
template<class T, std::size_t K>
        requires std::constructible_from<T, indeterminate_t>
[[nodiscard]] constexpr auto indeterminate_array() noexcept(std::is_nothrow_constructible_v<T, indeterminate_t>)
        -> std::array<T, K>
{
        return [&]<std::size_t... I>(std::index_sequence<I...>) -> std::array<T, K> {
                return {{(static_cast<void>(I), T(indeterminate))...}};
        }(std::make_index_sequence<K>());
}

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_INDETERMINATE_HPP
