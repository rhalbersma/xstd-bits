//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/intrinsic.hpp> // bools_per_block, expand_block
#include <cstdint>                        // uint64_t
#include <span>                           // span

// Each function is compiled once per target configuration, and its assembly read against the directives above it.
extern "C" {

// The byte table, a PDEP per byte where PDEP is fast, and under AVX-512BW one masked store with nothing to branch on.
// CHECK-LABEL: codegen_expand_block:
// PORTABLE-NOT: {{^\s+pdep\s}}
// FAST-BMI2: {{^\s+pdep\s}}
// AVX512BW-NOT: {{^\s+(j[a-z]+|call)\s}}
// AVX512BW: {{^\s+kmovq\s}}
// AVX512BW-NOT: {{^\s+(j[a-z]+|call)\s}}
// AVX512BW: {{^\s+vmovdqu(8|64)\s+(zmmword ptr|ZMMWORD PTR) \[r[a-z0-9]+\], zmm}}
// AVX512BW-NOT: {{^\s+(j[a-z]+|call)\s}}
// AVX512BW: {{^\s+ret}}
auto codegen_expand_block(std::uint64_t block, unsigned char* bools)
        -> void
{
        xstd::bits::detail::expand_block(block, std::span<unsigned char, xstd::bits::detail::bools_per_block>(bools, xstd::bits::detail::bools_per_block));
}

} // extern "C"
