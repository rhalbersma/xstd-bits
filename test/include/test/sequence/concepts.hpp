//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_CONCEPTS_HPP
#define TEST_SEQUENCE_CONCEPTS_HPP

#include <test/value_reference.hpp> // value_reference
#include <compare>                  // strong_ordering
#include <concepts>                 // convertible_to, regular, same_as, totally_ordered
#include <cstddef>                  // size_t
#include <functional>               // hash
#include <initializer_list>         // initializer_list
#include <iterator>                 // random_access_iterator
#include <optional>                 // optional
#include <ranges>                   // from_range, random_access_range
#include <tuple>                    // tuple_element, tuple_size
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

// The typedefs [container.reqmts] gives every container, pointer and const_pointer aside: packed bits have no address.
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
concept container_members = reversible_container_typedefs<C> and requires (C c, C const cc, C::size_type n) {
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

// [array.tuple]'s element half, which only a non-empty array has: tuple_element<I, array<T, N>> Mandates I < N, so
// element zero is a question that cannot be put to a width of nought -- on the packing or on std::array itself.
template<class C>
concept array_tuple_element = (std::tuple_size<C>::value == 0) or requires (C c, C const cc) {
        typename std::tuple_element<0, C>::type;
        typename std::tuple_element<0, C const>::type;
        get<0>(c);
        get<0>(cc);
        get<0>(std::move(c));
};

// [array]'s synopsis, data() aside, as one requires-expression: std::array<bool, N> and the packing both accept
// every line. data() is the only one a packed bool cannot answer -- a bit has no address -- and [array.tuple] IS
// answerable, so it is asked for here rather than excused along with it.
template<class C>
concept array_bool = container_members<C> and array_tuple_element<C> and requires (C c, bool b) {
        C();
        C{ b, b };
        c.fill(b);
        { std::tuple_size<C>::value } -> std::convertible_to<std::size_t>;
};

// [vector.bool]'s synopsis, likewise, with [vector.erasure] and the allocator: std::vector<bool> is the model and the packing answers every line of it.
template<class C, class A = C::allocator_type>
concept vector_bool = container_members<C> and requires (C c, C o, C const cc, C::size_type n, bool b, A a, std::initializer_list<bool> il, bool const* first, bool const* last, C::const_iterator p) {
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
        { c.emplace_back()   } -> std::same_as<typename C::reference>;
        c.push_back(b);
        c.pop_back();
        { c.emplace(p, b)             } -> std::same_as<typename C::iterator>;
        { c.emplace(p)                } -> std::same_as<typename C::iterator>;
        { c.insert(p, b)              } -> std::same_as<typename C::iterator>;
        { c.insert(p, n, b)           } -> std::same_as<typename C::iterator>;
        { c.insert(p, first, last)    } -> std::same_as<typename C::iterator>;
        { c.insert(p, il)             } -> std::same_as<typename C::iterator>;
        { c.erase(p)                  } -> std::same_as<typename C::iterator>;
        { c.erase(p, p)               } -> std::same_as<typename C::iterator>;
        c.clear();
        c.flip();
        // The PROXY's flip, which is not the container's above: [vector.bool] has required it of
        // std::vector<bool>::reference since C++98, and asking only for the container's left the proxy's unasked.
        { c[n].flip() } -> std::same_as<void>;
        C::swap(c[n], c[n]);
        { erase(c, b)                          } -> std::same_as<typename C::size_type>;
        { erase_if(c, [](bool) { return true; }) } -> std::same_as<typename C::size_type>;
        { std::hash<C>()(cc) } -> std::same_as<std::size_t>;
};

// [inplace.vector]'s synopsis, data() aside. Written from THAT clause and not from [vector.bool] minus its
// allocator, which is what it used to be: the middle column's counterpart spells four capacity members static and
// gives push_back a reference to return, and a checklist copied from the dynamic column asks for none of it.
template<class C>
concept inplace_vector_bool = container_members<C> and requires (C c, C o, C const cc, C::size_type n, bool b, std::initializer_list<bool> il, bool const* first, bool const* last, C::const_iterator p) {
        C();
        C(n);
        C(n, b);
        C(first, last);
        C(cc);
        C(std::move(o));
        C(il);
        c = cc;
        c = std::move(o);
        c = il;
        c.assign(first, last);
        c.assign(n, b);
        c.assign(il);
        // [inplace.vector.capacity]: the capacity is the TYPE's, so these four answer without an object.
        { C::capacity() } -> std::same_as<typename C::size_type>;
        { C::max_size() } -> std::same_as<typename C::size_type>;
        C::reserve(n);
        C::shrink_to_fit();
        c.resize(n);
        c.resize(n, b);
        // [inplace.vector.modifiers]: push_back returns the reference here, where [vector.bool]'s returns nothing.
        { c.emplace_back(b)  } -> std::same_as<typename C::reference>;
        { c.emplace_back()   } -> std::same_as<typename C::reference>;
        { c.push_back(b)     } -> std::same_as<typename C::reference>;
        c.pop_back();
        // The non-throwing door and the unchecked one, which are the whole reason this column is a container apart.
        { c.try_emplace_back(b)       } -> std::same_as<std::optional<typename C::reference>>;
        { c.try_push_back(b)          } -> std::same_as<std::optional<typename C::reference>>;
        { c.unchecked_emplace_back(b) } -> std::same_as<typename C::reference>;
        { c.unchecked_push_back(b)    } -> std::same_as<typename C::reference>;
        { c.emplace(p, b)             } -> std::same_as<typename C::iterator>;
        { c.emplace(p)                } -> std::same_as<typename C::iterator>;
        { c.insert(p, b)              } -> std::same_as<typename C::iterator>;
        { c.insert(p, n, b)           } -> std::same_as<typename C::iterator>;
        { c.insert(p, first, last)    } -> std::same_as<typename C::iterator>;
        { c.insert(p, il)             } -> std::same_as<typename C::iterator>;
        { c.erase(p)                  } -> std::same_as<typename C::iterator>;
        { c.erase(p, p)               } -> std::same_as<typename C::iterator>;
        c.clear();
        { erase(c, b)                          } -> std::same_as<typename C::size_type>;
        { erase_if(c, [](bool) { return true; }) } -> std::same_as<typename C::size_type>;
};

// What the packing adds over [inplace.vector], which its unpacked counterpart has no reason to carry: the bitwise
// vocabulary of a bit container, and the hash every value this library compares it also hashes.
template<class C>
concept packed_inplace_vector_bool = requires (C c, C const cc, C::size_type n) {
        c.flip();
        { c[n].flip() } -> std::same_as<void>;
        { std::hash<C>()(cc) } -> std::same_as<std::size_t>;
};

// [vector.bool]'s C++23 lines, apart so the model can be held to them where its standard library has them (__cpp_lib_containers_ranges).
template<class C, class A = C::allocator_type>
concept vector_bool_ranges = requires (C c, A a, std::initializer_list<bool> il, C::const_iterator p) {
        C(std::from_range, il);
        C(std::from_range, il, a);
        c.assign_range(il);
        { c.insert_range(p, il) } -> std::same_as<typename C::iterator>;
        c.append_range(il);
};

// The same lines without the allocator-extended constructor, for the column that has no allocator to extend them with.
template<class C>
concept inplace_vector_bool_ranges = requires (C c, std::initializer_list<bool> il, C::const_iterator p) {
        C(std::from_range, il);
        c.assign_range(il);
        { c.insert_range(p, il) } -> std::same_as<typename C::iterator>;
        c.append_range(il);
};

} // namespace test::sequence

#endif // TEST_SEQUENCE_CONCEPTS_HPP
