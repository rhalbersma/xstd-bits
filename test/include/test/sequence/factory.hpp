//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_FACTORY_HPP
#define TEST_SEQUENCE_FACTORY_HPP

#include <test/dynamic.hpp> // dynamic
#include <algorithm>        // min
#include <cstddef>          // size_t
#include <ranges>           // iota
#include <type_traits>      // integral_constant
#include <vector>           // vector

namespace test::sequence {

// A width the type fixes, as std::array's is: it has no resize to change it.
template<class X>
concept static_width = not test::dynamic<X>;

// A run-time width under a capacity the type carries, asked of the type alone as [inplace.vector.capacity] has it.
template<class X>
concept static_capacity = test::dynamic<X> and requires { typename std::integral_constant<std::size_t, X::capacity()>; };

// A static width is its own limit, a capacity caps the sweep's, and an unbounded one takes the sweep's.
template<class X, std::size_t Limit>
inline constexpr auto limit_v = [] -> std::size_t {
        if constexpr (static_width<X>) {
                return X().size();
        } else if constexpr (static_capacity<X>) {
                return std::ranges::min(X::capacity(), Limit);
        } else {
                return Limit;
        }
}();

// A static width ignores n and a run-time width is resized to it; each position then holds what pred says.
template<class X>
[[nodiscard]] auto make_sequence(std::size_t n, auto pred)
        -> X
{
        auto a = X();
        if constexpr (test::dynamic<X>) {
                a.resize(n);
        }
        for (auto const i : std::views::iota(0UZ, a.size())) {
                a[i] = pred(i);
        }
        return a;
}

// A period of 21 that no block width divides, so every boundary lands inside the pattern.
[[nodiscard]] constexpr auto stripes(std::size_t i) noexcept
        -> bool
{
        return (i % 3UZ == 0UZ) or (i % 7UZ == 1UZ);
}

// The bools a sequence holds, which is the value every effect below is stated over.
template<class X>
[[nodiscard]] auto model_of(X const& a)
        -> std::vector<bool>
{
        return std::vector<bool>(a.begin(), a.end());
}

} // namespace test::sequence

#endif // TEST_SEQUENCE_FACTORY_HPP
