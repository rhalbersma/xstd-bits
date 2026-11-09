//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm/bit_copy.hpp>  // bit_copy
#include <xstd/bits/bit_algorithm/bit_count.hpp> // bit_count
#include <xstd/bits/bit_algorithm/bit_fill.hpp>  // bit_fill
#include <xstd/bits/bit_algorithm/bit_find.hpp>  // bit_find
#include <xstd/bits/bit_span/bit_span.hpp>       // bit_span
#include <xstd/bits/bit_vector/bit_vector.hpp>   // bit_vector
#include <cstddef>                               // ptrdiff_t, size_t

namespace {

using vec = xstd::bit_vector;

[[nodiscard]] auto at(vec const& v, std::size_t n)
        -> vec::const_iterator
{
        return v.begin() + static_cast<std::ptrdiff_t>(n);
}

[[nodiscard]] auto at(vec& v, std::size_t n)
        -> vec::iterator
{
        return v.begin() + static_cast<std::ptrdiff_t>(n);
}

} // namespace

// Each iterator form beside its range form over the window of the same positions: the shapes share one implementation.
extern "C" {

// CHECK-DAG: codegen_window_count:
auto codegen_window_count(vec const* v, std::size_t lo, std::size_t hi)
        -> std::ptrdiff_t
{
        return xstd::ranges::bit_count(xstd::bit_span(*v).first(hi).last(hi - lo), true);
}

// CHECK-DAG: codegen_iterators_count:
auto codegen_iterators_count(vec const* v, std::size_t lo, std::size_t hi)
        -> std::ptrdiff_t
{
        return xstd::ranges::bit_count(at(*v, lo), at(*v, hi), true);
}

// CHECK-DAG: codegen_window_find:
auto codegen_window_find(vec const* v, std::size_t lo, std::size_t hi)
        -> std::ptrdiff_t
{
        auto const w = xstd::bit_span(*v).first(hi).last(hi - lo);
        return xstd::ranges::bit_find(w, true) - w.begin();
}

// CHECK-DAG: codegen_iterators_find:
auto codegen_iterators_find(vec const* v, std::size_t lo, std::size_t hi)
        -> std::ptrdiff_t
{
        return xstd::ranges::bit_find(at(*v, lo), at(*v, hi), true) - at(*v, lo);
}

// CHECK-DAG: codegen_window_fill:
auto codegen_window_fill(vec* v, std::size_t lo, std::size_t hi)
        -> void
{
        xstd::ranges::bit_fill(xstd::bit_span(*v).first(hi).last(hi - lo), true);
}

// CHECK-DAG: codegen_iterators_fill:
auto codegen_iterators_fill(vec* v, std::size_t lo, std::size_t hi)
        -> void
{
        xstd::bit_fill(at(*v, lo), at(*v, hi), true);
}

// CHECK-DAG: codegen_window_copy:
auto codegen_window_copy(vec const* v, std::size_t lo, std::size_t hi, vec* out, std::size_t d)
        -> void
{
        xstd::ranges::bit_copy(xstd::bit_span(*v).first(hi).last(hi - lo), at(*out, d));
}

// CHECK-DAG: codegen_iterators_copy:
auto codegen_iterators_copy(vec const* v, std::size_t lo, std::size_t hi, vec* out, std::size_t d)
        -> void
{
        xstd::bit_copy(at(*v, lo), at(*v, hi), at(*out, d));
}

} // extern "C"
