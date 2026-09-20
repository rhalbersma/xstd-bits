//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_DYNAMIC_HPP
#define TEST_DYNAMIC_HPP

#include <utility> // declval

namespace test {

template<class T>
concept dynamic = requires (T&& t) {
        t.resize(std::declval<typename T::size_type>());
        t.resize(std::declval<typename T::size_type>(), std::declval<bool>());
};

} // namespace test

#endif // TEST_DYNAMIC_HPP
