//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FINITE_SET_HPP
#define XSTD_BITS_BIT_FINITE_SET_HPP

// Header <set> synopsis                                   [associative.set.syn]

#include <compare>                                 // strong_ordering
#include <initializer_list>                        // initializer_list

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/memory.hpp>                    // align_up
#include <concepts>                                // constructible_from
#include <cstddef>                                 // size_t
#include <limits>                                  // digits

namespace xstd {

// 23.4.6, class template set
template<std::size_t N, xstd::unsigned_integer Block = std::size_t> 
class bit_finite_set;

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator== (const bit_finite_set<N, Block>& x, const bit_finite_set<N, Block>& y) noexcept -> bool;
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator<=>(const bit_finite_set<N, Block>& x, const bit_finite_set<N, Block>& y) noexcept -> std::strong_ordering;
template<std::size_t N, xstd::unsigned_integer Block>               constexpr void swap       (      bit_finite_set<N, Block>& x,       bit_finite_set<N, Block>& y) noexcept(noexcept(x.swap(y)));

// 23.4.6.3, erasure for set
template<std::size_t N, xstd::unsigned_integer Block, class Predicate>
constexpr auto erase_if(bit_finite_set<N, Block>& c, Predicate pred) -> bit_finite_set<N, Block>::size_type;

namespace aligned {

template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
using bit_finite_set = xstd::bit_finite_set<xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), Block>;

}       // namespace aligned
}       // namespace xstd

// NOLINTBEGIN(readability-duplicate-include): synopsis and implementation each list what that section needs.
#include <boost/hash2/hash_append.hpp>             // hash_append
#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/bits/block_sequence.hpp>            // block_array
#include <xstd/bits/ranges.hpp>                    // const_iterator, const_reference
#include <cassert>                                 // assert
#include <compare>                                 // strong_ordering
#include <concepts>                                // constructible_from
#include <cstddef>                                 // ptrdiff_t, size_t
#include <functional>                              // less
#include <initializer_list>                        // initializer_list
#include <iterator>                                // make_reverse_iterator, reverse_iterator,
                                       // input_iterator, sentinel_for
#include <limits>                                  // digits
#include <ranges>                                  // begin, empty, end, from_range_t, rbegin, rend
                                       // input_range
#include <utility>                                 // forward, move, pair
// NOLINTEND(readability-duplicate-include)

// Class template set [set], Overview [set.overview]

namespace xstd {

template<std::size_t N, xstd::unsigned_integer Block>
class bit_finite_set
{
        block_array<Block, N> m_bits{};

        // The door reaches the storage; nothing else needs to. [design.md#the-door]
        friend struct bit_traits<bit_finite_set>;

        // ADL rather than a specialization, because this type is ours to add hidden friends to.
        [[nodiscard]] friend constexpr auto block_count(const bit_finite_set& c) noexcept -> std::size_t { return c.m_bits.num_blocks(); }
        [[nodiscard]] friend constexpr auto block_at(const bit_finite_set& c, std::size_t i) noexcept -> Block { return c.m_bits.block(i); }

        [[nodiscard]] friend constexpr auto find_first(const bit_finite_set& c)                noexcept -> std::size_t { return c.m_bits.find_first(); }
        [[nodiscard]] friend constexpr auto find_last (const bit_finite_set& c)                noexcept -> std::size_t { return c.m_bits.find_last();  }
        [[nodiscard]] friend constexpr auto find_next (const bit_finite_set& c, std::size_t n) noexcept -> std::size_t { return c.m_bits.exclusive_find_next(n); }
        [[nodiscard]] friend constexpr auto find_prev (const bit_finite_set& c, std::size_t n) noexcept -> std::size_t { return c.m_bits.exclusive_find_prev(n); }

        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bit_finite_set const* v) noexcept
        {
                boost::hash2::hash_append(h, f, v->m_bits);
        }

public:
        // types
        using key_type               = std::size_t;
        using key_compare            = std::less<key_type>;
        using value_type             = key_type;
        using value_compare          = key_compare;
        using block_type             = Block;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = xstd::ranges::set_reference<bit_finite_set>;
        using const_reference        = reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = xstd::ranges::set_iterator<bit_finite_set>;
        using const_iterator         = iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // 23.4.6.2, construct/copy/destroy
        [[nodiscard]] constexpr bit_finite_set() noexcept = default;

        template<std::input_iterator I, std::sentinel_for<I> S>
        [[nodiscard]] constexpr bit_finite_set(I first, S last) noexcept
                requires std::constructible_from<value_type, decltype(*first)>
        {
                insert(first, last);
        }

        template<std::ranges::input_range R>
        [[nodiscard]] constexpr bit_finite_set(std::from_range_t, R&& rg) noexcept
                requires std::constructible_from<value_type, decltype(*std::ranges::begin(rg))>
        {
                insert(std::ranges::begin(rg), std::ranges::end(rg));
        }

        [[nodiscard]] constexpr bit_finite_set(std::initializer_list<value_type> il) noexcept
        {
                insert(il.begin(), il.end());
        }

        constexpr auto operator=(std::initializer_list<value_type> il) noexcept
                -> bit_finite_set&
        {
                m_bits.reset();
                insert(il.begin(), il.end());
                return *this;
        }

        friend constexpr auto operator==  <>(const bit_finite_set&, const bit_finite_set&) noexcept -> bool;
        friend constexpr auto operator<=> <>(const bit_finite_set&, const bit_finite_set&) noexcept -> std::strong_ordering;

        // iterators
        [[nodiscard]] constexpr auto begin (this auto&& self) noexcept { return xstd::ranges::set_begin(self); }
        [[nodiscard]] constexpr auto end   (this auto&& self) noexcept { return xstd::ranges::set_end  (self); }
        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept { return std::make_reverse_iterator(self.end()  ); }
        [[nodiscard]] constexpr auto rend  (this auto&& self) noexcept { return std::make_reverse_iterator(self.begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept { return begin();  }
        [[nodiscard]] constexpr auto cend()    const noexcept { return end();    }
        [[nodiscard]] constexpr auto crbegin() const noexcept { return rbegin(); }
        [[nodiscard]] constexpr auto crend()   const noexcept { return rend();   }

        // capacity
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_bits.none(); }
        [[nodiscard]] constexpr auto full()  const noexcept -> bool { return m_bits.all();  }

        [[nodiscard]]        constexpr auto     size() const noexcept -> size_type { return m_bits.count();           }
        [[nodiscard]] static constexpr auto max_size()       noexcept -> size_type { return N; }

        // element access
        [[nodiscard]] constexpr auto front() const noexcept -> reference { return { *this, m_bits.find_front() }; }
        [[nodiscard]] constexpr auto back()  const noexcept -> reference { return { *this, m_bits.find_back()  }; }

        // 23.4.6.4, modifiers
        template<class... Args>
        constexpr auto emplace(Args&&... args) noexcept
                -> std::pair<iterator, bool>
                requires (sizeof...(args) == 1)
        {
                return do_insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(const_iterator position, Args&&... args) noexcept
                -> iterator
                requires (sizeof...(args) == 1)
        {
                return do_insert(position, value_type(std::forward<Args>(args)...));
        }

        constexpr auto insert(                         const value_type&  x) noexcept -> std::pair<iterator, bool> { return do_insert(          x);            }
        constexpr auto insert(                               value_type&& x) noexcept -> std::pair<iterator, bool> { return do_insert(          std::move(x)); }
        constexpr auto insert(const_iterator position, const value_type&  x) noexcept ->           iterator        { return do_insert(position, x);            }
        constexpr auto insert(const_iterator position,       value_type&& x) noexcept ->           iterator        { return do_insert(position, std::move(x)); }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr void insert(I first, S last) noexcept
                requires std::constructible_from<value_type, decltype(*first)>
        {
                while (first != last) {
                        m_bits.set(*first++);
                }
        }

        template<std::ranges::input_range R>
        constexpr void insert_range(R&& rg) noexcept
                requires std::constructible_from<value_type, decltype(*std::ranges::begin(rg))>
        {
                insert(std::ranges::begin(rg), std::ranges::end(rg));
        }

        constexpr void insert(std::initializer_list<value_type> ilist) noexcept
        {
                insert(ilist.begin(), ilist.end());
        }

        constexpr void fill() noexcept
        {
                m_bits.set();
        }

        constexpr auto erase(const_iterator position) noexcept
                -> iterator
        {
                assert(position != end());
                m_bits.reset(*position++);
                return position;
        }

        constexpr auto erase(const key_type& x) noexcept
                -> size_type
        {
                // The one guard stopping a write: without it an out-of-range key clears a foreign bit. [design.md#total-lookups-on-the-container]
                return x < N and m_bits.erase(x);
        }

        constexpr auto erase(const_iterator first, const_iterator last) noexcept
                -> iterator
        {
                while (first != last) {
                        m_bits.reset(*first++);
                }
                return last;
        }

        constexpr void swap(bit_finite_set& other) noexcept(noexcept(this->m_bits.swap(other.m_bits)))
        {
                this->m_bits.swap(other.m_bits);
        }

        constexpr void clear() noexcept
        {
                m_bits.reset();
        }

        constexpr void complement(value_type x) noexcept
        {
                m_bits.flip(x);
        }

        constexpr void complement() noexcept
        {
                m_bits.flip();
        }

        constexpr auto operator&=(const bit_finite_set& other) noexcept -> bit_finite_set& { this->m_bits &= other.m_bits; return *this; }
        constexpr auto operator|=(const bit_finite_set& other) noexcept -> bit_finite_set& { this->m_bits |= other.m_bits; return *this; }
        constexpr auto operator^=(const bit_finite_set& other) noexcept -> bit_finite_set& { this->m_bits ^= other.m_bits; return *this; }
        constexpr auto operator-=(const bit_finite_set& other) noexcept -> bit_finite_set& { this->m_bits -= other.m_bits; return *this; }

        constexpr auto operator<<=(std::size_t n) noexcept -> bit_finite_set& { m_bits <<= n; return *this; }
        constexpr auto operator>>=(std::size_t n) noexcept -> bit_finite_set& { m_bits >>= n; return *this; }

        // observers
        [[nodiscard]] constexpr auto   key_comp() const noexcept -> key_compare   { return {}; }
        [[nodiscard]] constexpr auto value_comp() const noexcept -> value_compare { return {}; }

        // set operations, every one total over key_type as std::set's are. [design.md#total-lookups-on-the-container]
        [[nodiscard]] constexpr auto contains(const key_type& x) const noexcept -> bool      { return x < N and m_bits.test(x); }
        [[nodiscard]] constexpr auto count   (const key_type& x) const noexcept -> size_type { return contains(x);         }

        [[nodiscard]] constexpr auto find(this auto&& self, const key_type& x) noexcept -> iterator
        {
                if (self.contains(x)) {
                        return { &self, x };
                }
                return self.end();
        }
        // The two bounds differ by the + 1 that separates them, an inclusive primitive having no seam at 0.
        [[nodiscard]] constexpr auto lower_bound(this auto&& self, const key_type& x) noexcept -> iterator                      { return { &self, x < N ? self.m_bits.inclusive_find_next(x    ) : N }; }
        [[nodiscard]] constexpr auto upper_bound(this auto&& self, const key_type& x) noexcept -> iterator                      { return { &self, x < N ? self.m_bits.inclusive_find_next(x + 1) : N }; }
        [[nodiscard]] constexpr auto equal_range(this auto&& self, const key_type& x) noexcept -> std::pair<iterator, iterator> { return { self.lower_bound(x), self.upper_bound(x) };               }

        [[nodiscard]] constexpr auto is_subset_of       (const bit_finite_set& other) const noexcept -> bool { return this->m_bits.is_subset_of       (other.m_bits); }
        [[nodiscard]] constexpr auto is_proper_subset_of(const bit_finite_set& other) const noexcept -> bool { return this->m_bits.is_proper_subset_of(other.m_bits); }
        [[nodiscard]] constexpr auto intersects         (const bit_finite_set& other) const noexcept -> bool { return this->m_bits.intersects         (other.m_bits); }

private:
        constexpr auto do_insert(                value_type x) noexcept -> std::pair<iterator, bool> {                return { { this, x }, m_bits.insert(x) }; }
        constexpr auto do_insert(const_iterator, value_type x) noexcept ->           iterator        { m_bits.set(x); return   { this, x };                     }
};

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator== (const bit_finite_set<N, Block>& x, const bit_finite_set<N, Block>& y) noexcept -> bool { return x.m_bits == y.m_bits; }

// bit_finite_set orders as std::set<int> does: lexicographically over its ascending sequence of set-bit indices.
template<std::size_t N, xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto operator<=>(const bit_finite_set<N, Block>& x, const bit_finite_set<N, Block>& y) noexcept
        -> std::strong_ordering
{
        return ranges::set_three_way(x, y);
}
template<std::size_t N, xstd::unsigned_integer Block>               constexpr void swap       (      bit_finite_set<N, Block>& x,       bit_finite_set<N, Block>& y) noexcept(noexcept(x.swap(y)))    { x.swap(y);                    }

// 23.4.6.3 Erasure                                                [set.erasure]
template<std::size_t N, xstd::unsigned_integer Block, class Predicate>
constexpr auto erase_if(bit_finite_set<N, Block>& c, Predicate pred)
        -> bit_finite_set<N, Block>::size_type
{
        auto original_size = c.size();
        for (auto i = c.begin(), last = c.end(); i != last;) {
                if (pred(*i)) {
                        i = c.erase(i);
                } else {
                        ++i;
                }
        }
        return original_size - c.size();
}

// bitwise operators
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator~(const bit_finite_set<N, Block>& lhs) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv.complement(); return nrv; }

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator&(const bit_finite_set<N, Block>& lhs, const bit_finite_set<N, Block>& rhs) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv &= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator|(const bit_finite_set<N, Block>& lhs, const bit_finite_set<N, Block>& rhs) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv |= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator^(const bit_finite_set<N, Block>& lhs, const bit_finite_set<N, Block>& rhs) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator-(const bit_finite_set<N, Block>& lhs, const bit_finite_set<N, Block>& rhs) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv -= rhs; return nrv; }

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator<<(const bit_finite_set<N, Block>& lhs, std::size_t n) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv <<= n; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator>>(const bit_finite_set<N, Block>& lhs, std::size_t n) noexcept -> bit_finite_set<N, Block> { auto nrv = lhs; nrv >>= n; return nrv; }


// Each entry unwraps to the storage's own door; one friend declaration replaces the six hidden friends. [design.md#the-door]
template<std::size_t N, xstd::unsigned_integer Block>
struct bit_traits<bit_finite_set<N, Block>>
{
        using bits_type = bit_finite_set<N, Block>;
        using backend   = bit_traits<block_array<Block, N>>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size (bits_type const& c)                noexcept -> std::size_t { return backend::size(c.m_bits);  }
        [[nodiscard]] static constexpr auto at   (bits_type const& c, std::size_t n) noexcept -> bool        { return backend::at(c.m_bits, n); }
        [[nodiscard]] static constexpr auto count(bits_type const& c)                noexcept -> std::size_t { return backend::count(c.m_bits); }

        static constexpr void assign(bits_type& c, std::size_t n, bool value) noexcept { backend::assign(c.m_bits, n, value); }
        static constexpr void insert(bits_type& c, std::size_t n)             noexcept { backend::insert(c.m_bits, n);        }
        static constexpr void fill  (bits_type& c, bool value)                noexcept { backend::fill(c.m_bits, value);      }

        [[nodiscard]] static constexpr auto num_blocks(bits_type const& c)                noexcept -> std::size_t { return backend::num_blocks(c.m_bits); }
        [[nodiscard]] static constexpr auto block     (bits_type const& c, std::size_t i) noexcept                { return backend::block(c.m_bits, i);   }

        [[nodiscard]] static constexpr auto find_first(bits_type const& c)                noexcept -> std::size_t { return backend::find_first(c.m_bits);   }
        [[nodiscard]] static constexpr auto find_last (bits_type const& c)                noexcept -> std::size_t { return backend::find_last(c.m_bits);    }
        [[nodiscard]] static constexpr auto find_next (bits_type const& c, std::size_t n) noexcept -> std::size_t { return backend::find_next(c.m_bits, n); }
        [[nodiscard]] static constexpr auto find_prev (bits_type const& c, std::size_t n) noexcept -> std::size_t { return backend::find_prev(c.m_bits, n); }

        [[nodiscard]] static constexpr auto set_three_way     (bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return backend::set_three_way(x.m_bits, y.m_bits);      }
        [[nodiscard]] static constexpr auto sequence_three_way(bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return backend::sequence_three_way(x.m_bits, y.m_bits); }
};

}       // namespace xstd

#endif // XSTD_BITS_BIT_FINITE_SET_HPP
