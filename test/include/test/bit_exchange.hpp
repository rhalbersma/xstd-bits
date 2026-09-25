//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BIT_EXCHANGE_HPP
#define TEST_BIT_EXCHANGE_HPP

#include <xstd/bits/bit_cast.hpp>         // bit_cast
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage
#include <concepts>                       // same_as

namespace test {

// The exchanges asked as templates: a non-dependent requirement hard-errors instead of failing.
template<class Reading, class B>
concept exchanges_from_bits = requires (B const& b) {
        { Reading(xstd::from_bit_storage, b) } -> std::same_as<Reading>;
};

template<class Reading, class B>
concept exchanges_to_bits = requires (Reading const& r) {
        { r.template to_bits<B>() } -> std::same_as<B>;
};

// Both directions for words that are bit storage, which is what an owner of a static width answers.
template<class Reading, class B>
concept exchanges_bits = exchanges_from_bits<Reading, B> and exchanges_to_bits<Reading, B>;

// A cast into Reading from anything that has bit storage of its width.
template<class Reading, class B>
concept casts_from = requires (B const& b) {
        { xstd::bit_cast<Reading>(b) } -> std::same_as<Reading>;
};

// Both directions of the cast, which an owner answers and a view does not.
template<class Reading, class B>
concept casts_between = casts_from<Reading, B> and requires (Reading const& r) {
        { xstd::bit_cast<B>(r) } -> std::same_as<B>;
};

} // namespace test

#endif // TEST_BIT_EXCHANGE_HPP
