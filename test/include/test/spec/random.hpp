//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_RANDOM_HPP
#define TEST_SPEC_RANDOM_HPP

#include <test/sequence/factory.hpp>      // make_sequence, static_capacity, static_width
#include <test/set/exhaustive.hpp>        // static_capacity, static_width
#include <test/spec/input.hpp>            // key_list, key_vector, keyed, origin, three, two
#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <algorithm>                      // min
#include <array>                          // array
#include <bit>                            // countl_zero
#include <charconv>                       // from_chars
#include <cstddef>                        // size_t
#include <cstdint>                        // uint64_t
#include <cstdlib>                        // free, getenv
#include <limits>                         // numeric_limits
#include <optional>                       // nullopt, optional
#include <random>                         // mt19937_64
#include <ranges>                         // iota, to
#include <string>                         // string
#include <system_error>                   // errc
#include <utility>                        // move
#include <vector>                         // vector

// Sampled keys at every width, including those no exhaustive sweep reaches, reproducible anywhere from one seed.
namespace test::spec::random {

inline constexpr auto default_seed    = 187ULL;
inline constexpr auto default_samples = 1UZ;

// Unset, unreadable and out of range all read as absent, so a typo falls back to the default rather than to zero.
[[nodiscard]] inline auto environment(char const* name)
        -> std::optional<std::uint64_t>
{
#ifdef _MSC_VER

        char* buffer = nullptr;
        auto length  = 0UZ;
        if (_dupenv_s(&buffer, &length, name) != 0 or buffer == nullptr) {
                return std::nullopt;
        }
        auto const text = std::string(buffer);
        std::free(buffer); // _dupenv_s allocates with malloc

#else

        char const* const buffer = std::getenv(name);
        if (buffer == nullptr) {
                return std::nullopt;
        }
        auto const text = std::string(buffer);

#endif

        auto value              = std::uint64_t();
        auto const [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc() or end != text.data() + text.size()) {
                return std::nullopt;
        }
        return value;
}

// XSTD_BITS_TEST_SEED reruns a failure from the seed its context reports.
[[nodiscard]] inline auto seed()
        -> std::uint64_t
{
        return environment("XSTD_BITS_TEST_SEED").value_or(default_seed);
}

// XSTD_BITS_TEST_SAMPLES widens every sampled sweep at once, for a soak run.
[[nodiscard]] inline auto samples()
        -> std::size_t
{
        return static_cast<std::size_t>(environment("XSTD_BITS_TEST_SAMPLES").value_or(default_samples));
}

// The standard fixes mt19937_64's output but no distribution's, so the reduction to a range is written here.
class engine
{
        std::mt19937_64 m_bits;

public:
        [[nodiscard]] explicit engine(std::uint64_t s)
                : m_bits(s)
        {}

        // Uniform over [0, n) by rejection under the covering power of two, which never consumes a draw for n == 1.
        [[nodiscard]] auto below(std::uint64_t n)
                -> std::uint64_t
        {
                if (n <= 1) {
                        return 0;
                }
                auto const mask = std::numeric_limits<std::uint64_t>::max() >> static_cast<unsigned>(std::countl_zero(n - 1));
                for (;;) {
                        if (auto const x = m_bits() & mask; x < n) {
                                return x;
                        }
                }
        }

        // Uniform over [0, j] three times in four, and otherwise a block's first or last position at or below j.
        [[nodiscard]] auto position(std::size_t j, std::size_t digits)
                -> std::size_t
        {
                if (below(4) != 0) {
                        return below(j + 1);
                }
                auto const block = below((j / digits) + 1);
                auto const first = block * digits;
                return block != 0 and below(2) != 0 ? first - 1 : first;
        }
};

// The block the bias aims at: the type's own, and the widest builtin block for the standard library's models.
template<class X>
inline constexpr auto block_digits_v = [] -> std::size_t {
        if constexpr (requires { xstd::bits::detail::owned_storage<X>::bits_type::bits_per_block; }) {
                return xstd::bits::detail::owned_storage<X>::bits_type::bits_per_block;
        } else {
                return 64UZ;
        }
}();

// A fixed width is the type's, a capacity is its own width, and an unbounded set is sampled one past 2048 bits.
template<class X>
[[nodiscard]] auto width()
        -> std::size_t
{
        if constexpr (test::set::static_width<X> or test::set::static_capacity<X>) {
                return X().max_size();
        } else {
                return 2049UZ;
        }
}

// Empty, one key, sparse, half, dense, all but one, and full.
[[nodiscard]] inline auto densities(std::size_t n)
        -> std::array<std::size_t, 7>
{
        return {0UZ, std::ranges::min(n, 1UZ), n / 16UZ, n / 2UZ, n - (n / 16UZ), n - std::ranges::min(n, 1UZ), n};
}

// Floyd's sampling: k distinct keys below n in the order drawn, which any draw within [0, j] keeps distinct.
[[nodiscard]] inline auto keys(engine& g, std::size_t n, std::size_t k, std::size_t digits)
        -> std::vector<std::size_t>
{
        auto drawn  = std::vector<bool>(n);
        auto result = std::vector<std::size_t>();
        result.reserve(k);
        for (auto const j : std::views::iota(n - k, n)) {
                auto t = g.position(j, digits);
                if (drawn[t]) {
                        t = j;
                }
                drawn[t] = true;
                result.push_back(t);
        }
        return result;
}

// A sample's origin: the seed it was drawn from, its width, the key count of each operand, and its index.
[[nodiscard]] inline auto sampled(std::size_t n, std::array<std::size_t, 3> k, std::size_t operands, std::size_t i)
        -> origin
{
        return {.tier = "random", .name = "sample", .width = n, .seed = seed(), .sample = i, .keys = k, .operands = operands};
}

// The keys of each sample in the order drawn, below a width n, biased to the edges of blocks of the given digits.
inline auto sample_key_vectors(std::size_t n, std::size_t digits, auto fun)
        -> void
{
        auto g = engine(seed());
        for (auto const k : densities(n)) {
                for (auto const i : std::views::iota(0UZ, samples())) {
                        auto const v = keys(g, n, k, digits);
                        fun(sampled(n, {k}, 1, i), v);
                }
        }
}

// Every sample's keys in the order drawn.
[[nodiscard]] inline auto key_samples(std::size_t n, std::size_t digits)
        -> std::vector<key_list>
{
        auto result = std::vector<key_list>();
        sample_key_vectors(n, digits, [&](origin const& from, key_vector const& v) -> void { result.push_back({.from = from, .a = v}); });
        return result;
}

// Each sample with the keys a lookup most often gets wrong: both ends of the width and one drawn.
[[nodiscard]] inline auto keyed_samples(std::size_t n, std::size_t digits)
        -> std::vector<keyed<key_vector>>
{
        auto result = std::vector<keyed<key_vector>>();
        if (n == 0) {
                return result;
        }
        auto g = engine(seed() + 1);
        sample_key_vectors(n, digits, [&](origin const& from, key_vector const& v) -> void {
                for (auto const k : {0UZ, n - 1UZ, g.position(n - 1UZ, digits)}) {
                        result.push_back({.from = from, .a = v, .k = k});
                }
        });
        return result;
}

// Each density against itself and against one drawn, so both operands range from empty to full at linear cost.
[[nodiscard]] inline auto pair_samples(std::size_t n, std::size_t digits)
        -> std::vector<two<key_vector>>
{
        auto result  = std::vector<two<key_vector>>();
        auto g       = engine(seed());
        auto const d = densities(n);
        for (auto const ka : d) {
                for (auto const kb : {ka, d[g.below(d.size())]}) {
                        for (auto const i : std::views::iota(0UZ, samples())) {
                                auto a = keys(g, n, ka, digits);
                                auto b = keys(g, n, kb, digits);
                                result.push_back({.from = sampled(n, {ka, kb}, 2, i), .a = std::move(a), .b = std::move(b)});
                        }
                }
        }
        return result;
}

// Seven triples per sample, their densities drawn, since all 343 would cost more than they find.
[[nodiscard]] inline auto triple_samples(std::size_t n, std::size_t digits)
        -> std::vector<three<key_vector>>
{
        auto result  = std::vector<three<key_vector>>();
        auto g       = engine(seed());
        auto const d = densities(n);
        for (auto const i : std::views::iota(0UZ, samples() * d.size())) {
                auto const ka = d[g.below(d.size())];
                auto const kb = d[g.below(d.size())];
                auto const kc = d[g.below(d.size())];
                auto a        = keys(g, n, ka, digits);
                auto b        = keys(g, n, kb, digits);
                auto c        = keys(g, n, kc, digits);
                result.push_back({.from = sampled(n, {ka, kb, kc}, 3, i), .a = std::move(a), .b = std::move(b), .c = std::move(c)});
        }
        return result;
}

// A sequence is as wide as its type or its capacity, and sampled one past 2048 bits where its width is unbounded.
template<class X>
[[nodiscard]] auto sequence_width()
        -> std::size_t
{
        if constexpr (test::sequence::static_width<X>) {
                return X().size();
        } else if constexpr (test::sequence::static_capacity<X>) {
                return X::capacity();
        } else {
                return 2049UZ;
        }
}

} // namespace test::spec::random

#endif // TEST_SPEC_RANDOM_HPP
