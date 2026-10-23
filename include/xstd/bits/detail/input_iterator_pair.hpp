//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_INPUT_ITERATOR_PAIR_HPP
#define XSTD_BITS_DETAIL_INPUT_ITERATOR_PAIR_HPP

#include <concepts> // derived_from, same_as
#include <iterator> // input_iterator, input_iterator_tag, iterator_traits, sentinel_for

namespace xstd::bits::detail {

// [container.requirements.general]/69: qualified by the category iterator_traits names, so never an integral type.
template<class I>
concept qualifies_as_input_iterator = requires { typename std::iterator_traits<I>::iterator_category; } and std::derived_from<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>;

// A C++17 input iterator need not be default-constructible or readable through const, as C++20's iterators must be.
template<class I, class S>
concept input_iterator_pair = (std::input_iterator<I> and std::sentinel_for<S, I>) or (std::same_as<S, I> and qualifies_as_input_iterator<I>);

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_INPUT_ITERATOR_PAIR_HPP
