//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_ROTATION_HPP
#define TEST_SEQUENCE_ROTATION_HPP

#include <algorithm> // count, equal, reverse
#include <cstddef>   // size_t
#include <cstdint>   // uint64_t
#include <limits>    // numeric_limits
#include <memory>    // addressof
#include <ranges>    // iota
#include <vector>    // vector

// P3103R2's rotl, rotr and reverse on a sequence, against std::ranges::rotate and std::ranges::reverse over its bools.
namespace test::sequence {

// Up to the first width every pattern is enumerated, and up to the second every turn; past each, a sample.
inline constexpr auto exhaustive_rotation_width = 12UZ;
inline constexpr auto every_turn_width          = 64UZ;

// splitmix64, so every standard library draws the same scatter.
[[nodiscard]] constexpr auto next_word(std::uint64_t& state) noexcept
        -> std::uint64_t
{
        state += 0x9E37'79B9'7F4A'7C15ULL;
        auto z = state;
        z      = (z ^ (z >> 30U)) * 0xBF58'476D'1CE4'E5B9ULL;
        z      = (z ^ (z >> 27U)) * 0x94D0'49BB'1331'11EBULL;
        return z ^ (z >> 31U);
}

// Every pattern of a narrow width; the empty, full, end and periodic ones and some scattered ones of a wider one.
[[nodiscard]] inline auto rotation_patterns(std::size_t width)
        -> std::vector<std::vector<bool>>
{
        auto patterns = std::vector<std::vector<bool>>();
        if (width <= exhaustive_rotation_width) {
                for (auto const word : std::views::iota(0UZ, 1UZ << width)) {
                        auto& p = patterns.emplace_back(width);
                        for (auto const i : std::views::iota(0UZ, width)) {
                                p[i] = ((word >> i) & 1UZ) != 0UZ;
                        }
                }
                return patterns;
        }
        patterns.emplace_back(width, false);
        patterns.emplace_back(width, true);
        auto& ends   = patterns.emplace_back(width, false);
        ends.front() = ends.back() = true;
        for (auto const period : {3UZ, 7UZ}) {
                auto& p = patterns.emplace_back(width);
                for (auto const i : std::views::iota(0UZ, width)) {
                        p[i] = i % period == 0UZ;
                }
        }
        auto state       = std::uint64_t{width};
        auto const draws = width <= every_turn_width ? 16UZ : 2UZ;
        for ([[maybe_unused]] auto const draw : std::views::iota(0UZ, draws)) {
                auto& p = patterns.emplace_back(width);
                for (auto const i : std::views::iota(0UZ, width)) {
                        p[i] = (next_word(state) & 3U) == 0U;
                }
        }
        return patterns;
}

// Bits 0 and 8 of a blank ten: rotr(1) takes them to 9 and 7, rotl(3) on to 2 and 0, and reverse() back to 7 and 9.
template<class S>
[[nodiscard]] constexpr auto permutes_ten_bits(S a)
        -> bool
{
        a[0] = true;
        a[8] = true;
        a.rotr(1UZ);
        auto const turned_right = a[9] and a[7] and a.count() == 2UZ;
        a.rotl(3UZ);
        auto const turned_left = a[2] and a[0] and a.count() == 2UZ;
        a.reverse();
        return turned_right and turned_left and a[7] and a[9] and a.count() == 2UZ;
}

// The object a permutation is applied through: the owner itself, unless a test hands in a view over it.
struct as_owner
{
        template<class O>
        [[nodiscard]] auto operator()(O& o) const noexcept
                -> O&
        {
                return o;
        }
};

// A copy of blank with the pattern written position by position, so its padding is the blank's.
template<class O, class Through>
[[nodiscard]] auto with_pattern(O const& blank, std::vector<bool> const& pattern, Through through)
        -> O
{
        auto o     = blank;
        auto&& seq = through(o);
        for (auto const i : std::views::iota(0UZ, pattern.size())) {
                seq[i] = pattern[i];
        }
        return o;
}

// The bools agree, and so does the count, which the padding would inflate had a permutation left bits in it.
template<class S>
[[nodiscard]] auto holds(S const& got, std::vector<bool> const& want)
        -> bool
{
        return std::ranges::equal(got, want) and got.count() == static_cast<std::size_t>(std::ranges::count(want, true));
}

// Bit i takes bit (i + turn) % size(), by index: libstdc++ 15's ranges::rotate loses a vector<bool> bit.
[[nodiscard]] inline auto turned(std::vector<bool> const& model, std::size_t turn)
        -> std::vector<bool>
{
        auto out = std::vector<bool>(model.size());
        for (auto const i : std::views::iota(0UZ, model.size())) {
                out[i] = model[(i + turn) % model.size()];
        }
        return out;
}

// Named, never a temporary: clang 23 crashes on a deducing-this call with an rvalue self.
template<class O, class Through>
[[nodiscard]] auto rotation_disagreements(O const& o, std::vector<bool> const& model, std::size_t n, Through through)
        -> int
{
        auto const turn  = model.empty() ? 0UZ : n % model.size();
        auto const right = turned(model, turn);
        auto const left  = turned(model, model.empty() ? 0UZ : model.size() - turn);

        auto r               = o;
        auto&& rs            = through(r);
        auto l               = o;
        auto&& ls            = through(l);
        auto const* const rr = std::addressof(rs.rotr(n));
        auto const* const lr = std::addressof(ls.rotl(n));
        return static_cast<int>(not holds(rs, right) or rr != std::addressof(rs)) + static_cast<int>(not holds(ls, left) or lr != std::addressof(ls));
}

// Every turn in [0, 2 size()] of a narrow width; of a wider one, those within one of a multiple of sixteen.
[[nodiscard]] inline auto rotation_turns(std::size_t width)
        -> std::vector<std::size_t>
{
        auto turns = std::vector<std::size_t>();
        for (auto const n : std::views::iota(0UZ, (2UZ * width) + 1UZ)) {
                if (width <= every_turn_width or (n + 1UZ) % 16UZ <= 2UZ) {
                        turns.push_back(n);
                }
        }
        turns.push_back(std::numeric_limits<std::size_t>::max());
        return turns;
}

// Each turn both ways, one that wraps size_t among them, and the reversal twice, each disagreement counted.
template<class O, class Through>
[[nodiscard]] auto permutation_disagreements(O const& o, Through through)
        -> int
{
        auto copy          = o;
        auto&& seq         = through(copy);
        auto const model   = std::vector<bool>(seq.begin(), seq.end());
        auto disagreements = 0;
        for (auto const n : rotation_turns(model.size())) {
                disagreements += rotation_disagreements(o, model, n, through);
        }

        auto reversed = model;
        std::ranges::reverse(reversed);
        auto const* const vr = std::addressof(seq.reverse());
        disagreements += static_cast<int>(not holds(seq, reversed) or vr != std::addressof(seq));
        seq.reverse();
        disagreements += static_cast<int>(not holds(seq, model));
        return disagreements;
}

// Every pattern of the blank's width, permuted through the owner or a view over it.
template<class O, class Through = as_owner>
[[nodiscard]] auto permutation_sweep(O const& blank, Through through = {})
        -> int
{
        auto copy          = blank;
        auto const width   = through(copy).size();
        auto disagreements = 0;
        for (auto const& pattern : rotation_patterns(width)) {
                disagreements += permutation_disagreements(with_pattern(blank, pattern, through), through);
        }
        return disagreements;
}

} // namespace test::sequence

#endif // TEST_SEQUENCE_ROTATION_HPP
