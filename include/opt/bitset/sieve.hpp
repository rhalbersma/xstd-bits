#ifndef OPT_BITSET_SIEVE_HPP
#define OPT_BITSET_SIEVE_HPP

//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/ranges/set_view.hpp>           // set_view
#include <cstddef>                                 // size_t
#include <ranges>                                  // take_while

namespace xstd {

template<class X>
concept legacy_bitset = requires(X& a, std::size_t pos)
{
        a.set();
        a.reset(pos);
};

template<class X>
concept modern_bitset = requires(X& a, std::size_t pos)
{
        a.fill();
        a.erase(pos);
};

// A static width is its own; a run-time one, boost's or ours, is resized to the count.
template<class X>
struct generate_empty
{
        auto operator()(std::size_t n) const
        {
                auto x = X();
                if constexpr (requires { x.resize(n); }) {
                        x.resize(n);
                }
                return x;
        }
};

template<class X>
auto fill(X& empty)
{
        if constexpr (legacy_bitset<X>) {
                empty.set();
        } else if constexpr (modern_bitset<X>) {
                empty.fill();
        } else {
                static_assert(false);
        }
}

template<class X>
auto sift(X& primes, std::size_t m)
{
        if constexpr (legacy_bitset<X>) {
                primes.reset(m);
        } else if constexpr (modern_bitset<X>) {
                primes.erase(m);
        } else {
                static_assert(false);
        }
}

template<class X>
struct generate_candidates
{
        auto operator()(auto n) const
        {
                auto candidates = generate_empty<X>()(n);
                fill(candidates);
                sift(candidates, 0);
                sift(candidates, 1);
                return candidates;
        }
};

template<class X>
auto sift_primes0(std::size_t n)
{
        auto primes = generate_candidates<X>()(n);
        for (std::size_t p
                : xstd::set_view(primes)
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
        auto primes = generate_candidates<X>()(n);
        for (std::size_t p : xstd::set_view(primes)) {
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
