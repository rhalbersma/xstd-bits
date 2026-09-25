//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_VOCABULARY_HPP
#define TEST_BITSET_VOCABULARY_HPP

#include <concepts> // convertible_to, regular, same_as
#include <cstddef>  // size_t

namespace test::bitset {

// The vocabulary std::bitset, boost::dynamic_bitset and our storage share: a claim the tests check, not a constraint.
template<class C>
concept vocabulary =
        std::regular<C> and
        requires (C const& c, std::size_t n) {
                { c.size() } -> std::convertible_to<std::size_t>;
                { c.count() } -> std::convertible_to<std::size_t>;
                { c.test(n) } -> std::convertible_to<bool>;
                { c.all() } -> std::convertible_to<bool>;
                { c.any() } -> std::convertible_to<bool>;
                { c.none() } -> std::convertible_to<bool>;
        } and
        requires (C& b, std::size_t n) {
                { b.set() } -> std::same_as<C&>;
                { b.set(n) } -> std::same_as<C&>;
                { b.reset() } -> std::same_as<C&>;
                { b.reset(n) } -> std::same_as<C&>;
                { b.flip() } -> std::same_as<C&>;
                { b.flip(n) } -> std::same_as<C&>;
        } and
        requires (C& b, C const& c, std::size_t n) {
                { b &= c } -> std::same_as<C&>;
                { b |= c } -> std::same_as<C&>;
                { b ^= c } -> std::same_as<C&>;
                { b <<= n } -> std::same_as<C&>;
                { b >>= n } -> std::same_as<C&>;
        };

} // namespace test::bitset

#endif // TEST_BITSET_VOCABULARY_HPP
