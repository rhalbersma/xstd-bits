//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <fuzz/decoder.hpp>                         // checker, decoder, run, throws
#include <xstd/bits/algorithm.hpp>                  // bit_all_of, bit_any_of, bit_count, bit_mismatch, bit_none_of
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector, bit_bounded_vector
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <algorithm>                                // count, equal, fill, mismatch
#include <array>                                    // array
#include <compare>                                  // is_eq, is_lt
#include <cstddef>                                  // ptrdiff_t, size_t
#include <cstdint>                                  // uint8_t, uint64_t
#include <functional>                               // hash
#include <iterator>                                 // next
#include <new>                                      // bad_alloc
#include <ranges>                                   // from_range, iota, reverse, ssize
#include <stdexcept>                                // out_of_range
#include <tuple>                                    // tuple
#include <type_traits>                              // type_identity
#include <utility>                                  // move, swap
#include <vector>                                   // vector

namespace {

// The sequence reading's owners: a heap, a static width, a capacity, and a small buffer, each across blocks.
using owners = std::tuple<
        xstd::bit_vector,
        xstd::basic_bit_vector<std::uint8_t>,
        xstd::bit_array<1>,
        xstd::basic_bit_array<std::uint8_t, 17>,
        xstd::bit_array<65>,
        xstd::bit_array<200>,
        xstd::basic_bit_bounded_vector<std::uint8_t, 17>,
        xstd::bit_bounded_vector<130>,
        xstd::basic_bit_small_vector<std::uint64_t, 64>>;

using model = std::vector<bool>;

// libc++'s vector<bool> moves a tail with a mask shifted by the word's full width, so the model never inserts mid-way.
[[nodiscard]] auto spliced(model const& m, std::size_t pos, model const& middle)
        -> model
{
        auto const split = std::next(m.begin(), static_cast<std::ptrdiff_t>(pos));
        auto result      = model(m.begin(), split);
        result.insert(result.end(), middle.begin(), middle.end());
        result.insert(result.end(), split, m.end());
        return result;
}

// A run-time size stays within a few blocks, which is where growth and the clear tail are exercised.
inline constexpr auto dynamic_bits = 520UZ;

template<class X>
concept growing = requires (X& x) { x.resize(0UZ); };

// The longest the fuzzer grows X to: its capacity where it has one in reach, else a few blocks.
template<class X>
[[nodiscard]] auto ceiling()
        -> std::size_t
{
        auto const m = X().max_size();
        return m < dynamic_bits ? m : dynamic_bits;
}

// A capacity the fuzzer can reach, past which growth is refused rather than allocated.
template<class X>
[[nodiscard]] auto capped()
        -> bool
{
        return X().max_size() <= dynamic_bits;
}

template<class X>
auto expect_equal(fuzz::checker const& check, X const& x, model const& m)
        -> void
{
        check.expect(x.size() == m.size(), "size");
        check.expect(x.empty() == m.empty(), "empty");
        check.expect(std::ranges::equal(x, m), "forward iteration");
        check.expect(std::ranges::equal(x | std::views::reverse, m | std::views::reverse), "reverse iteration");
        for (auto const i : std::views::iota(0UZ, m.size())) {
                check.expect(x[i] == m[i], "operator[]");
        }
        if (not m.empty()) {
                check.expect(x.front() == m.front(), "front");
                check.expect(x.back() == m.back(), "back");
        }
}

// The position an owner's iterator names, as an index the model's can be compared against.
template<class X, class I>
[[nodiscard]] auto index_of(X const& x, I it)
        -> std::size_t
{
        return static_cast<std::size_t>(it - x.begin());
}

// A copy of y at x's size, so the elementwise operators see two operands of one size.
template<class X>
auto align(X& z, model& mz, std::size_t size)
        -> void
{
        if constexpr (growing<X>) {
                z.resize(size);
                mz.resize(size);
        }
}

template<class X>
auto expect_unchanged_on_refusal(fuzz::checker const& check, X& x, model const& m, auto grow)
        -> void
{
        auto const before = x;
        check.expect(fuzz::throws<std::bad_alloc>([&] -> void { grow(x); }), "bad_alloc past the capacity");
        check.expect(x == before, "unchanged by the refused growth");
        expect_equal(check, x, m);
}

[[nodiscard]] auto bools(fuzz::decoder& in, std::size_t n)
        -> model
{
        auto result = model();
        for ([[maybe_unused]] auto const k : std::views::iota(0UZ, n)) {
                result.push_back(in.boolean());
        }
        return result;
}

template<class X>
auto fuzz_one(fuzz::decoder& in)
        -> void
{
        auto const top = ceiling<X>();
        auto check     = fuzz::checker();
        auto xs        = std::array<X, 2>();
        auto ms        = std::array<model, 2>{model(xs[0].size()), model(xs[1].size())};
        while (not in.empty()) {
                auto const i    = in.byte() % 2UZ;
                auto const j    = in.byte() % 2UZ;
                auto& x         = xs[i];
                auto& m         = ms[i];
                auto const& y   = xs[j];
                auto const& my  = ms[j];
                auto const size = m.size();
                switch (in.byte() % 20U) {
                        case 0: {
                                check.step("operator[], at, reference");
                                if (size != 0UZ) {
                                        auto const pos = in.below(size);
                                        switch (in.byte() % 3U) {
                                                case 0: {
                                                        auto const val = in.boolean();
                                                        x[pos]         = val;
                                                        m[pos]         = val;
                                                        break;
                                                }
                                                case 1: {
                                                        x.at(pos).flip();
                                                        m.at(pos).flip();
                                                        break;
                                                }
                                                default: {
                                                        auto const val = in.boolean();
                                                        x.at(pos)      = val;
                                                        m.at(pos)      = val;
                                                        break;
                                                }
                                        }
                                }
                                break;
                        }
                        case 1: {
                                check.step("at past the size");
                                auto const pos = size + in.below(70UZ);
                                check.expect(fuzz::throws<std::out_of_range>([&] -> void { static_cast<void>(static_cast<X const&>(x).at(pos)); }), "at out_of_range");
                                break;
                        }
                        case 2: {
                                check.step("flip(), fill");
                                if (in.boolean()) {
                                        x.flip();
                                        m.flip();
                                } else {
                                        auto const val = in.boolean();
                                        x.fill(val);
                                        std::ranges::fill(m, val);
                                }
                                break;
                        }
                        case 3: {
                                check.step("&=, |=, ^=");
                                auto z  = y;
                                auto mz = my;
                                align(z, mz, size);
                                auto const op = in.byte() % 3U;
                                switch (op) {
                                        case 0: {
                                                x &= z;
                                                break;
                                        }
                                        case 1: {
                                                x |= z;
                                                break;
                                        }
                                        default: {
                                                x ^= z;
                                                break;
                                        }
                                }
                                for (auto const k : std::views::iota(0UZ, size)) {
                                        m[k] = op == 0U ? (m[k] and mz[k]) : (op == 1U ? (m[k] or mz[k]) : (m[k] != mz[k]));
                                }
                                break;
                        }
                        case 4: {
                                check.step("~, &, |, ^");
                                auto z  = y;
                                auto mz = my;
                                align(z, mz, size);
                                auto mnot = m;
                                mnot.flip();
                                auto mand = m;
                                auto mor  = m;
                                auto mxor = m;
                                for (auto const k : std::views::iota(0UZ, size)) {
                                        mand[k] = m[k] and mz[k];
                                        mor[k]  = m[k] or mz[k];
                                        mxor[k] = m[k] != mz[k];
                                }
                                expect_equal(check, ~x, mnot);
                                expect_equal(check, x & z, mand);
                                expect_equal(check, x | z, mor);
                                expect_equal(check, x ^ z, mxor);
                                break;
                        }
                        case 5: {
                                check.step("bit_count, bit_all_of, bit_any_of, bit_none_of, bit_mismatch");
                                auto const n = std::ranges::count(m, true);
                                check.expect(xstd::bit_count(x) == n, "bit_count");
                                check.expect(xstd::bit_all_of(x) == (n == std::ranges::ssize(m)), "bit_all_of");
                                check.expect(xstd::bit_any_of(x) == (n != 0), "bit_any_of");
                                check.expect(xstd::bit_none_of(x) == (n == 0), "bit_none_of");
                                auto z  = y;
                                auto mz = my;
                                align(z, mz, size);
                                if (z.size() == size) {
                                        auto const first = std::ranges::mismatch(m, mz).in1;
                                        check.expect(xstd::bit_mismatch(x, z).in1 - x.begin() == first - m.begin(), "bit_mismatch");
                                }
                                break;
                        }
                        case 6: {
                                check.step("==, <=>, hash");
                                check.expect((x == y) == (m == my), "==");
                                auto const order  = x <=> y;
                                auto const morder = m <=> my;
                                check.expect(std::is_eq(order) == std::is_eq(morder), "<=> equal");
                                check.expect(std::is_lt(order) == std::is_lt(morder), "<=> less");
                                if (m == my) {
                                        check.expect(std::hash<X>()(x) == std::hash<X>()(y), "equal sequences hash alike");
                                }
                                break;
                        }
                        case 7: {
                                check.step("swap");
                                if (in.boolean()) {
                                        xs[0].swap(xs[1]);
                                } else {
                                        swap(xs[0], xs[1]);
                                }
                                std::swap(ms[0], ms[1]);
                                break;
                        }
                        case 8: {
                                check.step("copy, move");
                                if (in.boolean()) {
                                        x = y;
                                } else {
                                        auto tmp = y;
                                        x        = std::move(tmp);
                                }
                                m = my;
                                expect_equal(check, X(x), m);
                                break;
                        }
                        case 9: {
                                check.step("push_back, emplace_back, pop_back");
                                if constexpr (growing<X>) {
                                        if (in.boolean()) {
                                                auto const val = in.boolean();
                                                if (size < top) {
                                                        if (in.boolean()) {
                                                                x.push_back(val);
                                                        } else {
                                                                check.expect(x.emplace_back(val) == val, "emplace_back's reference");
                                                        }
                                                        m.push_back(val);
                                                } else if (capped<X>()) {
                                                        expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.push_back(val); });
                                                }
                                        } else if (size != 0UZ) {
                                                x.pop_back();
                                                m.pop_back();
                                        }
                                }
                                break;
                        }
                        case 10: {
                                check.step("resize");
                                if constexpr (growing<X>) {
                                        auto const n = in.below(top + 1UZ);
                                        if (in.boolean()) {
                                                auto const val = in.boolean();
                                                x.resize(n, val);
                                                m.resize(n, val);
                                        } else {
                                                x.resize(n);
                                                m.resize(n);
                                        }
                                        if (capped<X>()) {
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.resize(top + 1UZ + in.below(70UZ)); });
                                        }
                                }
                                break;
                        }
                        case 11: {
                                check.step("clear");
                                if constexpr (growing<X>) {
                                        x.clear();
                                        m.clear();
                                }
                                break;
                        }
                        case 12: {
                                check.step("insert(position, value), insert(position, n, value)");
                                if constexpr (growing<X>) {
                                        auto const pos       = in.below(size + 1UZ);
                                        auto const val       = in.boolean();
                                        auto const position  = std::next(x.cbegin(), static_cast<std::ptrdiff_t>(pos));
                                        auto const mposition = std::next(m.cbegin(), static_cast<std::ptrdiff_t>(pos));
                                        if (in.boolean()) {
                                                auto const n  = in.below(top - size + 1UZ);
                                                auto const it = x.insert(position, n, val);
                                                m             = spliced(m, pos, model(n, val));
                                                check.expect(index_of(x, it) == pos, "position");
                                        } else if (size < top) {
                                                auto const it  = x.insert(position, val);
                                                auto const mit = m.insert(mposition, val);
                                                check.expect(index_of(x, it) == static_cast<std::size_t>(mit - m.begin()), "single insert's position");
                                        } else if (capped<X>()) {
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.insert(z.cbegin(), 1UZ + in.below(70UZ), val); });
                                        }
                                }
                                break;
                        }
                        case 13: {
                                check.step("insert_range, insert(position, first, last)");
                                if constexpr (growing<X>) {
                                        auto const pos = in.below(size + 1UZ);
                                        auto const rg  = bools(in, in.below(top - size + 1UZ));
                                        auto const it  = in.boolean() ? x.insert_range(std::next(x.cbegin(), static_cast<std::ptrdiff_t>(pos)), rg) : x.insert(std::next(x.cbegin(), static_cast<std::ptrdiff_t>(pos)), rg.begin(), rg.end());
                                        m              = spliced(m, pos, model(rg.begin(), rg.end()));
                                        check.expect(index_of(x, it) == pos, "position");
                                }
                                break;
                        }
                        case 14: {
                                check.step("erase(position), erase(first, last)");
                                if constexpr (growing<X>) {
                                        if (size != 0UZ) {
                                                auto lo = in.below(size);
                                                auto hi = in.below(size + 1UZ);
                                                if (hi < lo) {
                                                        std::swap(lo, hi);
                                                }
                                                if (in.boolean()) {
                                                        auto const it  = x.erase(std::next(x.cbegin(), static_cast<std::ptrdiff_t>(lo)));
                                                        auto const mit = m.erase(std::next(m.cbegin(), static_cast<std::ptrdiff_t>(lo)));
                                                        check.expect(index_of(x, it) == static_cast<std::size_t>(mit - m.begin()), "position");
                                                } else {
                                                        auto const it  = x.erase(std::next(x.cbegin(), static_cast<std::ptrdiff_t>(lo)), std::next(x.cbegin(), static_cast<std::ptrdiff_t>(hi)));
                                                        auto const mit = m.erase(std::next(m.cbegin(), static_cast<std::ptrdiff_t>(lo)), std::next(m.cbegin(), static_cast<std::ptrdiff_t>(hi)));
                                                        check.expect(index_of(x, it) == static_cast<std::size_t>(mit - m.begin()), "range position");
                                                }
                                        }
                                }
                                break;
                        }
                        case 15: {
                                check.step("assign(n, value), assign_range");
                                if constexpr (growing<X>) {
                                        if (in.boolean()) {
                                                auto const n   = in.below(top + 1UZ);
                                                auto const val = in.boolean();
                                                x.assign(n, val);
                                                m.assign(n, val);
                                        } else {
                                                auto const rg = bools(in, in.below(top + 1UZ));
                                                x.assign_range(rg);
                                                m.assign(rg.begin(), rg.end());
                                        }
                                }
                                break;
                        }
                        case 16: {
                                check.step("append_range");
                                if constexpr (growing<X>) {
                                        if (in.boolean()) {
                                                auto const rg = bools(in, in.below(top - size + 1UZ));
                                                x.append_range(rg);
                                                m.insert(m.end(), rg.begin(), rg.end());
                                        } else if (size + my.size() <= top) {
                                                // [sequence.reqmts] forbids appending a vector to itself.
                                                auto const other  = y;
                                                auto const mother = my;
                                                x.append_range(other);
                                                m.insert(m.end(), mother.begin(), mother.end());
                                        } else if (capped<X>()) {
                                                auto const other = y;
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.append_range(other); });
                                        }
                                }
                                break;
                        }
                        case 17: {
                                check.step("reserve, shrink_to_fit, capacity");
                                if constexpr (growing<X>) {
                                        auto const n = in.below(top + 1UZ);
                                        x.reserve(n);
                                        check.expect(x.capacity() >= n, "capacity after reserve");
                                        x.shrink_to_fit();
                                        check.expect(x.capacity() >= size, "capacity after shrink_to_fit");
                                        if (capped<X>()) {
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.reserve(top + 1UZ + in.below(70UZ)); });
                                        }
                                }
                                break;
                        }
                        case 18: {
                                check.step("try_push_back");
                                if constexpr (requires { x.try_push_back(true); }) {
                                        auto const val = in.boolean();
                                        auto const r   = x.try_push_back(val);
                                        check.expect(r.has_value() == (size < top), "try_push_back's answer");
                                        if (r) {
                                                check.expect(*r == val, "try_push_back's reference");
                                                m.push_back(val);
                                        }
                                }
                                break;
                        }
                        case 19: {
                                check.step("construct from a size and a value, or from a range");
                                if constexpr (growing<X>) {
                                        if (in.boolean()) {
                                                auto const n   = in.below(top + 1UZ);
                                                auto const val = in.boolean();
                                                x              = X(n, val);
                                                m              = model(n, val);
                                        } else {
                                                auto const rg = bools(in, in.below(top + 1UZ));
                                                x             = X(std::from_range, rg);
                                                m             = rg;
                                        }
                                }
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
