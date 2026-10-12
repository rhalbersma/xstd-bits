//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_OPERATIONS_HPP
#define TEST_SET_OPERATIONS_HPP

#include <cstddef> // size_t
#include <ranges>  // iota, to

// The four set algorithms against range-v3's views of the same name, over every pair of subsets of the first keys.
namespace test::set::operations {

// The set holding key i exactly where bit i of the mask is set, for the first n keys.
template<class S>
[[nodiscard]] auto from_mask(std::size_t mask, std::size_t n)
        -> S
{
        auto s = S();
        for (auto const i : std::views::iota(0UZ, n)) {
                if (((mask >> i) & 1UZ) != 0UZ) {
                        s.insert(i);
                }
        }
        return s;
}

// How many pairs the algorithm answers otherwise than the view materialized into the set's own type, or the operator.
template<class S, class Algorithm, class View, class Operator>
[[nodiscard]] auto disagreements(std::size_t n, Algorithm algorithm, View view, Operator op)
        -> int
{
        auto wrong = 0;
        for (auto const x : std::views::iota(0UZ, 1UZ << n)) {
                for (auto const y : std::views::iota(0UZ, 1UZ << n)) {
                        auto const s1     = from_mask<S>(x, n);
                        auto const s2     = from_mask<S>(y, n);
                        auto const result = algorithm(s1, s2);
                        wrong += static_cast<int>(result != (view(s1, s2, s1.key_comp()) | std::ranges::to<S>()) or result != op(s1, s2));
                }
        }
        return wrong;
}

} // namespace test::set::operations

#endif // TEST_SET_OPERATIONS_HPP
