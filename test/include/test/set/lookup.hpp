//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_LOOKUP_HPP
#define TEST_SET_LOOKUP_HPP

#include <cstddef>  // size_t
#include <iterator> // ranges::distance

// One value asked of a set and of a std::set of the same keys, which answers for any value its comparator orders.
namespace test::set {

// How often the two disagree on v: contains, count, find, both bounds, and erase with what it leaves.
template<class X, class Model>
[[nodiscard]] auto lookup_mismatches(X const& a, Model const& model, typename X::key_type const& v)
        -> std::size_t
{
        auto x            = a;
        auto const erased = x.erase(v);
        auto mismatches   = static_cast<std::size_t>(a.contains(v) != model.contains(v));
        mismatches += static_cast<std::size_t>(a.count(v) != model.count(v));
        mismatches += static_cast<std::size_t>((a.find(v) == a.end()) == model.contains(v));
        mismatches += static_cast<std::size_t>(std::ranges::distance(a.begin(), a.lower_bound(v)) != std::ranges::distance(model.begin(), model.lower_bound(v)));
        mismatches += static_cast<std::size_t>(std::ranges::distance(a.begin(), a.upper_bound(v)) != std::ranges::distance(model.begin(), model.upper_bound(v)));
        mismatches += static_cast<std::size_t>(erased != model.count(v));
        mismatches += static_cast<std::size_t>(x.size() + erased != a.size());
        return mismatches;
}

} // namespace test::set

#endif // TEST_SET_LOOKUP_HPP
