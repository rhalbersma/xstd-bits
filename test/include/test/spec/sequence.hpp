//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_SEQUENCE_HPP
#define TEST_SPEC_SEQUENCE_HPP

#include <test/container/allocator.hpp>             // non_propagating
#include <test/dynamic.hpp>                         // dynamic
#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/sequence/factory.hpp>                // make_sequence, static_capacity, static_width, stripes
#include <test/spec/input.hpp>                      // edge, exhaustive, key_vector, keyed, keyed_pair, memo, one, origin, rebuilt, two
#include <test/spec/random.hpp>                     // block_digits_v, key_samples, keyed_samples, pair_samples, sequence_width
#include <test/uint128.hpp>                         // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <algorithm>                                // min
#include <array>                                    // array
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                                   // iota
#include <tuple>                                    // tuple, tuple_cat
#include <utility>                                  // declval, move
#include <vector>                                   // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

// The candidates for the sequence reading, each column's model first, and the inputs a clause checks them over.
namespace test::spec::sequence {

// std::inplace_vector<bool, N> at the given capacities where the standard library has it, and nothing where not.
template<std::size_t... N>
using inplace_models = std::tuple<
#ifdef TEST_HAS_INPLACE_VECTOR

        std::inplace_vector<bool, N>...

#endif
        >;

// Every Block at an empty width, a single bit, either side of its first block boundaries, at 17 and 24, and far past.
using fixed = std::tuple<xstd::basic_bit_array<std::uint8_t, 0>, xstd::basic_bit_array<std::uint8_t, 1>, xstd::basic_bit_array<std::uint8_t, 7>, xstd::basic_bit_array<std::uint8_t, 8>, xstd::basic_bit_array<std::uint8_t, 9>, xstd::basic_bit_array<std::uint8_t, 16>, xstd::basic_bit_array<std::uint8_t, 17>, xstd::basic_bit_array<std::uint8_t, 24>, xstd::basic_bit_array<std::uint8_t, 257>, xstd::basic_bit_array<std::uint16_t, 15>, xstd::basic_bit_array<std::uint16_t, 16>, xstd::basic_bit_array<std::uint16_t, 17>, xstd::basic_bit_array<std::uint16_t, 24>, xstd::basic_bit_array<std::uint32_t, 17>, xstd::basic_bit_array<std::uint32_t, 24>, xstd::basic_bit_array<std::uint32_t, 31>, xstd::basic_bit_array<std::uint32_t, 32>, xstd::basic_bit_array<std::uint32_t, 33>, xstd::basic_bit_array<std::uint32_t, 1023>, xstd::basic_bit_array<std::uint64_t, 0>, xstd::basic_bit_array<std::uint64_t, 17>, xstd::basic_bit_array<std::uint64_t, 24>, xstd::basic_bit_array<std::uint64_t, 63>, xstd::basic_bit_array<std::uint64_t, 64>, xstd::basic_bit_array<std::uint64_t, 65>, xstd::basic_bit_array<std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                         ,
                         xstd::basic_bit_array<xstd::uint128, 17>, xstd::basic_bit_array<xstd::uint128, 24>, xstd::basic_bit_array<xstd::uint128, 127>, xstd::basic_bit_array<xstd::uint128, 128>, xstd::basic_bit_array<xstd::uint128, 129>, xstd::basic_bit_array<xstd::uint128, 2049>

#endif
                         >;

using array_models = std::tuple<std::array<bool, 0>, std::array<bool, 1>, std::array<bool, 8>, std::array<bool, 9>, std::array<bool, 17>, std::array<bool, 24>, std::array<bool, 64>, std::array<bool, 65>, std::array<bool, 1025>>;

using vector_model = std::tuple<std::vector<bool>>;

using dynamic = std::tuple<xstd::basic_bit_vector<std::uint8_t>, xstd::basic_bit_vector<std::uint64_t>>;

// Inline blocks that a sweep to its limit spills from, that a sweep fits in, and that hold half of what a sample spans.
using small = std::tuple<xstd::basic_bit_small_vector<std::uint8_t, 9>, xstd::basic_bit_small_vector<std::uint64_t, 64>, xstd::basic_bit_small_vector<std::uint64_t, 1024>>;

using bounded = std::tuple<xstd::basic_bit_bounded_vector<std::uint8_t, 0>, xstd::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 17>, xstd::basic_bit_bounded_vector<std::uint8_t, 24>, xstd::basic_bit_bounded_vector<std::uint64_t, 24>, xstd::basic_bit_bounded_vector<std::uint64_t, 65>, xstd::basic_bit_bounded_vector<std::uint64_t, 4097>>;

// [array]'s column: std::array<bool, N> and the fixed sequences.
using array_all = decltype(std::tuple_cat(std::declval<array_models>(), std::declval<fixed>()));

// [vector.bool]'s column: std::vector<bool>, the dynamic sequences and the small ones.
using vector_all = decltype(std::tuple_cat(std::declval<vector_model>(), std::declval<dynamic>(), std::declval<small>()));

// [inplace.vector]'s column: std::inplace_vector<bool, N> where the library has it, and the bounded sequences.
using inplace_vector_all = decltype(std::tuple_cat(std::declval<inplace_models<0, 9, 17, 24, 65, 4097>>(), std::declval<bounded>()));

// The two columns that grow, for [sequence.reqmts]'s modifiers.
using growable_all = decltype(std::tuple_cat(std::declval<vector_all>(), std::declval<inplace_vector_all>()));

using all = decltype(std::tuple_cat(std::declval<array_all>(), std::declval<growable_all>()));

// The owners that take an allocator, under one that keeps a ledger and refuses on request, std::vector<bool> first.
using ledgered = std::tuple<std::vector<bool, test::container::non_propagating<bool>>, xstd::basic_bit_vector<std::uint8_t, test::container::non_propagating<std::uint8_t>>, xstd::basic_bit_vector<std::uint64_t, test::container::non_propagating<std::uint64_t>>, xstd::basic_bit_small_vector<std::uint8_t, 9, test::container::non_propagating<std::uint8_t>>, xstd::basic_bit_small_vector<std::uint64_t, 64, test::container::non_propagating<std::uint64_t>>>;

// The positions a sequence holds without growing: a width or capacity in its type, a small one's inline bits, or none.
template<class X>
inline constexpr auto held_width_v = [] -> std::size_t {
        if constexpr (test::sequence::static_width<X>) {
                // NOLINTNEXTLINE(readability-static-accessed-through-instance): a function on the standard's owners.
                return X().size();
        } else if constexpr (test::sequence::static_capacity<X>) {
                return X::capacity();
        } else {
                return 0UZ;
        }
}();

template<class Block, std::size_t N, class Allocator>
inline constexpr auto held_width_v<xstd::basic_bit_small_vector<Block, N, Allocator>> = N;

#ifdef __clang__

// Clang shows a parameterless constexpr function can be constant by running it, here the whole enumeration.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-constexpr"

#endif

namespace inputs {

// A linear enumeration runs up to 64 positions, a quadratic one up to 32 and a cubic one up to 17.
template<class X>
inline constexpr auto linear = held_width_v<X> <= 64UZ;

template<class X>
inline constexpr auto quadratic = held_width_v<X> <= 32UZ;

template<class X>
inline constexpr auto cubic = held_width_v<X> <= 17UZ;

// A run-time width, which a sweep varies, where a static one is the type's.
template<class X>
inline constexpr auto grows = test::dynamic<X>;

// The limits a sweep takes: 128 positions for the edges, then 64, 32 and 16 by its order, capped by the type.
template<class X>
inline constexpr auto n0 = test::sequence::limit_v<X, 128UZ>;

template<class X>
inline constexpr auto n1 = test::sequence::limit_v<X, 64UZ>;

template<class X>
inline constexpr auto n2 = test::sequence::limit_v<X, 32UZ>;

template<class X>
inline constexpr auto n3 = test::sequence::limit_v<X, 16UZ>;

// The bools a sequence holds, which each type's sequence is rebuilt from.
using bools = std::vector<bool>;

// The bools of each enumeration, at the limits a type's sweep takes, shared by every type with the same limits.
namespace values {

[[nodiscard]] constexpr auto filled(std::size_t n, bool value)
        -> bools
{
        return bools(n, value); // NOLINT(modernize-return-braced-init-list): braces would pick the initializer_list of two bools
}

[[nodiscard]] constexpr auto striped(std::size_t n)
        -> bools
{
        auto result = bools(n);
        for (auto const i : std::views::iota(0UZ, n)) {
                result[i] = test::sequence::stripes(i);
        }
        return result;
}

// Value-initialized: no positions at a run-time width, every position clear at a static one.
[[nodiscard]] constexpr auto empty(bool grows, std::size_t n)
        -> bools
{
        return filled(grows ? 0UZ : n, false);
}

// Every width up to n where the width is the object's, and n alone where it is not.
[[nodiscard]] constexpr auto widths(bool grows, std::size_t n)
        -> std::vector<bools>
{
        auto result = std::vector<bools>();
        for (auto const w : std::views::iota(grows ? 0UZ : n, n + 1UZ)) {
                result.push_back(striped(w));
        }
        return result;
}

[[nodiscard]] constexpr auto sequences(bool grows, std::size_t n0, std::size_t n1, bool sweep)
        -> std::vector<one<bools>>
{
        auto result = std::vector<one<bools>>();
        result.push_back({.from = edge("empty", n0), .a = empty(grows, n0)});
        result.push_back({.from = edge("full", n0), .a = filled(n0, true)});
        if (sweep) {
                for (auto& a : widths(grows, n1)) {
                        result.push_back({.from = exhaustive("every width", n1), .a = std::move(a)});
                }
                for (auto const i : std::views::iota(0UZ, n1 + 1UZ)) {
                        auto a = filled(n1, false);
                        for (auto const j : std::views::iota(0UZ, i)) {
                                a[j] = true;
                        }
                        result.push_back({.from = exhaustive("every prefix", n1), .a = std::move(a)});
                }
                for (auto const i : std::views::iota(0UZ, n1)) {
                        auto a = filled(n1, false);
                        a[i] = true;
                        result.push_back({.from = exhaustive("every singleton", n1), .a = std::move(a)});
                }
        }
        return result;
}

[[nodiscard]] constexpr auto pairs(bool grows, std::size_t n0, std::size_t n2, bool sweep)
        -> std::vector<two<bools>>
{
        auto result = std::vector<two<bools>>();
        auto const e = empty(grows, n0);
        auto const f = filled(n0, true);
        result.push_back({.from = edge("empty and empty", n0), .a = e, .b = e});
        result.push_back({.from = edge("empty and full", n0), .a = e, .b = f});
        result.push_back({.from = edge("full and empty", n0), .a = f, .b = e});
        result.push_back({.from = edge("full and full", n0), .a = f, .b = f});
        if (sweep) {
                for (auto const i : std::views::iota(0UZ, n2)) {
                        for (auto const j : std::views::iota(0UZ, n2)) {
                                auto a = filled(n2, false);
                                auto b = filled(n2, false);
                                a[i] = true;
                                b[j] = true;
                                result.push_back({.from = exhaustive("every singleton pair", n2), .a = std::move(a), .b = std::move(b)});
                        }
                }
                auto const all_widths = widths(grows, n2);
                for (auto const& a : all_widths) {
                        for (auto const& b : all_widths) {
                                result.push_back({.from = exhaustive("every pair of widths", n2), .a = a, .b = b});
                        }
                }
        }
        return result;
}

[[nodiscard]] constexpr auto indexed(bool grows, std::size_t n0, std::size_t n1, bool sweep)
        -> std::vector<keyed<bools>>
{
        auto result = std::vector<keyed<bools>>();
        if (n0 != 0) {
                result.push_back({.from = edge("full", n0), .a = filled(n0, true), .k = 0UZ});
                result.push_back({.from = edge("full", n0), .a = filled(n0, true), .k = n0 - 1UZ});
        }
        if (sweep) {
                for (auto const& a : widths(grows, n1)) {
                        for (auto const i : std::views::iota(0UZ, a.size())) {
                                result.push_back({.from = exhaustive("every width at every position", n1), .a = a, .k = i});
                        }
                }
        }
        return result;
}

[[nodiscard]] constexpr auto index_pairs(bool grows, std::size_t n0, std::size_t n2, bool sweep)
        -> std::vector<keyed_pair<bools>>
{
        auto result = std::vector<keyed_pair<bools>>();
        if (n0 != 0) {
                result.push_back({.from = edge("full", n0), .a = filled(n0, true), .k = 0UZ, .l = n0 - 1UZ});
                result.push_back({.from = edge("full", n0), .a = filled(n0, true), .k = n0 - 1UZ, .l = 0UZ});
        }
        if (sweep) {
                for (auto const& a : widths(grows, n2)) {
                        for (auto const i : std::views::iota(0UZ, a.size())) {
                                for (auto const j : std::views::iota(0UZ, a.size())) {
                                        result.push_back({.from = exhaustive("every width at every pair of positions", n2), .a = a, .k = i, .l = j});
                                }
                        }
                }
        }
        return result;
}

[[nodiscard]] constexpr auto prefixes(bool grows, std::size_t n0, std::size_t n1, bool sweep)
        -> std::vector<one<bools>>
{
        auto result = std::vector<one<bools>>();
        result.push_back({.from = edge("empty", n0), .a = empty(grows, n0)});
        result.push_back({.from = edge("full", n0), .a = filled(n0, true)});
        if (sweep) {
                for (auto& a : widths(grows, n1)) {
                        result.push_back({.from = exhaustive("every width", n1), .a = std::move(a)});
                }
        }
        return result;
}

[[nodiscard]] constexpr auto positions(bool grows, std::size_t n0, std::size_t n2, bool sweep)
        -> std::vector<keyed<bools>>
{
        auto result = std::vector<keyed<bools>>();
        result.push_back({.from = edge("empty", n0), .a = empty(grows, n0), .k = 0UZ});
        for (auto const p : {0UZ, n0 / 2UZ, n0}) {
                result.push_back({.from = edge("full", n0), .a = filled(n0, true), .k = p});
        }
        if (sweep) {
                for (auto const& a : widths(grows, n2)) {
                        for (auto const p : std::views::iota(0UZ, a.size() + 1UZ)) {
                                result.push_back({.from = exhaustive("every width at every position", n2), .a = a, .k = p});
                        }
                }
        }
        return result;
}

[[nodiscard]] constexpr auto spans(bool grows, std::size_t n3, bool sweep)
        -> std::vector<keyed_pair<bools>>
{
        auto result = std::vector<keyed_pair<bools>>();
        for (auto const k : {0UZ, 1UZ, 9UZ}) {
                result.push_back({.from = edge("empty", n3), .a = empty(grows, n3), .k = 0UZ, .l = k});
        }
        if (sweep) {
                for (auto const& a : widths(grows, n3)) {
                        for (auto const p : std::views::iota(0UZ, a.size() + 1UZ)) {
                                for (auto const k : std::views::iota(0UZ, n3 + 1UZ)) {
                                        result.push_back({.from = exhaustive("every width, position and count", n3), .a = a, .k = p, .l = k});
                                }
                        }
                }
        }
        return result;
}

// The sampled keys set in n positions, and the first p of them.
[[nodiscard]] constexpr auto at_width(key_vector const& keys, std::size_t n)
        -> bools
{
        auto result = filled(n, false);
        for (auto const k : keys) {
                result[k] = true;
        }
        return result;
}

[[nodiscard]] constexpr auto prefix(key_vector const& keys, std::size_t n, std::size_t p)
        -> bools
{
        auto result = at_width(keys, n);
        result.resize(p);
        return result;
}

[[nodiscard]] inline auto sampled_sequences(std::size_t n, std::size_t digits)
        -> std::vector<one<bools>>
{
        auto result = std::vector<one<bools>>();
        for (auto const& [from, a] : random::key_samples(n, digits)) {
                result.push_back({.from = from, .a = at_width(a, n)});
        }
        return result;
}

[[nodiscard]] inline auto sampled_pairs(std::size_t n, std::size_t digits)
        -> std::vector<two<bools>>
{
        auto result = std::vector<two<bools>>();
        for (auto const& [from, a, b] : random::pair_samples(n, digits)) {
                result.push_back({.from = from, .a = at_width(a, n), .b = at_width(b, n)});
        }
        return result;
}

[[nodiscard]] inline auto sampled_indexed(std::size_t n, std::size_t digits)
        -> std::vector<keyed<bools>>
{
        auto result = std::vector<keyed<bools>>();
        for (auto const& [from, a, k] : random::keyed_samples(n, digits)) {
                result.push_back({.from = from, .a = at_width(a, n), .k = k});
        }
        return result;
}

// A position and its mirror, so both ends of a random sequence are reached.
[[nodiscard]] inline auto sampled_index_pairs(std::size_t n, std::size_t digits)
        -> std::vector<keyed_pair<bools>>
{
        auto result = std::vector<keyed_pair<bools>>();
        for (auto const& [from, a, k] : random::keyed_samples(n, digits)) {
                result.push_back({.from = from, .a = at_width(a, n), .k = k, .l = n - 1UZ - k});
        }
        return result;
}

// Up to and including the sampled position, so the full sequence is among them.
[[nodiscard]] inline auto sampled_prefixes(std::size_t n, std::size_t digits)
        -> std::vector<one<bools>>
{
        auto result = std::vector<one<bools>>();
        for (auto const& [from, a, k] : random::keyed_samples(n, digits)) {
                result.push_back({.from = from, .a = prefix(a, n, k + 1UZ)});
        }
        return result;
}

[[nodiscard]] inline auto sampled_positions(std::size_t n, std::size_t digits)
        -> std::vector<keyed<bools>>
{
        auto result = std::vector<keyed<bools>>();
        for (auto const& [from, a, k] : random::keyed_samples(n, digits)) {
                result.push_back({.from = from, .a = prefix(a, n, k), .k = k / 2UZ});
        }
        return result;
}

[[nodiscard]] inline auto sampled_spans(std::size_t n, std::size_t digits)
        -> std::vector<keyed_pair<bools>>
{
        auto result = std::vector<keyed_pair<bools>>();
        for (auto const& [from, a, k] : random::keyed_samples(n, digits)) {
                result.push_back({.from = from, .a = prefix(a, n, k), .k = k / 2UZ, .l = std::ranges::min(n - k, 33UZ)});
        }
        return result;
}

} // namespace values

template<class X>
struct sequence_of
{
        [[nodiscard]] constexpr auto operator()(bools const& v) const
                -> X
        {
                return test::sequence::make_sequence<X>(v.size(), [&](std::size_t i) -> bool { return v[i]; });
        }
};

template<class X, class Carrier>
using over = rebuilt<X, Carrier, sequence_of<X>>;

// The empty and the full sequence, every width, every prefix and every singleton.
template<class X>
[[nodiscard]] constexpr auto fixed_sequences()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.append(values::sequences(grows<X>, n0<X>, n1<X>, linear<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_sequences()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.share(memo<&values::sampled_sequences>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto sequences()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.share(memo<&values::sequences>(grows<X>, n0<X>, n1<X>, linear<X>));
        result.share(memo<&values::sampled_sequences>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// The empty and the full sequence in each order and against themselves, every pair of singletons and of widths.
template<class X>
[[nodiscard]] constexpr auto fixed_pairs()
        -> over<X, two<bools>>
{
        auto result = over<X, two<bools>>();
        result.append(values::pairs(grows<X>, n0<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_pairs()
        -> over<X, two<bools>>
{
        auto result = over<X, two<bools>>();
        result.share(memo<&values::sampled_pairs>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto pairs()
        -> over<X, two<bools>>
{
        auto result = over<X, two<bools>>();
        result.share(memo<&values::pairs>(grows<X>, n0<X>, n2<X>, quadratic<X>));
        result.share(memo<&values::sampled_pairs>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// The full sequence at its ends, and every width at every position it has.
template<class X>
[[nodiscard]] constexpr auto fixed_indexed()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.append(values::indexed(grows<X>, n0<X>, n1<X>, linear<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_indexed()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.share(memo<&values::sampled_indexed>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto indexed()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.share(memo<&values::indexed>(grows<X>, n0<X>, n1<X>, linear<X>));
        result.share(memo<&values::sampled_indexed>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// The full sequence at its two ends, and every width at every pair of positions it has.
template<class X>
[[nodiscard]] constexpr auto fixed_index_pairs()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.append(values::index_pairs(grows<X>, n0<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_index_pairs()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.share(memo<&values::sampled_index_pairs>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto index_pairs()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.share(memo<&values::index_pairs>(grows<X>, n0<X>, n2<X>, quadratic<X>));
        result.share(memo<&values::sampled_index_pairs>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// The empty and the full sequence and every width, and a random sequence up to a position, which leaves room to grow.
template<class X>
[[nodiscard]] constexpr auto fixed_prefixes()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.append(values::prefixes(grows<X>, n0<X>, n1<X>, linear<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_prefixes()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.share(memo<&values::sampled_prefixes>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto prefixes()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.share(memo<&values::prefixes>(grows<X>, n0<X>, n1<X>, linear<X>));
        result.share(memo<&values::sampled_prefixes>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// Every width at every position an insertion can go, and halfway into a random sequence up to a position.
template<class X>
[[nodiscard]] constexpr auto fixed_positions()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.append(values::positions(grows<X>, n0<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_positions()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.share(memo<&values::sampled_positions>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto positions()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.share(memo<&values::positions>(grows<X>, n0<X>, n2<X>, quadratic<X>));
        result.share(memo<&values::sampled_positions>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

// Every width, position and count, and up to 33 positions halfway into a random sequence up to a position.
template<class X>
[[nodiscard]] constexpr auto fixed_spans()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.append(values::spans(grows<X>, n3<X>, cubic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_spans()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.share(memo<&values::sampled_spans>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

template<class X>
[[nodiscard]] auto spans()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.share(memo<&values::spans>(grows<X>, n3<X>, cubic<X>));
        result.share(memo<&values::sampled_spans>(random::sequence_width<X>(), random::block_digits_v<X>));
        return result;
}

} // namespace inputs

#ifdef __clang__

#pragma clang diagnostic pop

#endif

} // namespace test::spec::sequence

#endif // TEST_SPEC_SEQUENCE_HPP
