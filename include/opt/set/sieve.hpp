#ifndef OPT_SET_SIEVE_HPP
#define OPT_SET_SIEVE_HPP

//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef> // size_t
#include <ranges>  // to
                   // begin, end, iota, range_value_t, take_while

namespace xstd {

template<class X>
auto sift(X& primes, std::size_t m)
{
        primes.erase(m);
}

// The sieve over any ordered set of integers, std::set's, std::flat_set's or ours. [design.md#the-sieve]
template<class X>
auto generate_candidates(std::size_t n)
{
        return std::views::iota(2UZ, n) | std::ranges::to<X>();
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

}       // namespace xstd

#endif  // include guard
