//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_GSLICE_INDEX_HPP
#define XSTD_BITS_DETAIL_GSLICE_INDEX_HPP

#include <algorithm> // max
#include <cassert>   // assert
#include <cstddef>   // size_t
#include <cstdint>   // uint64_t
#include <ranges>    // iota
#include <span>      // span
#include <utility>   // move
#include <vector>    // vector

// A gslice's positions, worked out once: one bit per position where they ascend, the positions themselves where not.
namespace xstd::bits::detail {

inline constexpr auto word_width = 64UZ;

// Each position in row-major order, the last length varying fastest, by an odometer over the lengths.
template<class F>
auto for_each_position(std::size_t start, std::span<std::size_t const> lengths, std::span<std::size_t const> strides, std::size_t count, F f)
        -> void
{
        auto const dims = lengths.size();
        auto counter    = std::vector<std::size_t>(dims);
        auto pos        = start;
        for ([[maybe_unused]] auto const t : std::views::iota(0UZ, count)) {
                f(pos);
                // The last counter steps, and one that runs out carries into the one before it.
                auto d = dims - 1UZ;
                ++counter[d];
                pos += strides[d];
                while (counter[d] == lengths[d] and d != 0UZ) {
                        pos -= lengths[d] * strides[d];
                        counter[d] = 0UZ;
                        --d;
                        ++counter[d];
                        pos += strides[d];
                }
        }
}

struct gslice_index
{
        std::size_t start = 0UZ;
        std::vector<std::size_t> lengths;
        std::vector<std::size_t> strides;
        std::size_t count = 0UZ;
        // One past the last position.
        std::size_t width = 0UZ;
        // One bit per position below width, where the positions ascend.
        std::vector<std::uint64_t> words;
        // The positions in row-major order, where they repeat or turn back.
        std::vector<std::size_t> positions;

        [[nodiscard]] gslice_index() = default;

        [[nodiscard]] gslice_index(std::size_t first, std::vector<std::size_t> dim_lengths, std::vector<std::size_t> dim_strides)
                : start(first)
                , lengths(std::move(dim_lengths))
                , strides(std::move(dim_strides))
                , count(count_of(lengths))
        {
                assert(lengths.size() == strides.size());
                if (count == 0UZ) {
                        return;
                }
                if (auto const span = ascending_span(); span != 0UZ) {
                        width = start + span;
                        words = std::vector<std::uint64_t>((width + word_width - 1UZ) / word_width);
                        for_each_position(start, lengths, strides, count, [&](std::size_t pos) noexcept -> void { words[pos / word_width] |= std::uint64_t{1} << (pos % word_width); });
                } else {
                        positions.reserve(count);
                        for_each_position(start, lengths, strides, count, [&](std::size_t pos) -> void {
                                positions.push_back(pos);
                                width = std::ranges::max(width, pos + 1UZ);
                        });
                }
        }

        // No lengths at all select nothing, as the standard libraries agree.
        [[nodiscard]] static auto count_of(std::span<std::size_t const> dim_lengths) noexcept
                -> std::size_t
        {
                auto result = dim_lengths.empty() ? 0UZ : 1UZ;
                for (auto const length : dim_lengths) {
                        result *= length;
                }
                return result;
        }

        // From the shape alone, the span the positions cover where row-major order ascends, each stride clearing all inside it.
        [[nodiscard]] auto ascending_span() const noexcept
                -> std::size_t
        {
                auto span = 1UZ;
                for (auto d = lengths.size() - 1UZ; d < lengths.size(); --d) {
                        if (lengths[d] < 2UZ) {
                                continue;
                        }
                        if (strides[d] < span) {
                                return 0UZ;
                        }
                        span += (lengths[d] - 1UZ) * strides[d];
                }
                return span;
        }

        [[nodiscard]] auto is_masked() const noexcept
                -> bool
        {
                return positions.empty();
        }
};

// The door through which the gslice proxy reads a bit_gslice's index.
struct gslice_access
{
        template<class G>
        [[nodiscard]] static auto index(G const& g) noexcept
                -> gslice_index const&
        {
                return *g.m_index;
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_GSLICE_INDEX_HPP
