#ifndef OPT_SET_SIEVE_HPP
#define OPT_SET_SIEVE_HPP

// Copyright Rein Halbersma 2014-2026. Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <opt/set/detail/isqrt.hpp> // isqrt
#include <algorithm>                // max, min
#include <cstddef>                  // size_t
#include <map>                      // map
#include <ranges>                   // to
                                    // begin, end, iota, range_value_t, take_while

namespace opt {

template<class X>
auto sift(X& primes, std::size_t m)
{
        primes.erase(m);
}

// The sieve over any ordered set of integers. Total in n: the primes under nothing are none, not a broken contract.
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
        for (auto p : candidates | std::views::take_while([&](auto x) { return x * x < n; })) {
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
                        } while (m < n);
                } else {
                        break;
                }
        }
        return primes;
}

// The twin primes themselves, both members of each pair (OEIS A001097), not the lesser of each (A001359).
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

// The two sieves that need no bound up front, where the one above materializes every candidate first.

// The incremental sieve: one entry per prime found, O(pi(n)) space, generating forever (O'Neill, JFP 19(1), 2009).
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
                                // Nothing strikes it, so it is prime, and it starts striking at its square.
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

// The segmented sieve: base primes below sqrt(n) once, then one reusable window of compile-time width.
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
        auto const base_bound = std::ranges::min(detail::isqrt(n - 1UZ) + 1UZ, n);
        auto const base = sift_primes1<X>(base_bound);
        for (auto const p : base) {
                primes.insert(static_cast<std::ranges::range_value_t<X>>(p));
        }

        // Named in full rather than lo and hi, which the Windows headers declare at namespace scope (C4459).
        auto window = Window();
        for (auto segment_lo = base_bound; segment_lo < n; segment_lo += width) {
                auto const segment_hi = std::ranges::min(segment_lo + width, n);
                window.fill();
                for (auto const p : base) {
                        // The first multiple of p at or above segment_lo, never below p * p.
                        auto const first = std::ranges::max(p * p, ((segment_lo + p - 1UZ) / p) * p);
                        for (auto m = first; m < segment_hi; m += p) {
                                window.erase(m - segment_lo);
                        }
                }
                for (auto const offset : window) {
                        if (segment_lo + offset >= segment_hi) {
                                break; // the last window is short, and the tail above segment_hi was never a candidate
                        }
                        primes.insert(static_cast<std::ranges::range_value_t<X>>(segment_lo + offset));
                }
        }
        return primes;
}

} // namespace opt

#endif // include guard
