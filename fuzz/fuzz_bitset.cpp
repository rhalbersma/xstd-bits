//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <fuzz/decoder.hpp>                     // checker, decoder, run, throws
#include <xstd/bits/bitset.hpp>                 // basic_bitset, bitset
#include <xstd/bits/bounded_bitset.hpp>         // basic_bounded_bitset, bounded_bitset
#include <xstd/bits/dynamic_bitset.hpp>         // basic_dynamic_bitset, dynamic_bitset
#include <xstd/bits/ext/boost/small_bitset.hpp> // basic_small_bitset
#include <boost/dynamic_bitset.hpp>             // dynamic_bitset, to_string
#include <array>                                // array
#include <bitset>                               // bitset
#include <compare>                              // is_eq, is_lt
#include <cstddef>                              // size_t
#include <cstdint>                              // uint8_t, uint64_t
#include <functional>                           // hash
#include <limits>                               // numeric_limits
#include <new>                                  // bad_alloc
#include <ranges>                               // iota
#include <stdexcept>                            // out_of_range, overflow_error
#include <string>                               // string
#include <tuple>                                // tuple
#include <type_traits>                          // type_identity
#include <utility>                              // move, swap

namespace {

// The bitset reading's owners: static widths across block boundaries, a heap, a small buffer, and a capacity.
using owners = std::tuple<
        xstd::bitset<1>,
        xstd::basic_bitset<std::uint8_t, 17>,
        xstd::bitset<64>,
        xstd::bitset<65>,
        xstd::bitset<200>,
        xstd::dynamic_bitset,
        xstd::basic_dynamic_bitset<std::uint8_t>,
        xstd::basic_small_bitset<std::uint64_t, 64>,
        xstd::basic_bounded_bitset<std::uint8_t, 17>,
        xstd::bounded_bitset<130>>;

inline constexpr auto size_max = std::numeric_limits<std::size_t>::max();

// A run-time width stays within a few blocks, which is where its growth and its tail are exercised.
inline constexpr auto dynamic_bits = 520UZ;

template<class X>
concept growing = requires (X& x) { x.resize(0UZ); };

// std::bitset at a width in the type, and boost::dynamic_bitset at one only known at run time.
template<class X>
struct counterpart
{
        using type = boost::dynamic_bitset<>;
};

template<class Block, std::size_t N>
struct counterpart<xstd::basic_bitset<Block, N>>
{
        using type = std::bitset<N>;
};

template<class X>
using model_t = counterpart<X>::type;

// The widest the fuzzer grows X to: its capacity where it has one in reach, else a few blocks.
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

template<class M>
[[nodiscard]] auto string_of(M const& m)
        -> std::string
{
        if constexpr (requires { m.template to_string<char>(); }) {
                return m.to_string();
        } else {
                auto s = std::string();
                boost::to_string(m, s);
                return s;
        }
}

// The first set position at or after pos, scanned on the model, npos where there is none.
template<class M>
[[nodiscard]] auto scan_next(M const& m, std::size_t pos)
        -> std::size_t
{
        for (auto const i : std::views::iota(pos < m.size() ? pos : m.size(), m.size())) {
                if (m.test(i)) {
                        return i;
                }
        }
        return size_max;
}

// The last set position below pos, scanned on the model, npos where there is none.
template<class M>
[[nodiscard]] auto scan_prev(M const& m, std::size_t pos)
        -> std::size_t
{
        auto const n = pos < m.size() ? pos : m.size();
        for (auto i = n - 1UZ; i < n; --i) {
                if (m.test(i)) {
                        return i;
                }
        }
        return size_max;
}

// The order the reading defines: the bit string's, most significant first, and boost's across two widths.
template<class M>
[[nodiscard]] auto three_way(M const& a, M const& b)
        -> std::strong_ordering
{
        if constexpr (requires { a < b; }) {
                return a < b ? std::strong_ordering::less : (b < a ? std::strong_ordering::greater : std::strong_ordering::equal);
        } else {
                return string_of(a) <=> string_of(b);
        }
}

template<class X, class M>
auto expect_equal(fuzz::checker const& check, X const& x, M const& m)
        -> void
{
        check.expect(x.size() == m.size(), "size");
        check.expect(x.count() == m.count(), "count");
        check.expect(x.any() == m.any(), "any");
        check.expect(x.none() == m.none(), "none");
        check.expect(x.all() == m.all(), "all");
        for (auto const i : std::views::iota(0UZ, m.size())) {
                check.expect(x[i] == m[i], "bit");
        }
}

// The model's word resized to the owner's width, so the bitwise operators see two operands of one size.
template<class X, class M>
auto align(X& z, M& mz, std::size_t width)
        -> void
{
        if constexpr (growing<X>) {
                z.resize(width);
                mz.resize(width);
        }
}

template<class X, class M>
auto expect_unchanged_on_refusal(fuzz::checker const& check, X& x, M const& m, auto grow)
        -> void
{
        auto const before = x;
        check.expect(fuzz::throws<std::bad_alloc>([&] -> void { grow(x); }), "bad_alloc past the capacity");
        check.expect(x == before, "unchanged by the refused growth");
        expect_equal(check, x, m);
}

// Mostly a translation within the width; else one at the top of size_t.
[[nodiscard]] auto shift_amount(fuzz::decoder& in, std::size_t width)
        -> std::size_t
{
        return in.byte() % 4U == 0U ? size_max - in.below(70UZ) : in.below(width + 70UZ);
}

template<class X>
auto fuzz_one(fuzz::decoder& in)
        -> void
{
        using M = model_t<X>;
        auto const top = ceiling<X>();
        auto check = fuzz::checker();
        auto xs = std::array<X, 2>();
        auto ms = std::array<M, 2>();
        while (not in.empty()) {
                auto const i = in.byte() % 2UZ;
                auto const j = in.byte() % 2UZ;
                auto& x = xs[i];
                auto& m = ms[i];
                auto const& y = xs[j];
                auto const& my = ms[j];
                auto const width = m.size();
                switch (in.byte() % 26U) {
                        case 0: {
                                check.step("set(pos, val), reset(pos), flip(pos)");
                                if (width != 0UZ) {
                                        auto const pos = in.below(width);
                                        switch (in.byte() % 3U) {
                                                case 0: {
                                                        auto const val = in.boolean();
                                                        x.set(pos, val);
                                                        m.set(pos, val);
                                                        break;
                                                }
                                                case 1: {
                                                        x.reset(pos);
                                                        m.reset(pos);
                                                        break;
                                                }
                                                default: {
                                                        x.flip(pos);
                                                        m.flip(pos);
                                                        break;
                                                }
                                        }
                                }
                                break;
                        }
                        case 1: {
                                check.step("operator[], reference");
                                if (width != 0UZ) {
                                        auto const pos = in.below(width);
                                        if (in.boolean()) {
                                                x[pos].flip();
                                                m[pos].flip();
                                        } else {
                                                auto const val = in.boolean();
                                                x[pos] = val;
                                                m[pos] = val;
                                        }
                                }
                                break;
                        }
                        case 2: {
                                check.step("test, at");
                                auto const pos = in.below(width + 70UZ);
                                if (pos < width) {
                                        check.expect(x.test(pos) == m.test(pos), "test");
                                        check.expect(static_cast<X const&>(x).at(pos) == m.test(pos), "at");
                                } else {
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { static_cast<void>(static_cast<X const&>(x).at(pos)); }), "at out_of_range");
                                }
                                break;
                        }
                        case 3: {
                                check.step("set, reset, flip past the width");
                                if constexpr (not growing<X>) {
                                        auto const pos = width + in.below(70UZ);
                                        auto const before = x;
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.set(pos); }), "set out_of_range");
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.reset(pos); }), "reset out_of_range");
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.flip(pos); }), "flip out_of_range");
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { static_cast<void>(x.test(pos)); }), "test out_of_range");
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { m.set(pos); }), "the model's set out_of_range");
                                        check.expect(x == before, "unchanged by the refused access");
                                }
                                break;
                        }
                        case 4: {
                                check.step("test_set");
                                if (width != 0UZ) {
                                        auto const pos = in.below(width);
                                        auto const val = in.boolean();
                                        check.expect(x.test_set(pos, val) == m.test(pos), "previous bit");
                                        m.set(pos, val);
                                }
                                break;
                        }
                        case 5: {
                                check.step("set(), reset(), flip()");
                                switch (in.byte() % 3U) {
                                        case 0: {
                                                x.set();
                                                m.set();
                                                break;
                                        }
                                        case 1: {
                                                x.reset();
                                                m.reset();
                                                break;
                                        }
                                        default: {
                                                x.flip();
                                                m.flip();
                                                break;
                                        }
                                }
                                break;
                        }
                        case 6: {
                                check.step("set(pos, len, val), reset(pos, len), flip(pos, len)");
                                auto const pos = in.below(width + 1UZ);
                                auto const len = in.below(width - pos + 1UZ);
                                auto const kind = in.byte() % 3U;
                                auto const val = in.boolean();
                                switch (kind) {
                                        case 0: {
                                                x.set(pos, len, val);
                                                break;
                                        }
                                        case 1: {
                                                x.reset(pos, len);
                                                break;
                                        }
                                        default: {
                                                x.flip(pos, len);
                                                break;
                                        }
                                }
                                for (auto const k : std::views::iota(pos, pos + len)) {
                                        m.set(k, kind == 0U ? val : (kind == 1U ? false : not m.test(k)));
                                }
                                break;
                        }
                        case 7: {
                                check.step("set(pos, len, val) past the width");
                                if constexpr (not growing<X>) {
                                        auto const pos = in.below(width + 2UZ);
                                        auto const len = width - (pos < width ? pos : width) + 1UZ + in.below(70UZ);
                                        auto const before = x;
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.set(pos, len, true); }), "ranged set out_of_range");
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.flip(pos, len); }), "ranged flip out_of_range");
                                        check.expect(x == before, "unchanged by the refused range");
                                }
                                break;
                        }
                        case 8:
                        case 9:
                        case 10:
                        case 11: {
                                check.step("&=, |=, ^=, -=");
                                auto z = y;
                                auto mz = my;
                                align(z, mz, width);
                                switch (in.byte() % 4U) {
                                        case 0: {
                                                x &= z;
                                                m &= mz;
                                                break;
                                        }
                                        case 1: {
                                                x |= z;
                                                m |= mz;
                                                break;
                                        }
                                        case 2: {
                                                x ^= z;
                                                m ^= mz;
                                                break;
                                        }
                                        default: {
                                                x -= z;
                                                m &= ~mz;
                                                break;
                                        }
                                }
                                break;
                        }
                        case 12: {
                                check.step("<<=");
                                auto const n = shift_amount(in, width);
                                x <<= n;
                                m <<= n;
                                break;
                        }
                        case 13: {
                                check.step(">>=");
                                auto const n = shift_amount(in, width);
                                x >>= n;
                                m >>= n;
                                break;
                        }
                        case 14: {
                                check.step("<<, >>, ~, &, |, ^, -");
                                auto const n = shift_amount(in, width);
                                expect_equal(check, x << n, m << n);
                                expect_equal(check, x >> n, m >> n);
                                expect_equal(check, ~x, ~m);
                                auto z = y;
                                auto mz = my;
                                align(z, mz, width);
                                expect_equal(check, x & z, m & mz);
                                expect_equal(check, x | z, m | mz);
                                expect_equal(check, x ^ z, m ^ mz);
                                expect_equal(check, x - z, m & ~mz);
                                break;
                        }
                        case 15: {
                                check.step("find_first, find_next, find_last, find_prev");
                                auto const pos = in.below(width + 3UZ);
                                check.expect(x.find_first() == scan_next(m, 0UZ), "find_first");
                                check.expect(x.find_next(pos) == scan_next(m, pos + 1UZ), "find_next");
                                check.expect(x.find_last() == scan_prev(m, width), "find_last");
                                check.expect(x.find_prev(pos) == scan_prev(m, pos), "find_prev");
                                break;
                        }
                        case 16: {
                                check.step("to_ulong, to_ullong, to_string");
                                check.expect(x.to_string() == string_of(m), "to_string");
                                auto const wide = scan_next(m, std::numeric_limits<unsigned long>::digits) != size_max;
                                if (wide) {
                                        check.expect(fuzz::throws<std::overflow_error>([&] -> void { static_cast<void>(x.to_ulong()); }), "to_ulong overflow_error");
                                        check.expect(fuzz::throws<std::overflow_error>([&] -> void { static_cast<void>(m.to_ulong()); }), "the model's to_ulong overflow_error");
                                } else {
                                        check.expect(x.to_ulong() == m.to_ulong(), "to_ulong");
                                        check.expect(x.to_ullong() == m.to_ulong(), "to_ullong");
                                }
                                break;
                        }
                        case 17: {
                                check.step("==, <=>, hash");
                                check.expect((x == y) == (m == my), "==");
                                auto const order = x <=> y;
                                auto const morder = three_way(m, my);
                                check.expect(std::is_eq(order) == std::is_eq(morder), "<=> equal");
                                check.expect(std::is_lt(order) == std::is_lt(morder), "<=> less");
                                if (m == my) {
                                        check.expect(std::hash<X>()(x) == std::hash<X>()(y), "equal bitsets hash alike");
                                }
                                break;
                        }
                        case 18: {
                                check.step("is_subset_of, is_proper_subset_of, intersects");
                                auto z = y;
                                auto mz = my;
                                align(z, mz, width);
                                auto const subset = (m & ~mz).none();
                                check.expect(x.is_subset_of(z) == subset, "is_subset_of");
                                check.expect(x.is_proper_subset_of(z) == (subset and m != mz), "is_proper_subset_of");
                                check.expect(x.intersects(z) == (m & mz).any(), "intersects");
                                check.expect(intersects(x, z) == (m & mz).any(), "intersects, the free function");
                                break;
                        }
                        case 19: {
                                check.step("swap");
                                if (in.boolean()) {
                                        xs[0].swap(xs[1]);
                                } else {
                                        swap(xs[0], xs[1]);
                                }
                                std::swap(ms[0], ms[1]);
                                break;
                        }
                        case 20: {
                                check.step("copy, move");
                                if (in.boolean()) {
                                        x = y;
                                } else {
                                        auto tmp = y;
                                        x = std::move(tmp);
                                }
                                m = my;
                                expect_equal(check, X(x), m);
                                break;
                        }
                        case 21: {
                                check.step("resize");
                                if constexpr (growing<X>) {
                                        auto const n = in.below(top + 1UZ);
                                        auto const val = in.boolean();
                                        x.resize(n, val);
                                        m.resize(n, val);
                                        if (capped<X>()) {
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.resize(top + 1UZ + in.below(70UZ), val); });
                                        }
                                }
                                break;
                        }
                        case 22: {
                                check.step("push_back, pop_back");
                                if constexpr (growing<X>) {
                                        if (in.boolean()) {
                                                if (width < top) {
                                                        auto const val = in.boolean();
                                                        x.push_back(val);
                                                        m.push_back(val);
                                                } else if (capped<X>()) {
                                                        expect_unchanged_on_refusal(check, x, m, [](X& z) -> void { z.push_back(true); });
                                                }
                                        } else if (width != 0UZ) {
                                                x.pop_back();
                                                m.pop_back();
                                        }
                                }
                                break;
                        }
                        case 23: {
                                check.step("clear");
                                if constexpr (growing<X>) {
                                        x.clear();
                                        m.clear();
                                        check.expect(x.empty(), "empty");
                                }
                                break;
                        }
                        case 24: {
                                check.step("append(block)");
                                if constexpr (growing<X>) {
                                        using block_type = X::block_type;
                                        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
                                        auto block = block_type{};
                                        for (auto const k : std::views::iota(0UZ, sizeof(block_type))) {
                                                block |= static_cast<block_type>(static_cast<block_type>(in.byte()) << (k * 8UZ));
                                        }
                                        if (width + digits <= top) {
                                                x.append(block);
                                                for (auto const k : std::views::iota(0UZ, digits)) {
                                                        m.push_back(((block >> k) & 1U) != 0U);
                                                }
                                        } else if (capped<X>()) {
                                                expect_unchanged_on_refusal(check, x, m, [&](X& z) -> void { z.append(block); });
                                        }
                                }
                                break;
                        }
                        case 25: {
                                check.step("construct from a width and a value");
                                auto const val = static_cast<unsigned long long>(in.below(1UZ << 16U)) * 0x0001'0001'0001'0001ULL;
                                if constexpr (growing<X>) {
                                        auto const n = in.below(top + 1UZ);
                                        x = X(n, val);
                                        m = M(n, static_cast<unsigned long>(val));
                                } else {
                                        x = X(val);
                                        m = M(val);
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
