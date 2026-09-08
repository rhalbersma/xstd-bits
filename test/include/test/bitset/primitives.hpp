//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BITSET_PRIMITIVES_HPP
#define TEST_BITSET_PRIMITIVES_HPP

#include <boost/test/unit_test.hpp>      // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_NE, BOOST_CHECK_THROW
#include <test/dynamic.hpp>              // dynamic
#include <xstd/bits/ownership.hpp>       // owned_storage
#include <xstd/bits/bit_set_view.hpp> // view
#include <cstddef>                       // size_t
#include <functional>                    // hash
#include <memory>                        // addressof
#include <set>                           // set
#include <sstream>                       // istringstream, stringstream
#include <stdexcept>                     // invalid_argument, out_of_range
#include <string>                        // string
#include <string_view>                   // string_view

namespace test::bitset {

// Nine primitives below NOLINT bugprone-exception-escape: the check reads the callee, not the guard. [design.md#exception-escape-nolints]

// These checks are on xstd::bitset's basic_string_view overload: std::bitset has none, and dynamic_bitset answers to its own contract.
template<class X>
concept fixed_string_view_constructible = requires { X(std::string_view()); } and not dynamic<X>;

// The wrapper at a run-time width is one of ours and answers as boost does; boost itself, which has the overload from 1.87, asserts where the wrapper throws.
template<class X>
concept dynamic_string_view_constructible = requires { X(std::string_view()); typename xstd::owned_storage<X>::bits_type; } and dynamic<X>;

// One function per tier, because the tiers are what this checks and a BOOST_CHECK_THROW is three branches to
// the complexity check: inline, the three of them nested under two if constexprs came to 64 against a threshold
// of 25, and none of that 64 was the logic. [design.md#one-function-per-tier]

// A width the text must fit: too long throws, whatever the text says.
template<class X>
auto check_string_view_at_a_static_width() -> void  // NOLINT(bugprone-exception-escape)
{
        constexpr auto N = X().size();
        auto const zeros = std::string(N, '0');
        BOOST_CHECK_THROW(                                                      // [bitset.cons]/3
                (static_cast<void>(X(std::string_view(zeros), N + 1))), std::out_of_range
        );
}

// The two a zero width has no room to state: every position set, and a character that is neither 0 nor 1.
template<class X>
auto check_string_view_at_a_nonzero_width() -> void  // NOLINT(bugprone-exception-escape)
{
        constexpr auto N = X().size();
        auto const ones = std::string(N, '1');
        BOOST_CHECK(X(std::string_view(ones)).all());                           // [bitset.cons]/4

        auto invalid = std::string(N, '0');
        invalid[N - 1] = '2';
        BOOST_CHECK_THROW(                                                      // [bitset.cons]/5
                (static_cast<void>(X(std::string_view(invalid)))), std::invalid_argument
        );
}

// A run-time width is boost's contract: the text read is the width, and the two throws are as at a static width.
template<class X>
auto check_string_view_at_a_run_time_width() -> void  // NOLINT(bugprone-exception-escape)
{
        BOOST_CHECK_EQUAL(X(std::string_view("0101")).size(), 4UZ);
        BOOST_CHECK(X(std::string_view("11")).all());
        BOOST_CHECK_THROW((static_cast<void>(X(std::string_view("01"), 3))),  std::out_of_range);
        BOOST_CHECK_THROW((static_cast<void>(X(std::string_view("012")))),   std::invalid_argument);
}

template<class X>
struct constructor
{
        auto operator()() const noexcept  // NOLINT(bugprone-exception-escape)
        {
                X a;
                BOOST_CHECK(a.none());                                          // [bitset.cons]/1

                // [bitset.cons]/2 describes the constructor taking unsigned long long
                if constexpr (fixed_string_view_constructible<X>) {
                        check_string_view_at_a_static_width<X>();
                        if constexpr (X().size() > 0) {
                                check_string_view_at_a_nonzero_width<X>();
                        }
                } else if constexpr (dynamic_string_view_constructible<X>) {
                        check_string_view_at_a_run_time_width<X>();
                }
        }
};

struct mem_bit_and_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self &= rhs;
                for (auto const N = self.size(); auto const i : std::views::iota(0UZ, N)) {
                        BOOST_CHECK_EQUAL(dst[i], not rhs[i] ? false : src[i]); // [bitset.members]/1
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/2
        }
};

struct mem_bit_or_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self |= rhs;
                for (auto const N = self.size(); auto const i : std::views::iota(0UZ, N)) {
                        BOOST_CHECK_EQUAL(dst[i], rhs[i] ? true : src[i]);      // [bitset.members]/3
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/4
        }
};

struct mem_bit_xor_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                auto const src = self;
                auto const& dst = self ^= rhs;
                for (auto const N = self.size(); auto const i : std::views::iota(0UZ, N)) {
                        BOOST_CHECK_EQUAL(dst[i], rhs[i] ? not src[i] : src[i]);// [bitset.members]/5
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/6
        }
};

// Set vocabulary, so a bitset at a static width has none of it, as std::bitset has none: guarded, like the three predicates below.
struct mem_bit_minus_assign
{
        template<class X>
        auto operator()(X& self, const X& rhs) const noexcept
        {
                if constexpr (requires { self -= rhs; }) {
                        auto const src = self;
                        auto const& dst = self -= rhs;
                        for (auto const N = self.size(); auto const i : std::views::iota(0UZ, N)) {
                                BOOST_CHECK_EQUAL(dst[i], rhs[i] ? false : src[i]);
                        }
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));
                }
        }
};

struct mem_shift_left_assign
{
        auto operator()(auto& self, std::size_t pos) const noexcept
        {
                auto const src = self;
                auto const& dst = self <<= pos;
                for (auto const N = self.size(); auto const I : std::views::iota(0UZ, N)) {
                        if (I < pos) {
                                BOOST_CHECK(not dst[I]);                        // [bitset.members]/7.1
                        } else {
                                BOOST_CHECK_EQUAL(dst[I], src[I - pos]);        // [bitset.members]/7.2
                        }
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/8
        }
};

struct mem_shift_right_assign
{
        auto operator()(auto& self, std::size_t pos) const noexcept
        {
                auto const src = self;
                auto const& dst = self >>= pos;
                for (auto const N = self.size(); auto const I : std::views::iota(0UZ, N)) {
                        if (pos >= N - I) {
                                BOOST_CHECK(not dst[I]);                        // [bitset.members]/9.1
                        } else {
                                BOOST_CHECK_EQUAL(dst[I], src[I + pos]);        // [bitset.members]/9.2
                        }
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/10
        }
};

struct mem_shift_left
{
        template<class X>
        auto operator()(const X& self, std::size_t pos) const noexcept
        {
                BOOST_CHECK_EQUAL(self << pos, X(self) <<= pos);                // [bitset.members]/11
        }
};

struct mem_shift_right
{
        template<class X>
        auto operator()(const X& self, std::size_t pos) const noexcept
        {
                BOOST_CHECK_EQUAL(self >> pos, X(self) >>= pos);                // [bitset.members]/12
        }
};

struct mem_set
{
        auto operator()(auto& self) const noexcept
        {
                auto const& dst = self.set();
                BOOST_CHECK(self.all());                                                // [bitset.members]/13
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));           // [bitset.members]/14
        }

        auto operator()(auto& self, std::size_t pos, bool val = true) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.set(pos, val);
                        for (auto const i : std::views::iota(0UZ, N)) {
                                BOOST_CHECK_EQUAL(dst[i], i == pos ? val : src[i]);     // [bitset.members]/15
                        }
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/16
                } else {
                        BOOST_CHECK_THROW(self.set(pos, val), std::out_of_range);       // [bitset.members]/17
                }
        }
};

struct mem_reset
{
        auto operator()(auto& self) const noexcept
        {
                auto const& dst = self.reset();
                BOOST_CHECK(self.none());                                               // [bitset.members]/18
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));           // [bitset.members]/19
        }

        auto operator()(auto& self, std::size_t pos) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.reset(pos);
                        for (auto const i : std::views::iota(0UZ, N)) {
                                BOOST_CHECK_EQUAL(dst[i], i == pos ? false : src[i]);   // [bitset.members]/20
                        }
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));   // [bitset.members]/21
                } else {
                        BOOST_CHECK_THROW(self.reset(pos), std::out_of_range);          // [bitset.members]/22
                }
        }
};

struct mem_bit_not
{
        template<class X>
        auto operator()(const X& self) const noexcept
        {
                auto x = X(self);                                                       // [bitset.members]/23
                BOOST_CHECK_EQUAL(~self, x.flip());                                     // [bitset.members]/24
        }
};

struct mem_flip
{
        auto operator()(auto& self) const noexcept
        {
                auto const src = self;
                auto const& dst = self.flip();
                for (auto const N = self.size(); auto const i : std::views::iota(0UZ, N)) {
                        BOOST_CHECK_NE(dst[i], src[i]);                                 // [bitset.members]/25
                }
                BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));           // [bitset.members]/26
        }

        auto operator()(auto& self, std::size_t pos) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        auto const src = self;
                        auto const& dst = self.flip(pos);
                        for (auto const i : std::views::iota(0UZ, N)) {
                                BOOST_CHECK_EQUAL(dst[i], i == pos ? not src[i] : src[i]);      // [bitset.members]/27
                        }
                        BOOST_CHECK_EQUAL(std::addressof(dst), std::addressof(self));           // [bitset.members]/28
                } else {
                        BOOST_CHECK_THROW(self.flip(pos), std::out_of_range);                   // [bitset.members]/29
                }
        }
};

struct mem_at
{
        auto operator()(const auto& self, std::size_t pos) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                auto const N = self.size();
                BOOST_CHECK(pos < N);                                                   // [bitset.members]/30
                BOOST_CHECK_EQUAL(self[pos], self.test(pos));                           // [bitset.members]/31
                BOOST_CHECK_NO_THROW(static_cast<void>(self[pos]));                     // [bitset.members]/32
        }

        // auto operator()(auto& self, std::size_t pos) const
        // {
        //         BOOST_CHECK(pos < bs.size());                                       // [bitset.members]/47
        //         BOOST_CHECK_EQUAL(at(bs, pos), test(bs, pos));                  // [bitset.members]/48
        //         //BOOST_CHECK_NO_THROW(at(bs, pos));                              // [bitset.members]/49
        // }

        // auto operator()(auto& self, std::size_t pos, bool val) const
        // {
        //         BOOST_CHECK(pos < fn_size(bs));                                 // [bitset.members]/47
        //         auto src = bs; set(src, pos, val);
        //         at(bs, pos, val);
        //         BOOST_CHECK_EQUAL(bs, src);                                     // [bitset.members]/48
        //         //BOOST_CHECK_NO_THROW(at(bs, pos, val));                         // [bitset.members]/49
        // }
};


// [bitset.members]/28-33 describe conversion functions to_ulong, to_ullong, to_string

struct mem_count
{
        auto operator()(const auto& self) const noexcept
        {
                auto const N = self.size();
                BOOST_CHECK_EQUAL(
                        self.count(),
                        std::ranges::fold_left(
                                std::views::iota(0UZ, N) | std::views::transform([&](auto i) {
                                        return self[i];
                                }), 0UZ, std::plus<>()
                        )
                );                                                              // [bitset.members]/43
        }
};

struct mem_size
{
        template<class X>
        auto operator()(const X& self) const noexcept
        {
                if constexpr (not dynamic<X>) {
                        BOOST_CHECK_EQUAL(self.size(), X().size());             // [bitset.members]/44
                }
        }
};

struct mem_equal_to
{
        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                auto const N = self.size();
                BOOST_CHECK_EQUAL(
                        self == rhs,
                        std::ranges::all_of(std::views::iota(0UZ, N), [&](auto i) {
                                return self[i] == rhs[i];
                        })
                );                                                              // [bitset.members]/45
                auto const lhs_view = xstd::bit_set_view(self);
                auto const rhs_view = xstd::bit_set_view(rhs);
#ifdef _MSC_VER
                BOOST_CHECK_EQUAL(
                        self == rhs,
                        std::ranges::equal(
                                lhs_view.begin(), lhs_view.end(),
                                rhs_view.begin(), rhs_view.end()
                        )
                );
#else
                // range version not working with Visual C++
                BOOST_CHECK_EQUAL(self == rhs, std::ranges::equal(lhs_view, rhs_view));
#endif
        }
};

// The bit string, most significant position first: the member where the type has one, boost's free function otherwise.
template<class X>
[[nodiscard]] auto bit_string(const X& x)
{
        if constexpr (requires { x.to_string(); }) {
                return x.to_string();
        } else {
                auto s = std::string();
                to_string(x, s);
                return s;
        }
}

// Two orderings. The set view's is std::set's over ascending positions, re-derived from each type's own iteration; the type's own, where it has one, is the bit string's, which is boost's operator<. [design.md#the-ordering-invariant]
struct mem_compare_three_way
{
        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                auto const lhs_view = xstd::bit_set_view(self);
                auto const rhs_view = xstd::bit_set_view(rhs);
                BOOST_CHECK(
                        (lhs_view <=> rhs_view) ==
                        std::lexicographical_compare_three_way(
                                lhs_view.begin(), lhs_view.end(),
                                rhs_view.begin(), rhs_view.end()
                        )
                );
                if constexpr (requires { self <=> rhs; }) {
                        BOOST_CHECK((self <=> rhs) == (bit_string(self) <=> bit_string(rhs)));
                } else if constexpr (requires { self < rhs; }) {
                        BOOST_CHECK_EQUAL(self < rhs, bit_string(self) < bit_string(rhs));
                }
        }
};

struct mem_test
{
        auto operator()(const auto& self, std::size_t pos) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                if (auto const N = self.size(); pos < N) {
                        BOOST_CHECK_EQUAL(self.test(pos), self[pos]);                                   // [bitset.members]/46
                } else {
                        BOOST_CHECK_THROW(static_cast<void>(self.test(pos)), std::out_of_range);        // [bitset.members]/47
                }
        }
};

struct mem_all
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.all(), self.count() == self.size());     // [bitset.members]/48
        }
};

struct mem_any
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.any(), self.count() != 0);               // [bitset.members]/49
        }
};

struct mem_none
{
        auto operator()(const auto& self) const noexcept
        {
                BOOST_CHECK_EQUAL(self.none(), self.count() == 0);              // [bitset.members]/50
        }
};

struct mem_is_subset_of
{
        template<class X>
        [[nodiscard]] static auto fn_is_subset_of(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.is_subset_of(rhs); }) {
                        return lhs.is_subset_of(rhs);
                } else {
                        return xstd::bit_set_view(lhs).is_subset_of(xstd::bit_set_view(rhs));
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_is_subset_of(self, rhs), (self & ~rhs).none());
        }
};

struct mem_is_proper_subset_of
{
        template<class X>
        [[nodiscard]] static auto fn_is_proper_subset_of(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.is_proper_subset_of(rhs); }) {
                        return lhs.is_proper_subset_of(rhs);
                } else {
                        return xstd::bit_set_view(lhs).is_proper_subset_of(xstd::bit_set_view(rhs));
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_is_proper_subset_of(self, rhs), mem_is_subset_of::fn_is_subset_of(self, rhs) and self != rhs);
        }
};

// The four edges is_proper_subset_of's multi-block paths need, which singleton and doubleton pairs never reach.
struct mem_is_proper_subset_of_edges
{
        template<class X>
        auto operator()(X& a, X&) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                auto const N = a.size();
                if (N == 0) {
                        return;
                }
                auto const lo = 0UZ;
                auto const hi = N - 1;
                auto const one = [&](std::size_t i)                -> X { auto x = a; x.set(i);           return x; };
                auto const two = [&](std::size_t i, std::size_t j) -> X { auto x = a; x.set(i); x.set(j); return x; };

                auto const check = mem_is_proper_subset_of();
                check(one(lo), one(lo));                // equal: every block compares the same
                check(one(lo), two(lo, hi));            // proper subset, differing in the last block
                check(one(hi), one(lo));                // not a subset, differing in the first block
                check(two(lo, hi), one(lo));            // a subset up to the last block, then not
        }
};

struct mem_intersects
{
        template<class X>
        [[nodiscard]] static auto fn_intersects(const X& lhs, const X& rhs) noexcept
        {
                if constexpr (requires { lhs.intersects(rhs); }) {
                        return lhs.intersects(rhs);
                } else {
                        return xstd::bit_set_view(lhs).intersects(xstd::bit_set_view(rhs));
                }
        }

        template<class X>
        auto operator()(const X& self, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(fn_intersects(self, rhs), (self & rhs).any());
        }
};

// [bitset.hash]/1 stipulates a std::hash<std::bitset<N>> specialization; equal values hash equal wherever one exists, and boost has none. [design.md#the-hashing-invariant]
struct op_hash
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                if constexpr (requires { std::hash<X>()(lhs); }) {
                        BOOST_CHECK_EQUAL(std::hash<X>()(lhs), std::hash<X>()(X(lhs)));
                        BOOST_CHECK(lhs != rhs or std::hash<X>()(lhs) == std::hash<X>()(rhs));
                }
        }
};

struct op_bit_and
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs & rhs, X(lhs) &= rhs);                    // [bitset.operators]/1
        }
};

struct op_bit_or
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs | rhs, X(lhs) |= rhs);                    // [bitset.operators]/2
        }
};

struct op_bit_xor
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                BOOST_CHECK_EQUAL(lhs ^ rhs, X(lhs) ^= rhs);                    // [bitset.operators]/3
        }
};

struct op_bit_minus
{
        template<class X>
        auto operator()(const X& lhs, const X& rhs) const noexcept
        {
                if constexpr (requires { lhs - rhs; }) {
                        auto nrv = lhs;
                        BOOST_CHECK_EQUAL(lhs - rhs, nrv -= rhs);
                }
        }
};

struct op_iostream
{
        template<class X>
        auto operator()(const X& x) const noexcept  // NOLINT(bugprone-exception-escape)
        {
                std::stringstream sstr;
                X y;
                sstr << x;
                sstr >> y;
                BOOST_CHECK_EQUAL(x, y);                                        // [bitset.operators]/4-8
        }
};

// A first character that is neither zero nor one stores nothing, exactly as an empty stream does: failbit unless N is zero.
template<class X>
struct op_istream_failure
{
        auto operator()() const noexcept  // NOLINT(bugprone-exception-escape)
        {
                if constexpr (fixed_string_view_constructible<X>) {
                        constexpr auto N = X().size();
                        for (auto const* input : { "", "2" }) {
                                auto is = std::istringstream(input);
                                auto x = X();
                                is >> x;
                                BOOST_CHECK(x.none());
                                BOOST_CHECK_EQUAL(is.fail(), N > 0);            // [bitset.operators]/6
                        }

                        // Fewer digits than N: the loop stops on eof rather than on N, and x = X(str) puts what was read in the low bits.
                        if constexpr (N > 1) {
                                auto is = std::istringstream("1");
                                auto x = X();
                                is >> x;
                                BOOST_CHECK(not is.fail());
                                BOOST_CHECK_EQUAL(x.count(), 1UZ);
                                BOOST_CHECK(x.test(0));                         // [bitset.operators]/6
                        }
                }
        }
};

} // namespace test::bitset

#endif // TEST_BITSET_PRIMITIVES_HPP
