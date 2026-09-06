//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_FACTORY_HPP
#define TEST_BITSET_FACTORY_HPP

#include <boost/dynamic_bitset_fwd.hpp> // dynamic_bitset
#include <test/dynamic.hpp>             // dynamic
#include <concepts>                     // unsigned_integral
#include <cstddef>                      // size_t

namespace test::bitset {

// A static width ignores the count; a growing one, ours or boost's, is resized to it.
template<class T>
struct factory
{
        constexpr auto operator()(std::size_t num_bits, bool value = false) const noexcept
        {
                T b;
                if constexpr (dynamic<T>) {
                        b.resize(num_bits, value);
                } else if (value) {
                        b.set();
                } else {
                        b.reset();
                }
                return b;
        }
};

template<std::unsigned_integral Block, class Allocator>
struct factory<boost::dynamic_bitset<Block, Allocator>>
{
        constexpr auto operator()(std::size_t num_bits, bool value = false) const noexcept
        {
                boost::dynamic_bitset<Block, Allocator> b;
                b.resize(num_bits, value);
                return b;
        }
};

template<class T>
auto make_bitset(std::size_t num_bits, bool value = false)
{
        return factory<T>()(num_bits, value);
}

} // namespace test::bitset

#endif // TEST_BITSET_FACTORY_HPP
