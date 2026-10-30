//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set/bit_fixed_set.hpp> // bit_fixed_set
#include <xstd/ints/bit.hpp>                   // countr_zero, popcount
#include <array>                               // array
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t

namespace {

using set1  = xstd::bit_fixed_set<64>;
using set2  = xstd::bit_fixed_set<128>;
using word2 = std::array<std::uint64_t, 2>;

} // namespace

// Each function is compiled once per target configuration, and its assembly read against the directives above it.
extern "C" {

auto codegen_set1_and(set1* lhs, set1 const* rhs)
        -> void
{
        *lhs &= *rhs;
}

auto codegen_word1_and(std::uint64_t* lhs, std::uint64_t const* rhs)
        -> void
{
        *lhs &= *rhs;
}

auto codegen_set1_or(set1* lhs, set1 const* rhs)
        -> void
{
        *lhs |= *rhs;
}

auto codegen_word1_or(std::uint64_t* lhs, std::uint64_t const* rhs)
        -> void
{
        *lhs |= *rhs;
}

auto codegen_set1_xor(set1* lhs, set1 const* rhs)
        -> void
{
        *lhs ^= *rhs;
}

auto codegen_word1_xor(std::uint64_t* lhs, std::uint64_t const* rhs)
        -> void
{
        *lhs ^= *rhs;
}

auto codegen_set1_minus(set1* lhs, set1 const* rhs)
        -> void
{
        *lhs -= *rhs;
}

auto codegen_word1_minus(std::uint64_t* lhs, std::uint64_t const* rhs)
        -> void
{
        *lhs &= ~*rhs;
}

[[nodiscard]] auto codegen_set1_size(set1 const* s)
        -> std::size_t
{
        return s->size();
}

[[nodiscard]] auto codegen_word1_size(std::uint64_t const* w)
        -> std::size_t
{
        return static_cast<std::size_t>(xstd::popcount(*w));
}

// The unchecked lookup, whose key is a precondition, as std::bitset's operator[] is beside test(pos).
[[nodiscard]] auto codegen_set1_lookup(set1 const* s, std::size_t x)
        -> bool
{
        return (*s)[x];
}

[[nodiscard]] auto codegen_word1_lookup(std::uint64_t const* w, std::size_t x)
        -> bool
{
        return ((*w >> x) & 1U) != 0;
}

// The lowest key taken and removed: a count of trailing zeros, then the lowest set bit cleared.
// CHECK-LABEL: codegen_set1_pop_front:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_set1_pop_front(set1* s)
        -> std::size_t
{
        auto const front = s->front();
        s->pop_front();
        return front;
}

[[nodiscard]] auto codegen_word1_pop_front(std::uint64_t* w)
        -> std::size_t
{
        auto const front = static_cast<std::size_t>(xstd::countr_zero(*w));
        *w &= *w - 1;
        return front;
}

auto codegen_set2_and(set2* lhs, set2 const* rhs)
        -> void
{
        *lhs &= *rhs;
}

auto codegen_word2_and(word2* lhs, word2 const* rhs)
        -> void
{
        auto const rhs_blocks = *rhs;
        auto lhs_blocks       = *lhs;
        lhs_blocks[0] &= rhs_blocks[0];
        lhs_blocks[1] &= rhs_blocks[1];
        *lhs = lhs_blocks;
}

auto codegen_set2_or(set2* lhs, set2 const* rhs)
        -> void
{
        *lhs |= *rhs;
}

auto codegen_word2_or(word2* lhs, word2 const* rhs)
        -> void
{
        auto const rhs_blocks = *rhs;
        auto lhs_blocks       = *lhs;
        lhs_blocks[0] |= rhs_blocks[0];
        lhs_blocks[1] |= rhs_blocks[1];
        *lhs = lhs_blocks;
}

auto codegen_set2_xor(set2* lhs, set2 const* rhs)
        -> void
{
        *lhs ^= *rhs;
}

auto codegen_word2_xor(word2* lhs, word2 const* rhs)
        -> void
{
        auto const rhs_blocks = *rhs;
        auto lhs_blocks       = *lhs;
        lhs_blocks[0] ^= rhs_blocks[0];
        lhs_blocks[1] ^= rhs_blocks[1];
        *lhs = lhs_blocks;
}

auto codegen_set2_minus(set2* lhs, set2 const* rhs)
        -> void
{
        *lhs -= *rhs;
}

auto codegen_word2_minus(word2* lhs, word2 const* rhs)
        -> void
{
        auto const rhs_blocks = *rhs;
        auto lhs_blocks       = *lhs;
        lhs_blocks[0] &= ~rhs_blocks[0];
        lhs_blocks[1] &= ~rhs_blocks[1];
        *lhs = lhs_blocks;
}

[[nodiscard]] auto codegen_set2_size(set2 const* s)
        -> std::size_t
{
        return s->size();
}

[[nodiscard]] auto codegen_word2_size(word2 const* w)
        -> std::size_t
{
        return static_cast<std::size_t>(xstd::popcount((*w)[0])) + static_cast<std::size_t>(xstd::popcount((*w)[1]));
}

// The reductions over three, six and eight blocks fold every block and test once, with nothing to branch on.
// CHECK-LABEL: codegen_set3_none:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_set3_none(xstd::bit_fixed_set<192> const* s)
        -> bool
{
        return s->empty();
}

// CHECK-LABEL: codegen_set3_equal:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_set3_equal(xstd::bit_fixed_set<192> const* lhs, xstd::bit_fixed_set<192> const* rhs)
        -> bool
{
        return *lhs == *rhs;
}

// CHECK-LABEL: codegen_set6_equal:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_set6_equal(xstd::bit_fixed_set<384> const* lhs, xstd::bit_fixed_set<384> const* rhs)
        -> bool
{
        return *lhs == *rhs;
}

// CHECK-LABEL: codegen_set8_equal:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_set8_equal(xstd::bit_fixed_set<512> const* lhs, xstd::bit_fixed_set<512> const* rhs)
        -> bool
{
        return *lhs == *rhs;
}

} // extern "C"
