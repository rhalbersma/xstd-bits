//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_REJECTION_HPP
#define TEST_SPEC_REJECTION_HPP

#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <concepts>                       // constructible_from
#include <cstddef>                        // size_t
#include <initializer_list>               // initializer_list
#include <limits>                         // numeric_limits
#include <memory>                         // allocator
#include <span>                           // dynamic_extent
#include <vector>                         // vector

// The members a clause can ask a candidate not to have, each a concept on the candidate so a failure names it.
namespace test::spec {

// Growth, in [sequence.reqmts]'s spelling, which boost::dynamic_bitset's members share.
template<class X>
concept has_resize = requires (X c, std::size_t n) { c.resize(n); };

template<class X>
concept has_push_back = requires (X c, bool b) { c.push_back(b); };

template<class X>
concept has_emplace_back = requires (X c, bool b) { c.emplace_back(b); };

template<class X>
concept has_pop_back = requires (X c) { c.pop_back(); };

template<class X>
concept has_clear = requires (X c) { c.clear(); };

template<class X>
concept has_reserve = requires (X c, std::size_t n) { c.reserve(n); };

template<class X>
concept has_shrink_to_fit = requires (X c) { c.shrink_to_fit(); };

template<class X>
concept has_capacity = requires (X const& c) { c.capacity(); };

template<class X>
concept has_empty = requires (X const& c) { c.empty(); };

template<class X>
concept has_assign_count = requires (X c, std::size_t n, bool b) { c.assign(n, b); };

template<class X>
concept has_append = requires (X c, X::block_type w) { c.append(w); };

// Insertion and erasure at an iterator, a shape the set and sequence readings share.
template<class X>
concept has_insert_at = requires (X c, X::value_type v) { c.insert(c.cbegin(), v); };

template<class X>
concept has_erase_at = requires (X c) { c.erase(c.cbegin()); };

// The set reading's modifiers, by key.
template<class X>
concept has_insert_key = requires (X c, std::size_t k) { c.insert(k); };

template<class X>
concept has_insert_iterators = requires (X c, std::vector<std::size_t> const& v) { c.insert(v.begin(), v.end()); };

template<class X>
concept has_insert_range = requires (X c, std::vector<std::size_t> const& v) { c.insert_range(v); };

template<class X>
concept has_insert_list = requires (X c, std::initializer_list<std::size_t> il) { c.insert(il); };

template<class X>
concept has_emplace = requires (X c, std::size_t k) { c.emplace(k); };

template<class X>
concept has_emplace_hint = requires (X c, std::size_t k) { c.emplace_hint(c.cbegin(), k); };

template<class X>
concept has_erase_key = requires (X c, std::size_t k) { c.erase(k); };

template<class X>
concept has_erase_range = requires (X c) { c.erase(c.cbegin(), c.cend()); };

template<class X>
concept has_assign_list = requires (X c, std::initializer_list<std::size_t> il) { c = il; };

// An allocator, as [container.alloc.reqmts] asks of an allocator-aware container.
template<class X>
concept has_allocator_type = requires { typename X::allocator_type; };

template<class X>
concept has_get_allocator = requires (X const& c) { c.get_allocator(); };

// What an allocator-extended form is handed: the candidate's own allocator, or a std::allocator where it names none.
template<class X>
struct allocator_argument
{
        using type = std::allocator<std::size_t>;
};

template<has_allocator_type X>
struct allocator_argument<X>
{
        using type = X::allocator_type;
};

template<class X>
using allocator_argument_t = allocator_argument<X>::type;

template<class X>
concept has_allocator_constructor = std::constructible_from<X, allocator_argument_t<X> const&>;

template<class X>
concept has_allocator_extended_copy = std::constructible_from<X, X const&, allocator_argument_t<X> const&>;

template<class X>
concept has_allocator_extended_move = std::constructible_from<X, X&&, allocator_argument_t<X> const&>;

// Whether the unsigned integer Block covers the width an owner has in its type, which its tag constructor asks of one.
template<class X, class Block>
inline constexpr bool covers_static_width_v = false;

template<class X, class Block>
        requires (xstd::bits::detail::owned_storage<X>::bits_type::extent != std::dynamic_extent)
inline constexpr bool covers_static_width_v<X, Block> = xstd::bits::detail::owned_storage<X>::bits_type::extent <= static_cast<std::size_t>(std::numeric_limits<Block>::digits);

// The shifts, in place and into a copy.
template<class X>
concept has_shift_left_assign = requires (X c, std::size_t n) { c <<= n; };

template<class X>
concept has_shift_right_assign = requires (X c, std::size_t n) { c >>= n; };

template<class X>
concept has_shift_left = requires (X const& c, std::size_t n) { c << n; };

template<class X>
concept has_shift_right = requires (X const& c, std::size_t n) { c >> n; };

// The bitwise operators, in place and into a copy, with the set reading's difference beside them.
template<class X>
concept has_and_assign = requires (X c, X const& d) { c &= d; };

template<class X>
concept has_or_assign = requires (X c, X const& d) { c |= d; };

template<class X>
concept has_xor_assign = requires (X c, X const& d) { c ^= d; };

template<class X>
concept has_minus_assign = requires (X c, X const& d) { c -= d; };

template<class X>
concept has_bit_and = requires (X const& c, X const& d) { c & d; };

template<class X>
concept has_bit_or = requires (X const& c, X const& d) { c | d; };

template<class X>
concept has_bit_xor = requires (X const& c, X const& d) { c ^ d; };

template<class X>
concept has_minus = requires (X const& c, X const& d) { c - d; };

template<class X>
concept has_complement = requires (X const& c) { ~c; };

} // namespace test::spec

#endif // TEST_SPEC_REJECTION_HPP
