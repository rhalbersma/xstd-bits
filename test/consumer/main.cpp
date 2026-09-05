//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_static_set.hpp>

int main()
{
        constexpr auto N = 100;
        auto s = xstd::bit_static_set<N>();
        s.insert(1);
        s.insert(2);
        s.insert(3);
        return s.size() == 3 ? 0 : 1;
}
