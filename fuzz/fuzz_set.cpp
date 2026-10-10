//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <fuzz/decoder.hpp>                      // checker, decoder, run, throws
#include <xstd/bits/algorithm/bit_disjoint.hpp>  // bit_disjoint
#include <xstd/bits/algorithm/bit_includes.hpp>  // bit_includes
#include <xstd/bits/bit_bounded_set.hpp>         // basic_bit_bounded_set, bit_bounded_set
#include <xstd/bits/bit_fixed_set.hpp>           // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set.hpp>                 // basic_bit_set, bit_set
#include <xstd/bits/ext/boost/bit_small_set.hpp> // basic_bit_small_set
#include <algorithm>                             // includes, set_difference, set_intersection, set_symmetric_difference, set_union
#include <array>                                 // array
#include <compare>                               // is_eq, is_lt
#include <cstddef>                               // size_t
#include <cstdint>                               // uint8_t, uint64_t
#include <functional>                            // hash
#include <iterator>                              // inserter
#include <limits>                                // numeric_limits
#include <new>                                   // bad_alloc
#include <ranges>                                // iota, reverse
#include <set>                                   // erase_if, set
#include <stdexcept>                             // length_error, out_of_range
#include <tuple>                                 // tuple
#include <type_traits>                           // type_identity
#include <utility>                               // move, swap
#include <vector>                                // vector

namespace {

// The set reading's owners at each kind of width: dynamic, small, fixed across block boundaries, and bounded.
using owners = std::tuple<
        xstd::bit_set,
        xstd::basic_bit_set<std::size_t, std::uint8_t>,
        xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64>,
        xstd::bit_fixed_set<1>,
        xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 17>,
        xstd::bit_fixed_set<64>,
        xstd::bit_fixed_set<65>,
        xstd::bit_fixed_set<200>,
        xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 17>,
        xstd::bit_bounded_set<130>>;

using model = std::set<std::size_t>;

// A whole-set complement needs a universe, which only a width in the type gives.
template<class X>
concept static_width = requires (X const& x) { ~x; };

// The keys a dynamic set is fed: past a few blocks, which is where its growth is exercised.
inline constexpr auto dynamic_keys = 320UZ;

inline constexpr auto size_max = std::numeric_limits<std::size_t>::max();

// A dynamic set's left shift grows it, so the fuzzer stops translating before the keys outgrow a few pages.
inline constexpr auto dynamic_limit = 1UZ << 14U;

template<class X>
[[nodiscard]] auto universe()
        -> std::size_t
{
        auto const m = X().max_size();
        return m < dynamic_keys ? m : dynamic_keys;
}

// A ceiling the fuzzer can reach: a width or a capacity in the type, rather than the address space.
template<class X>
[[nodiscard]] auto bounded()
        -> bool
{
        return X().max_size() <= dynamic_keys;
}

// Every owner's left shift: translate by n and keep what lands below max_size(), without overflowing on the way.
[[nodiscard]] auto shifted_left(model const& m, std::size_t n, std::size_t top)
        -> model
{
        auto result = model();
        for (auto const k : m) {
                if (n < top and k < top - n) {
                        result.insert(k + n);
                }
        }
        return result;
}

// Mostly a translation within reach of the keys; else one at the top of size_t, or at the lowest key's room to spare.
[[nodiscard]] auto shift_amount(fuzz::decoder& in, model const& m, std::size_t keys, std::size_t top)
        -> std::size_t
{
        switch (in.byte() % 4U) {
                case 0: {
                        return size_max - in.below(70UZ);
                }
                case 1: {
                        // One short of the room keeps the lowest key at max_size() - 1; one more and nothing is kept.
                        auto const room  = m.empty() ? top : top - *m.begin();
                        auto const extra = in.below(3UZ);
                        return extra > size_max - (room - 1UZ) ? size_max : room - 1UZ + extra;
                }
                default: {
                        return in.below(keys + 70UZ);
                }
        }
}

// A shift the fuzzer can afford: no key of a dynamic set may land past a few pages, unless none lands at all.
template<class X>
[[nodiscard]] auto affordable(model const& m, std::size_t n, std::size_t top)
        -> bool
{
        return bounded<X>() or m.empty() or n >= top - *m.begin() or (*m.rbegin() < dynamic_limit and n < dynamic_limit - *m.rbegin());
}

[[nodiscard]] auto shifted_right(model const& m, std::size_t n)
        -> model
{
        auto result = model();
        for (auto const k : m) {
                if (k >= n) {
                        result.insert(k - n);
                }
        }
        return result;
}

[[nodiscard]] auto intersection(model const& a, model const& b)
        -> model
{
        auto result = model();
        std::ranges::set_intersection(a, b, std::inserter(result, result.end()));
        return result;
}

[[nodiscard]] auto union_of(model const& a, model const& b)
        -> model
{
        auto result = model();
        std::ranges::set_union(a, b, std::inserter(result, result.end()));
        return result;
}

[[nodiscard]] auto symmetric_difference(model const& a, model const& b)
        -> model
{
        auto result = model();
        std::ranges::set_symmetric_difference(a, b, std::inserter(result, result.end()));
        return result;
}

[[nodiscard]] auto difference(model const& a, model const& b)
        -> model
{
        auto result = model();
        std::ranges::set_difference(a, b, std::inserter(result, result.end()));
        return result;
}

template<class X>
auto expect_equal(fuzz::checker const& check, X const& x, model const& m)
        -> void
{
        check.expect(x.size() == m.size(), "size");
        check.expect(x.empty() == m.empty(), "empty");
        auto it = x.begin();
        for (auto const k : m) {
                check.expect(it != x.end(), "forward iteration ends early");
                check.expect(static_cast<std::size_t>(*it) == k, "forward iteration");
                ++it;
        }
        check.expect(it == x.end(), "forward iteration runs on");
        auto rit = x.rbegin();
        for (auto const k : m | std::views::reverse) {
                check.expect(rit != x.rend(), "reverse iteration ends early");
                check.expect(static_cast<std::size_t>(*rit) == k, "reverse iteration");
                ++rit;
        }
        check.expect(rit == x.rend(), "reverse iteration runs on");
        if (not m.empty()) {
                check.expect(static_cast<std::size_t>(x.front()) == *m.begin(), "front");
                check.expect(static_cast<std::size_t>(x.back()) == *m.rbegin(), "back");
        }
}

// The iterator the owner returned against the one the model did: both at the end, or both at the same key.
template<class X, class I>
auto expect_position(fuzz::checker const& check, X const& x, I it, model const& m, model::const_iterator mit, char const* what)
        -> void
{
        check.expect((it == x.end()) == (mit == m.end()), what);
        if (mit != m.end()) {
                check.expect(static_cast<std::size_t>(*it) == *mit, what);
        }
}

template<class X>
auto fuzz_one(fuzz::decoder& in)
        -> void
{
        auto const keys = universe<X>();
        auto const top  = X().max_size();
        auto check      = fuzz::checker();
        auto xs         = std::array<X, 2>();
        auto ms         = std::array<model, 2>();
        while (not in.empty()) {
                auto const i   = in.byte() % 2UZ;
                auto const j   = in.byte() % 2UZ;
                auto& x        = xs[i];
                auto& m        = ms[i];
                auto const& y  = xs[j];
                auto const& my = ms[j];
                switch (in.byte() % 23U) {
                        case 0: {
                                check.step("insert(k)");
                                auto const k              = in.below(keys);
                                auto const [it, inserted] = x.insert(k);
                                check.expect(inserted == m.insert(k).second, "inserted");
                                check.expect(static_cast<std::size_t>(*it) == k, "position");
                                break;
                        }
                        case 1: {
                                check.step("insert(hint, k)");
                                auto const k  = in.below(keys);
                                auto const it = x.insert(in.boolean() ? x.begin() : x.end(), k);
                                m.insert(k);
                                check.expect(static_cast<std::size_t>(*it) == k, "position");
                                break;
                        }
                        case 2: {
                                check.step("erase(k)");
                                auto const k = in.below(keys);
                                check.expect(x.erase(k) == m.erase(k), "count");
                                break;
                        }
                        case 3: {
                                check.step("erase(position)");
                                auto const k = in.below(keys);
                                if (auto const it = x.lower_bound(k); it != x.end()) {
                                        auto const next  = x.erase(it);
                                        auto const mnext = m.erase(m.lower_bound(k));
                                        expect_position(check, x, next, m, mnext, "successor");
                                }
                                break;
                        }
                        case 4: {
                                check.step("erase(first, last)");
                                auto lo = in.below(keys);
                                auto hi = in.below(keys);
                                if (hi < lo) {
                                        std::swap(lo, hi);
                                }
                                auto const last  = x.erase(x.lower_bound(lo), x.lower_bound(hi));
                                auto const mlast = m.erase(m.lower_bound(lo), m.lower_bound(hi));
                                expect_position(check, x, last, m, mlast, "last");
                                break;
                        }
                        case 5: {
                                check.step("find, contains, count");
                                auto const k = in.below(keys + 2UZ);
                                expect_position(check, x, x.find(k), m, m.find(k), "find");
                                check.expect(x.contains(k) == m.contains(k), "contains");
                                check.expect(x.count(k) == m.count(k), "count");
                                break;
                        }
                        case 6: {
                                check.step("lower_bound, upper_bound, equal_range");
                                auto const k = in.below(keys + 2UZ);
                                expect_position(check, x, x.lower_bound(k), m, m.lower_bound(k), "lower_bound");
                                expect_position(check, x, x.upper_bound(k), m, m.upper_bound(k), "upper_bound");
                                auto const [first, last]   = x.equal_range(k);
                                auto const [mfirst, mlast] = m.equal_range(k);
                                expect_position(check, x, first, m, mfirst, "equal_range.first");
                                expect_position(check, x, last, m, mlast, "equal_range.second");
                                break;
                        }
                        case 7: {
                                check.step("toggle(k)");
                                auto const k = in.below(keys);
                                if (x.contains(k)) {
                                        static_cast<void>(x.erase(k));
                                } else {
                                        static_cast<void>(x.insert(k));
                                }
                                if (m.erase(k) == 0UZ) {
                                        m.insert(k);
                                }
                                break;
                        }
                        case 8: {
                                check.step("&=");
                                x &= y;
                                m = intersection(m, my);
                                break;
                        }
                        case 9: {
                                check.step("|=");
                                x |= y;
                                m = union_of(m, my);
                                break;
                        }
                        case 10: {
                                check.step("^=");
                                x ^= y;
                                m = symmetric_difference(m, my);
                                break;
                        }
                        case 11: {
                                check.step("-=");
                                x -= y;
                                m = difference(m, my);
                                break;
                        }
                        case 12: {
                                check.step("&, |, ^, -");
                                auto const& z  = xs[1UZ - j];
                                auto const& mz = ms[1UZ - j];
                                expect_equal(check, y & z, intersection(my, mz));
                                expect_equal(check, y | z, union_of(my, mz));
                                expect_equal(check, y ^ z, symmetric_difference(my, mz));
                                expect_equal(check, y - z, difference(my, mz));
                                break;
                        }
                        case 13: {
                                check.step("<<=");
                                auto const n = shift_amount(in, m, keys, top);
                                if (affordable<X>(m, n, top)) {
                                        x <<= n;
                                        m = shifted_left(m, n, top);
                                }
                                break;
                        }
                        case 14: {
                                check.step(">>=");
                                auto const n = shift_amount(in, m, keys, top);
                                x >>= n;
                                m = shifted_right(m, n);
                                break;
                        }
                        case 15: {
                                check.step("<<, >>");
                                auto const n = shift_amount(in, my, keys, top);
                                expect_equal(check, y >> n, shifted_right(my, n));
                                if (affordable<X>(my, n, top)) {
                                        expect_equal(check, y << n, shifted_left(my, n, top));
                                }
                                break;
                        }
                        case 16: {
                                check.step("swap");
                                if (in.boolean()) {
                                        xs[0].swap(xs[1]);
                                } else {
                                        swap(xs[0], xs[1]);
                                }
                                std::swap(ms[0], ms[1]);
                                break;
                        }
                        case 17: {
                                check.step("copy, move");
                                if (in.boolean()) {
                                        x = y;
                                        m = my;
                                } else {
                                        auto tmp = y;
                                        x        = std::move(tmp);
                                        m        = my;
                                }
                                auto const copied = X(x);
                                expect_equal(check, copied, m);
                                break;
                        }
                        case 18: {
                                check.step("==, <=>, is_subset_of, is_proper_subset_of, is_superset_of, is_proper_superset_of, intersects, hash");
                                check.expect((x == y) == (m == my), "==");
                                auto const order  = x <=> y;
                                auto const morder = m <=> my;
                                check.expect(std::is_eq(order) == std::is_eq(morder), "<=> equal");
                                check.expect(std::is_lt(order) == std::is_lt(morder), "<=> less");
                                auto const subset = std::ranges::includes(my, m);
                                check.expect(xstd::bit_includes(y, x) == subset, "bit_includes(y, x)");
                                auto const superset = std::ranges::includes(m, my);
                                check.expect(xstd::bit_includes(x, y) == superset, "bit_includes(x, y)");
                                check.expect(xstd::bit_disjoint(x, y) == intersection(m, my).empty(), "bit_disjoint");
                                if (m == my) {
                                        check.expect(std::hash<X>()(x) == std::hash<X>()(y), "equal sets hash alike");
                                }
                                break;
                        }
                        case 19: {
                                check.step("clear");
                                x.clear();
                                m.clear();
                                break;
                        }
                        case 20: {
                                check.step("insert_range");
                                auto keys_in = std::vector<std::size_t>();
                                for ([[maybe_unused]] auto const n : std::views::iota(0UZ, in.byte() % 8UZ)) {
                                        keys_in.push_back(in.below(keys));
                                }
                                if (in.boolean()) {
                                        x.insert_range(keys_in);
                                        m.insert(keys_in.begin(), keys_in.end());
                                } else {
                                        // [associative.reqmts] forbids a range into its own set.
                                        auto const other  = y;
                                        auto const mother = my;
                                        x.insert_range(other);
                                        m.insert(mother.begin(), mother.end());
                                }
                                break;
                        }
                        case 21: {
                                check.step("erase_if, fill, ~");
                                auto const r    = in.below(keys);
                                auto const d    = in.below(7UZ) + 1UZ;
                                auto const pred = [=](std::size_t k) -> bool { return k % d == r % d; };
                                check.expect(erase_if(x, pred) == std::erase_if(m, pred), "erase_if count");
                                if constexpr (static_width<X>) {
                                        auto const all = std::views::iota(0UZ, keys) | std::ranges::to<model>();
                                        if (in.boolean()) {
                                                x.fill();
                                                m = all;
                                        } else {
                                                x = ~x;
                                                m = difference(all, m);
                                        }
                                }
                                break;
                        }
                        case 22: {
                                check.step("insert past max_size()");
                                auto const before = x;
                                if constexpr (static_width<X>) {
                                        auto const k = top + in.below(70UZ);
                                        check.expect(fuzz::throws<std::out_of_range>([&] -> void { x.insert(k); }), "out_of_range past the width");
                                } else if (bounded<X>()) {
                                        auto const k = top + in.below(70UZ);
                                        check.expect(fuzz::throws<std::bad_alloc>([&] -> void { x.insert(k); }), "bad_alloc past the capacity");
                                } else {
                                        check.expect(fuzz::throws<std::length_error>([&] -> void { x.insert(size_max); }), "length_error past the address space");
                                }
                                check.expect(x == before, "unchanged by the failed insert");
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
