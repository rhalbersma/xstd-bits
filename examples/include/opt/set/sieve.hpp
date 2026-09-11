#ifndef OPT_SET_SIEVE_HPP
#define OPT_SET_SIEVE_HPP

//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm> // max, min
#include <cstddef>   // size_t
#include <map>       // map
#include <ranges>    // to
                     // begin, end, iota, range_value_t, take_while

namespace opt {

template<class X>
auto sift(X& primes, std::size_t m)
{
        primes.erase(m);
}

// The sieve over any ordered set of integers, std::set's, std::flat_set's or ours. [design.md#the-sieve]
// Total in n rather than asserting: iota(2, n) is a precondition violation below 2, and a sieve asked for the
// primes under nothing has an answer -- none -- rather than a contract to break.
template<class X>
auto generate_candidates(std::size_t n)
{
        return std::views::iota(2UZ, std::ranges::max(n, 2UZ)) | std::ranges::to<X>();
}

// Iterate a snapshot and guard with contains(): sift() erases, which invalidates a vector-backed X's cached end().
template<class X>
auto sift_primes0(std::size_t n)
{
        auto primes = generate_candidates<X>(n);
        auto const candidates = primes;
        for (auto p
                : candidates
                | std::views::take_while([&](auto x) { return x * x < n; })
        ) {
                if (not primes.contains(p)) {
                        continue;
                }
                for (auto m = p * p; m < n; m += p) {
                        sift(primes, m);
                }
        }
        return primes;
}

template<class X>
auto sift_primes1(std::size_t n)
{
        auto primes = generate_candidates<X>(n);
        auto const candidates = primes;
        for (auto p : candidates) {
                if (auto m = p * p; m < n) {
                        if (not primes.contains(p)) {
                                continue;
                        }
                        do {
                                sift(primes, m);
                                m += p;
                        } while(m < n);
                } else {
                        break;
                }
        }
        return primes;
}

// The twin primes themselves, both members of each pair -- {3, 5, 7, 11, 13, ...}, OEIS A001097 -- and not the
// lesser of each pair -- {3, 5, 11, 17, ...}, OEIS A001359. Both are called "the twin primes" in the wild, so the
// choice is named here rather than left to be read off the expected output of a test.
template<class X>
auto filter_twins(X const& primes)
{
        using key = std::ranges::range_value_t<X>;
        auto twins = X();
        auto first = std::ranges::begin(primes);
        auto const last = std::ranges::end(primes);
        if (first == last) {
                return twins;
        }
        auto prev = static_cast<key>(*first++);
        if (first == last) {
                return twins;
        }
        auto self = static_cast<key>(*first++);
        for (; first != last; ++first) {
                auto const next = static_cast<key>(*first);
                if (self - 2 == prev or self + 2 == next) {
                        twins.insert(self);
                }
                prev = self;
                self = next;
        }
        return twins;
}

// The two sieves that need no bound up front, where the one above materializes every candidate before it sifts a
// single one. [design.md#the-unbounded-sieves]

namespace detail::sieve {

// Newton on x * x - n, so the base bound is found in a few steps rather than a walk; the sieves below want it at
// run time. The step x <- (x + n / x) / 2 is exact in size_t: floor division keeps the sequence descending until it
// reaches floor(sqrt(n)), which is where y < x first fails.
//
// No n < 2 guard, because the loop is already total there and one would be a line no caller can reach: at n == 0 the
// seed y is 0 and the body never runs, so nothing divides by zero, and at n == 1 the seed equals x. For n >= 1 the
// iterate stays >= 1, so the division inside the loop is safe. Asserted at both ends in the tests rather than argued
// for here. [design.md#the-unbounded-sieves]
constexpr auto isqrt(std::size_t n) noexcept
        -> std::size_t
{
        auto x = n;
        auto y = (x + 1UZ) / 2UZ;
        while (y < x) {
                x = y;
                y = (x + (n / x)) / 2UZ;
        }
        return x;
}

}       // namespace detail::sieve

// The incremental sieve: no candidate array at all, and no n. It keeps one entry per prime found so far -- the next
// composite that prime will strike -- so the space is O(pi(n)) rather than O(n), and it generates forever.
// O'Neill, The Genuine Sieve of Eratosthenes, JFP 19(1), 2009. Slower per prime than the array sieve, which is the
// point of measuring it: what unboundedness costs. [design.md#the-unbounded-sieves]
class incremental_sieve
{
        // The composite each known prime will strike next, and which prime strikes it.
        std::map<std::size_t, std::size_t> m_strikes;
        std::size_t m_candidate = 1UZ;

public:
        [[nodiscard]] auto next()
                -> std::size_t
        {
                for (;;) {
                        ++m_candidate;
                        auto const it = m_strikes.find(m_candidate);
                        if (it == m_strikes.end()) {
                                // Nothing strikes it, so it is prime, and it starts striking at its square: every
                                // smaller multiple carries a smaller factor that is already striking.
                                m_strikes.emplace(m_candidate * m_candidate, m_candidate);
                                return m_candidate;
                        }
                        // Composite: move its striker forward to the next multiple no other striker has claimed.
                        auto const p = it->second;
                        m_strikes.erase(it);
                        auto m = m_candidate + p;
                        while (m_strikes.contains(m)) {
                                m += p;
                        }
                        m_strikes.emplace(m, p);
                }
        }
};

template<class X>
auto sift_primes_incremental(std::size_t n)
{
        auto primes = X();
        auto sieve = incremental_sieve();
        for (auto p = sieve.next(); p < n; p = sieve.next()) {
                primes.insert(p);
        }
        return primes;
}

// The segmented sieve: the base primes below sqrt(n) once, then one reusable window walked over the rest. Peak
// memory is O(sqrt(n) + W) whatever n is, and the window is a compile-time width that allocates nothing in the
// loop -- Window carries its own extent, so a bit_static_set<W> is the natural argument. [design.md#the-unbounded-sieves]
template<class X, class Window>
auto sift_primes_segmented(std::size_t n)
{
        constexpr auto width = Window().max_size();
        static_assert(width > 0UZ);

        auto primes = X();
        if (n <= 2UZ) {
                return primes;
        }

        // Base primes are those p with p * p < n, so every one of them is below isqrt(n - 1) + 1.
        auto const base_bound = std::ranges::min(detail::sieve::isqrt(n - 1UZ) + 1UZ, n);
        auto const base = sift_primes1<X>(base_bound);
        for (auto const p : base) {
                primes.insert(static_cast<std::ranges::range_value_t<X>>(p));
        }

        // Named in full rather than lo and hi, which the Windows headers declare at namespace scope: MSVC's C4459
        // reports the shadowing, and the benchmark leg treats it as an error.
        auto window = Window();
        for (auto segment_lo = base_bound; segment_lo < n; segment_lo += width) {
                auto const segment_hi = std::ranges::min(segment_lo + width, n);
                window.fill();
                for (auto const p : base) {
                        // The first multiple of p at or above segment_lo, never below p * p, which the base pass covered.
                        auto const first = std::ranges::max(p * p, ((segment_lo + p - 1UZ) / p) * p);
                        for (auto m = first; m < segment_hi; m += p) {
                                window.erase(m - segment_lo);
                        }
                }
                for (auto const offset : window) {
                        if (segment_lo + offset >= segment_hi) {
                                break;  // the last window is short, and the tail above segment_hi was never a candidate
                        }
                        primes.insert(static_cast<std::ranges::range_value_t<X>>(segment_lo + offset));
                }
        }
        return primes;
}

}       // namespace opt

#endif  // include guard
