//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_RANDOM_HPP
#define TEST_SPEC_RANDOM_HPP

#include <test/bitset/factory.hpp>        // make_bitset
#include <test/dynamic.hpp>               // dynamic
#include <test/sequence/factory.hpp>      // make_sequence, static_capacity, static_width
#include <test/set/exhaustive.hpp>        // static_capacity, static_width
#include <xstd/bits/detail/ownership.hpp> // owned_storage
#include <boost/test/unit_test.hpp>       // BOOST_TEST_CONTEXT
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
#include <vector>                         // vector

// Sampled keys over widths no exhaustive sweep can reach, reproducible on every standard library from one seed.
namespace test::spec::random {

inline constexpr auto default_seed = 187ULL;
inline constexpr auto default_samples = 1UZ;

// Unset, unreadable and out of range all read as absent, so a typo falls back to the default rather than to zero.
[[nodiscard]] inline auto environment(char const* name)
        -> std::optional<std::uint64_t>
{
#ifdef _MSC_VER

        char* buffer = nullptr;
        auto length = 0UZ;
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

        auto value = std::uint64_t();
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

// The block the bias aims at: the type's own, and the widest builtin word for the standard library's models.
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
        auto drawn = std::vector<bool>(n);
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

// The keys of each sample in the order drawn, below a width n, biased to the edges of blocks of the given digits.
inline auto sample_key_vectors(std::size_t n, std::size_t digits, auto fun)
        -> void
{
        auto const s = seed();
        auto g = engine(s);
        for (auto const k : densities(n)) {
                for (auto const i : std::views::iota(0UZ, samples())) {
                        auto const v = keys(g, n, k, digits);
                        BOOST_TEST_CONTEXT("seed " << s << ", width " << n << ", keys " << k << ", sample " << i)
                        {
                                fun(v, n);
                        }
                }
        }
}

// Each sample built by make with the keys a lookup most often gets wrong: both ends of the width and one drawn.
inline auto sample_key_pairs(std::size_t n, std::size_t digits, auto make, auto fun)
        -> void
{
        auto g = engine(seed() + 1);
        sample_key_vectors(n, digits, [&](auto const& v, std::size_t) {
                auto a = make(v, n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                for (auto const k : {0UZ, n - 1UZ, g.position(n - 1UZ, digits)}) {
                        BOOST_TEST_CONTEXT("key " << k)
                        {
                                fun(a, k);
                        }
                }
        });
}

// Each density against itself and against one drawn, so both operands range from empty to full at linear cost.
inline auto sample_pairs(std::size_t n, std::size_t digits, auto make, auto fun)
        -> void
{
        auto const s = seed();
        auto g = engine(s);
        auto const d = densities(n);
        for (auto const ka : d) {
                for (auto const kb : {ka, d[g.below(d.size())]}) {
                        for (auto const i : std::views::iota(0UZ, samples())) {
                                auto a = make(keys(g, n, ka, digits), n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                auto b = make(keys(g, n, kb, digits), n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                                BOOST_TEST_CONTEXT("seed " << s << ", width " << n << ", keys " << ka << " and " << kb << ", sample " << i)
                                {
                                        fun(a, b);
                                }
                        }
                }
        }
}

template<class X>
auto all_key_vectors(auto fun)
{
        sample_key_vectors(width<X>(), block_digits_v<X>, fun);
}

template<class X>
[[nodiscard]] auto set_from(std::vector<std::size_t> const& v, std::size_t)
        -> X
{
        return v | std::ranges::to<X>();
}

template<class X>
auto all_sets(auto fun)
{
        all_key_vectors<X>([&](auto const& v, std::size_t n) {
                auto a = set_from<X>(v, n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        });
}

template<class X>
auto all_set_key_pairs(auto fun)
{
        sample_key_pairs(width<X>(), block_digits_v<X>, set_from<X>, fun);
}

template<class X>
auto all_set_pairs(auto fun)
{
        sample_pairs(width<X>(), block_digits_v<X>, set_from<X>, fun);
}

// Seven triples per sample, their densities drawn, since all 343 would cost more than they find.
template<class X>
auto all_set_triples(auto fun)
{
        auto const s = seed();
        auto g = engine(s);
        auto const n = width<X>();
        auto const d = densities(n);
        for (auto const i : std::views::iota(0UZ, samples() * d.size())) {
                auto const ka = d[g.below(d.size())];
                auto const kb = d[g.below(d.size())];
                auto const kc = d[g.below(d.size())];
                auto a = keys(g, n, ka, block_digits_v<X>) | std::ranges::to<X>(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                auto b = keys(g, n, kb, block_digits_v<X>) | std::ranges::to<X>(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                auto c = keys(g, n, kc, block_digits_v<X>) | std::ranges::to<X>(); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                BOOST_TEST_CONTEXT("seed " << s << ", width " << n << ", keys " << ka << ", " << kb << " and " << kc << ", sample " << i)
                {
                        fun(a, b, c);
                }
        }
}

// A bitset is as wide as its type or its capacity, and sampled one past 2048 bits where its width is unbounded.
template<class X>
[[nodiscard]] auto bitset_width()
        -> std::size_t
{
        if constexpr (not test::dynamic<X>) {
                return X().size();
        } else if constexpr (test::set::static_capacity<X>) {
                return X().max_size();
        } else {
                return 2049UZ;
        }
}

// The keys set at the width, which a static width already has and a run-time width is resized to.
template<class X>
[[nodiscard]] auto bitset_from(std::vector<std::size_t> const& v, std::size_t n)
        -> X
{
        auto a = test::bitset::make_bitset<X>(n);
        for (auto const k : v) {
                a.set(k);
        }
        return a;
}

template<class X>
auto all_bitsets(auto fun)
{
        sample_key_vectors(bitset_width<X>(), block_digits_v<X>, [&](auto const& v, std::size_t n) {
                auto a = bitset_from<X>(v, n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        });
}

template<class X>
auto all_bitset_key_pairs(auto fun)
{
        sample_key_pairs(bitset_width<X>(), block_digits_v<X>, bitset_from<X>, fun);
}

template<class X>
auto all_bitset_pairs(auto fun)
{
        sample_pairs(bitset_width<X>(), block_digits_v<X>, bitset_from<X>, fun);
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

// The keys set at the width and every other position clear.
template<class X>
[[nodiscard]] auto sequence_from(std::vector<std::size_t> const& v, std::size_t n)
        -> X
{
        auto a = test::sequence::make_sequence<X>(n, [](std::size_t) -> bool { return false; });
        for (auto const k : v) {
                a[k] = true;
        }
        return a;
}

template<class X>
auto all_sequences(auto fun)
{
        sample_key_vectors(sequence_width<X>(), block_digits_v<X>, [&](auto const& v, std::size_t n) {
                auto a = sequence_from<X>(v, n); // NOLINT(misc-const-correctness): handed to fun, which some functors take by non-const reference
                fun(a);
        });
}

template<class X>
auto all_sequence_key_pairs(auto fun)
{
        sample_key_pairs(sequence_width<X>(), block_digits_v<X>, sequence_from<X>, fun);
}

template<class X>
auto all_sequence_pairs(auto fun)
{
        sample_pairs(sequence_width<X>(), block_digits_v<X>, sequence_from<X>, fun);
}

} // namespace test::spec::random

#endif // TEST_SPEC_RANDOM_HPP
