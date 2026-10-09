//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP
#define XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP

#include <cstddef>     // size_t
#include <iterator>    // input_iterator
#include <new>         // bad_alloc
#include <type_traits> // conditional_t, is_const_v, is_empty_v, remove_reference_t
#include <version>     // IWYU pragma: keep; __cpp_lib_inplace_vector

namespace xstd::bits::detail {

// The blocks of a capacity of nought: none, in an empty trivial type, as std::inplace_vector<Block, 0> holds them.
template<class Block>
class no_blocks
{
        template<class Self>
        using pointer_t = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, Block const*, Block*>;

public:
        // Nothing held, so any two are equal.
        [[nodiscard]] friend constexpr auto operator==(no_blocks const&, no_blocks const&) noexcept
                -> bool
        {
                return true;
        }

        // The null pointers of an empty contiguous range.
        template<class Self>
        [[nodiscard]] constexpr auto data(this Self&& /* self */) noexcept
                -> pointer_t<Self>
        {
                return nullptr;
        }

        [[nodiscard]] constexpr auto begin(this auto&& self) noexcept
                -> pointer_t<decltype(self)>
        {
                return self.data();
        }

        [[nodiscard]] constexpr auto end(this auto&& self) noexcept
                -> pointer_t<decltype(self)>
        {
                return self.data();
        }

        // The subscript is the built-in one: a member would have to be defined for an index no call can pass.
        [[nodiscard]] constexpr operator Block*() noexcept // NOLINT(misc-explicit-constructor): the subscript is this conversion.
        {
                return data();
        }

        [[nodiscard]] constexpr operator Block const*() const noexcept // NOLINT(misc-explicit-constructor): the subscript is this conversion.
        {
                return data();
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 0UZ;
        }

        [[nodiscard]] static constexpr auto capacity() noexcept
                -> std::size_t
        {
                return 0UZ;
        }

        [[nodiscard]] static constexpr auto max_size() noexcept
                -> std::size_t
        {
                return 0UZ;
        }

        // Growth past nought is std::inplace_vector's bad_alloc, and a resize to nought leaves nothing to change.
        static constexpr auto resize(std::size_t n, Block const& /* value */)
                -> void
        {
                if (n != 0UZ) {
                        throw std::bad_alloc();
                }
        }

        [[noreturn]] static constexpr auto push_back(Block const& /* value */)
                -> void
        {
                throw std::bad_alloc();
        }

        template<std::input_iterator I>
        static constexpr auto insert(Block const* /* pos */, I first, I last)
                -> void
        {
                if (first != last) {
                        throw std::bad_alloc();
                }
        }

        static constexpr auto clear() noexcept
                -> void
        {}
};

} // namespace xstd::bits::detail

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

// The bounded owners are constant-evaluable exactly where std::inplace_vector holds their blocks.
#define XSTD_BITS_HAS_CONSTEXPR_BOUNDED 1

namespace xstd::bits::detail {

template<class Block, std::size_t K>
using bounded_blocks = std::inplace_vector<Block, K>;

} // namespace xstd::bits::detail

#else

#include <boost/container/static_vector.hpp> // static_vector

namespace xstd::bits::detail {

// Inline blocks under a capacity the type carries, as std::inplace_vector's are, but not constant-evaluable.
template<class Block, std::size_t K>
using bounded_blocks = boost::container::static_vector<Block, K>;

} // namespace xstd::bits::detail

#endif

namespace xstd::bits::detail {

// An owner's K blocks: no_blocks at nought where the library's own type keeps a size, so the owner is an empty type.
template<class Block, std::size_t K>
using bounded_blocks_for = std::conditional_t<K == 0UZ and not std::is_empty_v<bounded_blocks<Block, 0>>, no_blocks<Block>, bounded_blocks<Block, K>>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BOUNDED_BLOCKS_HPP
