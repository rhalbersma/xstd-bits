//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BIT_EXCHANGE_HPP
#define TEST_BIT_EXCHANGE_HPP

#include <concepts> // same_as

namespace test {

// The byte exchange asked of the DOOR, which is what the named spelling leaves to ask about. is_constructible_v
// answered the old question because the door was a constructor; there is no trait for "this type has a static
// from_bits that accepts B", so it is a concept instead -- and a concept is the better question anyway, since it
// names the operation rather than a proxy for it.
//
// Being a TEMPLATE is not incidental. A bare requires-expression over concrete types puts a non-dependent
// requirement in the immediate context, where GCC reports an unsatisfied constraint as a hard error rather than
// as false -- which is precisely what an assertion of the form "this is NOT admitted" must not be.
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
