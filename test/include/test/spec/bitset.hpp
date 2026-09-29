//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_BITSET_HPP
#define TEST_SPEC_BITSET_HPP

#include <test/bitset/exhaustive.hpp>           // L0, L1, L2, L3, L4, limit_v, on0, on1, on2, on3, on4
#include <test/bitset/factory.hpp>              // make_bitset
#include <test/dynamic.hpp>                     // dynamic
#include <test/set/exhaustive.hpp>              // static_capacity
#include <test/spec/input.hpp>                  // edge, exhaustive, key_vector, keyed, memo, one, rebuild, rebuilt, two
#include <test/spec/random.hpp>                 // bitset_width, block_digits_v, key_samples, keyed_samples, pair_samples
#include <test/uint128.hpp>                     // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                 // basic_bitset
#include <xstd/bits/bounded_bitset.hpp>         // basic_bounded_bitset
#include <xstd/bits/detail/ownership.hpp>       // owned_storage
#include <xstd/bits/dynamic_bitset.hpp>         // basic_dynamic_bitset
#include <xstd/bits/ext/boost/small_bitset.hpp> // basic_small_bitset
#include <boost/dynamic_bitset.hpp>             // dynamic_bitset
#include <algorithm>                            // find
#include <bitset>                               // bitset
#include <cstddef>                              // size_t
#include <cstdint>                              // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                               // iota, to
#include <tuple>                                // tuple, tuple_cat
#include <utility>                              // declval, move
#include <vector>                               // vector

// The candidates for the bitset reading, the models first, and the inputs a clause checks them over.
namespace test::spec::bitset {

// A static width is std::bitset's and a run-time width boost::dynamic_bitset's.
using models = std::tuple<std::bitset<0>, std::bitset<1>, std::bitset<8>, std::bitset<17>, std::bitset<31>, std::bitset<32>, std::bitset<33>, std::bitset<63>, std::bitset<64>, std::bitset<65>, std::bitset<1025>, boost::dynamic_bitset<>>;

// Every Block empty, at a single bit, either side of its first two block boundaries, at 8, 17 and 24, and far past.
using fixed = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 1>, xstd::basic_bitset<std::uint8_t, 7>, xstd::basic_bitset<std::uint8_t, 8>, xstd::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 15>, xstd::basic_bitset<std::uint8_t, 16>, xstd::basic_bitset<std::uint8_t, 17>, xstd::basic_bitset<std::uint8_t, 24>, xstd::basic_bitset<std::uint8_t, 257>, xstd::basic_bitset<std::uint16_t, 0>, xstd::basic_bitset<std::uint16_t, 1>, xstd::basic_bitset<std::uint16_t, 8>, xstd::basic_bitset<std::uint16_t, 15>, xstd::basic_bitset<std::uint16_t, 16>, xstd::basic_bitset<std::uint16_t, 17>, xstd::basic_bitset<std::uint16_t, 24>, xstd::basic_bitset<std::uint16_t, 31>, xstd::basic_bitset<std::uint16_t, 32>, xstd::basic_bitset<std::uint16_t, 33>, xstd::basic_bitset<std::uint16_t, 48>, xstd::basic_bitset<std::uint32_t, 0>, xstd::basic_bitset<std::uint32_t, 1>, xstd::basic_bitset<std::uint32_t, 8>, xstd::basic_bitset<std::uint32_t, 17>, xstd::basic_bitset<std::uint32_t, 24>, xstd::basic_bitset<std::uint32_t, 31>, xstd::basic_bitset<std::uint32_t, 32>, xstd::basic_bitset<std::uint32_t, 33>, xstd::basic_bitset<std::uint32_t, 63>, xstd::basic_bitset<std::uint32_t, 64>, xstd::basic_bitset<std::uint32_t, 65>, xstd::basic_bitset<std::uint32_t, 1023>, xstd::basic_bitset<std::uint64_t, 0>, xstd::basic_bitset<std::uint64_t, 1>, xstd::basic_bitset<std::uint64_t, 8>, xstd::basic_bitset<std::uint64_t, 17>, xstd::basic_bitset<std::uint64_t, 24>, xstd::basic_bitset<std::uint64_t, 63>, xstd::basic_bitset<std::uint64_t, 64>, xstd::basic_bitset<std::uint64_t, 65>, xstd::basic_bitset<std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                         ,
                         xstd::basic_bitset<xstd::uint128, 0>, xstd::basic_bitset<xstd::uint128, 1>, xstd::basic_bitset<xstd::uint128, 8>, xstd::basic_bitset<xstd::uint128, 17>, xstd::basic_bitset<xstd::uint128, 24>, xstd::basic_bitset<xstd::uint128, 127>, xstd::basic_bitset<xstd::uint128, 128>, xstd::basic_bitset<xstd::uint128, 129>, xstd::basic_bitset<xstd::uint128, 2049>

#endif
                         >;

using dynamic = std::tuple<xstd::basic_dynamic_bitset<std::uint8_t>, xstd::basic_dynamic_bitset<std::uint64_t>>;

using bounded = std::tuple<xstd::basic_bounded_bitset<std::uint8_t, 0>, xstd::basic_bounded_bitset<std::uint8_t, 9>, xstd::basic_bounded_bitset<std::uint8_t, 17>, xstd::basic_bounded_bitset<std::uint8_t, 24>, xstd::basic_bounded_bitset<std::uint64_t, 24>, xstd::basic_bounded_bitset<std::uint64_t, 65>, xstd::basic_bounded_bitset<std::uint64_t, 4097>>;

// Inline blocks that a sweep to its limit spills from, that a sweep fits in, and that hold half of what a sample spans.
using small = std::tuple<xstd::basic_small_bitset<std::uint8_t, 9>, xstd::basic_small_bitset<std::uint64_t, 64>, xstd::basic_small_bitset<std::uint64_t, 1024>>;

using all = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed>(), std::declval<dynamic>(), std::declval<bounded>(), std::declval<small>()));

// The positions a bitset holds without growing: a width or capacity in its type, a small one's inline blocks, or none.
template<class X>
inline constexpr auto held_width_v = [] -> std::size_t {
        if constexpr (not test::dynamic<X>) {
                return X().size();
        } else if constexpr (test::set::static_capacity<X>) {
                return xstd::bits::detail::owned_storage<X>::bits_type::static_capacity();
        } else {
                return 0UZ;
        }
}();

template<class Block, std::size_t N, class Allocator>
inline constexpr auto held_width_v<xstd::basic_small_bitset<Block, N, Allocator>> = N;

#ifdef __clang__

// Clang shows a parameterless constexpr function can be constant by running it, here the whole enumeration.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-constexpr"

#endif

namespace inputs {

// A linear enumeration runs up to 64 positions, any other up to 17, and an unbounded bitset's at its limit.
template<class X>
inline constexpr auto linear = held_width_v<X> <= 64UZ;

template<class X>
inline constexpr auto quadratic = held_width_v<X> <= 17UZ;

template<class X>
inline constexpr auto quartic = held_width_v<X> <= 17UZ;

// What a bitset generator sets, as the positions set and the width they are set in, from which each type's is rebuilt.
class bit_positions
{
        std::size_t m_width = 0;
        key_vector m_set;

public:
        using size_type = std::size_t;

        [[nodiscard]] bit_positions() = default;

        // Distinct positions below a width, as a sample draws them.
        [[nodiscard]] constexpr bit_positions(size_type n, key_vector positions) noexcept
                : m_width(n)
                , m_set(std::move(positions))
        {}

        constexpr auto resize(size_type n, bool value = false)
                -> void
        {
                m_width = n;
                m_set.clear();
                if (value) {
                        m_set = std::views::iota(0UZ, n) | std::ranges::to<key_vector>();
                }
        }

        constexpr auto set(size_type pos)
                -> bit_positions&
        {
                if (std::ranges::find(m_set, pos) == m_set.end()) {
                        m_set.push_back(pos);
                }
                return *this;
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return m_width;
        }

        [[nodiscard]] constexpr auto positions() const noexcept
                -> key_vector const&
        {
                return m_set;
        }

        [[nodiscard]] constexpr auto count() const noexcept
                -> size_type
        {
                return m_set.size();
        }

        [[nodiscard]] constexpr auto none() const noexcept
                -> bool
        {
                return m_set.empty();
        }

        [[nodiscard]] constexpr auto all() const noexcept
                -> bool
        {
                return m_set.size() == m_width;
        }
};

// The positions of each enumeration, at the limits a type's sweep takes, shared by every type with the same limits.
namespace positions_of {

using test::bitset::on0::empty_set;
using test::bitset::on0::full_set;
using test::bitset::on1::all_cardinality_sets;
using test::bitset::on1::all_singleton_sets;
using test::bitset::on1::any_value;
using test::bitset::on2::all_doubleton_sets;
using test::bitset::on2::all_singleton_set_pairs;
using test::bitset::on3::all_triplet_sets;
using test::bitset::on4::all_doubleton_set_pairs;

[[nodiscard]] constexpr auto bitsets(std::size_t n0, std::size_t n1, bool sweep)
        -> std::vector<one<bit_positions>>
{
        auto result = std::vector<one<bit_positions>>();
        empty_set<bit_positions>([&](auto const& a) -> void { result.push_back({.from = edge("empty", n0), .a = a}); }, n0);
        full_set<bit_positions>([&](auto const& a) -> void { result.push_back({.from = edge("full", n0), .a = a}); }, n0);
        if (sweep) {
                all_cardinality_sets<bit_positions>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every cardinality", n1), .a = a}); }, n1);
                all_singleton_sets<bit_positions>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton", n1), .a = a}); }, n1);
        }
        return result;
}

[[nodiscard]] constexpr auto pairs(std::size_t n0, std::size_t n2, bool sweep)
        -> std::vector<two<bit_positions>>
{
        auto result = std::vector<two<bit_positions>>();
        empty_set<bit_positions>(
                [&](auto const& e) -> void {
                        full_set<bit_positions>(
                                [&](auto const& f) -> void {
                                        result.push_back({.from = edge("empty and empty", n0), .a = e, .b = e});
                                        result.push_back({.from = edge("empty and full", n0), .a = e, .b = f});
                                        result.push_back({.from = edge("full and empty", n0), .a = f, .b = e});
                                        result.push_back({.from = edge("full and full", n0), .a = f, .b = f});
                                },
                                n0
                        );
                },
                n0
        );
        if (sweep) {
                all_singleton_set_pairs<bit_positions>([&](auto const& a, auto const& b) -> void { result.push_back({.from = exhaustive("every singleton pair", n2), .a = a, .b = b}); }, n2);
        }
        return result;
}

// The empty and the full bitset at width n, each against b.
constexpr auto against_edges(std::vector<two<bit_positions>>& result, char const* name, std::size_t n, bit_positions const& b)
        -> void
{
        empty_set<bit_positions>([&](auto const& e) -> void { result.push_back({.from = exhaustive(name, n), .a = e, .b = b}); }, n);
        full_set<bit_positions>([&](auto const& f) -> void { result.push_back({.from = exhaustive(name, n), .a = f, .b = b}); }, n);
}

[[nodiscard]] constexpr auto edges_and_singletons(std::size_t n1)
        -> std::vector<two<bit_positions>>
{
        auto result = std::vector<two<bit_positions>>();
        all_singleton_sets<bit_positions>([&](auto const& b) -> void { against_edges(result, "edges and every singleton", n1, b); }, n1);
        return result;
}

[[nodiscard]] constexpr auto edges_and_doubletons(std::size_t n4)
        -> std::vector<two<bit_positions>>
{
        auto result = std::vector<two<bit_positions>>();
        all_doubleton_sets<bit_positions>([&](auto const& b) -> void { against_edges(result, "edges and every doubleton", n4, b); }, n4);
        return result;
}

[[nodiscard]] constexpr auto triplets_and_doubleton_pairs(std::size_t n3, std::size_t n4)
        -> std::vector<two<bit_positions>>
{
        auto result = std::vector<two<bit_positions>>();
        all_triplet_sets<bit_positions>([&](auto const& b) -> void { against_edges(result, "edges and every triplet", n3, b); }, n3);
        all_doubleton_sets<bit_positions>(
                [&](auto const& b) -> void {
                        all_singleton_sets<bit_positions>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton and doubleton", n3), .a = a, .b = b}); }, n3);
                },
                n3
        );
        all_doubleton_set_pairs<bit_positions>([&](auto const& a, auto const& b) -> void { result.push_back({.from = exhaustive("every doubleton pair", n4), .a = a, .b = b}); }, n4);
        return result;
}

[[nodiscard]] constexpr auto positions(std::size_t n1, bool sweep)
        -> std::vector<keyed<bit_positions>>
{
        auto result = std::vector<keyed<bit_positions>>();
        auto const with_positions = [&](char const* name, char const* every, bit_positions const& a) -> void {
                if (sweep) {
                        any_value<bit_positions>([&](std::size_t pos) -> void { result.push_back({.from = exhaustive(every, n1), .a = a, .k = pos}); }, n1);
                } else {
                        result.push_back({.from = edge(name, n1), .a = a, .k = 0UZ});
                        result.push_back({.from = edge(name, n1), .a = a, .k = n1 - 1UZ});
                        result.push_back({.from = edge(name, n1), .a = a, .k = n1});
                }
        };
        empty_set<bit_positions>([&](auto const& a) -> void { with_positions("empty", "empty at every position", a); }, n1);
        full_set<bit_positions>([&](auto const& a) -> void { with_positions("full", "full at every position", a); }, n1);
        return result;
}

[[nodiscard]] constexpr auto singleton_positions(std::size_t n1)
        -> std::vector<keyed<bit_positions>>
{
        auto result = std::vector<keyed<bit_positions>>();
        any_value<bit_positions>(
                [&](std::size_t pos) -> void {
                        all_singleton_sets<bit_positions>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton at every position", n1), .a = a, .k = pos}); }, n1);
                },
                n1
        );
        return result;
}

// Samples at width n, as the positions they set.
template<template<class> class Input>
[[nodiscard]] constexpr auto at_width(std::vector<Input<key_vector>> const& samples, std::size_t n)
        -> std::vector<Input<bit_positions>>
{
        auto const positions = [n](key_vector const& keys) -> bit_positions { return {n, keys}; };
        auto result = std::vector<Input<bit_positions>>();
        result.reserve(samples.size());
        for (auto const& x : samples) {
                result.push_back(rebuild<bit_positions>(x, positions));
        }
        return result;
}

// Sampled bitsets, sampled pairs, and sampled bitsets at the positions a lookup most often gets wrong, at width n.
[[nodiscard]] inline auto sampled_bitsets(std::size_t n, std::size_t digits)
        -> std::vector<one<bit_positions>>
{
        return at_width(random::key_samples(n, digits), n);
}

[[nodiscard]] inline auto sampled_pairs(std::size_t n, std::size_t digits)
        -> std::vector<two<bit_positions>>
{
        return at_width(random::pair_samples(n, digits), n);
}

[[nodiscard]] inline auto sampled_positions(std::size_t n, std::size_t digits)
        -> std::vector<keyed<bit_positions>>
{
        return at_width(random::keyed_samples(n, digits), n);
}

} // namespace positions_of

template<class X>
inline constexpr auto n0 = test::bitset::limit_v<X, test::bitset::L0>;

template<class X>
inline constexpr auto n1 = test::bitset::limit_v<X, test::bitset::L1>;

template<class X>
inline constexpr auto n2 = test::bitset::limit_v<X, test::bitset::L2>;

template<class X>
inline constexpr auto n3 = test::bitset::limit_v<X, test::bitset::L3>;

template<class X>
inline constexpr auto n4 = test::bitset::limit_v<X, test::bitset::L4>;

template<class X>
struct bitset_of
{
        [[nodiscard]] constexpr auto operator()(bit_positions const& p) const
                -> X
        {
                auto x = test::bitset::make_bitset<X>(p.size());
                for (auto const pos : p.positions()) {
                        x.set(pos);
                }
                return x;
        }
};

template<class X, class Carrier>
using over = rebuilt<X, Carrier, bitset_of<X>>;

// The empty and the full bitset, every cardinality, and every singleton.
template<class X>
[[nodiscard]] constexpr auto fixed_bitsets()
        -> over<X, one<bit_positions>>
{
        auto result = over<X, one<bit_positions>>();
        result.append(positions_of::bitsets(n0<X>, n1<X>, linear<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_bitsets()
        -> over<X, one<bit_positions>>
{
        auto result = over<X, one<bit_positions>>();
        result.share(memo<&positions_of::sampled_bitsets>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto bitsets()
        -> over<X, one<bit_positions>>
{
        auto result = over<X, one<bit_positions>>();
        result.share(memo<&positions_of::bitsets>(n0<X>, n1<X>, linear<X>));
        result.share(memo<&positions_of::sampled_bitsets>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

// The empty and the full bitset in each order and against themselves, and every pair of singletons.
template<class X>
[[nodiscard]] constexpr auto fixed_pairs()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.append(positions_of::pairs(n0<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_pairs()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.share(memo<&positions_of::sampled_pairs>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto pairs()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.share(memo<&positions_of::pairs>(n0<X>, n2<X>, quadratic<X>));
        result.share(memo<&positions_of::sampled_pairs>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

// The empty or the full bitset against few bits, one bit against two, and two against two: what an ordering turns on.
template<class X>
[[nodiscard]] constexpr auto fixed_pairs_with_doubletons()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.append(positions_of::pairs(n0<X>, n2<X>, quadratic<X>));
        if constexpr (linear<X>) {
                result.append(positions_of::edges_and_singletons(n1<X>));
        }
        if constexpr (quadratic<X>) {
                result.append(positions_of::edges_and_doubletons(n4<X>));
        }
        if constexpr (quartic<X>) {
                result.append(positions_of::triplets_and_doubleton_pairs(n3<X>, n4<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_pairs_with_doubletons()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.share(memo<&positions_of::sampled_pairs>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto pairs_with_doubletons()
        -> over<X, two<bit_positions>>
{
        auto result = over<X, two<bit_positions>>();
        result.share(memo<&positions_of::pairs>(n0<X>, n2<X>, quadratic<X>));
        if constexpr (linear<X>) {
                result.share(memo<&positions_of::edges_and_singletons>(n1<X>));
        }
        if constexpr (quadratic<X>) {
                result.share(memo<&positions_of::edges_and_doubletons>(n4<X>));
        }
        if constexpr (quartic<X>) {
                result.share(memo<&positions_of::triplets_and_doubleton_pairs>(n3<X>, n4<X>));
        }
        result.share(memo<&positions_of::sampled_pairs>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

// The empty and the full bitset at every position and one past, or at both ends and one past it at a wider width.
template<class X>
[[nodiscard]] constexpr auto fixed_positions()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.append(positions_of::positions(n1<X>, linear<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_positions()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.share(memo<&positions_of::sampled_positions>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto positions()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.share(memo<&positions_of::positions>(n1<X>, linear<X>));
        result.share(memo<&positions_of::sampled_positions>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

// Every singleton at every position and one past as well.
template<class X>
[[nodiscard]] constexpr auto fixed_positions_with_singletons()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.append(positions_of::positions(n1<X>, linear<X>));
        if constexpr (quadratic<X>) {
                result.append(positions_of::singleton_positions(n1<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_positions_with_singletons()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.share(memo<&positions_of::sampled_positions>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto positions_with_singletons()
        -> over<X, keyed<bit_positions>>
{
        auto result = over<X, keyed<bit_positions>>();
        result.share(memo<&positions_of::positions>(n1<X>, linear<X>));
        if constexpr (quadratic<X>) {
                result.share(memo<&positions_of::singleton_positions>(n1<X>));
        }
        result.share(memo<&positions_of::sampled_positions>(random::bitset_width<X>(), random::block_digits_v<X>));
        return result;
}

} // namespace inputs

#ifdef __clang__

#pragma clang diagnostic pop

#endif

} // namespace test::spec::bitset

#endif // TEST_SPEC_BITSET_HPP
