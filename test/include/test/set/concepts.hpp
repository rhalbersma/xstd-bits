//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_CONCEPTS_HPP
#define TEST_SET_CONCEPTS_HPP

#include <test/value_reference.hpp> // value_reference
#include <compare>                  // strong_ordering
#include <concepts>                 // regular, same_as, totally_ordered
#include <cstddef>                  // size_t
#include <functional>               // hash
#include <initializer_list>         // initializer_list
#include <iterator>                 // bidirectional_iterator
#include <ranges>                   // bidirectional_range, from_range
#include <utility>                  // move, pair

namespace test::set {

// The interface a bit-packed set shares with the set it packs: bidirectional over keys, not random-access over bool.
template<class C>
concept bit_set =
        std::regular<C> and std::totally_ordered<C> and std::ranges::bidirectional_range<C> and std::bidirectional_iterator<typename C::iterator> and value_reference<typename C::const_reference>;

// The [associative.reqmts] typedefs, pointer aside: packed bits have no address and a bitmap has no node.
template<class C>
concept set_typedefs = requires {
        typename C::key_type;
        typename C::key_compare;
        typename C::value_type;
        typename C::value_compare;
        typename C::size_type;
        typename C::difference_type;
        typename C::iterator;
        typename C::const_iterator;
        typename C::reverse_iterator;
        typename C::const_reverse_iterator;
};

// [set]'s synopsis as one requires-expression, with std::set<std::size_t> as the model.
template<class C>
concept set_size_t = set_typedefs<C> and requires (C c, C o, C const cc, C::key_type k, C::key_compare const comp, std::initializer_list<typename C::value_type> il, C::value_type const* first, C::value_type const* last, C::const_iterator p) {
        C();
        C(comp);
        C(first, last);
        C(first, last, comp);
        C(cc);
        C(std::move(o));
        C(il);
        C(il, comp);
        c = cc;
        c = std::move(o);
        c = il;
        { c.begin() } -> std::same_as<typename C::iterator>;
        { c.end() } -> std::same_as<typename C::iterator>;
        { cc.begin() } -> std::same_as<typename C::const_iterator>;
        { cc.end() } -> std::same_as<typename C::const_iterator>;
        { c.cbegin() } -> std::same_as<typename C::const_iterator>;
        { c.cend() } -> std::same_as<typename C::const_iterator>;
        { c.rbegin() } -> std::same_as<typename C::reverse_iterator>;
        { c.rend() } -> std::same_as<typename C::reverse_iterator>;
        { c.crbegin() } -> std::same_as<typename C::const_reverse_iterator>;
        { c.crend() } -> std::same_as<typename C::const_reverse_iterator>;
        { cc.empty() } -> std::same_as<bool>;
        { cc.size() } -> std::same_as<typename C::size_type>;
        { cc.max_size() } -> std::same_as<typename C::size_type>;
        // [set.modifiers]. emplace is variadic and a key is default-constructible, so the empty list is one it takes.
        { c.emplace(k) } -> std::same_as<std::pair<typename C::iterator, bool>>;
        { c.emplace() } -> std::same_as<std::pair<typename C::iterator, bool>>;
        { c.emplace_hint(p, k) } -> std::same_as<typename C::iterator>;
        { c.emplace_hint(p) } -> std::same_as<typename C::iterator>;
        { c.insert(k) } -> std::same_as<std::pair<typename C::iterator, bool>>;
        { c.insert(p, k) } -> std::same_as<typename C::iterator>;
        c.insert(first, last);
        c.insert(il);
        { c.erase(p) } -> std::same_as<typename C::iterator>;
        { c.erase(k) } -> std::same_as<typename C::size_type>;
        { c.erase(p, p) } -> std::same_as<typename C::iterator>;
        c.swap(c);
        swap(c, c);
        c.clear();
        { cc.key_comp() } -> std::same_as<typename C::key_compare>;
        { cc.value_comp() } -> std::same_as<typename C::value_compare>;
        // [set.ops].
        { c.find(k) } -> std::same_as<typename C::iterator>;
        { cc.find(k) } -> std::same_as<typename C::const_iterator>;
        { cc.count(k) } -> std::same_as<typename C::size_type>;
        { cc.contains(k) } -> std::same_as<bool>;
        { c.lower_bound(k) } -> std::same_as<typename C::iterator>;
        { cc.lower_bound(k) } -> std::same_as<typename C::const_iterator>;
        { c.upper_bound(k) } -> std::same_as<typename C::iterator>;
        { cc.upper_bound(k) } -> std::same_as<typename C::const_iterator>;
        { c.equal_range(k) } -> std::same_as<std::pair<typename C::iterator, typename C::iterator>>;
        { cc.equal_range(k) } -> std::same_as<std::pair<typename C::const_iterator, typename C::const_iterator>>;
        { cc == cc } -> std::same_as<bool>;
        { cc <=> cc } -> std::same_as<std::strong_ordering>;
        {
                erase_if(c, [](C::key_type) { return true; })
        } -> std::same_as<typename C::size_type>;
};

// [set.cons]'s allocator arguments, which only the column whose storage has an allocator can answer.
template<class C, class A = C::allocator_type>
concept set_size_t_allocator = requires (C c, C o, C const cc, A a, C::key_compare const comp, std::initializer_list<typename C::value_type> il, C::value_type const* first, C::value_type const* last) {
        typename C::allocator_type;
        C(a);
        C(comp, a);
        C(first, last, a);
        C(first, last, comp, a);
        C(cc, a);
        C(std::move(o), a);
        C(il, a);
        C(il, comp, a);
        { cc.get_allocator() } -> std::same_as<A>;
};

// [set.cons] and [set.modifiers]'s C++23 lines, apart so the model is held to them where it has them.
template<class C>
concept set_size_t_ranges = requires (C c, C::key_compare const comp, std::initializer_list<typename C::value_type> il) {
        C(std::from_range, il);
        C(std::from_range, il, comp);
        c.insert_range(il);
};

template<class C, class A = C::allocator_type>
concept set_size_t_ranges_allocator = requires (A a, C::key_compare const comp, std::initializer_list<typename C::value_type> il) {
        C(std::from_range, il, a);
        C(std::from_range, il, comp, a);
};

} // namespace test::set

#endif // TEST_SET_CONCEPTS_HPP
