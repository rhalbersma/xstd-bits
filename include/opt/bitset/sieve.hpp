#ifndef OPT_BITSET_SIEVE_HPP
#define OPT_BITSET_SIEVE_HPP

//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <cstddef>                    // size_t
#include <ranges>                     // take_while

namespace xstd {

// The sieve over any bitset, ours or another's, through the set reading: one vocabulary, whichever the bitset's own. [design.md#the-sieve]
// A static width is its own; a run-time one, boost's or ours, is resized to the count.
template<class X>
auto generate_candidates(std::size_t n)
{
        auto candidates = X();
        if constexpr (requires { candidates.resize(n); }) {
                candidates.resize(n);
        }
        auto const s = xstd::bit_set_view(candidates);
        s.fill();
        s.erase(0);
        s.erase(1);
        return candidates;
}

template<class X>
auto sift(X& primes, std::size_t m)
{
        xstd::bit_set_view(primes).erase(m);
}

template<class X>
auto sift_primes0(std::size_t n)
{
        auto primes = generate_candidates<X>(n);
        for (std::size_t p
                : xstd::bit_set_view(primes)
                | std::views::take_while([&](std::size_t x) { return x * x < n; })
        ) {
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
        for (std::size_t p : xstd::bit_set_view(primes)) {
                if (std::size_t m = p * p; m < n) {
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

template<class X>
auto filter_twins(X const& primes)
{
        return primes & (primes << 2 | primes >> 2);
}

}       // namespace xstd

#endif  // include guard
