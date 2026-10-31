//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <fuzz/decoder.hpp>                                // checker, decoder, run
#include <xstd/bits/bit/bit_convert.hpp>                   // bit_convert
#include <xstd/bits/bit_algorithm.hpp>                     // bit_all_of, bit_any_of, bit_count, bit_none_of
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // basic_bit_fixed_valarray, bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_inplace_valarray.hpp> // basic_bit_inplace_valarray, bit_inplace_valarray
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // basic_bit_valarray, bit_valarray
#include <xstd/bits/bit_vector.hpp>                        // bit_vector
#include <xstd/bits/ext/boost/bit_small_valarray.hpp>      // basic_bit_small_valarray
#include <algorithm>                                       // count, equal, fill, min
#include <array>                                           // array
#include <climits>                                         // INT_MIN
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint8_t
#include <ranges>                                          // iota, range_value_t, ssize
#include <tuple>                                           // tuple
#include <type_traits>                                     // type_identity
#include <utility>                                         // as_const, move, swap
#include <valarray>                                        // slice
#include <vector>                                          // vector

namespace {

// The valarray reading's owners: a heap, a static size, a capacity and inline blocks; slices and masks over each.
using owners = std::tuple<
        xstd::bit_valarray,
        xstd::basic_bit_valarray<std::vector<std::uint8_t>>,
        xstd::bit_fixed_valarray<1>,
        xstd::basic_bit_fixed_valarray<std::uint8_t, 17>,
        xstd::bit_fixed_valarray<65>,
        xstd::bit_fixed_valarray<200>,
        xstd::basic_bit_inplace_valarray<std::uint8_t, 17>,
        xstd::bit_inplace_valarray<130>,
        xstd::basic_bit_small_valarray<std::uint8_t, 17>>;

using model = std::vector<bool>;

// A run-time size stays within a few blocks, which is where the padding past the last element is exercised.
inline constexpr auto dynamic_bits = 520UZ;

template<class X>
concept growing = requires (X& x) { x.resize(0UZ); };

// The most a growing owner is resized to: its capacity where that is in reach, else a few blocks.
template<class X>
inline constexpr auto capacity = dynamic_bits;

template<class Blocks, std::size_t N>
        requires (N != std::dynamic_extent)
inline constexpr auto capacity<xstd::basic_bit_valarray<Blocks, N>> = N < dynamic_bits ? N : dynamic_bits;

// Equal elements, and a count over whole blocks that would see a bit set in the padding.
template<class X>
auto expect_equal(fuzz::checker const& check, X const& x, model const& m)
        -> void
{
        check.expect(x.size() == m.size(), "size");
        check.expect(std::ranges::equal(x, m), "elements");
        check.expect(xstd::ranges::bit_count(x, true) == std::ranges::count(m, true), "padding");
}

// The owner holding the model's bools, at the model's size.
template<class X>
[[nodiscard]] auto from_model(model const& m)
        -> X
{
        auto x = X();
        if constexpr (growing<X>) {
                x.resize(m.size());
        }
        for (auto const i : std::views::iota(0UZ, m.size())) {
                x[i] = m[i];
        }
        return x;
}

template<class F>
[[nodiscard]] auto elementwise(model const& a, model const& b, F f)
        -> model
{
        auto result = model(a.size());
        for (auto const i : std::views::iota(0UZ, a.size())) {
                result[i] = f(a[i], b[i]);
        }
        return result;
}

// Element i takes element i + n of the model, false outside it, or wrapped round it.
[[nodiscard]] auto shifted(model const& a, long long n, bool wrap)
        -> model
{
        auto result  = model(a.size());
        auto const s = static_cast<long long>(a.size());
        for (auto const i : std::views::iota(0LL, s)) {
                auto const j = wrap ? (((i + n) % s) + s) % s : i + n;
                if (j >= 0 and j < s) {
                        result[static_cast<std::size_t>(i)] = a[static_cast<std::size_t>(j)];
                }
        }
        return result;
}

// The eleven binary operators by number, on two bools for the model and on two operands for the owner.
[[nodiscard]] auto model_op(unsigned op, bool a, bool b)
        -> bool
{
        switch (op) {
                case 0:
                        return a and b;
                case 1:
                        return a or b;
                case 2:
                        return a != b;
                case 3:
                        return a == b;
                case 4:
                        return a < b;
                case 5:
                        return a > b;
                case 6:
                        return a <= b;
                case 7:
                        return a >= b;
                default:
                        return a != b;
        }
}

template<class A, class B>
[[nodiscard]] auto owner_op(unsigned op, A const& a, B const& b)
{
        switch (op) {
                case 0:
                        return a & b;
                case 1:
                        return a | b;
                case 2:
                        return a ^ b;
                case 3:
                        return a == b;
                case 4:
                        return a < b;
                case 5:
                        return a > b;
                case 6:
                        return a <= b;
                case 7:
                        return a >= b;
                case 8:
                        return a != b;
                case 9:
                        return a && b;
                default:
                        return a || b;
        }
}

// The positions a slice or a mask selects, in the order a gather reads them and a scatter writes them.
[[nodiscard]] auto gathered(model const& m, std::vector<std::size_t> const& positions)
        -> model
{
        auto result = model();
        for (auto const pos : positions) {
                result.push_back(m[pos]);
        }
        return result;
}

// One write through a proxy, position by position and in order, as the model takes it.
[[nodiscard]] auto scattered(unsigned op, model m, std::vector<std::size_t> const& positions, model const& b)
        -> model
{
        for (auto const j : std::views::iota(0UZ, positions.size())) {
                auto const pos = positions[j];
                m[pos]         = op == 0U ? b[j] : model_op(op - 1U, m[pos], b[j]);
        }
        return m;
}

template<class P, class A>
auto proxy_op(unsigned op, P const& p, A const& a)
        -> void
{
        switch (op) {
                case 0:
                        p = a;
                        break;
                case 1:
                        p &= a;
                        break;
                case 2:
                        p |= a;
                        break;
                default:
                        p ^= a;
                        break;
        }
}

// The positions of a slice reduced into range: a stride of zero repeats one position, at most as often as there are.
[[nodiscard]] auto slice_in(fuzz::decoder& in, std::size_t size)
        -> std::slice
{
        if (size == 0UZ) {
                return {0UZ, 0UZ, 1UZ};
        }
        auto const start  = in.below(size);
        auto const stride = in.below(size + 2UZ);
        auto const most   = stride == 0UZ ? std::ranges::min(size, 3UZ) : ((size - 1UZ - start) / stride) + 1UZ;
        return {start, in.below(most + 1UZ), stride};
}

// A gather compared with the model, or a write through the proxy from an array of the same blocks or from a scalar.
template<class X, class Selector>
auto fuzz_subscript(fuzz::decoder& in, fuzz::checker const& check, X& x, model& m, Selector const& selector, std::vector<std::size_t> const& positions)
        -> void
{
        using operand    = xstd::basic_bit_valarray<std::vector<std::ranges::range_value_t<typename X::block_container_type>>>;
        auto const count = positions.size();
        switch (in.byte() % 3U) {
                case 0: {
                        expect_equal(check, std::as_const(x)[selector], gathered(m, positions));
                        break;
                }
                case 1: {
                        auto const op = in.byte() % 4U;
                        auto b        = model(count);
                        for (auto&& e : b) {
                                e = in.boolean();
                        }
                        proxy_op(op, x[selector], from_model<operand>(b));
                        m = scattered(op, m, positions, b);
                        break;
                }
                default: {
                        auto const op  = in.byte() % 4U;
                        auto const val = in.boolean();
                        proxy_op(op, x[selector], val);
                        m = scattered(op, m, positions, model(count, val));
                        break;
                }
        }
}

[[nodiscard]] auto model_of_op(unsigned op, bool a, bool b)
        -> bool
{
        return op == 9U ? (a and b) : (op == 10U ? (a or b) : model_op(op, a, b));
}

template<class X>
auto fuzz_one(fuzz::decoder& in)
        -> void
{
        auto const top = capacity<X>;
        auto check     = fuzz::checker();
        auto xs        = std::array<X, 2>();
        auto ms        = std::array<model, 2>{model(xs[0].size()), model(xs[1].size())};
        while (not in.empty()) {
                auto const i    = in.byte() % 2UZ;
                auto const j    = in.byte() % 2UZ;
                auto& x         = xs[i];
                auto& m         = ms[i];
                auto const& my  = ms[j];
                auto const size = m.size();
                // The other operand at this one's size, its elements kept as far as they reach.
                auto my_sized = my;
                my_sized.resize(size);
                auto const y = from_model<X>(my_sized);
                switch (in.byte() % 14U) {
                        case 0: {
                                check.step("operator[]");
                                if (size != 0UZ) {
                                        auto const pos = in.below(size);
                                        auto const val = in.boolean();
                                        x[pos]         = val;
                                        m[pos]         = val;
                                }
                                break;
                        }
                        case 1: {
                                check.step("operator=(bool)");
                                auto const val = in.boolean();
                                x              = val;
                                std::ranges::fill(m, val);
                                break;
                        }
                        case 2: {
                                check.step("resize");
                                if constexpr (growing<X>) {
                                        auto const n   = in.below(top + 1UZ);
                                        auto const val = in.boolean();
                                        x.resize(n, val);
                                        m = model(n, val);
                                }
                                break;
                        }
                        case 3: {
                                check.step("&=, |=, ^= with an array");
                                auto const op = in.byte() % 3U;
                                switch (op) {
                                        case 0:
                                                x &= y;
                                                break;
                                        case 1:
                                                x |= y;
                                                break;
                                        default:
                                                x ^= y;
                                                break;
                                }
                                m = elementwise(m, my_sized, [&](bool a, bool b) -> bool { return model_op(op, a, b); });
                                break;
                        }
                        case 4: {
                                check.step("&=, |=, ^= with a bool");
                                auto const op  = in.byte() % 3U;
                                auto const val = in.boolean();
                                switch (op) {
                                        case 0:
                                                x &= val;
                                                break;
                                        case 1:
                                                x |= val;
                                                break;
                                        default:
                                                x ^= val;
                                                break;
                                }
                                m = elementwise(m, model(size, val), [&](bool a, bool b) -> bool { return model_op(op, a, b); });
                                break;
                        }
                        case 5: {
                                check.step("binary operators on two arrays");
                                auto const op = in.byte() % 11U;
                                expect_equal(check, owner_op(op, std::as_const(x), y), elementwise(m, my_sized, [&](bool a, bool b) -> bool { return model_of_op(op, a, b); }));
                                break;
                        }
                        case 6: {
                                check.step("binary operators on an array and a bool");
                                auto const op  = in.byte() % 11U;
                                auto const val = in.boolean();
                                auto const c   = model(size, val);
                                expect_equal(check, owner_op(op, std::as_const(x), val), elementwise(m, c, [&](bool a, bool b) -> bool { return model_of_op(op, a, b); }));
                                expect_equal(check, owner_op(op, val, std::as_const(x)), elementwise(c, m, [&](bool a, bool b) -> bool { return model_of_op(op, a, b); }));
                                break;
                        }
                        case 7: {
                                check.step("!, ~");
                                auto mnot = m;
                                mnot.flip();
                                expect_equal(check, not x, mnot);
                                expect_equal(check, ~x, mnot);
                                break;
                        }
                        case 8: {
                                check.step("shift, cshift");
                                auto const s = static_cast<long long>(size);
                                auto const n = in.boolean() ? INT_MIN : static_cast<int>(static_cast<long long>(in.below((4UZ * size) + 5UZ)) - (2LL * s) - 2LL);
                                expect_equal(check, x.shift(n), shifted(m, n, false));
                                expect_equal(check, x.cshift(n), shifted(m, n, true));
                                break;
                        }
                        case 9: {
                                check.step("<<=, >>=");
                                auto const n = in.below(size + 70UZ);
                                if (in.boolean()) {
                                        x <<= n;
                                        m = shifted(m, -static_cast<long long>(n), false);
                                } else {
                                        x >>= n;
                                        m = shifted(m, static_cast<long long>(n), false);
                                }
                                break;
                        }
                        case 10: {
                                check.step("apply, sum, min, max, bit_count, bit_all_of, bit_any_of, bit_none_of");
                                auto const n = std::ranges::count(m, true);
                                check.expect(x.sum() == (n != 0), "sum");
                                check.expect(x.min() == (n == std::ranges::ssize(m)), "min");
                                check.expect(x.max() == (n != 0), "max");
                                check.expect(xstd::ranges::bit_all_of(x) == (n == std::ranges::ssize(m)), "bit_all_of");
                                check.expect(xstd::ranges::bit_any_of(x) == (n != 0), "bit_any_of");
                                check.expect(xstd::ranges::bit_none_of(x) == (n == 0), "bit_none_of");
                                auto mnot = m;
                                mnot.flip();
                                expect_equal(check, x.apply([](bool b) -> bool { return not b; }), mnot);
                                expect_equal(check, x.apply([](bool const& b) -> bool { return b; }), m);
                                break;
                        }
                        case 11: {
                                check.step("swap, copy, move, bit_convert");
                                switch (in.byte() % 4U) {
                                        case 0: {
                                                swap(xs[0], xs[1]);
                                                std::swap(ms[0], ms[1]);
                                                break;
                                        }
                                        case 1: {
                                                x = xs[j];
                                                m = my;
                                                break;
                                        }
                                        case 2: {
                                                auto tmp = xs[j];
                                                x        = std::move(tmp);
                                                m        = my;
                                                break;
                                        }
                                        default: {
                                                x = xstd::bit_convert<X>(xstd::bit_convert<xstd::bit_vector>(x));
                                                break;
                                        }
                                }
                                break;
                        }
                        case 12: {
                                check.step("slice gather and scatter");
                                auto const s   = slice_in(in, size);
                                auto positions = std::vector<std::size_t>();
                                for (auto const k : std::views::iota(0UZ, s.size())) {
                                        positions.push_back(s.start() + (k * s.stride()));
                                }
                                fuzz_subscript(in, check, x, m, s, positions);
                                break;
                        }
                        case 13: {
                                check.step("mask gather and scatter");
                                auto positions = std::vector<std::size_t>();
                                for (auto const k : std::views::iota(0UZ, size)) {
                                        if (my_sized[k]) {
                                                positions.push_back(k);
                                        }
                                }
                                fuzz_subscript(in, check, x, m, y, positions);
                                break;
                        }
                        default: {
                                break;
                        }
                }
                expect_equal(check, xs[0], ms[0]);
                expect_equal(check, xs[1], ms[1]);
        }
}

} // namespace

extern "C" auto LLVMFuzzerTestOneInput(std::uint8_t const* data, std::size_t size)
        -> int
{
        fuzz::run<owners>(data, size, []<class X>(std::type_identity<X>, fuzz::decoder& in) -> void { fuzz_one<X>(in); });
        return 0;
}
