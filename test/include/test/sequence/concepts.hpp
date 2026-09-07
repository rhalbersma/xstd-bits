//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_CONCEPTS_HPP
#define TEST_SEQUENCE_CONCEPTS_HPP

#include <test/value_reference.hpp> // value_reference
#include <compare>                  // strong_ordering
#include <concepts>                 // regular, same_as, totally_ordered
#include <cstddef>                  // size_t
#include <functional>               // hash
#include <initializer_list>         // initializer_list
#include <iterator>                 // random_access_iterator
#include <ranges>                   // from_range, random_access_range
#include <utility>                  // move

namespace test::sequence {

// The interface a bit-packed sequence shares with the sequence it packs, named once rather than restated per container.
template<class C>
concept bit_sequence =
        std::regular<C>
    and std::totally_ordered<C>
    and std::ranges::random_access_range<C>
    and std::random_access_iterator<typename C::iterator>
    and value_reference<typename C::const_reference>;

// The typedefs [container.reqmts] gives every container, pointer and const_pointer aside: packed bits have no address. [design.md#the-sequence-contract]
template<class C>
concept container_typedefs = requires {
        typename C::value_type;
        typename C::reference;
        typename C::const_reference;
        typename C::iterator;
        typename C::const_iterator;
        typename C::difference_type;
        typename C::size_type;
};

template<class C>
concept reversible_container_typedefs = container_typedefs<C> and requires {
        typename C::reverse_iterator;
        typename C::const_reverse_iterator;
};

// [container.reqmts] and [container.rev.reqmts]: what any container answers, on a const one and a mutable one.
template<class C>
concept container_members = reversible_container_typedefs<C> and requires (C c, C const cc, typename C::size_type n) {
        { c.begin()    } -> std::same_as<typename C::iterator>;
        { c.end()      } -> std::same_as<typename C::iterator>;
        { cc.begin()   } -> std::same_as<typename C::const_iterator>;
        { cc.end()     } -> std::same_as<typename C::const_iterator>;
        { c.cbegin()   } -> std::same_as<typename C::const_iterator>;
        { c.cend()     } -> std::same_as<typename C::const_iterator>;
        { c.rbegin()   } -> std::same_as<typename C::reverse_iterator>;
        { c.rend()     } -> std::same_as<typename C::reverse_iterator>;
        { cc.rbegin()  } -> std::same_as<typename C::const_reverse_iterator>;
        { cc.rend()    } -> std::same_as<typename C::const_reverse_iterator>;
        { c.crbegin()  } -> std::same_as<typename C::const_reverse_iterator>;
        { c.crend()    } -> std::same_as<typename C::const_reverse_iterator>;
        { cc.empty()   } -> std::same_as<bool>;
        { cc.size()    } -> std::same_as<typename C::size_type>;
        { cc.max_size()} -> std::same_as<typename C::size_type>;
        { c[n]         } -> std::same_as<typename C::reference>;
        { cc[n]        } -> std::same_as<typename C::const_reference>;
        { c.at(n)      } -> std::same_as<typename C::reference>;
        { cc.at(n)     } -> std::same_as<typename C::const_reference>;
        { c.front()    } -> std::same_as<typename C::reference>;
        { cc.front()   } -> std::same_as<typename C::const_reference>;
        { c.back()     } -> std::same_as<typename C::reference>;
        { cc.back()    } -> std::same_as<typename C::const_reference>;
        { cc == cc     } -> std::same_as<bool>;
        { cc <=> cc    } -> std::same_as<std::strong_ordering>;
        c.swap(c);
        swap(c, c);
};

// [array]'s synopsis, data() and the tuple interface aside, as one requires-expression: std::array<bool, N> and the packing both accept every line. [design.md#the-sequence-contract]
template<class C>
concept array_bool = container_members<C> and requires (C c, bool b) {
        C();
        C{ b, b };
        c.fill(b);
};

// [vector.bool]'s synopsis, likewise, with [vector.erasure] and the allocator: std::vector<bool> is the model and the packing answers every line of it.
template<class C, class A = C::allocator_type>
concept vector_bool = container_members<C> and requires (C c, C o, C const cc, typename C::size_type n, bool b, A a, std::initializer_list<bool> il, bool const* first, bool const* last, typename C::const_iterator p) {
        typename C::allocator_type;
        C();
        C(a);
        C(n);
        C(n, a);
        C(n, b);
        C(n, b, a);
        C(first, last);
        C(first, last, a);
        C(cc);
        C(std::move(o));
        C(cc, a);
        C(std::move(o), a);
        C(il);
        C(il, a);
        c = cc;
        c = std::move(o);
        c = il;
        c.assign(first, last);
        c.assign(n, b);
        c.assign(il);
        { cc.get_allocator() } -> std::same_as<A>;
        { cc.capacity()      } -> std::same_as<typename C::size_type>;
        c.resize(n);
        c.resize(n, b);
        c.reserve(n);
        c.shrink_to_fit();
        { c.emplace_back(b)  } -> std::same_as<typename C::reference>;
        c.push_back(b);
        c.pop_back();
        { c.emplace(p, b)             } -> std::same_as<typename C::iterator>;
        { c.insert(p, b)              } -> std::same_as<typename C::iterator>;
        { c.insert(p, n, b)           } -> std::same_as<typename C::iterator>;
        { c.insert(p, first, last)    } -> std::same_as<typename C::iterator>;
        { c.insert(p, il)             } -> std::same_as<typename C::iterator>;
        { c.erase(p)                  } -> std::same_as<typename C::iterator>;
        { c.erase(p, p)               } -> std::same_as<typename C::iterator>;
        c.clear();
        c.flip();
        C::swap(c[n], c[n]);
        { erase(c, b)                          } -> std::same_as<typename C::size_type>;
        { erase_if(c, [](bool) { return true; }) } -> std::same_as<typename C::size_type>;
        { std::hash<C>()(cc) } -> std::same_as<std::size_t>;
};

// [vector.bool]'s C++23 lines, apart so the model can be held to them where its standard library has them (__cpp_lib_containers_ranges).
template<class C, class A = C::allocator_type>
concept vector_bool_ranges = requires (C c, A a, std::initializer_list<bool> il, typename C::const_iterator p) {
        C(std::from_range, il);
        C(std::from_range, il, a);
        c.assign_range(il);
        { c.insert_range(p, il) } -> std::same_as<typename C::iterator>;
        c.append_range(il);
};

} // namespace test::sequence

#endif // TEST_SEQUENCE_CONCEPTS_HPP
