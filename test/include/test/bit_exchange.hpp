//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BIT_EXCHANGE_HPP
#define TEST_BIT_EXCHANGE_HPP

#include <concepts> // same_as

namespace test {

// The byte exchange asked of the door, as a template: a non-dependent requirement hard-errors instead of failing.
template<class Reading, class B>
concept exchanges_from_bits = requires (B const& b) {
        { Reading::from_bits(b) } -> std::same_as<Reading>;
};

template<class Reading, class B>
concept exchanges_to_bits = requires (Reading const& r) {
        { r.template to_bits<B>() } -> std::same_as<B>;
};

// Both directions, which is what an owner of a static width answers. A view answers only the second.
template<class Reading, class B>
concept exchanges_bits = exchanges_from_bits<Reading, B> and exchanges_to_bits<Reading, B>;

} // namespace test

#endif // TEST_BIT_EXCHANGE_HPP
