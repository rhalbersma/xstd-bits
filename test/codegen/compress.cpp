//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit/bit_compress.hpp>      // bit_compress
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // bit_fixed_set
#include <xstd/ints/bit.hpp>                   // bit_compress
#include <cstdint>                             // uint64_t

namespace {

using set1 = xstd::bit_fixed_set<64>;
using set3 = xstd::bit_fixed_set<192>;

} // namespace

// Each function is compiled once per target configuration, and its assembly read against the directives above it.
extern "C" {

// A set packed into an integer is a PEXT per block, shifted past the positions of the blocks before it, with no branch.
// CHECK-LABEL: codegen_compress_set1:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// FAST-BMI2: {{^\s+pext\s}}
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call|pext)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_compress_set1(set1 const* x, set1 const* m)
        -> std::uint64_t
{
        return xstd::bit_compress<std::uint64_t>(*x, *m);
}

[[nodiscard]] auto codegen_compress_word1(std::uint64_t const* x, std::uint64_t const* m)
        -> std::uint64_t
{
        return xstd::bit_compress(*x, *m);
}

// CHECK-LABEL: codegen_compress_set3:
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// FAST-BMI2: {{^\s+pext\s}}
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// FAST-BMI2: {{^\s+pext\s}}
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call)\s}}
// FAST-BMI2: {{^\s+pext\s}}
// BRANCH-FREE-NOT: {{^\s+(j[a-z]+|call|pext)\s}}
// BRANCH-FREE: {{^\s+ret}}
[[nodiscard]] auto codegen_compress_set3(set3 const* x, set3 const* m)
        -> std::uint64_t
{
        return xstd::bit_compress<std::uint64_t>(*x, *m);
}

} // extern "C"
