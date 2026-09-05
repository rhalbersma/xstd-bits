//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

// Header <array> synopsis                                           [array.syn]

#include <compare> // strong_ordering

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/memory.hpp>                    // align_up
#include <cstddef>                                 // size_t
#include <limits>                                  // digits

namespace xstd {

// 23.3.3, class template bit_array
template<std::size_t N, xstd::unsigned_integer Block> struct bit_array;

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator== (const bit_array<N, Block>& x, const bit_array<N, Block>& y) noexcept -> bool;
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator<=>(const bit_array<N, Block>& x, const bit_array<N, Block>& y) noexcept -> std::strong_ordering;

// 23.3.4, specialized algorithms
template<std::size_t N, xstd::unsigned_integer Block>               constexpr void swap       (      bit_array<N, Block>& x,       bit_array<N, Block>& y) noexcept(noexcept(x.swap(y)));

namespace aligned {

template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
using bit_array = xstd::bit_array<xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), Block>;

}       // namespace aligned
}       // namespace xstd

// NOLINTBEGIN(readability-duplicate-include): synopsis and implementation each list what that section needs.
#include <xstd/bits/block_sequence.hpp> // block_array
#include <xstd/bits/ranges.hpp>         // begin, end, iterator, reference
#include <xstd/ints/memory.hpp>         // align_up
#include <cassert>                      // assert
#include <compare>                      // strong_ordering
#include <cstddef>                      // ptrdiff_t, size_t
#include <format>                       // format
#include <iterator>                     // make_reverse_iterator, reverse_iterator,
#include <limits>                       // digits
#include <source_location>              // source_location
#include <stdexcept>                    // out_of_range
#include <type_traits>                  // conditional_t
// NOLINTEND(readability-duplicate-include)

// Class template array [array], Overview [array.overview]

namespace xstd {

template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
struct bit_array
{
        block_array<Block, N> m_bits;

        // ADL rather than a specialization, because this type is ours to add hidden friends to.
        [[nodiscard]] friend constexpr auto block_count(const bit_array& c) noexcept -> std::size_t { return c.m_bits.num_blocks(); }
        [[nodiscard]] friend constexpr auto block_at(const bit_array& c, std::size_t i) noexcept -> Block { return c.m_bits.block(i); }

        [[nodiscard]] friend constexpr auto find_first(const bit_array&)                  noexcept -> std::size_t { return 0UZ;         }
        [[nodiscard]] friend constexpr auto find_last (const bit_array&)                  noexcept -> std::size_t { return N;           }
        [[nodiscard]] friend constexpr auto find_at   (const bit_array& c, std::size_t n) noexcept -> std::size_t { return c.m_bits.test(n); }
        friend constexpr void assign_at(bit_array& c, std::size_t n, bool value) noexcept { if (value) { c.m_bits.set(n); } else { c.m_bits.reset(n); } }

        // types
        using value_type             = bool;
        using block_type             = Block;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = xstd::ranges::sequence_reference<bit_array, false>;
        using const_reference        = xstd::ranges::sequence_reference<bit_array, true >;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = xstd::ranges::sequence_iterator<bit_array, false>;
        using const_iterator         = xstd::ranges::sequence_iterator<bit_array, true >;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // no explicit construct/copy/destroy for aggregate type

        constexpr void fill(const value_type& u) noexcept
        {
                if (u) {
                        m_bits.set();
                } else {
                        m_bits.reset();
                }
        }

        constexpr void swap(bit_array& other) noexcept(noexcept(this->m_bits.swap(other.m_bits)))
        {
                this->m_bits.swap(other.m_bits);
        } 

        // iterators
        [[nodiscard]] constexpr auto begin (this auto&& self) noexcept { return xstd::ranges::sequence_begin(self); }
        [[nodiscard]] constexpr auto end   (this auto&& self) noexcept { return xstd::ranges::sequence_end  (self); }
        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept { return std::make_reverse_iterator(self.end()  ); }
        [[nodiscard]] constexpr auto rend  (this auto&& self) noexcept { return std::make_reverse_iterator(self.begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept { return begin();  }
        [[nodiscard]] constexpr auto cend()    const noexcept { return end();    }
        [[nodiscard]] constexpr auto crbegin() const noexcept { return rbegin(); }
        [[nodiscard]] constexpr auto crend()   const noexcept { return rend();   }

        // capacity
        [[nodiscard]] constexpr auto    empty() const noexcept -> bool      { return N == 0; }
        [[nodiscard]] constexpr auto     size() const noexcept -> size_type { return N;      }
        [[nodiscard]] constexpr auto max_size() const noexcept -> size_type { return N;      }

        // element access; an explicit trailing return type, because plain auto cannot deduce from a braced-init-list return.
        template<class Self>
        using result_t = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const_reference, reference>;

        // The assert is on its own line: the coverage job drops assert branches by matching the start of the line.
        [[nodiscard]] constexpr auto operator[](this auto&& self, size_type n) noexcept -> result_t<decltype(self)>
        {
                assert(n < N);
                return { self, n };
        }

        [[nodiscard]] constexpr auto at(this auto&& self, size_type n) -> result_t<decltype(self)>
        {
                if (n < N) {
                        return { self, n };
                }
                throw out_of_range(n);
        }

        [[nodiscard]] constexpr auto front(this auto&& self) noexcept -> result_t<decltype(self)> { return { self, 0UZ   }; }
        [[nodiscard]] constexpr auto back (this auto&& self) noexcept -> result_t<decltype(self)> { return { self, N - 1 }; }

private:
        static constexpr auto out_of_range(std::size_t n, std::source_location const& loc = std::source_location::current())
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘n‘ is out of range [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), n, N
                        )
                );
        }        
};

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator== (const bit_array<N, Block>& x, const bit_array<N, Block>& y) noexcept -> bool { return x.m_bits == y.m_bits; }

// bit_array orders as a sequence of bool over every index, which is what std::array<bool, N>'s <=> would compute.
template<std::size_t N, xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto operator<=>(const bit_array<N, Block>& x, const bit_array<N, Block>& y) noexcept
        -> std::strong_ordering
{
        return ranges::sequence_three_way(x, y);
}

template<std::size_t N, xstd::unsigned_integer Block> constexpr void swap(bit_array<N, Block>& x, bit_array<N, Block>& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }


// Each entry unwraps to the storage's own door, which already answers all of them. [design.md#the-door]
template<std::size_t N, xstd::unsigned_integer Block>
struct bit_traits<bit_array<N, Block>>
{
        using bits_type = bit_array<N, Block>;
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

#endif // XSTD_BITS_BIT_ARRAY_HPP
